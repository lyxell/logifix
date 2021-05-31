#include "logifix.h"
#include "config.h"
#include <filesystem>
#include <future>
#include <git2.h>
#include <iostream>
#include <mutex>

namespace fs = std::filesystem;

static const char* TTY_HIDE_CURSOR = "\033[?25l";
static const char* TTY_SHOW_CURSOR = "\033[?25h";

static const char* colors[] = {
    "\033[m",   /* reset */
    "\033[1m",  /* bold */
    "\033[31m", /* red */
    "\033[32m", /* green */
    "\033[36m"  /* cyan */
};

enum { COLOR_RESET, COLOR_BOLD, COLOR_RED, COLOR_GREEN, COLOR_CYAN };

static int color_printer(const git_diff_delta* delta, const git_diff_hunk* hunk,
                         const git_diff_line* line, void* data) {

    (void)data;
    (void)delta;
    (void)hunk;

    switch (line->origin) {
        case GIT_DIFF_LINE_ADD_EOFNL:
        case GIT_DIFF_LINE_ADDITION:
            std::cout << colors[COLOR_GREEN];
            break;
        case GIT_DIFF_LINE_DEL_EOFNL:
        case GIT_DIFF_LINE_DELETION:
            std::cout << colors[COLOR_RED];
            break;
        case GIT_DIFF_LINE_FILE_HDR:
            std::cout << colors[COLOR_BOLD];
            break;
        case GIT_DIFF_LINE_HUNK_HDR:
            std::cout << colors[COLOR_CYAN];
            break;
        default:
            break;
    }

    if (line->origin == GIT_DIFF_LINE_CONTEXT ||
        line->origin == GIT_DIFF_LINE_ADDITION ||
        line->origin == GIT_DIFF_LINE_DELETION) {
        std::cout << line->origin;
    }

    size_t i = 0;

    while (true) {
        if (line->content[i] == '\0' || line->content[i] == '\n' || line->content[i] == '\r') break;
        std::cout << line->content[i++];
    }

    std::cout << colors[COLOR_RESET] << std::endl;

    return 0;
}

