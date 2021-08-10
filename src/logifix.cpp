#include "logifix.h"
#include "javadoc.h"
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <fmt/core.h>
#include <iostream>
#include <mutex>
#include <utility>
#include <nway.h>
#include <regex>
#include <thread>
#include <unordered_set>
#include <vector>

namespace logifix {

namespace timer {

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;

std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>
    start_times;
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
std::unordered_map<node_id,
                   std::unordered_map<rule_id, std::unordered_set<node_id>>>
    children;
std::unordered_map<node_id,
                   std::unordered_map<rule_id, std::unordered_set<std::string>>>
    children_strs;
std::unordered_map<node_id, std::set<std::pair<rule_id, node_id>>>
    taken_transitions;

std::vector<std::pair<std::string, std::tuple<size_t, size_t, std::string>>> nodes;

size_t create_id(const std::string& str, const std::tuple<size_t, size_t, std::string>& rewrite) {
    auto id = nodes.size();
    nodes.emplace_back(str, rewrite);
    return id;
}

size_t add_file(const std::string& file) {
    auto node_id = create_id("", {0, 0, file});
    pending_files.emplace_front(node_id);
    return node_id;
}

void disable_rule(const rule_id& rule) {
   disabled_rules.emplace(rule);
}

/**
 * Segments are left-inclusive and right-exclusive
 */
template <typename T>
bool segments_overlap(std::pair<T, T> a, std::pair<T, T> b) {
    if (a > b) std::swap(a, b);
    return a.second > b.first;
}

std::string apply_rewrite(const std::string& original, const std::tuple<size_t, size_t, std::string>& rewrite) {
    const auto& [start, end, replacement] = rewrite;
    return original.substr(0, start) + replacement + original.substr(end);
}

std::vector<std::tuple<size_t, size_t, std::string>> split_rewrite(const std::string& original, const std::tuple<size_t, size_t, std::string>& rewrite) {
    std::vector<std::tuple<size_t, size_t, std::string>> result;
    auto [start, end, replacement] = rewrite;
    auto a = original.substr(start, end - start);
    auto b = replacement;
    std::cerr << "LCS" << std::endl;
    std::cerr << a.size() << " " << b.size() << std::endl;
    auto lcs = nway::lcs(a, b);
    std::cerr << "LCS DONE" << std::endl;
    size_t a_pos = 0;
    size_t b_pos = 0;
    while (a_pos < a.size() || b_pos < b.size()) {
        std::cerr << "LOOPING" << std::endl;
        /* a and b agree */
        while (a_pos < a.size() && lcs[a_pos] && *lcs[a_pos] == b_pos) {
            a_pos++;
            b_pos++;
        }
        size_t a_start = a_pos;
        size_t b_start = b_pos;
        /* a has no matching position */
        while (a_pos < a.size() && !lcs[a_pos]) {
            a_pos++;
        }
        /* a has matching position but it is not that of b_pos */
        while ((a_pos == a.size() && b_pos < b.size()) || (a_pos < a.size() && lcs[a_pos] && *lcs[a_pos] != b_pos)) {
            b_pos++;
        }
        if (a_start != a_pos || b_start != b_pos) {
            result.emplace_back(start + a_start, start + a_pos, b.substr(b_start, b_pos - b_start));
        }
    }
    return result;
}

std::optional<std::string> get_recursive_merge_result_for_node(node_id node) {

    /* Base case: no transitions from node */
    if (taken_transitions[node].empty()) {
        auto [pstr, rewrite] = nodes[node];
        return apply_rewrite(pstr, rewrite);
    }

    /* Case 1: only one transition from node */
    if (taken_transitions[node].size() == 1) {
        return get_recursive_merge_result_for_node(
            taken_transitions[node].begin()->second);
    }

    /* Case 2: Multiple transitions from node */

    std::vector<std::string> to_be_merged;
    for (auto [rule, next_node] : taken_transitions[node]) {
        auto result = get_recursive_merge_result_for_node(next_node);
        if (result) {
            to_be_merged.emplace_back(*result);
        }
    }

    auto [pstr, rewrite] = nodes[node];
    auto diff = nway::diff(apply_rewrite(pstr, rewrite), to_be_merged);

    /* Return empty optional if merging failed */
    if (nway::has_conflict(diff)) {
        return {};
    }

    return nway::merge(diff);
}

std::vector<std::pair<rule_id, std::string>>
get_patches_for_file(node_id node) {
    std::vector<std::pair<rule_id, std::string>> rewrites;
    /* Go through the children of the node and collect all rewrites */
    for (auto [rule_id, node_id] : taken_transitions[node]) {
        auto result = get_recursive_merge_result_for_node(node_id);
        if (result) {
            rewrites.emplace_back(rule_id, *result);
        }
    }
    return rewrites;
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
    auto const concurrency = std::thread::hardware_concurrency();
    waiting_threads = 0;
    bool done = false;
    for (size_t i = 0; i < concurrency; i++) {
        thread_pool.emplace_back(std::thread([&] {
            while (true) {
                std::cerr << "NEW NODE" << std::endl;
                node_id current_node;
                std::string current_node_source;
                /* acquire work */
                {
                    std::unique_lock<std::mutex> lock(work_mutex);
                    if (pending_strings.empty() && pending_files.empty()) {
                        std::cerr << "WAITING" << std::endl;
                        waiting_threads++;
                        if (waiting_threads == concurrency) {
                            done = true;
                            cv.notify_all();
                        } else {
                            auto wakeup_when = [&]() {
                                return !pending_strings.empty() || done;
                            };
                            cv.wait(lock, wakeup_when);
                            waiting_threads--;
                        }
                        std::cerr << "DONE WAITING" << std::endl;
                    }
                    if (done) {
                        return;
                    }
                    if (pending_strings.empty()) {
                        current_node = pending_files.front();
                        pending_files.pop_front();
                        report_progress(0);
                    } else {
                        current_node = pending_strings.front();
                        pending_strings.pop_front();
                    }
                    auto [pstr, rewrite] = nodes[current_node];
                    current_node_source = apply_rewrite(pstr, rewrite);
                }

                std::vector<std::tuple<node_id, rule_id>> next_nodes;

                {
                    std::cerr << "GET PATCH" << std::endl;
                    auto rewrites = get_patches(current_node_source);
                    std::cerr << "GOT PATCH" << std::endl;
                    std::unique_lock<std::mutex> lock(work_mutex);
                    for (const auto& [rule, rewrite] : rewrites) {
                        auto next_node = create_id(current_node_source, rewrite);
                        parent[next_node] = {rule, current_node};
                        children[current_node][rule].emplace(next_node);
                        children_strs[current_node][rule].emplace(apply_rewrite(current_node_source, rewrite));
                        next_nodes.emplace_back(next_node, rule);
                    }
                }

                for (const auto& [next_node, rule] : next_nodes) {
                    std::unique_lock<std::mutex> lock(work_mutex);
                    auto creation_timer = timer::create("diffing", 0);
                    bool take_transition = true;
                    bool current_has_parent = parent.find(current_node) != parent.end();
                    if (current_has_parent) {

                        auto [parent_rule, parent_id] = parent[current_node];
                        auto [parent_pstr, parent_rewrite] = nodes[parent_id];
                        auto [curr_pstr, curr_rewrite] = nodes[current_node];
                        auto [next_pstr, next_rewrite] = nodes[next_node];

                        auto next_str = apply_rewrite(next_pstr, next_rewrite);

                        /* If the next_node is _exactly_ matching a child node
                         * of the parent we want to take the transition */
                        if (children_strs[parent_id][rule].find(next_str) != children_strs[parent_id][rule].end()) {
                            goto done;
                        }

                        const auto& [curr_start, curr_end, curr_replacement] = curr_rewrite;
                        const auto& [next_start, next_end, next_replacement] = next_rewrite;

                        /* If segments overlap we try to split them into smaller pieces */
                        if (segments_overlap<size_t>({curr_start, curr_end}, {next_start, next_end})) {
                            std::cerr << "SPLITTING" << std::endl;
                            std::cerr << "START " << curr_start << std::endl;
                            std::cerr << "END " << curr_end << std::endl;
                            std::cerr << "REPLACEMENT " << curr_replacement << std::endl;
                            auto curr_split = split_rewrite(curr_pstr, curr_rewrite);
                            auto next_split = split_rewrite(next_pstr, next_rewrite);
                            std::cerr << "SPLITTING PROC. DONE" << std::endl;
                            bool overlap = false;
                            for (auto x : curr_split) {
                                for (auto y : next_split) {
                                    if (segments_overlap<size_t>({std::get<0>(x), std::get<1>(x)}, 
                                                                 {std::get<0>(y), std::get<1>(y)})) {
                                        overlap = true;
                                        break;
                                    }
                                }
                            }
                            /* If there is still overlap, we need to take the transition */
                            if (!overlap) {
                                std::cerr << "SPLITTING WORKED" << std::endl;
                                std::cerr << "BEFORE" << std::endl;
                                std::cerr << curr_start << " " << curr_end << " " << curr_replacement.size() << " " << curr_replacement << std::endl;
                                std::cerr << "AFTER" << std::endl;
                                for (auto x : curr_split) {
                                    std::cerr << std::get<0>(x) << " " << std::get<1>(x) << " " << std::get<2>(x) << std::endl;
                                }
                                std::cerr << "------" << std::endl;
                                std::cerr << "BEFORE" << std::endl;
                                std::cerr << next_start << " " << next_end << " " << next_replacement << std::endl;
                                std::cerr << "AFTER" << std::endl;
                                for (auto x : next_split) {
                                    std::cerr << std::get<0>(x) << " " << std::get<1>(x) << " " << std::get<2>(x) << std::endl;
                                }
                                /* Adjust rewrites of curr_rewrite */
                                for (auto& x : curr_split) {
                                    int diff = 0;
                                    for (const auto& y : next_split) {
                                        /* If rewrite y starts before rewrite x we need to adjust x */
                                        if (std::get<0>(y) < std::get<0>(x)) {
                                            diff += std::get<2>(y).size() - int(std::get<1>(y) - std::get<0>(y));
                                        }
                                    }
                                    std::get<0>(x) -= diff;
                                    std::get<1>(x) -= diff;
                                }
                                /* sort rewrites in reverse so we can apply them one by one */
                                std::sort(curr_split.begin(), curr_split.end(), [](const auto& a, const auto& b) {
                                    return std::get<0>(a) > std::get<0>(b);
                                });
                                auto candidate_string = curr_pstr;
                                for (const auto& x : next_split) {
                                    std::cerr << "APPLYING REWRITE" << std::endl;
                                    candidate_string = apply_rewrite(curr_pstr, x);
                                }
                                if (children_strs[parent_id][rule].find(candidate_string) != children_strs[parent_id][rule].end()) {
                                    take_transition = false;
                                }
                            }
                        } else {
                            std::cerr << "NO NEED FOR SPLITTING" << std::endl;
                            std::string candidate_string;
                            /**
                             * Is the next_rewrite before the curr_rewrite?
                             * In this case there is no adjustment that needs
                             * to be made.
                             */
                            if (next_end < curr_start) {
                                std::cerr << "NO ADJUSTMENT" << std::endl;
                                candidate_string = apply_rewrite(curr_pstr, next_rewrite);
                                auto [a, b, repl] = next_rewrite;
                                std::cerr << repl << std::endl;
                            } else {
                                std::cerr << "ADJUSTMENT" << std::endl;
                                /**
                                 * In this branch next_rewrite occurs after curr_rewrite.
                                 * We need to adjust its positions.
                                 */
                                auto diff = size_t(curr_replacement.size() - int(curr_end - curr_start));
                                candidate_string = apply_rewrite(curr_pstr, {next_start - diff, next_end - diff, next_replacement});
                            }

                            if (children_strs[parent_id][rule].find(candidate_string) != children_strs[parent_id][rule].end()) {
                                take_transition = false;
                            }
                        }

                    }
done:
                    std::cerr << "DONE" << std::endl;
                    if (!current_has_parent && disabled_rules.find(rule) != disabled_rules.end()) {
                        take_transition = false;
                    }

                    if (take_transition) {
                        pending_strings.emplace_back(next_node);
                        taken_transitions[current_node].emplace(rule,
                                                                next_node);
                    }
                    timer::stop(creation_timer);
                }
                std::cerr << "DONE WITH NODE" << std::endl;

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
    auto* prog = souffle::ProgramFactory::newInstance(program_name);
    // timer::stop(creation_timer);

    /* add javadoc info to prog */
    auto tokens = sjp::lex(source);
    if (tokens) {
        souffle::Relation* javadoc_references =
            prog->getRelation("javadoc_references");
        for (auto token : *tokens) {
            if (std::get<0>(token) != sjp::token_type::multi_line_comment) {
                continue;
            }
            for (const auto& class_name :
                 javadoc::get_classes(std::string(std::get<1>(token)))) {
                javadoc_references->insert(souffle::tuple(
                    javadoc_references,
                    {prog->getSymbolTable().encode(filename),
                     prog->getSymbolTable().encode(class_name)}));
            }
        }
    }

    /* add ast info to prog */
    sjp::parse(prog, filename, source.c_str());

    auto creation_timer = timer::create("get_patches", 0);
    /* add source_code info to prog */
    souffle::Relation* source_code_relation = prog->getRelation("source_code");
    source_code_relation->insert(souffle::tuple(
        source_code_relation, {prog->getSymbolTable().encode(filename),
                               prog->getSymbolTable().encode(source)}));

    /* run program */
    prog->run();

    /* extract rewrites */
    souffle::Relation* relation = prog->getRelation("rewrite");
    std::set<std::pair<std::string, std::tuple<size_t, size_t, std::string>>> rewrites;

    for (souffle::tuple& output : *relation) {

        rule_id rule;
        std::string filename;
        int start;
        int end;
        std::string replacement;

        output >> rule >> filename >> start >> end >> replacement;

        rewrites.emplace(rule, std::tuple(start, end, replacement));
    }

    delete prog;
    timer::stop(creation_timer);

    return rewrites;
}

} // namespace logifix
