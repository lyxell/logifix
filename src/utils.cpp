#include "utils.h"

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
    return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
}

}
