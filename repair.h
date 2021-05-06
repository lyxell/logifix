#pragma once

#include "sjp/sjp.h"
#include <map>
#include <optional>

using node_ptr = std::shared_ptr<sjp::tree_node>;

class repair {
  private:
    std::unique_ptr<souffle::SouffleProgram> program;
    sjp::parser parser;
    std::vector<std::string> filenames;
    void insert_ast_node(std::shared_ptr<sjp::tree_node> node);
    void insert_parent_of(std::shared_ptr<sjp::tree_node> parent,
                          std::string symbol,
                          std::shared_ptr<sjp::tree_node> child);
    int ast_node_to_record(std::shared_ptr<sjp::tree_node> node);
    int vector_of_ast_nodes_to_record(
        const std::vector<std::shared_ptr<sjp::tree_node>>& nodes, int offset);
    std::tuple<std::string, int, int> get_ast_node_from_id(const char* filename,
                                                           int id);
    void insert_parent_of_list(node_ptr parent, std::string symbol,
                               const std::vector<node_ptr>& children);
    void insert_node_data(node_ptr node);
    int string_to_id(const std::string& str);

  public:
    repair();
    void add_file(const char* filename);
    void add_string(const char* filename, const char* content);
    void run();
    node_ptr get_ast(const char* filename);
    std::vector<std::tuple<int, int, std::string>>
    get_possible_repairs(const char* filename);
    std::vector<std::tuple<std::string,std::string,int,int>>
    get_variables_in_scope(const char* filename);
    std::map<std::tuple<std::string, int, int>, std::string>
    get_string_representation(const char* filename);
    node_ptr
    get_hovered_node(const char* filename, size_t buffer_position);

};
