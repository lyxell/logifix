#pragma once
#include <string_view>
#include <vector>
#include <string>
namespace utils {
std::vector<std::string> line_split(const std::string& str);
bool starts_with(const std::string& str, const std::string& prefix);
}
