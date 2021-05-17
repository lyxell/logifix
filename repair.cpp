#include "logifix.h"
#include <iostream>

static const char* PROGRAM_NAME = "logifix";

namespace logifix {

repair::repair() : program(souffle::ProgramFactory::newInstance(PROGRAM_NAME)) {
    assert(program != nullptr);
}

void repair::add_file(const char* filename) {
    std::ifstream t(filename);
    auto content = std::string((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());
    source_code[filename] = content;
    sjp::parse(program.get(), content.c_str());
}

void repair::add_string(const char* filename, const char* content) {
    source_code[filename] = content;
    sjp::parse(program.get(), content);
}

void repair::run() {
    program->run();
    //program->printAll();
}

std::vector<std::tuple<int, int, std::string, std::string>>
repair::get_possible_repairs(const char* filename) {
    auto get_ast_node_from_id = [this](int id) {
        const auto* record = program->getRecordTable().unpack(id, 3);
        assert(record != nullptr);
        return std::tuple(program->getSymbolTable().decode(record[0]), record[1],
                          record[2]);
    };
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
