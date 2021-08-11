#pragma once

#include "sjp.h"
#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <set>

namespace logifix {

using rule_id = std::string;
using node_id = size_t;
using patch_id = size_t;

using rewrite_collection = std::vector<std::tuple<size_t, size_t, std::string>>;

node_id add_file(const std::string& file);

void run(std::function<void(node_id)> report_progress);

std::set<std::pair<rule_id, std::tuple<size_t, size_t, std::string>>>
get_patches(const std::string&);

std::vector<patch_id> get_patches_for_file(node_id file);

void print_performance_metrics();

void disable_rule(const rule_id&);

std::vector<patch_id> get_patches_for_rule(rule_id rule);

std::vector<patch_id> get_all_patches();

std::string get_result(node_id, std::vector<patch_id>);

std::tuple<rule_id, node_id, std::string> get_patch_data(patch_id patch);

} // namespace logifix
