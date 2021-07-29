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

node_id add_file(const std::string& file);

void run(std::function<void(node_id)> report_progress);

bool edit_scripts_are_equal(std::string_view o, std::string_view a,
                            std::string_view b, std::string_view c);

std::set<std::pair<rule_id, std::string>> get_patches(const std::string&);

std::vector<std::pair<rule_id, std::string>>
get_patches_for_file(node_id file);

void print_performance_metrics();

} // namespace logifix
