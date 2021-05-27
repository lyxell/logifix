#include "logifix.h"
#include <cmath>
#include <filesystem>
#include <future>
#include <git2.h>
#include <iostream>
#include <mutex>
#include <unistd.h>

namespace fs = std::filesystem;

std::string output;

static const char* colors[] = {
    "\033[m",   /* reset */
    "\033[1m",  /* bold */
    "\033[31m", /* red */
    "\033[32m", /* green */
    "\033[36m"  /* cyan */
};

int diff_output(const git_diff_delta* d, const git_diff_hunk* h,
                const git_diff_line* l, void* p) {

    (void)d;
    (void)h;

    if (l->origin == GIT_DIFF_LINE_CONTEXT ||
        l->origin == GIT_DIFF_LINE_ADDITION ||
        l->origin == GIT_DIFF_LINE_DELETION)
        output += l->origin;

    output += l->content;

    return 0;
}

static int color_printer(const git_diff_delta* delta, const git_diff_hunk* hunk,
                         const git_diff_line* line, void* data) {
    int *last_color = (int*)data, color = 0;

    (void)delta;
    (void)hunk;

    if (*last_color >= 0) {
        switch (line->origin) {
        case GIT_DIFF_LINE_ADDITION:
            color = 3;
            break;
        case GIT_DIFF_LINE_DELETION:
            color = 2;
            break;
        case GIT_DIFF_LINE_ADD_EOFNL:
            color = 3;
            break;
        case GIT_DIFF_LINE_DEL_EOFNL:
            color = 2;
            break;
        case GIT_DIFF_LINE_FILE_HDR:
            color = 1;
            break;
        case GIT_DIFF_LINE_HUNK_HDR:
            color = 4;
            break;
        default:
            break;
        }

        if (color != *last_color) {
            if (*last_color == 1 || color == 1)
                output += colors[0];
            output += colors[color];
            *last_color = color;
        }
    }

    return diff_output(delta, hunk, line, nullptr);
}

std::string perform_rewrites(
    const std::string& input,
    std::vector<std::tuple<int, size_t, size_t, std::string, std::string>>
        rewrites) {
    /* expand rewrites that only does deletion to also remove whitespace at the
     * beginning of the line */
    for (auto& [rule_number, start, end, replacement, mess] : rewrites) {
        if (replacement.empty()) {
            while (start > 1 &&
                   (input[start - 1] == ' ' || input[start - 1] == '\t')) {
                start--;
            }
            if (start > 2 && input[start - 2] == '\r' &&
                input[start - 1] == '\n')
                start -= 2;
            else if (start > 1 && input[start - 1])
                start--;
        }
    }
    /* sort rewrites */
    std::sort(rewrites.begin(), rewrites.end(), [](auto x, auto y) {
        if (std::get<1>(x) == std::get<1>(y))
            return std::get<2>(x) < std::get<2>(y);
        return std::get<1>(x) > std::get<1>(y);
    });
    size_t curr = 0;
    std::string result;
    while (curr < input.size()) {
        /* discard rewrites that happened before curr */
        while (rewrites.size() && std::get<1>(rewrites.back()) < curr)
            rewrites.pop_back();
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

struct options {
    bool apply;
    int rule_number;
    std::set<std::string> files;
};

int main(int argc, char** argv) {
    options opt = {.apply = false, .rule_number = -1, .files = {}};
    for (int i = 1; i < argc; i++) {
        std::string s(argv[i]);
        if (s == "--apply") {
            opt.apply = true;
        } else if (s.substr(0, 8) == "--rules=") {
            opt.rule_number = std::stoi(s.substr(8));
        } else {
            if (fs::is_directory(s)) {
                for (const auto& entry : fs::recursive_directory_iterator(s)) {
                    if (entry.path().extension() == ".java") {
                        opt.files.emplace(entry.path().lexically_normal());
                    }
                }
            } else {
                opt.files.emplace(fs::path(std::move(s)).lexically_normal());
            }
        }
    }

    if (opt.rule_number == -1) {
        std::cerr << "No rule specified" << std::endl;
        return 1;
    }

    if (opt.files.empty()) {
        std::cerr << "No files specified" << std::endl;
        return 1;
    }

    git_libgit2_init();

    std::cerr << "\033[?25l" << std::flush;

    /* perform analysis */
    int count = 0;
    std::mutex print_mutex;
    std::vector<std::future<void>> futures;
    for (const auto& file : opt.files) {
        futures.emplace_back(
            std::async(std::launch::async, [&opt, &count, &file, &print_mutex] {
                logifix::program program;
                program.add_file(file.c_str());
                program.run();

                /* filter rewrites by their id */
                std::vector<
                    std::tuple<int, size_t, size_t, std::string, std::string>>
                    rewrites;
                for (auto r : program.get_possible_rewrites(file.c_str())) {
                    if (std::get<0>(r) == opt.rule_number)
                        rewrites.emplace_back(std::move(r));
                }

                auto input = program.get_source_code(file.c_str());
                auto output = perform_rewrites(input, std::move(rewrites));

                if (opt.apply) {
                    std::ofstream f(file);
                    f << output;
                    f.close();
                } else {
                    /* create diff */
                    git_diff* diff;
                    git_buf buf = {};
                    git_patch* patch = NULL;
                    git_patch_from_buffers(&patch, input.c_str(), input.size(),
                                           file.c_str(), output.c_str(),
                                           output.size(), file.c_str(), NULL),
                        git_patch_to_buf(&buf, patch);
                    git_diff_from_buffer(&diff, buf.ptr, buf.size);
                    git_patch_free(patch);
                    git_buf_dispose(&buf);
                    std::lock_guard<std::mutex> lock(print_mutex);
                    if (git_diff_num_deltas(diff) > 0) {
                        int color = isatty(fileno(stdout)) ? 0 : -1;
                        git_diff_print(diff, GIT_DIFF_FORMAT_PATCH,
                                       color_printer, &color);
                    }
                    git_diff_free(diff);
                }
                {
                    count++;
                    std::lock_guard<std::mutex> lock(print_mutex);
                    size_t num_slots = 60;
                    size_t progress = std::floor(
                        double(count) / double(opt.files.size()) * num_slots);
                    std::cerr << "[";
                    for (size_t i = 0; i < progress; i++) {
                        std::cerr << "#";
                    }
                    for (size_t i = 0; i < num_slots - progress; i++) {
                        std::cerr << ".";
                    }
                    std::cerr << "] " << count << "/" << opt.files.size()
                              << " files analyzed\r" << std::flush;
                }
            }));
    }

    for (auto& f : futures)
        f.wait();

    std::cerr << "\033[?25h" << std::endl;

    if (!opt.apply) {
        std::cout << output << std::endl;
    }

    return 0;
}
