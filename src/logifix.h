#pragma once

#include "sjp.h"
#include <set>
#include <map>
#include <functional>
#include <optional>
#include <algorithm>

namespace logifix {

using rule_id = std::string;
using node_id = size_t;

node_id add_file(std::string file);

void run(std::function<void(node_id)> report_progress);

bool edit_scripts_are_equal(std::string o, std::string a, std::string b, std::string c);

std::set<std::pair<rule_id,std::string>> get_rewrites(std::string);

std::vector<std::pair<rule_id, std::string>> get_rewrites_for_file(std::string file);

void print_performance_metrics();

} // namespace logifix
