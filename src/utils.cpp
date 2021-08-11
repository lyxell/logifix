#include "utils.h"
#include <algorithm>
#include <map>

namespace utils {

std::vector<std::string> line_split(const std::string& str) {
    auto view = std::string_view{str};
    std::vector<std::string> result;
    while (!view.empty()) {
        size_t i;
        for (i = 0; i < view.size() - 1; i++) {
            if (view[i] == '\n') {
                break;
            }
        }
        result.emplace_back(view.substr(0, i + 1));
        view = view.substr(i + 1);
    }
    return result;
}

bool starts_with(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.substr(str.size() - suffix.size()) == suffix;
}

std::string rtrim(std::string s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](auto c) { return !std::isspace(c); })
                .base(),
            s.end());
    return s;
}

bool string_has_only_whitespace(const std::string& str) {
    return std::all_of(str.begin(), str.end(),
                       [](char c) { return std::isspace(c) != 0; });
}

std::string::const_iterator find_first_non_space(const std::string& str) {
    return std::find_if(str.begin(), str.end(),
                        [](char c) { return std::isspace(c) == 0; });
}

std::string detect_indentation(const std::string& str) {
    auto lines = utils::line_split(str);
    std::vector<std::string> indentations;
    for (auto line : lines) {
        auto first_non_space_at = utils::find_first_non_space(line);
        if (first_non_space_at == line.end() ||
            first_non_space_at == line.begin()) {
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
    std::map<std::string, double> multiplier = {
        {"  ", 0.5}, {"    ", 0.25}, {"\t", 0.5}};
    std::map<std::string, double> candidates;
    for (size_t i = 1; i < indentations.size(); i++) {
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
    std::string best_candidate = "    ";
    size_t best_result = 0;
    for (auto [k, v] : candidates) {
        if (v > best_result) {
            best_result = v;
            best_candidate = k;
        }
    }
    return best_candidate;
}

std::string detect_line_terminator(const std::string& str) {
    auto lines = utils::line_split(str);
    size_t lf = 0;
    size_t crlf = 0;
    for (const auto& line : lines) {
        if (utils::ends_with(line, "\r\n")) {
            crlf++;
        } else {
            lf++;
        }
    }
    if (crlf > lf) {
        return "\r\n";
    } else {
        return "\n";
    }
}

} // namespace utils
