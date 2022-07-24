#pragma once
#include <string>
#include <string_view>
#include <vector>
namespace utils {
std::vector<std::string> line_split(const std::string& str);
bool starts_with(const std::string& str, const std::string& prefix);
bool ends_with(const std::string& str, const std::string& suffix);
std::string rtrim(std::string str);
std::string ltrim(std::string str);
bool string_has_only_whitespace(const std::string& str);
std::string::const_iterator find_first_non_space(const std::string& str);
std::string detect_indentation(const std::string& str);
std::string detect_line_terminator(const std::string& str);

} // namespace utils
