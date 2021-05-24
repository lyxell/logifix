#include <git2.h>
#include <iostream>
#include "../logifix.h"

std::string apply_rewrites(const std::string& input, std::vector<std::tuple<int,int,int,std::string,std::string>> rewrites) {
    /* expand rewrites that only does deletion to also remove whitespace at the beginning of the line */
    for (auto& [rule_number, start, end, replacement, mess] : rewrites) {
        if (replacement.empty()) {
            while (start > 1 && (input[start-1] == ' ' || input[start-1] == '\t')) {
                start--;
            }
            if (start > 2 && input[start-2] == '\r' && input[start-1] == '\n') start -= 2;
            else if (start > 1 && input[start-1]) start--;
        }
    }
    /* sort rewrites */
    std::sort(rewrites.begin(), rewrites.end(), [](auto x, auto y) {
        if (std::get<1>(x) == std::get<1>(y)) return std::get<2>(x) < std::get<2>(y);
        return std::get<1>(x) > std::get<1>(y);
    });
    size_t curr = 0;
    std::string result;
    while (curr < input.size()) {
        /* discard rewrites that happened before curr */
        while (rewrites.size() && std::get<1>(rewrites.back()) < curr) rewrites.pop_back();
        if (!rewrites.size()) {
            result += input.substr(curr);
            break;
        }
        auto [rule_number, start, end, replacement, mess] = rewrites.back();
        rewrites.pop_back();
        result += input.substr(curr, start - curr);
        result += replacement;
        curr = end;
    }
    return result;
}

int main(int argc, char** argv) {
    git_libgit2_init();
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " Filename.java rule_number" << std::endl;
        return 1;
    }
    int rule_number = std::stoi(argv[2]);
    logifix::program program;
    program.add_file(argv[1]);
    program.run();
    auto source_code = program.get_source_code(argv[1]);
    std::vector<std::tuple<int,int,int,std::string,std::string>> rewrites;
    // filter rewrites by their id
    for (auto r : program.get_possible_rewrites(argv[1])) {
        if (std::get<0>(r) == rule_number) rewrites.emplace_back(std::move(r));
    }
    std::cout << apply_rewrites(source_code, std::move(rewrites));
    return 0;

}
