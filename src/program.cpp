#include "logifix.h"
#include <filesystem>
#include <unordered_set>
#include <regex>
#include <iostream>

using rewrite_t = std::tuple<std::string,std::string,size_t,size_t,std::string>;

static const char* PROGRAM_NAME = "logifix";

static const std::regex JAVADOC_LINK_REGEX(R"(\{@(link|linkplain) (.*?)\})");
static const std::regex JAVADOC_SEE_REGEX(R"(@see (.*))");

static std::vector<std::string> get_classes_from_javadoc_link(std::string s) {

    std::vector<std::string> result;
    std::string class_str;

    // Parse class
    size_t pos = 0;
    while (pos < s.size() && s[pos] != '#' && s[pos] != '(' && s[pos] != ' ') {
        pos++;
    }
    result.emplace_back(s.substr(0, pos));

    // Parse method
    if (pos == s.size()) return result;
    s = s.substr(pos);
    pos = 0;
    if (s[pos] == '#') {
        s = s.substr(1);
        while (pos < s.size() && s[pos] != '(' && s[pos] != ' ') {
            pos++;
        }
        if (pos == s.size()) return result;
        s = s.substr(pos);
        pos = 0;
    }

    // Parse parameters
    if (s[pos] == '(') {
        s = s.substr(1);
        std::string curr;
        while (pos < s.size() && s[pos] != ')') {
            if (s[pos] == ',') {
                result.emplace_back(curr);
                curr = {};
                pos++;
                continue;
            }
            curr += s[pos];
            pos++;
        }
        result.emplace_back(curr);
    }

    return result;
}

static std::set<std::string> get_classes_from_javadoc(std::string s) {
    std::set<std::string> result;
    {
        auto words_begin = std::sregex_iterator(s.begin(), s.end(), JAVADOC_LINK_REGEX);
        auto words_end = std::sregex_iterator();
        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            for (auto class_name : get_classes_from_javadoc_link(match[2])) {
                class_name.erase(std::remove_if(class_name.begin(), class_name.end(), ::isspace), class_name.end());
                if (!class_name.empty()) {
                    result.emplace(class_name);
                }
            }
        }
    }
    {
        auto words_begin = std::sregex_iterator(s.begin(), s.end(), JAVADOC_SEE_REGEX);
        auto words_end = std::sregex_iterator();
        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            for (auto class_name : get_classes_from_javadoc_link(match[1])) {
                class_name.erase(std::remove_if(class_name.begin(), class_name.end(), ::isspace), class_name.end());
                if (!class_name.empty()) {
                    result.emplace(class_name);
                }
            }
        }
    }
    return result;
}

namespace logifix {

program::program() {
    prog = souffle::ProgramFactory::newInstance(PROGRAM_NAME);
    assert(prog != nullptr);
}

program::~program() {
    delete prog;
}

void program::add_string(const char* filename, const char* content) {
    files.emplace(filename, content);
    auto lex_result = sjp::lex(filename, (const uint8_t*) content);
    if (lex_result) {
        auto comments = lex_result->second;
        souffle::Relation* javadoc_references = prog->getRelation("javadoc_references");
        for (auto comment : comments) { 
            for (auto class_name : get_classes_from_javadoc(comment)) {
                javadoc_references->insert(
                    souffle::tuple(javadoc_references, {prog->getSymbolTable().encode(filename),
                                              prog->getSymbolTable().encode(class_name)}));
            }
        }
    }
    sjp::parse(prog, filename, content);
    assert(content != nullptr);
    souffle::Relation* relation = prog->getRelation("source_code");
    relation->insert(
        souffle::tuple(relation, {prog->getSymbolTable().encode(filename),
                                  prog->getSymbolTable().encode(content)}));
}

std::set<std::pair<std::string, std::string>> program::run(std::string file, std::set<std::string> rules) {
    add_string("original", file.c_str());

    std::set<rewrite_t> rewrites;
    std::map<std::string,std::string> file_set;
    std::set<std::string> has_children;

    std::map<std::string, rewrite_t> created_from;

    while (true) {

        // run program
        prog->run();

        // collect all new rewrites
        souffle::Relation* relation = prog->getRelation("rewrite");
        assert(relation != nullptr);
        std::vector<rewrite_t> new_rewrites;
        for (souffle::tuple& output : *relation) {
            std::string rule;
            std::string filename;
            int start;
            int end;
            std::string replacement;
            output >> rule >> filename >> start >> end >> replacement;

            rewrite_t rewrite = {rule, filename, start, end, replacement};

            // skip this rewrite if it has been applied to this file before
            if (rewrites.find(rewrite) != rewrites.end()) continue;

            rewrites.emplace(rewrite);

            // skip this rewrite if it exists in a parent
            // FIXME this could be simplified with nway
            if (created_from.find(filename) != created_from.end()) {
                auto [prule, parent_filename, pstart, pend, prepl] = created_from[filename];
                if (end <= pstart) {
                    rewrite_t matching_rewrite = {rule, parent_filename, start, end, replacement};
                    if (rewrites.find(matching_rewrite) != rewrites.end()) {
                        continue;
                    }
                } else if (start >= (pstart + prepl.size())) {
                    auto diff = (pend - pstart) - prepl.size();
                    rewrite_t matching_rewrite = {rule, parent_filename, start + diff, end + diff, replacement};
                    if (rewrites.find(matching_rewrite) != rewrites.end()) {
                        continue;
                    }
                }
            }

            new_rewrites.emplace_back(rewrite);
        }


        std::vector<std::string> new_files;
        for (auto new_rewrite : new_rewrites) {
            auto [rule, filename, start, end, replacement] = new_rewrite;
            // do not perform rewrites of the original file that the user is not interested in
            if (filename == "original" && !rules.empty() && rules.find(rule) == rules.end()) {
                continue;
            }
            auto source = files[filename];
            auto dest = source.substr(0, start) + replacement + source.substr(end);
            auto dest_filename = filename + "-[" + rule + "," + std::to_string(start) + "," + std::to_string(end) + "," + std::to_string(std::hash<std::string>{}(replacement)) + "]";
            has_children.emplace(source);
            created_from.emplace(dest_filename, new_rewrite);
            if (file_set.find(dest) == file_set.end()) {
                new_files.emplace_back(dest);
                file_set.emplace(dest, dest_filename);
                add_string(dest_filename.c_str(), dest.c_str());
            }
        }

        // if there are no new source files, break
        if (new_files.size() == 0) {
            break;
        }

    }

    std::set<std::pair<std::string, std::string>> result;

    for (auto [str,fn] : file_set) {
        if (has_children.find(str) == has_children.end()) {
            auto curr = fn;
            while (std::get<1>(created_from[curr]) != "original") {
                curr = std::get<1>(created_from[curr]);
            }
            result.emplace(str, std::get<0>(created_from[curr]));
        }
    }
    
    return result;

}

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
        int rule;
        std::string filename;
        int start;
        int end;
        std::string replacement;
        output >> rule >> filename >> start >> end >> replacement;
        result.emplace_back(rule, start, end, replacement);
    }
    return result;
}

std::vector<std::string> program::get_variables_in_scope(int id) {
    return {};
    /*
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
    return result;*/
}

std::vector<int> program::get_point_of_declaration(int id) {
    souffle::Relation* relation = prog->getRelation("point_of_declaration");
    assert(relation != nullptr);
    std::vector<int> result;
    for (auto& output : *relation) {
        int rel_id;
        int declaration;
        output >> rel_id >> declaration;
        if (rel_id == id) {
            result.emplace_back(declaration);
        }
    }
    return result;
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
