#include "repair.hpp"
#include <iostream>

repair::repair() : parser(),
                   program(souffle::ProgramFactory::newInstance("program")) {
    assert(program != NULL);
}

node_ptr repair::get_ast(const char* filename) {
    return parser.get_ast(filename);
}

void repair::add_file(const char* filename) {
    filenames.push_back(filename);
    parser.add_file(filename);
}

void repair::add_string(const char* filename, const char* content) {
    souffle::Relation* source_code = program->getRelation("source_code");
    assert(source_code != NULL);
    souffle::tuple tuple(source_code);
    tuple << std::string(content);
    source_code->insert(tuple);
    filenames.push_back(filename);
    parser.add_string(filename, content);
}

std::tuple<std::string,int,int>
repair::get_ast_node_from_id(const char* filename, int id) {
    auto record = program->getRecordTable().unpack(id, 3);
    assert(record != NULL);
    return std::tuple(
            program->getSymbolTable().decode(record[0]),
            record[1],
            record[2]);
}

int repair::ast_node_to_record(node_ptr node) {
    if (!node) return 0;
    std::array<souffle::RamDomain, 3> data = {
        program->getSymbolTable().encode(node->get_name()),
        node->get_start_token(),
        node->get_end_token()
    };
    return program->getRecordTable().pack(data.data(), 3);
}

int repair::vector_of_ast_nodes_to_record(const std::vector<node_ptr>& nodes,
        int offset) {
    if (offset == nodes.size()) return 0; 
    std::array<souffle::RamDomain, 2> data = {
        ast_node_to_record(nodes[offset]),
        vector_of_ast_nodes_to_record(nodes, offset + 1)
    };
    return program->getRecordTable().pack(data.data(), 2);
}

void repair::insert_ast_node(node_ptr node) {
    auto relation = program->getRelation("ast_node");
    assert(relation != NULL);
    relation->insert(souffle::tuple(relation, {ast_node_to_record(node)}));
}

int repair::string_to_id(const std::string& str) {
    return program->getSymbolTable().encode(str);
}

void repair::insert_parent_of(node_ptr parent, std::string s, node_ptr child) {
    auto relation = program->getRelation("parent_of");
    assert(relation != NULL);
    relation->insert(souffle::tuple(relation, {ast_node_to_record(parent),
                                               string_to_id(s),
                                               ast_node_to_record(child)}));
}

void repair::insert_parent_of_list(node_ptr parent, std::string s,
                                   std::vector<node_ptr> children) {
    auto relation = program->getRelation("parent_of_list");
    assert(relation != NULL);
    relation->insert(souffle::tuple(relation,
                {ast_node_to_record(parent),
                 string_to_id(s),
                 vector_of_ast_nodes_to_record(children, 0)}));
}

void repair::insert_node_data(node_ptr node) {
    if (node == nullptr) return;
    insert_ast_node(node);
    for (auto [symbol, child] : node->get_parent_of()) {
        insert_parent_of(node, symbol, child);
        insert_node_data(child);
    }
    for (auto [symbol, children] : node->get_parent_of_list()) {
        insert_parent_of_list(node, symbol, children);
        for (auto child : children) {
            insert_node_data(child);
        }
    }
}

void repair::run() {
    parser.run();
    for (auto name : filenames) {
        insert_node_data(parser.get_ast(name.c_str()));
    }
    program->run();
}

std::map<std::tuple<std::string,int,int>, std::string>
repair::get_repairable_nodes(const char* filename) {
    std::map<std::tuple<std::string,int,int>, std::string> result;
    souffle::Relation* relation = program->getRelation("rewrite");
    assert(relation != NULL);
    for (souffle::tuple& output : *relation) {
        int id;
        std::string out;
        output >> id >> out;
        if (id == 0) continue;
        result.emplace(get_ast_node_from_id(filename, id), out);
    }
    return result;
}

std::map<std::tuple<std::string,int,int>,std::vector<std::string>>
repair::get_reachable_declared_variables(const char* filename) {
    std::map<std::tuple<std::string,int,int>, std::vector<std::string>> result;
    souffle::Relation* relation =
        program->getRelation("reachable_declared_variable");
    assert(relation != NULL);
    for (auto &output : *relation) {
        int id;
        std::string out;
        output >> id >> out;
        if (id == 0) continue;
        result[get_ast_node_from_id(filename, id)].push_back(out);
    }
    return result;
}

