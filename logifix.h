#pragma once

#include "sjp/sjp.h"
#include <map>
#include <optional>

namespace logifix {

class program {
  private:
    std::unordered_map<std::string, std::string> source_code;
    std::unique_ptr<souffle::SouffleProgram> prog;

  public:
    program();
    void add_file(const char* filename);
    void add_string(const char* filename, const char* content);
    void run();
    void print();
    std::string get_source_code(const char* filename);
    std::vector<std::tuple<int, size_t, size_t, std::string, std::string>>
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
