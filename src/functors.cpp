#include <souffle/SouffleInterface.h>
#include "utils.h"

extern "C" { 
 souffle::RamDomain decrease_indentation(souffle::SymbolTable* symbolTable, souffle::RecordTable* recordTable,
        souffle::RamDomain arg) {
    const std::string& str = symbolTable->decode(arg);
    auto lines = utils::line_split(str);
    std::string result;
    for (const auto& line : lines) {
        if (utils::starts_with(line, "    ")) {
            result += line.substr(4);
        } else {
            result += line;
        }
    }
    return symbolTable->encode(result);
 }
}
