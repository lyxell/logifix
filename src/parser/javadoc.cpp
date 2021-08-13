#include <regex>
#include <set>

namespace {
const std::regex JAVADOC_LINK_REGEX(R"(\{@(link|linkplain) (.*?)\})");
const std::regex JAVADOC_SEE_THROWS_REGEX(R"(@(see|throws) (.*))");
} // namespace

namespace logifix::parser::javadoc {

/**
 * Expects a string such as java.util.Collection#add(java.lang.Object)
 */
auto get_classes_from_link(std::string s) -> std::vector<std::string> {

    std::vector<std::string> result;
    std::string class_str;

    // Parse class
    size_t pos = 0;
    while (pos < s.size() && s[pos] != '#' && s[pos] != '(' && s[pos] != ' ') {
        pos++;
    }
    result.emplace_back(s.substr(0, pos));

    // Parse method
    if (pos == s.size()) {
        return result;
    }
    s = s.substr(pos);
    pos = 0;
    if (s[pos] == '#') {
        s = s.substr(1);
        while (pos < s.size() && s[pos] != '(' && s[pos] != ' ') {
            pos++;
        }
        if (pos == s.size()) {
            return result;
        }
        s = s.substr(pos);
        pos = 0;
    }

    // Parse parameters
    if (s[pos] == '(') {
        s = s.substr(1);
        std::string curr;
        while (pos < s.size() && s[pos] != ')') {
            if (s[pos] == ',') {
                result.emplace_back(curr);
                curr = {};
                pos++;
                continue;
            }
            curr += s[pos];
            pos++;
        }
        result.emplace_back(curr);
    }

    return result;
}

/**
 * Expects a multi-line comment
 */
auto get_classes(std::string s) -> std::set<std::string> {
    std::set<std::string> result;
    for (auto [idx, regex] :
         {std::pair(2, JAVADOC_LINK_REGEX), std::pair(2, JAVADOC_SEE_THROWS_REGEX)}) {
        auto words_begin = std::sregex_iterator(s.begin(), s.end(), regex);
        auto words_end = std::sregex_iterator();
        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            for (auto class_name : get_classes_from_link(match[idx])) {
                class_name.erase(std::remove_if(class_name.begin(), class_name.end(), ::isspace),
                                 class_name.end());
                if (!class_name.empty()) {
                    result.emplace(class_name);
                }
            }
        }
    }
    return result;
}

} // namespace logifix::parser::javadoc
