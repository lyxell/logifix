#include <git2.h>
#include <vector>
#include "libgit.h"

namespace {

struct printer_data {
    std::vector<std::string> lines;
};

int line_printer(const git_diff_delta* delta, const git_diff_hunk* hunk,
                 const git_diff_line* line, void* p) {
    (void)delta;
    (void)hunk;
    auto* data = (printer_data*)p;
    auto& result = data->lines.emplace_back();
    if (line->origin == GIT_DIFF_LINE_CONTEXT ||
        line->origin == GIT_DIFF_LINE_ADDITION ||
        line->origin == GIT_DIFF_LINE_DELETION) {
        result += line->origin;
    }
    const char* ptr = line->content;
    while (true) {
        if (*ptr == '\0' || *ptr == '\n' || *ptr == '\r') {
            break;
        }
        result += *ptr;
        ptr++;
    }
    return 0;
}

} // namespace

namespace libgit {

std::vector<std::string> create_patch(std::string filename, std::string before,
                                      std::string after) {
    git_libgit2_init();
    printer_data data;
    git_patch* patch = nullptr;
    git_patch_from_buffers(&patch, before.c_str(), before.size(),
                           filename.c_str(), after.c_str(), after.size(),
                           filename.c_str(), nullptr);
    git_patch_print(patch, line_printer, &data);
    git_patch_free(patch);
    return data.lines;
}

} // namespace libgit
