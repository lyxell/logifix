#include "logifix.h"
#include "javadoc.h"
#include <nway.h>
#include <queue>
#include <filesystem>
#include <vector>
#include <unordered_set>
#include <thread>
#include <condition_variable>
#include <regex>
#include <mutex>
#include <iostream>

namespace logifix {

    std::unordered_map<std::string, node_id> file_to_node;
    std::unordered_map<node_id, std::string> node_to_file;

    std::vector<std::thread> thread_pool;

    std::condition_variable cv;
    size_t waiting_threads;
    std::queue<node_id> pending_nodes;
    std::mutex work_mutex;
    std::unordered_set<node_id> visited_nodes;
    std::unordered_map<node_id, std::set<std::pair<rule_id, node_id>>> parents;
    std::unordered_map<node_id, std::set<std::pair<rule_id, node_id>>> children;
    std::unordered_map<node_id, bool> has_continuation;
    std::unordered_map<node_id, bool> is_real_node;

    size_t string_to_node_id(std::string str) {
        if (file_to_node.find(str) != file_to_node.end()) {
            return file_to_node[str];
        }
        auto node_id = file_to_node.size();
        file_to_node.emplace(str, node_id);
        node_to_file.emplace(node_id, str);
        return node_id;
    }

    size_t add_file(std::string file) {
        auto node_id = string_to_node_id(file);
        pending_nodes.emplace(node_id);
        visited_nodes.emplace(node_id);
        return node_id;
    }

    std::vector<std::pair<rule_id, std::string>> get_rewrites_for_file(std::string file) {
        std::vector<std::pair<rule_id, std::string>> rewrites;
        auto node = string_to_node_id(file);
        /* Go through the children of the node and collect all rewrites */
        for (auto [rule, child] : children[node]) {
            std::vector<std::string> to_be_merged;
            std::queue<node_id> q;
            q.emplace(child);
            while (!q.empty()) {
                auto i = q.front();
                q.pop();
                if (!has_continuation[i] && is_real_node[i]) {
                    to_be_merged.emplace_back(node_to_file[i]);
                }
                for (auto [j_rule, j] : children[i]) {
                    q.emplace(j);
                }
            }
            auto diff = nway::diff(node_to_file[child], to_be_merged);
            if (nway::has_conflict(diff)) {
                std::cout << "Found a diff with conflicts" << std::endl;
                exit(1);
            }
            rewrites.emplace_back(rule, nway::merge(diff));
        }
        return rewrites;
    }

