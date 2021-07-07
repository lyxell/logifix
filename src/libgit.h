#pragma once

#include <vector>
#include <string>

namespace libgit {

std::vector<std::string> create_patch(std::string filename, std::string before,
                                      std::string after);

} // namespace libgit
