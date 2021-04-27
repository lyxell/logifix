#pragma once

#include <map>
#include <optional>
#include "sjp/sjp.hpp"


class repair {
private:
    souffle::SouffleProgram *program;
    sjp::parser parser;
    std::vector<std::string> filenames;
    void insert_ast_node(std::tuple<std::string, int, int> tuple);
    void insert_parent_of(std::tuple<std::string, int, int> parent,
                              std::string symbol,
                              std::optional<std::tuple<std::string, int, int>> child);
    std::array<souffle::RamDomain, 3>
    ast_node_to_ram_domain(std::tuple<std::string, int, int> tuple);
    std::tuple<std::string,int,int>
    get_ast_node_from_id(const char* filename, int id);
public:
    repair();
    ~repair();
    void add_file(const char* filename);
    void add_string(const char* filename, const char* content);
    void run();
    std::shared_ptr<sjp::tree_node>
        get_ast(const char* filename);
    std::map<std::tuple<std::string,int,int>,std::string>
        get_repairable_nodes(const char* filename);
    std::map<std::tuple<std::string,int,int>,std::string>
        get_string_representation(const char* filename);
};

