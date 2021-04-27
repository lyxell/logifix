#include "repair.hpp"
#include <iostream>

repair::repair() : parser() {
    program = souffle::ProgramFactory::newInstance("program");
    assert(program != NULL);
}

repair::~repair() {
    delete program;
}

std::shared_ptr<sjp::tree_node>
repair::get_ast(const char* filename) {
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

std::array<souffle::RamDomain, 3>
repair::ast_node_to_ram_domain(std::tuple<std::string, int, int> tuple) {
    return {
        program->getSymbolTable().encode(std::get<0>(tuple)),
        std::get<1>(tuple),
        std::get<2>(tuple)
    };
}

void repair::insert_ast_node(std::tuple<std::string, int, int> node) {
    auto node_data = ast_node_to_ram_domain(node);
    souffle::Relation* ast_node = program->getRelation("ast_node");
    assert(ast_node != NULL);
    souffle::tuple tuple(ast_node);
    tuple << program->getRecordTable().pack(node_data.data(), 3);
    ast_node->insert(tuple);
}

void repair::insert_parent_of(std::tuple<std::string, int, int> parent,
                              std::string symbol,
                              std::optional<std::tuple<std::string, int, int>> child) {
    auto parent_data = ast_node_to_ram_domain(parent);
    souffle::Relation* parent_of = program->getRelation("parent_of");
    assert(parent_of != NULL);
    souffle::tuple tuple(parent_of);
    tuple << program->getRecordTable().pack(parent_data.data(), 3);
    tuple << symbol;
    if (child) {
        auto child_data = ast_node_to_ram_domain(*child);
        tuple << program->getRecordTable().pack(child_data.data(), 3);
    } else {
        tuple << 0;
    }
    parent_of->insert(tuple);
}

void repair::run() {
    parser.run();
    for (auto name : filenames) {
        // create ast_node and parent_of relation
        auto ast = parser.get_ast(name.c_str());
        std::vector<std::shared_ptr<sjp::tree_node>> nodes {ast};
        while (nodes.size()) {
            auto parent = nodes.back();
            nodes.pop_back();
            if (parent == nullptr) continue;
            auto parent_data = std::tuple(parent->get_name(),
                                          parent->get_start_token(),
                                          parent->get_end_token());
            insert_ast_node(parent_data);
            for (auto [sym, child] : parent->get_parent_of()) {
                if (child) {
                    auto child_data = std::tuple(child->get_name(),
                                                  child->get_start_token(),
                                                  child->get_end_token());
                    insert_ast_node(child_data);
                    insert_parent_of(parent_data, sym, child_data);
                    nodes.push_back(child);
                } else {
                    insert_parent_of(parent_data, sym, {});
                }
            }
        }

    }
    program->run();
}

std::map<std::tuple<std::string,int,int>, std::string>
repair::get_repairable_nodes(const char* filename) {
    std::map<std::tuple<std::string,int,int>, std::string> result;
    auto string_rep = get_string_representation(filename);
    souffle::Relation* relation = program->getRelation("rewrite");
    assert(relation != NULL);
    for (auto &output : *relation) {
        int id;
        std::string out;
        output >> id >> out;
        if (id == 0) continue;
        result.emplace(get_ast_node_from_id(filename, id), out);
    }
    return result;
}

std::map<std::tuple<std::string,int,int>, std::string>
repair::get_string_representation(const char* filename) {
    std::map<std::tuple<std::string,int,int>, std::string> result;
    souffle::Relation* relation = program->getRelation("string_representation");
    assert(relation != NULL);
    for (auto &output : *relation) {
        int id;
        output >> id;
        if (id == 0) continue;
        std::string symbol;
        output >> symbol;
        result.emplace(get_ast_node_from_id(filename, id), symbol);
    }
    return result;
}



