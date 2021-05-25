#include "logifix.h"
#include <iostream>
#include <filesystem>

static const char* PROGRAM_NAME = "logifix";

namespace logifix {

program::program() : prog(souffle::ProgramFactory::newInstance(PROGRAM_NAME)) {
    assert(prog != nullptr);
}

void program::add_file(const char* filename) {
    if (!std::filesystem::exists(filename)) {
        std::cerr << "File " << filename << " does not exist" << std::endl;
        exit(1);
    }
    std::ifstream t(filename);
    auto string = std::string((std::istreambuf_iterator<char>(t)),
                               std::istreambuf_iterator<char>());
    if (string.size() == 0) return;
    add_string(filename, string.c_str());
}

void program::add_string(const char* filename, const char* content) {
    sjp::parse(prog.get(), filename, content);
    assert(content != nullptr);
    source_code["x"] = content;
    souffle::Relation* relation = prog->getRelation("source_code");
    relation->insert(
        souffle::tuple(relation, {prog->getSymbolTable().encode(filename), prog->getSymbolTable().encode(content)}));
}

std::string program::get_source_code(const char* filename) {
    return source_code["x"];
}

void program::run() {
    typedef std::chrono::high_resolution_clock hclock;
    auto t1 = hclock::now();
    prog->run();
    auto t2 = hclock::now();
#ifdef DEBUG
    std::cerr << "+"
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)
                     .count()
              << " ms (analysis)" << std::endl;
#endif
}

void program::print() {
    prog->printAll();
}

std::vector<std::tuple<int, int, int, std::string, std::string>>
program::get_possible_rewrites(const char* filename) {
    auto get_ast_node_from_id = [this](int id) {
        const auto* record = prog->getRecordTable().unpack(id, 4);
        assert(record != nullptr);
        return std::tuple(prog->getSymbolTable().decode(record[0]), record[2],
                          record[3]);
    };
    std::vector<std::tuple<int, int, int, std::string, std::string>> result;
    souffle::Relation* relation = prog->getRelation("rewrite");
    assert(relation != nullptr);
    for (souffle::tuple& output : *relation) {
        int rule_number;
        int id;
        std::string replacement;
        std::string tuple_filename;
        std::string message;
        output >> rule_number >> message >> tuple_filename >> id >> replacement;
        if (id == 0) {
            continue;
        }
        //if (tuple_filename != filename) continue;
        auto [str, a, b] = get_ast_node_from_id(id);
        result.emplace_back(rule_number, a, b, replacement, message);
    }
    return result;
}

std::vector<std::string>
program::get_variables_in_scope(int id) {
    std::vector<std::string> result;
    souffle::Relation* relation = prog->getRelation("in_scope");
    assert(relation != nullptr);
    for (auto& output : *relation) {
        int rel_id;
        std::string identifier;
        int type;
        output >> rel_id >> identifier >> type;
        if (rel_id != id) continue;
        result.emplace_back(std::move(identifier));
    }
    return result;
}

int program::get_point_of_declaration(int id) {
    souffle::Relation* relation = prog->getRelation("point_of_declaration");
    assert(relation != nullptr);
    for (auto& output : *relation) {
        int rel_id;
        int declaration;
        output >> rel_id >> declaration;
        if (rel_id == id) return declaration;
    }
    return 0;
}

std::tuple<std::string, int, int> program::get_node_properties(int id) {
    if (id == 0)
        return {};
    const auto* record = prog->getRecordTable().unpack(id, 4);
    assert(record != nullptr);
    return std::tuple(prog->getSymbolTable().decode(record[0]), record[2],
                      record[3]);
}

int program::get_root() {
    souffle::Relation* relation = prog->getRelation("root");
    for (auto& output : *relation) {
        int id;
        output >> id;
        return id;
    }
    return 0;
}

std::vector<std::pair<std::string, int>> program::get_children(int node) {
    souffle::Relation* relation = prog->getRelation("parent_of");
    std::vector<std::pair<std::string, int>> result;
    for (auto& output : *relation) {
        int parent;
        std::string symbol;
        int child;
        output >> parent >> symbol >> child;
        if (parent == node) {
            result.emplace_back(std::move(symbol), child);
        }
    }
    return result;
}

std::vector<std::pair<std::string, std::vector<int>>>
program::get_child_lists(int node) {
    souffle::Relation* relation = prog->getRelation("parent_of_list");
    std::vector<std::pair<std::string, std::vector<int>>> result;
    for (auto& output : *relation) {
        int parent;
        std::string symbol;
        int list;
        output >> parent >> symbol >> list;
        if (parent != node)
            continue;
        std::vector<int> children;
        while (list) {
            const auto* record = prog->getRecordTable().unpack(list, 2);
            children.emplace_back(record[0]);
            list = record[1];
        }
        result.emplace_back(std::move(symbol), std::move(children));
    }
    return result;
}

} // namespace logifix