    void run() {
        auto const concurrency = std::thread::hardware_concurrency();
        waiting_threads = 0;
        bool finished = false;
        for (size_t i = 0; i < concurrency; i++) {
            thread_pool.emplace_back(std::thread([&] {
                while (true) {
                    node_id current_node;
                    /* acquire work */
                    {
                        std::unique_lock<std::mutex> lock(work_mutex);
                        if (pending_nodes.empty()) {
                            waiting_threads++;
                            if (waiting_threads == concurrency) {
                                finished = true;
                                cv.notify_all();
                            } else {
                                auto wakeup_when = [&](){ return !pending_nodes.empty() || finished; };
                                cv.wait(lock, wakeup_when);
                                waiting_threads--;
                            }
                        }
                        /* if we get to this point there is either work available or we are finished */
                        if (finished) return;
                        current_node = pending_nodes.front();
                        pending_nodes.pop();
                    }

                    std::vector<std::pair<node_id, rule_id>> next_nodes;

                    /* perform expensive computation and store result */
                    {
                        auto rewrites = get_rewrites(node_to_file[current_node]);
                        for (auto [rule, next_node_src] : rewrites) {
                            std::unique_lock<std::mutex> lock(work_mutex);
                            node_id next_node = string_to_node_id(next_node_src);
                            parents[next_node].emplace(rule, current_node);
                            children[current_node].emplace(rule, next_node);
                            if (visited_nodes.find(next_node) == visited_nodes.end()) {
                                visited_nodes.emplace(next_node);
                                next_nodes.emplace_back(next_node, rule);
                            }
                        }
                    }

                    /* find more work */
                    std::vector<node_id> add_to_pending;
                    for (auto [next_node, rule] : next_nodes) {
                        if (should_make_transition(current_node, next_node, rule)) {
                            std::unique_lock<std::mutex> lock(work_mutex);
                            is_real_node[next_node] = true;
                            has_continuation[current_node] = true;
                            add_to_pending.push_back(next_node);
                        }
                    }

                    /* add work to the queue */
                    if (!add_to_pending.empty()) {
                        std::unique_lock<std::mutex> lock(work_mutex);
                        for (auto id : add_to_pending) {
                            pending_nodes.emplace(id);
                        }
                    }

                    /* notify all threads that there is more work available */
                    if (!add_to_pending.empty()) {
                        cv.notify_all();
                    }
                }
            }));
        }
        for (auto& t : thread_pool) {
            t.join();
        }
        std::cout << visited_nodes.size() << std::endl;
    }

/**
 * We are in a scenario like the following
 *
 *      (O)──(A)──(B)
 *       │
 *       └──(C)
 *
 * and we would like to know if there is some (C) that relates to (O) in
 * the way that (B) relates to (A), if that is the case, we do not want
 * to perform the transition, since these changes will be incorporated in
 * other branches
 */
bool should_make_transition(node_id a, node_id b, rule_id rule) {
    std::vector<std::pair<node_id, node_id>> oc_pairs;
    for (const auto& [o_rule, o] : parents[a]) {
        for (const auto& [c_rule, c] : children[o]) {
            if (rule == c_rule) {
                oc_pairs.emplace_back(o, c);
            }
        }
    }
    for (const auto& [o, c] : oc_pairs) {
        auto diff = nway::diff(node_to_file[o], {node_to_file[a], node_to_file[b], node_to_file[c]});
        auto all_agree = std::all_of(diff.begin(), diff.end(), [](const auto& hunk) {
            const auto& [oi, candidates] = hunk;
            auto& ai = candidates[0];
            auto& bi = candidates[1];
            auto& ci = candidates[2];
            return ai == bi || bi == ci;
        });
        if (all_agree) {
            return false;
        }
    }
    return true;
}

/**
 * Given a source file, create a new Soufflé program, run the analysis, extract
 * and perform rewrites and finally return the set of resulting strings and the
 * rule ids for each rewrite.
 */
std::set<std::pair<rule_id,std::string>> get_rewrites(std::string source) {

    const char* program_name = "logifix";
    const char* filename = "file";

    auto* prog = souffle::ProgramFactory::newInstance(program_name);

    /* add javadoc info to prog */
    auto lex_result = sjp::lex(filename, (const uint8_t*) source.c_str());
    if (lex_result) {
        auto comments = lex_result->second;
        souffle::Relation* javadoc_references = prog->getRelation("javadoc_references");
        for (auto comment : comments) { 
            for (auto class_name : javadoc::get_classes(comment)) {
                javadoc_references->insert(
                    souffle::tuple(javadoc_references, {prog->getSymbolTable().encode(filename),
                                              prog->getSymbolTable().encode(class_name)}));
            }
        }
    }

    /* add ast info to prog */
    sjp::parse(prog, filename, source.c_str());

    /* add source_code info to prog */
    souffle::Relation* source_code_relation = prog->getRelation("source_code");
    source_code_relation->insert(
        souffle::tuple(source_code_relation, {prog->getSymbolTable().encode(filename),
                                  prog->getSymbolTable().encode(source)}));

    /* run program */
    prog->run();

    /* extract rewrites */
    souffle::Relation* relation = prog->getRelation("rewrite");
    std::set<std::pair<std::string, std::string>> rewrites;

    for (souffle::tuple& output : *relation) {

        rule_id rule;
        std::string filename;
        int start;
        int end;
        std::string replacement;

        output >> rule >> filename >> start >> end >> replacement;

        rewrites.emplace(rule, source.substr(0, start) + replacement + source.substr(end));

    }

    delete prog;

    return rewrites;

}


} // namespace logifix
