#include <iostream>
#include "../logifix.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " Filename.java rule_number" << std::endl;
        return 1;
    }
    int rule_number = std::stoi(argv[2]);
    logifix::program program;
    program.add_file(argv[1]);
    program.run();
    std::vector<std::tuple<int,int,int,std::string,std::string>> rewrites;
    for (auto r : program.get_possible_rewrites(argv[1])) {
        if (std::get<0>(r) == rule_number) rewrites.emplace_back(std::move(r));
    }
    std::sort(rewrites.begin(), rewrites.end(), [](auto x, auto y) {
        if (std::get<1>(x) == std::get<1>(y)) return std::get<2>(x) < std::get<2>(y);
        return std::get<1>(x) > std::get<1>(y);
    });
    auto source_code = program.get_source_code(argv[1]);
    size_t curr = 0;
    std::string result;
    while (curr < source_code.size()) {
        // discard rewrites that happened before curr
        while (rewrites.size() && std::get<1>(rewrites.back()) < curr) rewrites.pop_back();
        if (!rewrites.size()) {
            result += source_code.substr(curr);
            break;
        }
        auto [rule_number, start, end, repl, mess] = rewrites.back();
        rewrites.pop_back();
        result += source_code.substr(curr, start - curr);
        result += repl;
        curr = end;
    }

    // if we introduced empty lines, remove them
    // (note that this will break as soon as we introduce rewrites that changes the line count)
    std::istringstream prev(source_code);
    std::istringstream next(result);

    std::string prev_line;
    std::string next_line;

    auto blank = [](const std::string& str) {
        return str.find_first_not_of("\t\n\r ") == std::string::npos;
    };
    bool output = false;
    std::string final_str;
    while (true) {
        if (!std::getline(next, next_line)) break;
        std::getline(prev, prev_line);
        if (blank(next_line) && !blank(prev_line)) continue;
        final_str += next_line + "\n";
    }
    // conform to files without trailing newline
    if (source_code[source_code.size()-1] != '\n') final_str.pop_back();
    std::cout << final_str;
    return 0;
}
