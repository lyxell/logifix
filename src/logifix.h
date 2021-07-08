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

bool edit_scripts_are_equal(std::string_view o, std::string_view a, std::string_view b, std::string_view c);

std::set<std::pair<rule_id,std::string>> get_rewrites(size_t);

std::vector<std::pair<rule_id, std::string>> get_rewrites_for_file(std::string file);

void print_performance_metrics();

} // namespace logifix
