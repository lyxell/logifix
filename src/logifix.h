#pragma once

#include "parser/parser.h"
#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <unordered_set>
#include <set>

namespace logifix {

using rule_id = std::string;
using node_id = size_t;
using patch_id = size_t;
using rewrite_type = std::tuple<size_t, size_t, std::string>;
using rewrite_collection = std::vector<rewrite_type>;

struct node_data_type {
    node_id id;
    rule_id creation_rule;
    node_id parent;
    rewrite_collection creation_rewrites;
    std::string source_code;
    std::unordered_set<std::string> children_hashset;
    std::vector<node_id> children;
};

class program {

private:

    std::unordered_set<rule_id> disabled_rules;
    std::deque<node_id> pending_root_nodes;
    std::deque<node_id> pending_child_nodes;
    size_t id_counter = 0;
    std::unordered_map<node_id, node_data_type> node_data;

    auto
    run_datalog_analysis(const std::string&) const -> std::set<std::pair<rule_id, rewrite_type>>;
    auto print_performance_metrics() -> void;
    auto print_merge_conflict(const std::string&, rewrite_collection, const std::vector<node_id>&) const -> void;
    auto create_id() -> size_t;
    auto apply_rewrite(const std::string&, const rewrite_type&) const -> std::string;
    auto apply_rewrites(const std::string&, rewrite_collection) const -> std::string;
    auto adjust_rewrites(const rewrite_collection&, const rewrite_collection&) const -> rewrite_collection;
    auto rewrites_invert(const std::string&, rewrite_collection) const -> rewrite_collection;
    auto rewrite_collections_overlap(const rewrite_collection&,
                                          const rewrite_collection&) const -> bool;
    auto rewrite_collection_overlap(const rewrite_collection&) const -> bool;
    auto split_rewrite(const std::string& original, const rewrite_type&) const -> rewrite_collection;
    auto get_recursive_merge_result_for_node(node_id) const -> std::string;
    auto post_process(const std::string&, const std::string&) const -> std::string;

public:

    auto add_file(const std::string&) -> node_id;
    auto run(std::function<void(node_id)>) -> void;
    auto disable_rule(const rule_id&) -> void;
    auto print_graphviz_data() const -> void;
    auto add_relations(node_id id, std::vector<std::tuple<node_id, node_id, std::string>>& result) const -> void;
    auto print_json_relations(node_id) const -> void;
    auto print_json_data(node_id, std::string) const -> void;
    auto get_patch_data(patch_id) const -> std::tuple<rule_id, node_id, std::string>;
    auto get_all_patches() const -> std::vector<patch_id>;
    auto get_patches_for_rule(const rule_id&) const -> std::vector<patch_id>;
    auto get_patches_for_file(node_id) const -> std::vector<patch_id>;
    auto get_result(node_id, const std::vector<patch_id>&) const -> std::string;

};

} // namespace logifix
