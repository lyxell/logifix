#pragma once

#include "sjp/sjp.hpp"

namespace repair {

    class program {
    private:
        sjp::parser parser;
    public:
        program();
        ~program();
        void add_file(const char* filename);
        void add_string(const char* filename, const char* content);
        void run();
        std::vector<std::tuple<std::string,int,int>>
        get_ast_nodes(const char* filename);
    };

}
