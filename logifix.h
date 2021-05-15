#pragma once

#include "sjp/sjp.h"
#include <map>
#include <optional>

namespace logifix {

class repair {
  private:
    std::unordered_map<std::string, std::string> source_code;
    std::unordered_map<std::string, sjp::ast> asts;
    std::unique_ptr<souffle::SouffleProgram> program;
    void insert_ast_node(sjp::ast&, sjp::ast_node);
    void insert_parent_of(sjp::ast& ast,
                          sjp::ast_node parent,
                          const std::string& symbol,
                          sjp::ast_node child);
    int ast_node_to_record(sjp::ast& ast, sjp::ast_node node);
    int vector_of_ast_nodes_to_record(sjp::ast&,
        const std::vector<sjp::ast_node>&, size_t);
    std::tuple<std::string, int, int> get_ast_node_from_id(int id);
    void insert_parent_of_list(sjp::ast&, sjp::ast_node, std::string,
                               const std::vector<sjp::ast_node>&);
    void insert_node_data(sjp::ast&, sjp::ast_node);
    int string_to_id(const std::string& str);

  public:
    repair();
    void add_file(const char* filename);
    void add_string(const char* filename, const char* content);
    void run();
    sjp::ast get_ast(const char* filename);
    std::vector<std::tuple<int, int, std::string, std::string>>
    get_possible_repairs(const char* filename);
    std::vector<std::tuple<std::string,std::string,int,int>>
    get_variables_in_scope(const char* filename);

};

}
