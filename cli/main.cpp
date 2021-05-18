#include <iostream>
#include "../logifix.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " Filename.java" << std::endl;
        return 1;
    }
    logifix::program program;
    program.add_file(argv[1]);
    program.run();
    std::cout << program.get_root() << std::endl;
    return 0;
}