std::string read_file(std::string_view path) {
    constexpr auto read_size = std::size_t{4096};
    auto stream = std::ifstream {path.data()};
    stream.exceptions(std::ios_base::badbit);

    auto out = std::string{};
    auto buf = std::string(read_size, '\0');
    while (stream.read(& buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}

bool ask_user_about_rewrite(const std::string& input, const std::string& replacement, size_t start, size_t end, const std::string& file, std::mutex& io_mutex) {
    std::string output = input.substr(0, start) + replacement + input.substr(end);
    git_patch* patch = NULL;
    git_patch_from_buffers(&patch, input.c_str(), input.size(),
                           file.c_str(), output.c_str(),
                           output.size(), file.c_str(), NULL);
    git_patch_print(patch, color_printer, NULL);
    git_patch_free(patch);
    std::cout << colors[COLOR_CYAN] << "Apply these changes? [y,N]"
              << colors[COLOR_RESET] << std::endl;
    std::string line;
    std::getline(std::cin, line);
    return line.size() > 0 && line[0] == 'y' || line[0] == 'Y';
}

struct options {
    bool recurse;
    bool apply;
    int rule_number;
    std::set<std::string> files;
};

struct stats {
    std::map<std::string,size_t> files_changed;
};

void print_usage() {
    std::cout << "Usage: " << PROJECT_NAME << " [OPTIONS] [PATH ...]" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << PROJECT_NAME << " --auto --rules=1125,1155 src/main" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --apply           Rewrite files on disk (default)" << std::endl;
    std::cout << "  --color           Colorize diffs when asking for confirmation (default)" << std::endl;
    std::cout << "  --interactive     Prompt user to confirm each rewrite (default)" << std::endl;
    std::cout << "  --no-apply        Do not rewrite files on disk, output patch to stdout instead" << std::endl;
    std::cout << "  --no-color        Do not colorize diffs when asking for confirmation " << std::endl;
    std::cout << "  --no-interactive  Do not prompt user for confirmation, apply all rewrites" << std::endl;
    std::cout << "  --no-recurse      Do not recursively search for files" << std::endl;
    std::cout << "  --recurse         Recursively search for files if PATH is folder (default)" << std::endl;
    std::cout << "  --rules=<rules>   Comma-separated set of rules to find patches for" << std::endl;
    std::cout << "  --suffix=<suffix> Only consider files ending with suffix (default=\".java\")" << std::endl;
    std::cout << "  --version         Print version information and exit" << std::endl;

}

int main(int argc, char** argv) {

    options opt = {
        .recurse = true,
        .apply = false,
        .rule_number = -1,
        .files = {}
    };
    stats s = {};

    for (int i = 1; i < argc; i++) {
        std::string s(argv[i]);
        if (s == "--no-recurse") {
            opt.recurse = false;
        } else if (s == "--apply") {
            opt.apply = true;
        } else if (s.substr(0, 8) == "--rules=") {
            opt.rule_number = std::stoi(s.substr(8));
        } else {
            if (fs::is_directory(s)) {
                if (opt.recurse) {
                    for (const auto& entry : fs::recursive_directory_iterator(s)) {
                        if (entry.path().extension() == ".java") {
                            opt.files.emplace(entry.path().lexically_normal());
                        }
                    }
                } else {
                    for (const auto& entry : fs::directory_iterator(s)) {
                        if (entry.path().extension() == ".java") {
                            opt.files.emplace(entry.path().lexically_normal());
                        }
                    }
                }
            } else {
                opt.files.emplace(fs::path(std::move(s)).lexically_normal());
            }
        }
    }

    if (opt.rule_number == -1) {
        print_usage();
        return 1;
    }

    if (opt.files.empty()) {
        print_usage();
        return 1;
    }

    git_libgit2_init();

    /* Perform analysis */
    int count = 0;
    bool found_first_rewrite = false;
    std::mutex io_mutex;
    std::vector<std::future<void>> futures;

    for (const auto& file : opt.files) {
        futures.emplace_back(
            std::async(std::launch::async, [&opt, &count, &file, &io_mutex,
                                            &found_first_rewrite, &s] {
                logifix::program program;
                std::string input = read_file(file);
                program.add_string(file.c_str(), input.c_str());
                program.run();

                /* Filter rewrites by their id */
                std::vector<
                    std::tuple<int, size_t, size_t, std::string>>
                    rewrites;
                for (auto r : program.get_possible_rewrites(file.c_str())) {
                    if (std::get<0>(r) == opt.rule_number)
                        rewrites.emplace_back(std::move(r));
                }

                /**
                 * Expand rewrites that only does deletion to also remove
                 * whitespace at the beginning of the line as well as the
                 * trailing newline on the previous line
                 */
                for (auto& [rule_number, start, end, replacement] :
                     rewrites) {
                    if (replacement.empty()) {
                        /* expand start to include whitespace */
                        while (start > 1 && input[start - 1] != '\n' &&
                               std::isspace(input[start - 1])) {
                            start--;
                        }
                        /* expand start to include trailing newline */
                        if (start > 2 && input.substr(start - 2, 2) == "\r\n") {
                            start -= 2;
                        } else if (start > 1 &&
                                   input.substr(start - 1, 1) == "\n") {
                            start--;
                        }
                    }
                }

                /* Sort rewrites */
                std::sort(rewrites.begin(), rewrites.end(), [](auto x, auto y) {
                    if (std::get<1>(x) == std::get<1>(y))
                        return std::get<2>(x) < std::get<2>(y);
                    return std::get<1>(x) > std::get<1>(y);
                });

                /* The resulting file */
                size_t curr_pos = 0;
                size_t num_changes = 0;
                std::string result;

                std::lock_guard<std::mutex> lock(io_mutex);

                while (curr_pos < input.size()) {

                    /* discard rewrites that we can't apply because of conflicts */
                    while (rewrites.size() && std::get<1>(rewrites.back()) < curr_pos) {
                        rewrites.pop_back();
                    }

                    if (!rewrites.size()) {
                        result += input.substr(curr_pos);
                        break;
                    }

                    auto [_, start, end, replacement] = rewrites.back();

                    if (opt.apply || ask_user_about_rewrite(input, replacement, start, end, file, io_mutex)) {
                        num_changes++;
                        result += input.substr(curr_pos, start - curr_pos);
                        result += replacement;
                        curr_pos = end;
                    }

                    rewrites.pop_back();

                }

                if (result != input) {
                    std::ofstream f(file);
                    f << result;
                    f.close();
                }

                if (num_changes > 0) {
                    s.files_changed.emplace(file, num_changes);
                }

                if (!found_first_rewrite) {
                    count++;
                    if (file.size() > 50) {
                        std::cerr << "analyzing " << file.substr(0, 50) << " ...\r" << std::flush;
                    } else {
                        std::cerr << "analyzing " << file << "\r" << std::flush;
                    }
                }
            }));
    }

    for (auto& f : futures) {
        f.wait();
    }

    size_t sum_rewrites = 0;
    for (const auto& [file, changes] : s.files_changed) {
        sum_rewrites += changes;
    }

    std::cerr << opt.files.size() << " files analyzed" << std::endl;
    std::cerr << sum_rewrites << " rewrites applied across ";
    std::cerr << s.files_changed.size() << " files" << std::endl;
    for (const auto& [file, changes] : s.files_changed) {
        std::cerr << '\t' << changes << " changes in " << file << std::endl;
    }
    std::cerr << std::endl;

    return 0;
}
