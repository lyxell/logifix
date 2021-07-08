#pragma once

namespace logifix {

namespace utils {

template <typename T>
void print_diff(const std::vector<std::tuple<T, std::vector<T>>>& d) {
    for (const auto& [original, candidates] : d) {
        /* collect sequences that differ from original */
        std::vector<size_t> unstable_idx;
        for (size_t i = 0; i < candidates.size(); i++) {
            if (candidates[i] != original) {
                unstable_idx.emplace_back(i);
            }
        }
        if (unstable_idx.empty()) {
            std::cout << original;
            continue;
        }
        /* compare sequences that differ from original */
        bool differ = false;
        for (size_t i = 1; i < unstable_idx.size(); i++) {
            if (candidates[unstable_idx[0]] != candidates[unstable_idx[i]]) {
                differ = true;
                break;
            }
        }
        if (!differ) {
            std::cout << candidates[unstable_idx[0]];
            continue;
        }
        std::cout << std::endl << "CONFLICT START <<<<<<<<<<<<<<<<" << std::endl;
        std::cout << original << std::endl;
        std::cout << "***********************************" << std::endl;
        for (size_t i = 0; i < unstable_idx.size(); i++) {
            std::cout << candidates[unstable_idx[i]] << std::endl;
            if (i + 1 < unstable_idx.size()) {
                std::cout << "-----------------------------------" << std::endl;
            }
        }
        std::cout << "CONFLICT END >>>>>>>>>>>>>>>>" << std::endl;
    }
}

}

}
