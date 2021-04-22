#include "program.hpp"
#include <iostream>

repair::program::program() : parser("repair") {

}

repair::program::~program() {

}

std::shared_ptr<sjp::tree_node> repair::program::get_ast(const char* filename) {
    return parser.get_ast(filename);
}

void repair::program::add_file(const char* filename) {
    parser.add_file(filename);
}

void repair::program::add_string(const char* filename, const char* content) {
    parser.add_string(filename, content);
}

void repair::program::run() {
    parser.run();
}

std::vector<std::tuple<std::string,int,int>>
repair::program::get_repairable_nodes(const char* filename) {
    std::vector<std::tuple<std::string,int,int>> result;
    souffle::Relation* relation = parser.get_relation("rewrite");
    assert(relation != NULL);
    for (auto &output : *relation) {
        int id;
        output >> id;
        if (id == 0) continue;
        result.push_back(parser.get_ast_node_from_id(filename, id));
    }
    return result;
}

std::vector<std::tuple<std::string,int,int>>
repair::program::get_ast_nodes(const char* filename) {
    return parser.get_ast_nodes(filename);
}



