#include "logifix.h"
#include "tty.h"
#include "config.h"
#include <regex>
#include <filesystem>
#include <future>
#include <stack>
#include <cctype>
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
    std::optional<std::string> ignore;
};

static const char* COLOR_BOLD  = "\033[1m";
static const char* COLOR_RESET = "\033[m";
static const char* COLOR_RED   = "\033[31m";
static const char* COLOR_GREEN = "\033[32m";
static const char* COLOR_CYAN  = "\033[36m";

struct printer_opts {
    bool color;
    bool print_file_header;
};

bool string_has_only_whitespace(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](char c) { return std::isspace(c) != 0; });
}

std::vector<std::string> line_split(std::string str) {
    std::vector<std::string> result;
    while (!str.empty()) {
        size_t i;
        for (i = 0; i < str.size() - 1; i++) {
            if (str[i] == '\n') break;
        }
        result.emplace_back(str.substr(0, i + 1));
        str = str.substr(i + 1);
    }
    return result;
}

std::string get_char() {
    switch (getchar()) {
    case 0x0d: return "return";
    case 0x1b:
        switch (getchar()) {
            case 0x5b:
                switch (getchar()) {
                    case 0x41: return "up";
                    case 0x42: return "down";
                    case 0x44: return "left";
                    case 0x43: return "right";
                    default: break;
                }
                break;
            default: break;
        }
        break;
    default: break;
    }
    return "unknown";
}

static int color_printer(const git_diff_delta* delta, const git_diff_hunk* hunk,
                         const git_diff_line* line, void* data) {


    auto* opts = (printer_opts*) data;

    /* Skip added empty lines since they will not be included in end result */
    auto* end = line->content;
    while (*end != '\0' && *end != '\n') end++;
    std::string content(line->content, end);
    if (line->origin == GIT_DIFF_LINE_ADDITION && string_has_only_whitespace(content)) {
        return 0;
    }

    if (!opts->print_file_header && line->origin == GIT_DIFF_LINE_FILE_HDR) {
        return 0;
    }

    if (!opts->print_file_header && line->origin == GIT_DIFF_LINE_HUNK_HDR) {
        return 0;
    }

    (void)delta;
    (void)hunk;

    if (opts->color) {
        switch (line->origin) {
            case GIT_DIFF_LINE_ADD_EOFNL:
            case GIT_DIFF_LINE_ADDITION:
                std::cout << COLOR_GREEN;
                break;
            case GIT_DIFF_LINE_DEL_EOFNL:
            case GIT_DIFF_LINE_DELETION:
                std::cout << COLOR_RED;
                break;
            case GIT_DIFF_LINE_FILE_HDR:
                std::cout << COLOR_BOLD;
                break;
            case GIT_DIFF_LINE_HUNK_HDR:
                std::cout << COLOR_CYAN;
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
        // render tab as four spaces
        if (line->content[i] == '\t') {
            std::cout << "    ";
        } else {
            std::cout << line->content[i];
        }
        i++;
    }

    std::cout << std::endl;

    if (opts->color) {
        std::cout << COLOR_RESET;
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
    std::cout << std::endl;
    std::cout << COLOR_BOLD << COLOR_GREEN << "?" << COLOR_RESET;
    std::cout << COLOR_BOLD << " Do you want to use this rewrite?" << COLOR_RESET;
    std::cout << " [y,N] " << COLOR_RESET;
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
        .ignore         = {},
    };

    std::vector<std::tuple<std::string,std::function<void(std::string)>,std::string>> opts;

    auto print_usage_and_exit = [&opts](int exit_code) {
        std::cout << COLOR_BOLD << "USAGE" << COLOR_RESET << std::endl;
        std::cout << "  " << PROJECT_NAME << " [OPTIONS] PATH [PATH ...]" << std::endl;
        std::cout << std::endl;
        std::cout << COLOR_BOLD << "EXAMPLES" << COLOR_RESET << std::endl;
        std::cout << "  " << PROJECT_NAME << " --rules=1125,1155 src/main src/test" << std::endl;
        std::cout << std::endl;
        std::cout << COLOR_BOLD << "OPTIONS" << COLOR_RESET << std::endl;
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
            if (ss.peek() == ',') {
                ss.ignore();
            }
        }
    };

    opts = {
        {"--apply",           [&](std::string str) { options.apply = true; },        "Rewrite files on disk (default)"},
        {"--color",           [&](std::string str) { options.color = true; },        "Colorize diffs (default)"},
        {"--extension=<ext>", [&](std::string str) { options.extension = str; },     "Only consider files ending with extension (default=\".java\")"},
        {"--help",            [&](std::string str) { print_usage_and_exit(0); },     "Print this information and exit"},
        {"--interactive",     [&](std::string str) { options.interactive = true; },  "Prompt for confirmation before each rewrite (default)"},
        {"--ignore=<pattern>",[&](std::string str) { options.ignore = str; },        "Pattern for files to ignore"},
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
        std::cmatch m;
        auto argument = arguments.back();
        if (!fs::exists(argument)) {
            std::cout << "Error: Path '" << argument << "' does not exist" << std::endl;
            std::cout << std::endl;
            print_usage_and_exit(1);
        }
        if (fs::is_directory(argument)) {
            if (options.recurse) {
                for (const auto& entry : fs::recursive_directory_iterator(argument)) {
                    if (!entry.is_regular_file()) continue;
                    if (entry.path().extension() != ".java") continue;
                    std::string s = entry.path().lexically_normal();
                    if (options.ignore && s.find(*(options.ignore)) != std::string::npos) continue;
                    options.files.emplace(entry.path().lexically_normal());
                }
            } else {
                for (const auto& entry : fs::directory_iterator(argument)) {
                    if (!entry.is_regular_file()) continue;
                    if (entry.path().extension() != ".java") continue;
                    std::string s = entry.path().lexically_normal();
                    if (options.ignore && s.find(*(options.ignore)) != std::string::npos) continue;
                    options.files.emplace(entry.path().lexically_normal());
                }
            }
        } else {
            options.files.emplace(fs::path(std::move(argument)).lexically_normal());
        }
        arguments.pop_back();
    }

    return options;
}

