#include "logifix.h"
#include "javadoc.h"
#include <filesystem>
#include <unordered_set>
#include <regex>
#include <iostream>

namespace logifix {

    using node_id = size_t;

    std::unordered_map<std::string, node_id> file_to_node;
    std::unordered_map<node_id, std::string> node_to_file;

    std::vector<std::thread> thread_pool;

    std::condition_variable cv;
    size_t waiting_threads;
    std::queue<node_id> pending_nodes;
    std::mutex work_mutex;
    std::unordered_set<node_id> visited_nodes;
    std::unordered_map<node_id, std::pair<rule_id, node_id>> parents;
    std::unordered_map<node_id, std::pair<rule_id, node_id>> children;

    void add_file(std::string file) {
        auto node_id = file_to_node.size();
        file_to_node.emplace(file, node_id);
        node_to_file.emplace(node_id, file);
        pending_nodes.emplace(node_id);
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
                        waiting_threads++;
                        /* if all threads are waiting, there is no more work and we are finished */
                        if (waiting_threads == concurrency) {
                            finished = true;
                            cv.notify_all();
                        /* wait until there is work available */
                        } else if (pending_nodes.empty()) {
                            auto wakeup_when = [&](){ return !pending_nodes.empty() || finished; };
                            cv.wait(lock, wakeup_when);
                        }
                        /* if we get to this point there is either work available or we are finished */
                        if (finished) return;
                        waiting_threads--;
                        current_node = pending_nodes.front();
                        pending_nodes.pop_front();
                        /* mark node as visited */
                        visited_nodes.add(node);
                    }

                    std::vector<node_id> next_nodes;

                    /* perform expensive computation and store result */
                    {
                        auto rewrites = get_rewrites(current_node);
                        std::unique_lock<std::mutex> lock(work_mutex);
                        for (auto [rule, next_node_src] : result) {
                            node_id next_node = string_to_node_id(next_node_src);
                            parents[next_node].emplace_back(rule, node);
                            children[node].emplace_back(rule, node);
                            if (!visited[node]) {
                                visited[node] = true;
                                next_nodes.emplace_back(node);
                            }
                        }
                    }

                    /* find more work */
                    std::vector<node_id> add_to_pending;
                    for (auto next_node : next_nodes) {
                        if (should_make_transition(current_node, next_node)) {
                            pending_nodes.emplace_back(next_node);
                        }
                    }

                    /* add work to the queue */
                    if (!add_to_pending.empty()) {
                        std::unique_lock<std::mutex> lock(work_mutex);
                        for (auto id : add_to_pending) {
                            pending_nodes.emplace_back(id);
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
    }

/**
 * We are in a scenario like the following
 *
 *      (O)─→(A)─→(B)
 *       │
 *       └─→(C)
 *
 * and we would like to know if there is some (C) that relates to (O) in
 * the way that (B) relates to (A), if that is the case, we do not want
 * to perform the transition, since these changes will be incorporated in
 * other branches
 */
bool should_make_transition(node_id a, node_id b) {
    bool make_transition = true;
    std::vector<std::pair<node_id, node_id>> oc_pairs;
    for (auto o : parents[a]) {
        for (auto c : children[o]) {
            oc_pairs.emplace_back(o, c);
        }
    }
    for (auto [o, c] : oc_pairs) {
        auto diff = nway::diff(node_to_string[o], {node_to_string[a], node_to_string[b], node_to_string[c]});
        /* TODO use std::all_of */
        /* loop through each hunk */
        bool edit_scripts_have_same_effects = true;
        for (auto [oi, candidates] : diff) {
            auto [ai, bi, ci] = candidates;
            /* positions where A_i == B_i are irrelevant */
            if (ai == bi) continue;
            /**
             * Now, since A_i != B_i this change must
             * have been introduced in (A_i → B_i)
             * so we simply check if this change was
             * also introduced in (O_i → C_i) */
            if (bi == ci) continue;
            /**
             * Now we have a scenario where B_i has changed
             * in (A_i → B_i) but not in (O_i → C_i)
             * so these edit script do not have the same
             * effects
             */
            edit_scripts_have_same_effects = false;
            break;
        }
        if (edit_scripts_have_same_effects) {
            return true;
        }
    }
}

/**
 * Given a source file, create a new Soufflé program, run the analysis, extract
 * and perform rewrites and finally return the set of resulting strings and the
 * rule ids for each rewrite.
 */
std::vector<std::pair<rule_id,std::string>> get_rewrites(std::string source) {

    is_computed = true;

    const char* program_name = "logifix";
    const char* filename = "file";

    auto* prog = souffle::ProgramFactory::newInstance(program_name);

    /* add javadoc info to prog */
    auto lex_result = sjp::lex(filename, (const uint8_t*) content);
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
    sjp::parse(prog, file, content);

    /* add source_code info to prog */
    souffle::Relation* relation = prog->getRelation("source_code");
    relation->insert(
        souffle::tuple(relation, {prog->getSymbolTable().encode(file),
                                  prog->getSymbolTable().encode(content)}));

    /* run program */
    prog->run();

    /* extract rewrites */
    souffle::Relation* relation = prog->getRelation("rewrite");
    std::vector<std::pair<std::string, std::string>> rewrites;

    for (souffle::tuple& output : *relation) {

        rule_id rule;
        std::string filename;
        int start;
        int end;
        std::string replacement;

        output >> rule >> filename >> start >> end >> replacement;

        rewrites.emplace_back(rule, source.substr(0, start) + replacement + source.substr(end));

    }

    delete prog;

    return rewrites;

}


} // namespace logifix
