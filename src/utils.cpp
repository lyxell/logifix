#include "utils.h"
#include <algorithm>
#include <map>

namespace utils {

/**
 * Split a string into lines by looking for a '\n' character.
 * Keeps the '\n' characters at the end of each line.
 */
auto line_split(const std::string& str) -> std::vector<std::string> {
    auto result = std::vector<std::string>{};
    auto view = std::string_view{str};
    while (!view.empty()) {
        auto i = view.find('\n');
        if (i == std::string::npos) {
            result.emplace_back(view);
            break;
        }
        result.emplace_back(view.substr(0, i + 1));
        view = view.substr(i + 1);
    }
    return result;
}

auto starts_with(const std::string& str, const std::string& prefix) -> bool {
    return str.rfind(prefix, 0) == 0;
}

auto ends_with(const std::string& str, const std::string& suffix) -> bool {
    return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
}

auto rtrim(std::string s) -> std::string {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](auto c) { return !std::isspace(c); }).base(),
            s.end());
    return s;
}

auto string_has_only_whitespace(const std::string& str) -> bool {
    return std::all_of(str.begin(), str.end(), [](char c) { return std::isspace(c) != 0; });
}

auto find_first_non_space(const std::string& str) -> std::string::const_iterator {
    return std::find_if(str.begin(), str.end(), [](char c) { return std::isspace(c) == 0; });
}

auto detect_indentation(const std::string& str) -> std::string {
    const auto TWO_SPACE_MULTIPLIER = 0.5;
    const auto FOUR_SPACE_MULTIPLIER = 0.25;
    const auto TAB_MULTIPLIER = 0.5;
    auto lines = utils::line_split(str);
    auto indentations = std::vector<std::string>{};
    for (auto line : lines) {
        auto first_non_space_at = utils::find_first_non_space(line);
        if (first_non_space_at == line.end() || first_non_space_at == line.begin()) {
            continue;
        }
        auto indent = line.substr(0, first_non_space_at - line.begin());
        /* Check that indentation has only one type of char */
        if (std::any_of(indent.begin(), indent.end(),
                        [indent](char c) { return c != indent[0]; })) {
            continue;
        }
        if (!indentations.empty() && indentations.back() == indent) {
            continue;
        }
        indentations.push_back(indent);
    }
    auto multiplier = std::map<std::string, double>{
        {"  ", TWO_SPACE_MULTIPLIER}, {"    ", FOUR_SPACE_MULTIPLIER}, {"\t", TAB_MULTIPLIER}};
    auto candidates = std::map<std::string, double>{};
    for (auto i = std::size_t{1}; i < indentations.size(); i++) {
        auto a = indentations[i - 1];
        auto b = indentations[i];
        if (a.size() > b.size()) {
            std::swap(a, b);
        }
        if (b.find(a) != 0) {
            continue;
        }
        auto candidate = b.substr(a.size());
        candidates[candidate] += 1 + multiplier[candidate];
    }
    auto best_candidate = std::string{"    "};
    auto best_result = 0;
    for (auto [k, v] : candidates) {
        if (v > best_result) {
            best_result = v;
            best_candidate = k;
        }
    }
    return best_candidate;
}

auto detect_line_terminator(const std::string& str) -> std::string {
    auto lf = 0;
    auto crlf = 0;
    for (const auto& line : utils::line_split(str)) {
        if (utils::ends_with(line, "\r\n")) {
            crlf++;
        } else if (utils::ends_with(line, "\n")) {
            lf++;
        }
    }
    if (crlf > lf) {
        return "\r\n";
    }
    return "\n";
}

} // namespace utils
