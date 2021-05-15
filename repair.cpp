#include "logifix.h"
#include <iostream>

static const char* PROGRAM_NAME = "logifix";

namespace logifix {

repair::repair() : program(souffle::ProgramFactory::newInstance(PROGRAM_NAME)) {
    assert(program != nullptr);
}

sjp::ast repair::get_ast(const char* filename) {
    return asts[filename];
}

void repair::add_file(const char* filename) {
    std::ifstream t(filename);
    auto content = std::string((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());
    source_code[filename] = content;
    asts[filename] = sjp::parse_string(content.c_str());
}

void repair::add_string(const char* filename, const char* content) {
    asts[filename] = sjp::parse_string(content);
}

std::tuple<std::string, int, int>
repair::get_ast_node_from_id(int id) {
    const auto* record = program->getRecordTable().unpack(id, 3);
    assert(record != nullptr);
    return std::tuple(program->getSymbolTable().decode(record[0]), record[1],
                      record[2]);
}

int repair::ast_node_to_record(sjp::ast& ast, sjp::ast_node node) {
    if (!node) {
        return 0;
    }
    std::array<souffle::RamDomain, 3> arr = {
        program->getSymbolTable().encode(ast.name[node]),
        int32_t(ast.starts_at[node]), int32_t(ast.ends_at[node])};
    return program->getRecordTable().pack(arr.data(), 3);
}

int repair::vector_of_ast_nodes_to_record(sjp::ast& ast, const std::vector<sjp::ast_node>& nodes,
                                          size_t offset) {
    if (offset == nodes.size()) {
        return 0;
    }
    std::array<souffle::RamDomain, 2> data = {
        ast_node_to_record(ast, nodes[offset]),
        vector_of_ast_nodes_to_record(ast, nodes, offset + 1)};
    return program->getRecordTable().pack(data.data(), 2);
}

void repair::insert_ast_node(sjp::ast& ast, sjp::ast_node node) {
    auto* relation = program->getRelation("ast_node");
    assert(relation != nullptr);
    relation->insert(souffle::tuple(relation, {ast_node_to_record(ast, node)}));
}

int repair::string_to_id(const std::string& str) {
    return program->getSymbolTable().encode(str);
}

void repair::insert_parent_of(sjp::ast& ast, sjp::ast_node parent, const std::string& s, sjp::ast_node child) {
    auto* relation = program->getRelation("parent_of");
    assert(relation != nullptr);
    relation->insert(
        souffle::tuple(relation, {ast_node_to_record(ast, parent), string_to_id(s),
                                  ast_node_to_record(ast, child)}));
}

void repair::insert_parent_of_list(sjp::ast& ast, sjp::ast_node parent, std::string s,
                                   const std::vector<sjp::ast_node>& children) {
    auto* relation = program->getRelation("parent_of_list");
    assert(relation != nullptr);
    relation->insert(
        souffle::tuple(relation, {ast_node_to_record(ast, parent), string_to_id(s),
                                  vector_of_ast_nodes_to_record(ast, children, 0)}));
}

void repair::insert_node_data(sjp::ast& ast, sjp::ast_node node) {
    if (node == 0) {
        return;
    }
    insert_ast_node(ast, node);
    for (const auto& [symbol, child] : ast.parent_of[node]) {
        insert_parent_of(ast, node, symbol, child);
        insert_node_data(ast, child);
    }
    for (const auto& [symbol, children] : ast.parent_of_list[node]) {
        insert_parent_of_list(ast, node, symbol, children);
        for (const auto& child : children) {
            insert_node_data(ast, child);
        }
    }
}

void repair::run() {
    program->run();
    program->printAll();
}

std::vector<std::tuple<int, int, std::string, std::string>>
repair::get_possible_repairs(const char* filename) {
    std::vector<std::tuple<int, int, std::string, std::string>> result;
    souffle::Relation* relation = program->getRelation("rewrite");
    assert(relation != nullptr);
    for (souffle::tuple& output : *relation) {
        int id;
        std::string replacement;
        std::string message;
        output >> message >> id >> replacement;
        if (id == 0) {
            continue;
        }
        auto [str, a, b] = get_ast_node_from_id(id);
        result.emplace_back(a, b, replacement, message);
    }
    return result;
}

std::vector<std::tuple<std::string,std::string,int,int>>
repair::get_variables_in_scope(const char* filename) {
    std::vector<std::tuple<std::string,std::string,int,int>> result;
    souffle::Relation* relation = program->getRelation("is_in_scope_output");
    assert(relation != nullptr);
    for (auto& output : *relation) {
        std::string variable;
        std::string type;
        int starts_at;
        int ends_at;
        output >> variable >> type >> starts_at >> ends_at;
        result.emplace_back(variable, type, starts_at, ends_at);
    }
    return result;
}

}