size_t multi_choice(std::string question, std::vector<std::string> alternatives) {
    std::cout << COLOR_BOLD << COLOR_GREEN << "?" << COLOR_RESET;
    std::cout << COLOR_BOLD << " " << question << COLOR_RESET;
    std::cout << " [Use arrows to move] " << COLOR_RESET;
    size_t cursor = 0;
    size_t scroll = 0;
    size_t height = 10;
    bool found = false;
    while (true) {
        if (cursor < scroll) {
            scroll = cursor;
        } else if (cursor == scroll + height) {
            scroll = cursor - height + 1;
        }
        for (size_t i = scroll; i < std::min(alternatives.size(), scroll + height); i++) {
            std::cout << std::endl;
            if (cursor == i) {
                std::cout << "> ";
            } else {
                std::cout << "  ";
            }
            std::cout << alternatives[i] << "\x1b[K";
            std::cout << COLOR_RESET;
        }
        if (found) {
            std::cout << std::endl;
            std::cout << std::endl;
            return cursor;
        }
        for (size_t i = scroll; i < std::min(alternatives.size(), scroll + height); i++) {
            std::cout << "\x1b[A";
        }
        auto res = get_char();
        if (res == "up" && cursor > 0) {
            cursor--;
        }
        if (res == "down" && cursor + 1 < alternatives.size()) {
            cursor++;
        }
        if (res == "return") {
            found = true;
        }
    }
    return 0;
}

