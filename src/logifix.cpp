#include "logifix.h"
#include "javadoc.h"
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fmt/core.h>
#include <iostream>
#include <mutex>
#include <nway.h>
#include <regex>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

namespace logifix {

namespace timer {

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;

std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> start_times;
std::vector<std::pair<std::string, size_t>> event_data;

std::vector<std::pair<size_t, std::pair<std::string, size_t>>> events;

std::mutex timer_mutex;

size_t create(std::string name, size_t node_id) {
    std::unique_lock<std::mutex> lock(timer_mutex);
    auto id = start_times.size();
    start_times.emplace_back(high_resolution_clock::now());
    event_data.emplace_back(std::move(name), node_id);
    return id;
}

void stop(size_t id) {
    std::unique_lock<std::mutex> lock(timer_mutex);
    auto end = high_resolution_clock::now();
    auto diff = duration_cast<microseconds>(end - start_times[id]).count();
    events.emplace_back(diff, event_data[id]);
}

} // namespace timer

std::vector<std::thread> thread_pool;

std::condition_variable cv;
size_t waiting_threads;
std::unordered_set<rule_id> disabled_rules;
std::deque<node_id> pending_files;
std::deque<node_id> pending_strings;
std::mutex work_mutex;
size_t id_counter = 0;
std::unordered_map<node_id, std::pair<rule_id, node_id>> parent;
std::unordered_map<node_id, std::unordered_map<rule_id, std::unordered_set<std::string>>>
    children_strs;
std::unordered_map<node_id, std::set<std::pair<rule_id, node_id>>> taken_transitions;

std::vector<std::pair<std::string, rewrite_collection>> nodes;

size_t create_id(const std::string& str, const rewrite_collection& rewrites) {
    auto id = nodes.size();
    nodes.emplace_back(str, rewrites);
    return id;
}

size_t add_file(const std::string& file) {
    auto node_id = create_id("", {std::tuple(0ul, 0ul, file)});
    pending_files.emplace_front(node_id);
    return node_id;
}

void disable_rule(const rule_id& rule) { disabled_rules.emplace(rule); }

/**
 * Segments are left-inclusive and right-exclusive
 */
template <typename T> bool segments_overlap(std::pair<T, T> a, std::pair<T, T> b) {
    if (a > b) {
        std::swap(a, b);
    }
    return a.second > b.first;
}

std::string apply_rewrite(const std::string& original,
                          const std::tuple<size_t, size_t, std::string>& rewrite) {
    const auto& [start, end, replacement] = rewrite;
    return original.substr(0, start) + replacement + original.substr(end);
}

std::string apply_rewrites(std::string original, rewrite_collection rewrites) {
    /* sort rewrites in reverse so we can apply them one by one */
    std::sort(rewrites.begin(), rewrites.end(),
              [](const auto& a, const auto& b) { return std::get<0>(a) > std::get<0>(b); });
    for (const auto& rewrite : rewrites) {
        original = apply_rewrite(original, rewrite);
    }
    return original;
}

rewrite_collection adjust_rewrites(const rewrite_collection& before,
                                   const rewrite_collection& after) {
    rewrite_collection result;
    for (const auto& x : after) {
        int diff = 0;
        for (const auto& y : before) {
            /* If rewrite y starts before rewrite x we need to adjust x */
            if (std::get<0>(y) <= std::get<0>(x)) {
                diff += int(std::get<2>(y).size()) - int(std::get<1>(y) - std::get<0>(y));
            }
        }
        result.emplace_back(std::get<0>(x) + diff, std::get<1>(x) + diff, std::get<2>(x));
    }
    return result;
}

rewrite_collection rewrites_invert(const std::string& original, rewrite_collection rewrites) {
    rewrite_collection result;
    int diff = 0;
    std::sort(rewrites.begin(), rewrites.end());
    for (auto [start, end, replacement] : rewrites) {
        result.emplace_back(start + diff, start + replacement.size() + diff,
                            original.substr(start, end - start));
        diff += int(replacement.size()) - int(end - start);
    }
    return result;
}

bool rewrite_collections_overlap(const rewrite_collection& left, const rewrite_collection& right) {
    for (const auto& [xs, xe, xr] : left) {
        for (const auto& [ys, ye, yr] : right) {
            if (segments_overlap<size_t>({xs, xe}, {ys, ye})) {
                return true;
            }
        }
    }
    return false;
}

bool rewrite_collection_overlap(const rewrite_collection& coll) {
    for (size_t i = 0; i < coll.size(); i++) {
        for (size_t j = 0; j < coll.size(); j++) {
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

std::vector<std::tuple<size_t, size_t, std::string>>
split_rewrite(const std::string& original, const std::tuple<size_t, size_t, std::string>& rewrite) {
    std::vector<std::tuple<size_t, size_t, std::string>> result;
    const auto& [start, end, replacement] = rewrite;
    auto a = original.substr(start, end - start);
    auto b = replacement;
    auto a_tokens = *sjp::lex(a);
    auto b_tokens = *sjp::lex(b);
    auto lcs = nway::lcs(a_tokens, b_tokens);
    size_t a_pos = 0;
    size_t b_pos = 0;

    auto tokens_len_sum = [](sjp::token_collection tokens, size_t num) -> size_t {
        auto slice = sjp::token_collection(tokens.begin(), tokens.begin() + num);
        return sjp::token_collection_to_string(slice).size();
    };

    while (a_pos < a_tokens.size() || b_pos < b_tokens.size()) {
        /* a and b agree */
        while (a_pos < a_tokens.size() && lcs[a_pos] && *lcs[a_pos] == b_pos) {
            a_pos++;
            b_pos++;
        }
        size_t a_start = a_pos;
        size_t b_start = b_pos;
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

std::string get_recursive_merge_result_for_node(node_id node) {

    if (taken_transitions[node].empty()) {
        auto [pstr, rewrites] = nodes[node];
        return apply_rewrites(pstr, rewrites);
    }

    if (taken_transitions[node].size() == 1) {
        return get_recursive_merge_result_for_node(taken_transitions[node].begin()->second);
    }

    // Will never happen with new algorithm
    std::exit(1);
}

std::string post_process(const std::string& original, const std::string& changed) {
    auto rewrites = rewrites_invert(
        original, split_rewrite(original, std::tuple(0ul, original.size(), changed)));
    rewrite_collection result;
    for (const auto& [rule, rewrite] : get_patches(changed)) {
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

std::vector<patch_id> get_patches_for_file(node_id node) {
    auto [pstr, rws] = nodes[node];
    auto original = apply_rewrites(pstr, rws);
    std::vector<patch_id> result;
    for (auto [rule_id, child_id] : taken_transitions[node]) {
        result.emplace_back(child_id);
    }
    return result;
}

std::vector<patch_id> get_patches_for_rule(rule_id rule) {
    std::vector<patch_id> result;
    for (size_t node = 0; node < nodes.size(); node++) {
        if (parent.find(node) == parent.end()) {
            for (auto [transition_rule, node_id] : taken_transitions[node]) {
                if (rule == transition_rule) {
                    result.emplace_back(node_id);
                }
            }
        }
    }
    return result;
}

std::tuple<rule_id, node_id, std::string> get_patch_data(patch_id patch) {
    auto [r, p] = parent[patch];
    return {r, p, get_recursive_merge_result_for_node(patch)};
}

std::string get_result(node_id parent, const std::vector<patch_id>& patches) {
    auto [parent_pstr, parent_rewrites] = nodes[parent];
    auto parent_source = apply_rewrites(parent_pstr, parent_rewrites);
    rewrite_collection all_rewrites;
    for (auto patch : patches) {
        auto data = get_patch_data(patch);
        auto result = get_recursive_merge_result_for_node(patch);
        auto rewrites = split_rewrite(parent_source, std::tuple(0ul, parent_source.size(), result));
        all_rewrites.insert(all_rewrites.end(), rewrites.begin(), rewrites.end());
    }
    std::sort(all_rewrites.begin(), all_rewrites.end());
    all_rewrites.erase(std::unique(all_rewrites.begin(), all_rewrites.end()), all_rewrites.end());
    if (rewrite_collection_overlap(all_rewrites)) {
        std::cerr << "MERGING FAILED" << std::endl;
        std::sort(all_rewrites.begin(), all_rewrites.end());
        for (auto [s, e, repl] : all_rewrites) {
            std::cerr << s << " " << e << " " << repl << std::endl;
        }
        std::exit(1);
    }
    return post_process(parent_source, apply_rewrites(parent_source, all_rewrites));
}

std::vector<patch_id> get_all_patches() {
    std::vector<patch_id> result;
    for (size_t node = 0; node < nodes.size(); node++) {
        if (parent.find(node) == parent.end()) {
            for (auto [rule_id, node_id] : taken_transitions[node]) {
                result.emplace_back(node_id);
            }
        }
    }
    return result;
}

void print_performance_metrics() {
    constexpr auto MAX_FILES_SHOWN = size_t{10};
    constexpr auto MICROSECONDS_PER_SECOND = 1000.0 * 1000.0;
    std::map<std::string, size_t> time_per_event_type;
    std::map<node_id, size_t> time_per_node_id;
    for (auto [time, data] : timer::events) {
        auto [type, node_id] = data;
        time_per_event_type[type] += time;
        time_per_node_id[node_id] += time;
    }
    fmt::print(stderr, "\n");
    for (const auto& [e, tot] : time_per_event_type) {
        fmt::print(stderr, "{:20} {:20}\n", e, tot / MICROSECONDS_PER_SECOND);
    }
    std::vector<std::pair<size_t, node_id>> node_data;
    node_data.reserve(time_per_node_id.size());
    for (auto [e, tot] : time_per_node_id) {
        node_data.emplace_back(tot, e);
    }
    std::sort(node_data.begin(), node_data.end());
    for (int i = 0; i < std::min(MAX_FILES_SHOWN, node_data.size()); i++) {
        auto [tot, e] = node_data[i];
        fmt::print(stderr, "{:20} {:20}\n", e, tot / MICROSECONDS_PER_SECOND);
    }
}

void run(std::function<void(node_id)> report_progress) {
    auto const concurrency = 1;
    waiting_threads = 0;
    bool done = false;
    for (size_t i = 0; i < concurrency; i++) {
        thread_pool.emplace_back(std::thread([&] {
            while (true) {
                node_id current_node{};
                std::string current_node_source;
                /* acquire work */
                {
                    std::unique_lock<std::mutex> lock(work_mutex);
                    if (pending_strings.empty() && pending_files.empty()) {
                        waiting_threads++;
                        if (waiting_threads == concurrency) {
                            done = true;
                            cv.notify_all();
                        } else {
                            auto wakeup_when = [&]() { return !pending_strings.empty() || done; };
                            cv.wait(lock, wakeup_when);
                            waiting_threads--;
                        }
                    }
                    if (done) {
                        return;
                    }
                    if (pending_strings.empty()) {
                        current_node = pending_files.front();
                        pending_files.pop_front();
                        report_progress(current_node);
                    } else {
                        current_node = pending_strings.front();
                        pending_strings.pop_front();
                    }
                    auto [pstr, rewrites] = nodes[current_node];
                    current_node_source = apply_rewrites(pstr, rewrites);
                }

                std::vector<std::tuple<node_id, rule_id, bool>> next_nodes;

                {
                    auto rewrites = get_patches(current_node_source);
                    std::unique_lock<std::mutex> lock(work_mutex);
                    for (const auto& [rule, rewrite] : rewrites) {
                        auto next_node = create_id(current_node_source,
                                                   split_rewrite(current_node_source, rewrite));
                        parent[next_node] = {rule, current_node};
                        children_strs[current_node][rule].emplace(
                            apply_rewrite(current_node_source, rewrite));
                        next_nodes.emplace_back(next_node, rule, true);
                    }
                }

                for (auto& [next_node, rule, take_transition] : next_nodes) {
                    if (rule == "remove_redundant_parentheses") {
                        take_transition = false;
                        continue;
                    }
                    std::unique_lock<std::mutex> lock(work_mutex);
                    bool current_has_parent = parent.find(current_node) != parent.end();
                    if (current_has_parent) {
                        auto [parent_rule, parent_id] = parent[current_node];
                        auto [curr_pstr, curr_rewrites] = nodes[current_node];
                        auto [next_pstr, next_rewrites] = nodes[next_node];
                        auto next_str = apply_rewrites(next_pstr, next_rewrites);
                        auto inverted = rewrites_invert(curr_pstr, curr_rewrites);
                        auto adjusted = adjust_rewrites(inverted, next_rewrites);
                        if (!rewrite_collections_overlap(inverted, next_rewrites)) {
                            auto candidate_string = apply_rewrites(curr_pstr, adjusted);
                            if (children_strs[parent_id][rule].find(candidate_string) !=
                                children_strs[parent_id][rule].end()) {
                                take_transition = false;
                            }
                        }
                    } else if (disabled_rules.find(rule) != disabled_rules.end()) {
                        take_transition = false;
                    } else {
                        take_transition = true;
                    }
                }

                std::unique_lock<std::mutex> lock(work_mutex);

                bool current_has_parent = parent.find(current_node) != parent.end();
                if (current_has_parent) {
                    rewrite_collection rewrites;
                    for (const auto& [next_node, rule, take_transition] : next_nodes) {
                        if (take_transition) {
                            auto [next_pstr, next_rewrites] = nodes[next_node];
                            rewrites.insert(rewrites.end(), next_rewrites.begin(),
                                            next_rewrites.end());
                        }
                    }
                    if (!rewrites.empty()) {
                        if (rewrite_collection_overlap(rewrites)) {
                            std::cerr << "UNEXPECTED OVERLAP" << std::endl;
                            std::sort(rewrites.begin(), rewrites.end());
                            for (auto [s, e, repl] : rewrites) {
                                std::cerr << s << " " << e << " " << repl << " " << repl.size()
                                          << std::endl;
                            }
                            std::exit(1);
                        } else {
                            std::string rule = "merged";
                            auto next_node = create_id(current_node_source, rewrites);
                            parent[next_node] = {rule, current_node};
                            pending_strings.emplace_back(next_node);
                            taken_transitions[current_node].emplace(rule, next_node);
                        }
                    }
                } else {
                    for (const auto& [next_node, rule, take_transition] : next_nodes) {
                        if (take_transition) {
                            pending_strings.emplace_back(next_node);
                            taken_transitions[current_node].emplace(rule, next_node);
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
std::set<std::pair<rule_id, std::tuple<size_t, size_t, std::string>>>
get_patches(const std::string& source) {

    const char* program_name = "logifix";
    const char* filename = "file";

    // auto creation_timer = timer::create("program create", node_id);
    auto prog = std::unique_ptr<souffle::SouffleProgram>(
        souffle::ProgramFactory::newInstance(program_name));
    // timer::stop(creation_timer);

    /* add javadoc info to prog */
    auto tokens = sjp::lex(source);
    if (tokens) {
        souffle::Relation* javadoc_references = prog->getRelation("javadoc_references");
        for (auto token : *tokens) {
            if (std::get<0>(token) != sjp::token_type::multi_line_comment) {
                continue;
            }
            for (const auto& class_name : javadoc::get_classes(std::string(std::get<1>(token)))) {
                javadoc_references->insert(souffle::tuple(
                    javadoc_references, {prog->getSymbolTable().encode(filename),
                                         prog->getSymbolTable().encode(class_name)}));
            }
        }
    }

    /* add ast info to prog */
    sjp::parse(prog.get(), filename, source.c_str());

    auto creation_timer = timer::create("get_patches", 0);

    /* add source_code info to prog */
    souffle::Relation* source_code_relation = prog->getRelation("source_code");
    source_code_relation->insert(
        souffle::tuple(source_code_relation, {prog->getSymbolTable().encode(filename),
                                              prog->getSymbolTable().encode(source)}));

    /* run program */
    prog->run();

    /* extract rewrites */
    souffle::Relation* relation = prog->getRelation("rewrite");
    std::set<std::pair<std::string, std::tuple<size_t, size_t, std::string>>> rewrites;

    for (souffle::tuple& output : *relation) {

        rule_id rule;
        std::string filename;
        int start{};
        int end{};
        std::string replacement;

        output >> rule >> filename >> start >> end >> replacement;

        rewrites.emplace(rule, std::tuple(start, end, replacement));
    }

    timer::stop(creation_timer);

    return rewrites;
}

} // namespace logifix
