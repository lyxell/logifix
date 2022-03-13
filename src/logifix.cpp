#include "logifix.h"
#include "javadoc.h"
#include "timer.h"
#include "utils.h"
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <mutex>
#include <numeric>
#include <nway.h>
#include <regex>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

namespace logifix {

namespace {
/**
 * Check if two 2d segments overlap.
 * Segments are left-inclusive and right-exclusive
 */
template <typename T> auto segments_overlap(std::pair<T, T> a, std::pair<T, T> b) -> bool {
    if (a > b) {
        std::swap(a, b);
    }
    return a.second > b.first;
}
} // namespace

auto program::create_id() -> size_t {
    return id_counter++;
}

auto program::add_file(const std::string& file) -> size_t {
    node_data_type node;
    node.id = create_id();
    node.source_code = file;
    node.creation_rule = "file";
    node.parent = node.id;
    node_data[node.id] = node;
    pending_root_nodes.emplace_front(node.id);
    return node.id;
}

auto program::disable_rule(const rule_id& rule) -> void { disabled_rules.emplace(rule); }

/**
 * Take a string and a rewrite, apply the rewrite and return the
 * result.
 */
auto program::apply_rewrite(const std::string& original, const rewrite_type& rewrite) const
    -> std::string {
    const auto& [start, end, replacement] = rewrite;
    return original.substr(0, start) + replacement + original.substr(end);
}

/**
 * Take a string and a collection of rewrites, apply all rewrites and return the
 * result.
 */
auto program::apply_rewrites(const std::string& original, rewrite_collection rewrites) const
    -> std::string {
    /* Sort rewrites in reverse so we can apply them one by one */
    std::sort(rewrites.begin(), rewrites.end(), std::greater<>());
    return std::accumulate(rewrites.begin(), rewrites.end(), original,
                           [this](const std::string& acc, const rewrite_type& rewrite) {
                               return apply_rewrite(acc, rewrite);
                           });
}

auto program::adjust_rewrites(const rewrite_collection& before,
                              const rewrite_collection& after) const -> rewrite_collection {
    auto result = rewrite_collection{};
    for (const auto& [xs, xe, xr] : after) {
        auto diff = 0;
        for (const auto& [ys, ye, yr] : before) {
            /* If rewrite y starts before rewrite x we need to adjust x */
            if (ys <= xs) {
                diff += int(yr.size()) - int(ye - ys);
            }
        }
        result.emplace_back(xs + diff, xe + diff, xr);
    }
    return result;
}

auto program::rewrites_invert(const std::string& original, rewrite_collection rewrites) const
    -> rewrite_collection {
    auto result = rewrite_collection{};
    auto diff = 0;
    std::sort(rewrites.begin(), rewrites.end());
    for (auto [start, end, replacement] : rewrites) {
        result.emplace_back(start + diff, start + replacement.size() + diff,
                            original.substr(start, end - start));
        diff += int(replacement.size()) - int(end - start);
    }
    return result;
}

auto program::rewrite_collections_overlap(const rewrite_collection& left,
                                          const rewrite_collection& right) const -> bool {
    for (const auto& [xs, xe, xr] : left) {
        for (const auto& [ys, ye, yr] : right) {
            if (segments_overlap<size_t>({xs, xe}, {ys, ye})) {
                return true;
            }
        }
    }
    return false;
}

auto program::rewrite_collection_overlap(const rewrite_collection& coll) const -> bool {
    for (auto i = std::size_t{}; i < coll.size(); i++) {
        for (auto j = std::size_t{}; j < coll.size(); j++) {
            if (i == j) {
                continue;
            }
            if (segments_overlap<size_t>({std::get<0>(coll[i]), std::get<1>(coll[i])},
                                         {std::get<0>(coll[j]), std::get<1>(coll[j])})) {
                return true;
            }
        }
    }
    return false;
}

/**
 * Use LCS algorithm to split a rewrite into multiple smaller rewrites if possible.
 */
auto program::split_rewrite(const std::string& original, const rewrite_type& rewrite) const
    -> rewrite_collection {
    auto result = rewrite_collection{};
    const auto& [start, end, replacement] = rewrite;
    auto a = original.substr(start, end - start);
    auto b = replacement;
    auto a_tokens = *parser::lex(a);
    auto b_tokens = *parser::lex(b);
    auto lcs = nway::lcs(a_tokens, b_tokens);
    auto a_pos = std::size_t{};
    auto b_pos = std::size_t{};

    auto tokens_len_sum = [](parser::token_collection tokens, size_t num) -> size_t {
        auto slice = parser::token_collection(tokens.begin(), tokens.begin() + num);
        return parser::token_collection_to_string(slice).size();
    };

    while (a_pos < a_tokens.size() || b_pos < b_tokens.size()) {
        /* a and b agree */
        while (a_pos < a_tokens.size() && lcs[a_pos] && *lcs[a_pos] == b_pos) {
            a_pos++;
            b_pos++;
        }
        auto a_start = a_pos;
        auto b_start = b_pos;
        /* a has no matching position */
        while (a_pos < a_tokens.size() && !lcs[a_pos]) {
            a_pos++;
        }
        /* a has matching position but it is not that of b_pos */
        while ((a_pos == a_tokens.size() && b_pos < b_tokens.size()) ||
               (a_pos < a_tokens.size() && lcs[a_pos] && *lcs[a_pos] != b_pos)) {
            b_pos++;
        }
        if (a_start != a_pos || b_start != b_pos) {
            result.emplace_back(
                start + tokens_len_sum(a_tokens, a_start), start + tokens_len_sum(a_tokens, a_pos),
                b.substr(tokens_len_sum(b_tokens, b_start),
                         tokens_len_sum(b_tokens, b_pos) - tokens_len_sum(b_tokens, b_start)));
        }
    }
    return result;
}

auto program::get_recursive_merge_result_for_node(node_id id) const -> std::string {
    auto node = node_data.at(id);
    for (auto child_id : node.children) {
        if (node_data.at(child_id).creation_rule == "merge") {
            return get_recursive_merge_result_for_node(child_id);
        }
    }
    return node.source_code;
}

auto program::post_process(const std::string& original, const std::string& changed) const
    -> std::string {
    auto rewrites = rewrites_invert(
        original, split_rewrite(original, std::tuple(0ul, original.size(), changed)));
    auto result = rewrite_collection{};
    for (const auto& [rule, rewrite] : run_datalog_analysis(changed)) {
        if (rule != "remove_redundant_parentheses") {
            continue;
        }
        auto [a_start, a_end, a_replacement] = rewrite;
        for (auto [b_start, b_end, b_replacement] : rewrites) {
            if ((a_start >= int(b_start) - 1 && a_start <= b_end + 1) ||
                (a_end >= int(b_start) - 1 && a_end <= b_end + 1)) {
                result.emplace_back(rewrite);
                break;
            }
        }
    }
    return apply_rewrites(changed, result);
}

auto program::get_patches_for_file(node_id id) const -> std::vector<patch_id> {
    std::vector<patch_id> result;
    for (auto child_id : node_data.at(id).children) {
        auto child = node_data.at(child_id);
        if (child.creation_rule == "remove_redundant_parentheses") {
            continue;
        }
        if (disabled_rules.find(child.creation_rule) != disabled_rules.end()) {
            continue;
        }
        result.emplace_back(child.id);
    }
    return result;
}

auto program::get_patches_for_rule(const rule_id& rule) const -> std::vector<patch_id> {
    auto result = std::vector<patch_id>{};
    for (auto node_id = std::size_t{}; node_id < id_counter; node_id++) {
        auto node = node_data.at(node_id);
        if (node.parent == node_id) {
            for (auto child_id : node_data.at(node.id).children) {
                if (rule == node_data.at(child_id).creation_rule) {
                    result.emplace_back(child_id);
                }
            }
        }
    }
    return result;
}

auto program::get_patch_data(patch_id patch) const -> std::tuple<rule_id, node_id, std::string> {
    auto node = node_data.at(patch);
    return {node.creation_rule, node.parent, get_recursive_merge_result_for_node(node.id)};
}

auto program::print_merge_conflict(const std::string& source, rewrite_collection rewrites,
                                   const std::vector<node_id>& node_ids) const -> void {
    fmt::print(stderr, fg(fmt::terminal_color::red), "\nFatal error: ");
    fmt::print("Unexpected merge conflict\n");
    std::sort(rewrites.begin(), rewrites.end());
    auto fragment_start = std::get<0>(rewrites.front());
    auto fragment_end = std::get<1>(rewrites.back());
    fmt::print(stderr, "Related code fragment: {}\n",
               source.substr(fragment_start, fragment_end - fragment_start));
    for (const auto& node_id : node_ids) {
        auto node = node_data.at(node_id);
        auto parent = node_data.at(node.parent);
        fmt::print(stderr, "Rule: ");
        fmt::print(stderr, fg(fmt::terminal_color::cyan), "{}\n", node.creation_rule);
        for (auto [start, end, replacement] : node.creation_rewrites) {
            fmt::print(stderr, "    Original: {} Replacement: {} Position: {}-{}\n",
                       parent.source_code.substr(start, end - start), replacement, start, end);
        }
    }
}

auto program::get_result(node_id parent_id, const std::vector<patch_id>& patches) const
    -> std::string {
    auto parent = node_data.at(parent_id);
    auto all_rewrites = rewrite_collection{};
    for (auto patch : patches) {
        auto data = get_patch_data(patch);
        auto result = get_recursive_merge_result_for_node(patch);
        auto rewrites = split_rewrite(parent.source_code, std::tuple(0ul, parent.source_code.size(), result));
        all_rewrites.insert(all_rewrites.end(), rewrites.begin(), rewrites.end());
    }
    std::sort(all_rewrites.begin(), all_rewrites.end());
    all_rewrites.erase(std::unique(all_rewrites.begin(), all_rewrites.end()), all_rewrites.end());
    if (rewrite_collection_overlap(all_rewrites)) {
        print_merge_conflict(parent.source_code, all_rewrites, patches);
        std::exit(1);
    }
    return post_process(parent.source_code, apply_rewrites(parent.source_code, all_rewrites));
}

auto program::get_all_patches() const -> std::vector<patch_id> {
    auto result = std::vector<patch_id>{};
    for (auto node_id = std::size_t{}; node_id < id_counter; node_id++) {
        auto node = node_data.at(node_id);
        if (node.parent == node.id) {
            for (auto child_id : node_data.at(node.id).children) {
                auto child = node_data.at(child_id);
                if (disabled_rules.find(child.creation_rule) != disabled_rules.end()) {
                    continue;
                }
                if (child.creation_rule == "remove_redundant_parentheses") {
                    continue;
                }
                result.emplace_back(child_id);
            }
        }
    }
    return result;
}

auto program::print_performance_metrics() -> void {
    constexpr auto MAX_FILES_SHOWN = std::size_t{10};
    constexpr auto MICROSECONDS_PER_SECOND = 1000.0 * 1000.0;
    auto time_per_event_type = std::map<std::string, size_t>{};
    for (auto [time, data] : timer::events) {
        auto type = data;
        time_per_event_type[type] += time;
    }
    fmt::print(stderr, "\n");
    for (const auto& [e, tot] : time_per_event_type) {
        fmt::print(stderr, "{:20} {:20}\n", e, tot / MICROSECONDS_PER_SECOND);
    }
}

auto program::print_graphviz_data() const -> void {
    std::cout << "digraph {" << std::endl;
    size_t i = 0;

    for (auto node_id = std::size_t{}; node_id < id_counter; node_id++) {
        auto node = node_data.at(node_id);
        if (node.creation_rule == "remove_redundant_parentheses") continue;
        if (node.creation_rule == "merge") {
            std::cout << "    " << node.parent << " -> " << node.id << " [style = dashed label=\"" << node.creation_rule << "\"];" << std::endl;
        } else {
            std::cout << "    " << node.parent << " -> " << node.id << " [label = \"" << node.creation_rule << "\"];" << std::endl;
        }
    }
    std::cout << "}" << std::endl;
}

auto program::run(std::function<void(node_id)> report_progress) -> void {
    auto work_mutex = std::mutex{};
    auto cv = std::condition_variable{};
    auto waiting_threads = std::size_t{};
    auto thread_pool = std::vector<std::thread>{};
    auto const concurrency = std::thread::hardware_concurrency();
    auto done = false;
    for (auto i = std::size_t{}; i < concurrency; i++) {
        thread_pool.emplace_back(std::thread([&] {
            while (true) {
                node_data_type current_node;
                node_data_type parent_node;
                bool current_node_has_parent = false;
                /* acquire work */
                {
                    auto lock = std::unique_lock{work_mutex};
                    if (pending_child_nodes.empty() && pending_root_nodes.empty()) {
                        waiting_threads++;
                        if (waiting_threads == concurrency) {
                            done = true;
                            cv.notify_all();
                        } else {
                            auto wakeup_when = [&]() { return !pending_child_nodes.empty() || done; };
                            cv.wait(lock, wakeup_when);
                            waiting_threads--;
                        }
                    }
                    if (done) {
                        return;
                    }
                    if (pending_child_nodes.empty()) {
                        current_node = node_data[pending_root_nodes.front()];
                        pending_root_nodes.pop_front();
                        report_progress(current_node.id);
                    } else {
                        current_node = node_data[pending_child_nodes.front()];
                        pending_child_nodes.pop_front();
                        current_node_has_parent = true;
                        parent_node = node_data[current_node.parent];
                    }
                }

                auto next_nodes = std::vector<node_data_type>{};

                {
                    auto rewrites = run_datalog_analysis(current_node.source_code);
                    auto lock = std::unique_lock{work_mutex};
                    for (const auto& [rule, rewrite] : rewrites) {
                        node_data_type next_node;
                        next_node.id = create_id();
                        next_node.creation_rule = rule;
                        next_node.source_code = apply_rewrite(current_node.source_code, rewrite);
                        next_node.creation_rewrites = split_rewrite(current_node.source_code, rewrite);
                        next_node.parent = current_node.id;
                        node_data[current_node.id].children_hashset.emplace(next_node.source_code);
                        node_data[next_node.id] = next_node;
                        next_nodes.emplace_back(next_node);
                        node_data[current_node.id].children.emplace_back(next_node.id);
                    }
                }

                if (!current_node_has_parent) {
                    for (const auto& next_node : next_nodes) {
                        if (next_node.creation_rule == "remove_redundant_parentheses") {
                            continue;
                        }
                        if (disabled_rules.find(next_node.creation_rule) != disabled_rules.end()) {
                            continue;
                        }
                        auto lock = std::unique_lock{work_mutex};
                        pending_child_nodes.emplace_back(next_node.id);
                    }
                } else {

                    auto rewrites = rewrite_collection{};
                    std::vector<node_id> taken_nodes;

                    for (const auto& next_node : next_nodes) {
                        if (next_node.creation_rule == "remove_redundant_parentheses") {
                            continue;
                        }
                        auto inverted = rewrites_invert(parent_node.source_code, current_node.creation_rewrites);
                        /* Make sure that the inverted rewrites and the rewrites for the next node do not have any overlap */
                        if (!rewrite_collections_overlap(inverted, next_node.creation_rewrites)) {
                            /**
                             * We "backport" the rewrites for the next node to apply to
                             * the parent node and check if there's any child with
                             * source code which matches these rewrites
                             */
                            auto adjusted = adjust_rewrites(inverted, next_node.creation_rewrites);
                            auto candidate_string = apply_rewrites(parent_node.source_code, adjusted);
                            if (parent_node.children_hashset.find(candidate_string) !=
                                parent_node.children_hashset.end()) {
                                continue;
                            }
                        }
                        taken_nodes.emplace_back(next_node.id);
                        rewrites.insert(rewrites.end(), next_node.creation_rewrites.begin(),
                                        next_node.creation_rewrites.end());
                    }

                    if (!rewrites.empty()) {
                        if (rewrite_collection_overlap(rewrites)) {
                            print_merge_conflict(current_node.source_code, rewrites, taken_nodes);
                            std::exit(1);
                        } else {
                            auto lock = std::unique_lock{work_mutex};
                            node_data_type next_node;
                            next_node.id = create_id();
                            next_node.creation_rule = "merge";
                            next_node.source_code = apply_rewrites(current_node.source_code, rewrites);
                            next_node.creation_rewrites = rewrites;
                            next_node.parent = current_node.id;
                            node_data[next_node.id] = next_node;
                            pending_child_nodes.emplace_back(next_node.id);
                            node_data[current_node.id].children.emplace_back(next_node.id);
                        }
                    }

                }

                /* notify all threads that there is more work available */
                cv.notify_all();
            }
        }));
    }
    for (auto& t : thread_pool) {
        t.join();
    }
}

/**
 * Given a source file, create a new Soufflé program, run the analysis, extract
 * and perform rewrites and finally return the set of resulting strings and the
 * rule ids for each rewrite.
 */
auto program::run_datalog_analysis(const std::string& source) const
    -> std::set<std::pair<rule_id, rewrite_type>> {

    const auto* program_name = "logifix";
    const auto* filename = "file";

    auto prog = std::unique_ptr<souffle::SouffleProgram>(
        souffle::ProgramFactory::newInstance(program_name));

    /* add javadoc info to prog */
    auto tokens = parser::lex(source);
    if (tokens) {
        auto* javadoc_references = prog->getRelation("javadoc_references");
        for (auto token : *tokens) {
            if (std::get<0>(token) != parser::token_type::multi_line_comment) {
                continue;
            }
            for (const auto& class_name :
                 parser::javadoc::get_classes(std::string(std::get<1>(token)))) {
                javadoc_references->insert(souffle::tuple(
                    javadoc_references, {prog->getSymbolTable().encode(filename),
                                         prog->getSymbolTable().encode(class_name)}));
            }
        }
    }

    /* add ast info to prog */
    parser::parse(prog.get(), filename, source.c_str());

    /* add source_code info to prog */
    auto* source_code_relation = prog->getRelation("source_code");
    source_code_relation->insert(
        souffle::tuple(source_code_relation, {prog->getSymbolTable().encode(filename),
                                              prog->getSymbolTable().encode(source)}));

    /* run program */
    prog->run();
    // prog->printAll();

    /* extract rewrites */
    auto* relation = prog->getRelation("rewrite");
    auto rewrites = std::set<std::pair<std::string, rewrite_type>>{};

    for (auto& output : *relation) {

        auto rule = rule_id{};
        auto filename = std::string{};
        auto start = 0;
        auto end = 0;
        auto replacement = std::string{};

        output >> rule >> filename >> start >> end >> replacement;

        rewrites.emplace(rule, std::tuple(start, end, replacement));
    }

    return rewrites;
}

} // namespace logifix
