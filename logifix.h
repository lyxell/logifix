#pragma once

#include "sjp/sjp.h"
#include <map>
#include <optional>

namespace logifix {

class repair {
  private:
    std::unordered_map<std::string, std::string> source_code;
    std::unique_ptr<souffle::SouffleProgram> program;

  public:
    repair();
    void add_file(const char* filename);
    void add_string(const char* filename, const char* content);
    void run();
    std::vector<std::tuple<int, int, std::string, std::string>>
    get_possible_repairs(const char* filename);
    std::vector<std::tuple<std::string,std::string,int,int>>
    get_variables_in_scope(const char* filename);

};

}
