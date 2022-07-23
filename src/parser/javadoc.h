#pragma once

#include <string>
#include <vector>
#include <set>

namespace logifix::parser::javadoc {

auto get_classes_from_link(std::string s) -> std::vector<std::string>;
auto get_classes(std::string s) -> std::set<std::string>;

} // namespace logifix::parser::javadoc
