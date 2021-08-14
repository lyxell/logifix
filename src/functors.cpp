#include "utils.h"
#include <souffle/SouffleInterface.h>

extern "C" {
souffle::RamDomain decrease_indentation(souffle::SymbolTable* symbolTable,
                                        souffle::RecordTable* recordTable, souffle::RamDomain arg) {
    const std::string& str = symbolTable->decode(arg);
    auto lines = utils::line_split(str);
    std::string result;
    for (const auto& line : lines) {
        if (utils::starts_with(line, "    ")) {
            result += line.substr(4);
        } else if (utils::starts_with(line, "\t")) {
            result += line.substr(1);
        } else {
            result += line;
        }
    }
    return symbolTable->encode(result);
}

souffle::RamDomain type_to_string(souffle::SymbolTable* symbolTable,
                                  souffle::RecordTable* recordTable, souffle::RamDomain arg) {
    const souffle::RamDomain* tuple = recordTable->unpack(arg, 3);
    const std::string& cl = symbolTable->decode(tuple[1]);
    std::vector<std::string> type_args;
    auto curr = tuple[2];
    while (curr != 0) {
        const auto* pair = recordTable->unpack(curr, 2);
        type_args.emplace_back(
            symbolTable->decode(type_to_string(symbolTable, recordTable, pair[0])));
        curr = pair[1];
    }
    std::string result = cl;
    if (!type_args.empty()) {
        result += "<";
        for (const auto& arg : type_args) {
            result += arg + ",";
        }
        result.pop_back();
        result += ">";
    }
    return symbolTable->encode(result);
}

souffle::RamDomain type_to_qualified_string(souffle::SymbolTable* symbolTable,
                                            souffle::RecordTable* recordTable,
                                            souffle::RamDomain arg) {
    const souffle::RamDomain* tuple = recordTable->unpack(arg, 3);
    const std::string& package = symbolTable->decode(tuple[0]);
    const std::string& cl = symbolTable->decode(tuple[1]);
    std::vector<std::string> type_args;
    auto curr = tuple[2];
    while (curr != 0) {
        const auto* pair = recordTable->unpack(curr, 2);
        type_args.emplace_back(
            symbolTable->decode(type_to_qualified_string(symbolTable, recordTable, pair[0])));
        curr = pair[1];
    }
    std::string result = package + "." + cl;
    if (!type_args.empty()) {
        result += "<";
        for (const auto& arg : type_args) {
            result += arg + ",";
        }
        result.pop_back();
        result += ">";
    }
    return symbolTable->encode(result);
}
}