int main(int argc, char** argv) {

    options_t options = parse_options(argc, argv);

    stats s = {};

    git_libgit2_init();


    int curr_threads = 0;
    std::mutex io_mutex;
    std::mutex thread_mutex;
    std::vector<std::thread> thread_pool;
    std::map<std::string,std::vector<std::string>> rewrites;

    std::vector<std::string> file_stack(options.files.begin(), options.files.end());

    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++) {
        thread_pool.emplace_back(
            std::thread([&] {
                while (true) {
                    std::string file;
                    {
                        std::lock_guard<std::mutex> lock(thread_mutex);
                        if (file_stack.empty()) return;
                        file = file_stack.back();
                        file_stack.pop_back();
                    }
                    std::unique_ptr<logifix::program> program = std::make_unique<logifix::program>();
                    std::string input = read_file(file);
                    auto result = program->run(input, options.rules);
                    program = nullptr;
                    if (result.size() == 0) continue;
                    /*std::lock_guard<std::mutex> lock(io_mutex);
                    if (options.color) {
                        std::cerr << COLOR_BOLD;
                    }
                    std::cerr << "File " << file << std::endl;
                    if (options.color) {
                        std::cerr << COLOR_RESET;
                    }
                    std::vector<std::string> chosen_rewrites;
                    size_t curr = 1;
                    for (auto rewrite : result) {
                        if (options.color) {
                            std::cerr << COLOR_BOLD;
                        }
                        std::cerr << "Rewrite " << curr << "/" << result.size() << std::endl;
                        if (options.color) {
                            std::cerr << COLOR_RESET;
                        }
                        curr++;
                        if (!options.interactive || ask_user_about_rewrite(file, input, rewrite, options.color)) {
                            chosen_rewrites.emplace_back(rewrite);
                        }

                    }
                    auto diff = nway::diff(input, chosen_rewrites);
                    assert(!nway::has_conflict(diff));
                    auto output = nway::merge(diff);
                    */
                    for (auto output : result) {
                        if (output != input) {
                            /* post-processing, remove introduced empty lines */
                            /*
                            auto output_lines = line_split(std::move(output));
                            auto input_lines = line_split(std::move(input));
                            auto lcs = nway::longest_common_subsequence(output_lines, input_lines);
                            std::string processed;
                            for (size_t i = 0; i < output_lines.size(); i++) {
                                if (lcs.find(i) == lcs.end() && string_has_only_whitespace(output_lines[i])) {
                                    continue;
                                }
                                processed += output_lines[i];
                            }
                            rewrites[file].emplace_back(processed);
                            */
                            rewrites[file].emplace_back(output);
                        }
                    }
                }
            }));
    }

    for (auto& f : thread_pool) {
        f.join();
    }

    size_t rewrites_sum = 0;
    for (const auto& [file, rws] : rewrites) {
        rewrites_sum += rws.size();
    }

    tty_enable_cbreak_mode();

    if (options.interactive) {
        std::cout << std::endl;
        std::cout << COLOR_BOLD << "Found " << rewrites_sum << " rewrites across ";
        std::cout << rewrites.size() << " files " << COLOR_RESET << std::endl << std::endl;
        size_t selection;
        selection = multi_choice("What would you like to do?", {
            "Review rewrites by rule",
            "Review rewrites by file",
            "Exit without doing anything",
        });

        if (selection == 1) {
            std::vector<std::string> keys;
            std::vector<std::string> options;
            for (auto& [k, v] : rewrites) {
                keys.emplace_back(k);
                options.emplace_back(k + " (" + std::to_string(v.size()) + ")");
            }
            selection = multi_choice("Which file would you like to review?", options);
            tty_disable_cbreak_mode();
            auto file = keys[selection];
            size_t curr = 1;
            for (auto rewrite : rewrites[file]) {
                std::cout << std::endl;
                std::cout << "-----------------------------------------------------------" << std::endl;
                std::cout << std::endl << COLOR_BOLD << "Rewrite " << curr++ << "/" << rewrites[file].size() << " • " << file << COLOR_RESET << std::endl << std::endl;
                std::string input = read_file(file);
                ask_user_about_rewrite("", input, rewrite, true);

            }
        }
    }

    tty_disable_cbreak_mode();

    /*
    for (auto [filename, after] : patch) {
        if (options.apply) {
            std::ofstream f(filename);
            f << after;
            f.close();
        } else {
            std::string before = read_file(filename);
            print_patch(filename, before, after, {options.color, true});
        }
    }*/

    return 0;
}
