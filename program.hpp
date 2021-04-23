#pragma once

#include <map>
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
        std::shared_ptr<sjp::tree_node>
        get_ast(const char* filename);
        std::vector<std::tuple<std::string,int,int>>
        get_ast_nodes(const char* filename);
        std::map<std::tuple<std::string,int,int>,
                           std::tuple<std::string,int,int>>
        get_repairable_nodes(const char* filename);
        std::vector<std::tuple<std::string,int,int,std::string>>
        get_pretty_print(const char* filename);
    };

}
