#pragma once

#include "sjp.h"
#include <set>
#include <map>
#include <optional>
#include <algorithm>

namespace logifix {

class program {
  private:
    souffle::SouffleProgram* prog;
    std::map<std::string, std::string> files;
    void add_string(const char* filename, const char* content);

  public:
    program();
    ~program();
    std::set<std::pair<std::string, std::string>> run(std::string file, std::set<std::string> rules);
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
