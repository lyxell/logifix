#include "logifix.h"
#include "config.h"
#include <filesystem>
#include <future>
#include <git2.h>
#include <iostream>
#include <set>
#include <tuple>
#include <mutex>
#include <nway.h>

namespace fs = std::filesystem;

static const char* TTY_HIDE_CURSOR = "\033[?25l";
static const char* TTY_SHOW_CURSOR = "\033[?25h";

struct options_t {
    bool apply;
    bool color;
    bool recurse;
    bool interactive;
    std::string extension;
    std::set<std::string> files;
    std::set<int> rules;
};

static const char* colors[] = {
    "\033[m",   /* reset */
    "\033[1m",  /* bold */
    "\033[31m", /* red */
    "\033[32m", /* green */
    "\033[36m"  /* cyan */
};

enum { COLOR_RESET, COLOR_BOLD, COLOR_RED, COLOR_GREEN, COLOR_CYAN };

struct printer_opts {
    bool color;
    bool print_file_header;
};

static int color_printer(const git_diff_delta* delta, const git_diff_hunk* hunk,
                         const git_diff_line* line, void* data) {

    printer_opts* opts = (printer_opts*) data;

    if (!opts->print_file_header && line->origin == GIT_DIFF_LINE_FILE_HDR) return 0;

    (void)delta;
    (void)hunk;

    if (opts->color) {
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

    std::cout << std::endl;

    if (opts->color) {
        std::cout << colors[COLOR_RESET];
    }

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

void print_patch(std::string filename, std::string before, std::string after, printer_opts opts) {
    git_patch* patch = nullptr;
    git_patch_from_buffers(&patch, before.c_str(), before.size(),
                           filename.c_str(), after.c_str(),
                           after.size(), filename.c_str(), nullptr);
    git_patch_print(patch, color_printer, &opts);
    git_patch_free(patch);
}

bool ask_user_about_rewrite(std::string filename, std::string before, std::string after, bool color) {
    print_patch(std::move(filename), std::move(before), std::move(after), {color, false});
    if (color) {
        std::cout << colors[COLOR_CYAN];
    }
    std::cout << "Apply these changes? [y,N]" << std::endl;
    if (color) {
        std::cout << colors[COLOR_RESET] ;
    }
    std::string line;
    std::getline(std::cin, line);
    return line.size() > 0 && line[0] == 'y' || line[0] == 'Y';
}

struct stats {
    std::map<std::string,size_t> files_changed;
};

void print_version_and_exit() {
    std::cout << PROJECT_VERSION << std::endl;
    std::exit(0);
}

options_t parse_options(int argc, char** argv) {
    options_t options = {
        .apply          = true,
        .color          = true,
        .recurse        = true,
        .interactive    = true,
        .extension      = ".java",
        .files          = {},
        .rules          = {},
    };

    std::vector<std::tuple<std::string,std::function<void(std::string)>,std::string>> opts;

    auto print_usage_and_exit = [&opts](int exit_code) {
        std::cout << "Usage: " << PROJECT_NAME << " [OPTIONS] PATH [PATH ...]" << std::endl;
        std::cout << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "  " << PROJECT_NAME << " --rules=1125,1155 src/main src/test" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        for (const auto [option, _, description] : opts) {
            std::cout << " " << std::setw(19) << std::left << option << description << std::endl;
        }
        std::cout << std::endl;
        std::cout << PROJECT_NAME << " was created by Anton Lyxell, more information and the" << std::endl;
        std::cout << "latest release can be found at " << PROJECT_URL << std::endl;
        std::exit(exit_code);
    };

    auto parse_rules = [&](std::string str) {
        std::stringstream ss(str);
        for (int i; ss >> i;) {
            options.rules.emplace(i);
            if (ss.peek() == ',') ss.ignore();
        }
    };

    opts = {
        {"--apply",           [&](std::string str) { options.apply = true; },        "Rewrite files on disk (default)"},
        {"--color",           [&](std::string str) { options.color = true; },        "Colorize diffs (default)"},
        {"--extension=<ext>", [&](std::string str) { options.extension = str; },     "Only consider files ending with extension (default=\".java\")"},
        {"--help",            [&](std::string str) { print_usage_and_exit(0); },     "Print this information and exit"},
        {"--interactive",     [&](std::string str) { options.interactive = true; },  "Prompt for confirmation before each rewrite (default)"},
        {"--no-apply",        [&](std::string str) { options.apply = false; },       "Do not rewrite files on disk, output patch to stdout instead"},
        {"--no-color",        [&](std::string str) { options.color = false; },       "Do not colorize diffs"},
        {"--no-interactive",  [&](std::string str) { options.interactive = false; }, "Do not prompt for confirmation, apply all rewrites"},
        {"--no-recurse",      [&](std::string str) { options.recurse = false; },     "Do not recursively search in subfolders"},
        {"--recurse",         [&](std::string str) { options.recurse = true; },      "Recursively search in subfolders (default)"},
        {"--rules=<rules>",   [&](std::string str) { parse_rules(str); },            "Comma-separated set of rules to find rewrites for"},
        {"--version",         [&](std::string str) { print_version_and_exit(); },    "Print version information and exit"},
    };

    std::vector<std::string> arguments;
    for (int i = 1; i < argc; i++) {
        arguments.emplace_back(argv[i]);
    }
    std::reverse(arguments.begin(), arguments.end());

    while (arguments.size()) {
        auto argument = arguments.back();
        if (argument.size() < 2 || argument.substr(0, 2) != "--") break;
        bool found = false;
        for (const auto [option, fn, description] : opts) {
            if (option.find("=") != std::string::npos) {
                auto opt_part = option.substr(0, option.find("=") + 1);
                if (argument.size() >= opt_part.size() && argument.substr(0, opt_part.size()) == opt_part) {
                    fn(argument.substr(opt_part.size()));
                    found = true;
                    break;
                }
            } else {
                if (argument.substr(0, option.size()) == option) {
                    fn("");
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            std::cout << "Error: Found invalid argument '" << argument << "'" << std::endl;
            std::cout << std::endl;
            print_usage_and_exit(1);
        }
        arguments.pop_back();
    }

    if (arguments.empty()) {
        std::cout << "Error: No path specified" << std::endl;
        std::cout << std::endl;
        print_usage_and_exit(1);
    }

    while (arguments.size()) {
        auto argument = arguments.back();
        if (!fs::exists(argument)) {
            std::cout << "Error: Path '" << argument << "' does not exist" << std::endl;
            std::cout << std::endl;
            print_usage_and_exit(1);
        }
        if (fs::is_directory(argument)) {
            if (options.recurse) {
                for (const auto& entry : fs::recursive_directory_iterator(argument)) {
                    if (entry.path().extension() == ".java") {
                        options.files.emplace(entry.path().lexically_normal());
                    }
                }
            } else {
                for (const auto& entry : fs::directory_iterator(argument)) {
                    if (entry.path().extension() == ".java") {
                        options.files.emplace(entry.path().lexically_normal());
                    }
                }
            }
        } else {
            options.files.emplace(fs::path(std::move(argument)).lexically_normal());
        }
        arguments.pop_back();
    }

    return options;
}

int main(int argc, char** argv) {

    options_t options = parse_options(argc, argv);

    stats s = {};

    git_libgit2_init();

    /* Perform analysis */
    int count = 0;
    bool found_first_rewrite = false;
    std::mutex io_mutex;
    std::vector<std::future<void>> futures;
    std::map<std::string,std::string> patch;

    for (const auto& file : options.files) {
        futures.emplace_back(
            std::async(std::launch::async, [&] {
                logifix::program program;
                std::string input = read_file(file);
                auto result = program.run(input, options.rules);
                program = {};
                if (result.size() == 0) return;
                std::lock_guard<std::mutex> lock(io_mutex);
                if (options.color) {
                    std::cerr << colors[COLOR_BOLD];
                }
                std::cerr << "File " << file << std::endl;
                if (options.color) {
                    std::cerr << colors[COLOR_RESET];
                }
                std::vector<std::string> chosen_rewrites;
                size_t curr = 1;
                for (auto rewrite : result) {
                    if (options.color) {
                        std::cerr << colors[COLOR_BOLD];
                    }
                    std::cerr << "Rewrite " << curr << "/" << result.size() << std::endl;
                    if (options.color) {
                        std::cerr << colors[COLOR_RESET];
                    }
                    curr++;
                    if (!options.interactive || ask_user_about_rewrite(file, input, rewrite, options.color)) {
                        chosen_rewrites.emplace_back(rewrite);
                    }

                }
                auto diff = nway::diff(input, chosen_rewrites);
                assert(!nway::has_conflicts(diff));
                auto output = nway::merge(diff);
                if (output != input) {
                    if (options.apply) {
                        std::ofstream f(file);
                        f << output;
                        f.close();
                    } else {
                        patch.emplace(file, output);
                    }
                }
            }));
    }

    for (auto& f : futures) {
        f.wait();
    }

    if (!options.apply) {
        for (auto [filename, after] : patch) {
            std::string before = read_file(filename);
            print_patch(filename, before, after, {options.color, true});
        }
    }

    return 0;
}
