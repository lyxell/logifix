#pragma once

#include "sjp.h"
#include <set>
#include <map>
#include <optional>
#include <algorithm>

/**
 * Finds intersection of two one dimensional line segments
 */
template <typename T>
static std::optional<std::pair<T, T>>
find_intersection(std::pair<T, T> a, std::pair<T, T> b) {
    if (a > b) {
        std::swap(a, b);
    }
    if (a.second <= b.first) {
        return {};
    }
    return std::pair(b.first, std::min(a.second, b.second));
}

namespace logifix {

using rewrite_t = std::tuple<int, size_t, size_t, std::string>;

class file_state {
public:
    std::set<rewrite_t> available_rewrites;
    std::string file_content;
    file_state perform_rewrite(rewrite_t rewrite) {
        auto [rule_number, start, end, replacement] = rewrite;
        file_state new_state;
        std::vector<rewrite_t> vec(available_rewrites.begin(), available_rewrites.end());
        /* Remove all intersecting rewrites */
        vec.erase(std::remove_if(vec.begin(), 
                                 vec.end(),
                                 [start,end](const auto& x) { return find_intersection(std::pair(std::get<1>(x), std::get<2>(x)),
                                                                                       std::pair(start, end)).has_value(); }),
               vec.end());
        /* Update positions of remaining rewrites that starts after current rewrite */
        for (auto& [crule_number, cstart, cend, creplacement] : vec) {
            auto original_length = end - start;
            auto new_length = replacement.size();
            long diff = long(new_length) - long(original_length);
            if (cstart >= end) {
                cstart += diff;
                cend += diff;
            }
        }
        new_state.available_rewrites = std::set(vec.begin(), vec.end());
        new_state.file_content = file_content.substr(0, start) + replacement + file_content.substr(end);
        return new_state;
    }
};

class program {
  private:
    std::unique_ptr<souffle::SouffleProgram> prog;
    std::map<std::string, std::string> files;

  public:
    program();
    void add_string(const char* filename, const char* content);
    void run();
    void print();
    std::vector<std::tuple<int, size_t, size_t, std::string>>
    get_possible_rewrites(const char* filename);
    std::vector<std::string>
    get_variables_in_scope(int id);
    int get_root();
    std::tuple<std::string, int, int> get_node_properties(int id);
    std::vector<std::pair<std::string, int>> get_children(int node);
    std::vector<std::pair<std::string, std::vector<int>>>
    get_child_lists(int node);
    int get_point_of_declaration(int id);
};

} // namespace logifix
