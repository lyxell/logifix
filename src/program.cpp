#include "logifix.h"
#include <filesystem>
#include <iostream>

static void replace_all(std::string& source, const std::string& from, const std::string& to)
{
    std::string new_string;
    new_string.reserve(source.length());
    std::string::size_type last_pos = 0;
    std::string::size_type find_pos;
    while (std::string::npos != (find_pos = source.find(from, last_pos))) {
        new_string.append(source, last_pos, find_pos - last_pos);
        new_string += to;
        last_pos = find_pos + from.length();
    }
    new_string += source.substr(last_pos);
    source.swap(new_string);
}

/* workaround for https://github.com/souffle-lang/souffle/issues/1947 */
static void unescape(std::string& str) {
    replace_all(str, "%U+0022%", "\"");
}

static const char* PROGRAM_NAME = "logifix";

namespace logifix {

program::program() : prog(souffle::ProgramFactory::newInstance(PROGRAM_NAME)) {
    assert(prog != nullptr);
}

void program::add_string(const char* filename, const char* content) {
    files.emplace(filename, content);
    sjp::parse(prog.get(), filename, content);
    assert(content != nullptr);
    souffle::Relation* relation = prog->getRelation("source_code");
    relation->insert(
        souffle::tuple(relation, {prog->getSymbolTable().encode(filename),
                                  prog->getSymbolTable().encode(content)}));
}

void program::run() { prog->run(); }

void program::print() { prog->printAll(); }

std::vector<std::tuple<int, size_t, size_t, std::string>>
program::get_possible_rewrites(const char* filename) {
    auto get_ast_node_from_id = [this](int id) {
        const auto* record = prog->getRecordTable().unpack(id, 4);
        assert(record != nullptr);
        return std::tuple(prog->getSymbolTable().decode(record[0]),
                          size_t(record[2]), size_t(record[3]));
    };
    std::vector<std::tuple<int, size_t, size_t, std::string>>
        result;
    souffle::Relation* relation = prog->getRelation("rewrite");
    assert(relation != nullptr);
    for (souffle::tuple& output : *relation) {
        int record_id;
        output >> record_id;
        const auto* record = prog->getRecordTable().unpack(record_id, 5);
        assert(record != nullptr);
        auto replacement = prog->getSymbolTable().decode(record[4]);
        // workaround for https://github.com/souffle-lang/souffle/issues/1947
        unescape(replacement);
        result.emplace_back(record[0], record[2], record[3], replacement);
    }
    return result;
}

std::vector<std::string> program::get_variables_in_scope(int id) {
    std::vector<std::string> result;
    souffle::Relation* relation = prog->getRelation("in_scope");
    assert(relation != nullptr);
    for (auto& output : *relation) {
        int rel_id;
        std::string identifier;
        int type;
        output >> rel_id >> identifier >> type;
        if (rel_id != id)
            continue;
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
        if (rel_id == id)
            return declaration;
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
