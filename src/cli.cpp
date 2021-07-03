#include "logifix.h"
#include "tty.h"
#include "config.h"
#include <regex>
#include <filesystem>
#include <future>
#include <cstdio>
#include <stack>
#include <cctype>
#include <git2.h>
#include <iostream>
#include <set>
#include <tuple>
#include <mutex>
#include <nway.h>
#include <fmt/core.h>
#include <fmt/color.h>

using namespace std::string_literals;
namespace fs = std::filesystem;

using rewrite_collection = std::vector<std::tuple<std::string, std::string, std::string, bool>>;

extern std::vector<std::tuple<std::string, std::string, std::string>> rule_data;

struct options_t {
    bool accept_all;
    bool in_place;
    bool patch;
    std::set<std::string> files;
    std::set<std::string> accepted;
};

struct printer_opts {
    bool color;
    bool print_file_header;
    FILE* fp;
};

enum class key {
    down,
    left,
    ret,
    right,
    up,
    unknown
};

static const char* COLOR_BOLD       = "\033[1m";
static const char* COLOR_CYAN       = "\033[36m";
static const char* COLOR_GREEN      = "\033[32m";
static const char* COLOR_RED        = "\033[31m";
static const char* COLOR_RESET      = "\033[m";
static const char* TTY_CLEAR_TO_EOL = "\033[K";
static const char* TTY_CURSOR_UP    = "\033[A";
static const char* TTY_HIDE_CURSOR  = "\033[?25l";
static const char* TTY_SHOW_CURSOR  = "\033[?25h";

static std::string read_file(std::string_view path) {
    constexpr auto read_size = std::size_t {4096};
    auto stream = std::ifstream {path.data()};
    stream.exceptions(std::ios_base::badbit);
    auto out = std::string {};
    auto buf = std::string(read_size, '\0');
    while (stream.read(& buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}

static key get_keypress() {
    switch (getchar()) {
    case 0x0d: return key::ret;
    case  'k': return key::up;
    case  'j': return key::down;
    case  'h': return key::left;
    case  'l': return key::right;
    case 0x1b:
        switch (getchar()) {
            case 0x5b:
                switch (getchar()) {
                    case 0x41: return key::up;
                    case 0x42: return key::down;
                    case 0x44: return key::left;
                    case 0x43: return key::right;
                    default: break;
                }
                break;
            default: break;
        }
        break;
    default: break;
    }
    return key::unknown;
}

static int color_printer(const git_diff_delta* delta, const git_diff_hunk* hunk,
                         const git_diff_line* line, void* data) {

    (void)delta;
    (void)hunk;

    auto* opts = (printer_opts*) data;

    if (!opts->print_file_header && (line->origin == GIT_DIFF_LINE_FILE_HDR || line->origin == GIT_DIFF_LINE_HUNK_HDR)) {
        return 0;
    }

    if (opts->color) {
        switch (line->origin) {
            case GIT_DIFF_LINE_ADD_EOFNL:
            case GIT_DIFF_LINE_ADDITION:
                fprintf(opts->fp, COLOR_GREEN);
                break;
            case GIT_DIFF_LINE_DEL_EOFNL:
            case GIT_DIFF_LINE_DELETION:
                fprintf(opts->fp, COLOR_RED);
                break;
            case GIT_DIFF_LINE_FILE_HDR:
                fprintf(opts->fp, COLOR_BOLD);
                break;
            case GIT_DIFF_LINE_HUNK_HDR:
                fprintf(opts->fp, COLOR_CYAN);
                break;
            default:
                break;
        }
    }

    if (line->origin == GIT_DIFF_LINE_CONTEXT ||
        line->origin == GIT_DIFF_LINE_ADDITION ||
        line->origin == GIT_DIFF_LINE_DELETION) {
        fprintf(opts->fp, "%c", line->origin);
    }

    size_t i = 0;

    while (true) {
        if (line->content[i] == '\0' || line->content[i] == '\n' || line->content[i] == '\r') break;
        // render tab as four spaces
        if (line->content[i] == '\t') {
            fprintf(opts->fp, "    ");
        } else {
            fprintf(opts->fp, "%c", line->content[i]);
        }
        i++;
    }

    fprintf(opts->fp, "\n");

    if (opts->color) {
        fprintf(opts->fp, COLOR_RESET);
    }

    return 0;
}

static void print_patch(std::string filename, std::string before, std::string after, printer_opts opts) {
    git_patch* patch = nullptr;
    git_patch_from_buffers(&patch, before.c_str(), before.size(),
                           filename.c_str(), after.c_str(),
                           after.size(), filename.c_str(), nullptr);
    git_patch_print(patch, color_printer, &opts);
    git_patch_free(patch);
}

static options_t parse_options(int argc, char** argv) {

    options_t options = {
        .accept_all     = false,
        .in_place       = false,
        .patch          = false,
        .files          = {},
        .accepted       = {},
    };

    std::vector<std::tuple<std::string,std::function<void(std::string)>,std::string>> opts;

    auto print_version_and_exit = []() {
        std::cout << PROJECT_VERSION << std::endl;
        std::exit(0);
    };

    auto print_usage = []() {
        fmt::print(fmt::emphasis::bold, "USAGE\n");
        fmt::print("  {} [flags] path [path ...]\n\n", PROJECT_NAME);
    };

    auto print_flags = [&opts]() {
        fmt::print(fmt::emphasis::bold, "FLAGS\n");
        for (const auto& [option, _, description] : opts) {
            std::cout << " " << std::setw(19) << std::left << option << description << std::endl;
        }
        std::cout << std::endl;
    };

    auto print_examples = []() {
        fmt::print(fmt::emphasis::bold, "EXAMPLES\n\n");
        fmt::print("  {} src/main src/test\n\n", PROJECT_NAME);
        fmt::print("  {} src/main --in-place --accept=S1125,S1155 Test.java\n\n", PROJECT_NAME);
    };

    auto parse_accepted = [&](std::string str) {
        std::stringstream ss(str);
        while (ss.good()) {
            std::string substr;
            std::getline(ss, substr, ',');
            options.accepted.emplace(substr);
        }
    };

    opts = {
        {"--accept-all",      [&](std::string str) { options.accept_all = true; },   "Accept all rewrites without asking"},
        {"--accept=<rules>",  [&](std::string str) { parse_accepted(str);  },        "Comma-separated list of rules to accept"},
        {"--in-place",        [&](std::string str) { options.in_place = true; },        "Disable interaction, rewrite files on disk"},
        {"--patch",           [&](std::string str) { options.patch = true; },        "Disable interaction, output a patch to stdout"},
        {"--help",            [&](std::string str) { print_usage(); print_flags(); print_examples(); std::exit(0); },     "Print this information and exit"},
        {"--version",         [&](std::string str) { print_version_and_exit(); },    "Print version information and exit"},
    };

    std::vector<std::string> arguments;
    for (auto i = 1; i < argc; i++) {
        arguments.emplace_back(argv[i]);
    }
    std::reverse(arguments.begin(), arguments.end());

    while (arguments.size()) {
        auto argument = arguments.back();
        if (argument.size() < 2 || argument.substr(0, 2) != "--") break;
        auto found = false;
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
            fmt::print("Error: Found invalid flag '{}'\n\n", argument);
            print_usage();
            print_flags();
            std::exit(1);
        }
        arguments.pop_back();
    }

    if (arguments.empty()) {
        print_usage();
        print_flags();
        print_examples();
        std::exit(0);
    }

    while (arguments.size()) {
        std::cmatch m;
        auto argument = arguments.back();
        if (!fs::exists(argument)) {
            fmt::print("Error: Path '{}' does not exist\n\n", argument);
            print_usage();
            std::exit(1);
        }
        if (fs::is_directory(argument)) {
            for (const auto& entry : fs::recursive_directory_iterator(argument)) {
                if (!entry.is_regular_file()) continue;
                if (entry.path().extension() != ".java") continue;
                std::string s = entry.path().lexically_normal();
                options.files.emplace(entry.path().lexically_normal());
            }
        } else {
            options.files.emplace(fs::path(std::move(argument)).lexically_normal());
        }
        arguments.pop_back();
    }

    return options;
}

static int multi_choice(std::string question, std::vector<std::string> alternatives, bool exit_on_left = false) {
    tty_enable_cbreak_mode();
    std::cout << TTY_HIDE_CURSOR;
    fmt::print(fg(fmt::terminal_color::green) | fmt::emphasis::bold, "?");
    fmt::print(fmt::emphasis::bold, " {} ", question);
    if (exit_on_left) {
        fmt::print("[Use arrows to move, left to finish]");
    } else {
        fmt::print("[Use arrows to move]");
    }
    int cursor = 0;
    size_t scroll = 0;
    size_t height = 15;
    bool found = false;
    while (true) {
        if (cursor != -1 && cursor < scroll) {
            scroll = cursor;
        } else if (cursor != -1 && cursor == scroll + height) {
            scroll = cursor - height + 1;
        }
        for (auto i = scroll; i < std::min(alternatives.size(), scroll + height); i++) {
            if (cursor == i) {
                fmt::print("\n> {}", alternatives[i]);
            } else {
                fmt::print("\n  {}", alternatives[i]);
            }
            std::cout << TTY_CLEAR_TO_EOL;
        }
        if (found) {
            std::cout << std::endl;
            std::cout << std::endl;
            std::cout << TTY_SHOW_CURSOR;
            tty_disable_cbreak_mode();
            return cursor;
        }
        for (auto i = scroll; i < std::min(alternatives.size(), scroll + height); i++) {
            std::cout << TTY_CURSOR_UP;
        }
        auto key_press = get_keypress();
        if (key_press == key::left && exit_on_left) {
            cursor = -1;
            found = true;
        } else if (key_press == key::up && cursor > 0) {
            cursor--;
        } else if (key_press == key::down && cursor + 1 < alternatives.size()) {
            cursor++;
        } else if (key_press == key::ret || key_press == key::right) {
            found = true;
        }
    }
    return 0;
}

static std::string post_process(std::string before, std::string after) {
    auto find_first_non_space = [](const std::string& str) {
        return std::find_if(str.begin(), str.end(), [](char c) { return std::isspace(c) == 0; });
    };
    auto line_split = [](std::string str) -> std::vector<std::string> {
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
    };
    auto bracket_balance = [](const std::string& str) {
        int result = 0;
        for (char c : str) {
            if (c == '(' || c == '{') result++;
            else if (c == ')' || c == '}') result--;
        }
        return result;
    };
    auto detect_indentation = [line_split, find_first_non_space](std::string str) {
        auto lines = line_split(str);
        std::vector<std::string> indentations;
        for (auto line : lines) {
            auto first_non_space_at = find_first_non_space(line);
            if (first_non_space_at == line.end() || first_non_space_at == line.begin()) continue;
            auto indent = line.substr(0, first_non_space_at - line.begin());
            if (std::any_of(indent.begin(), indent.end(), [indent](char c) { return c != indent[0]; })) {
                continue;
            }
            indentations.push_back(indent);
        }
        std::map<std::string, size_t> candidates;
        for (size_t i = 1; i < indentations.size(); i++) {
            auto a = indentations[i-1];
            auto b = indentations[i];
            if (a == b) continue;
            if (a > b) std::swap(a, b);
            if (b.find(a) != 0) continue;
            candidates[b.substr(a.size())]++;
        }
        std::string best_candidate = "    ";
        size_t best_result = 0;
        for (auto [k, v] : candidates) {
            if (v > best_result) {
                best_result = v;
                best_candidate = k;
            }
        }
        return best_candidate;
    };

    auto indentation = detect_indentation(before);

    auto string_has_only_whitespace = [](const auto& str) {
        return std::all_of(str.begin(), str.end(), [](char c) { return std::isspace(c) != 0; });
    };

    auto after_lines = line_split(std::move(after));
    auto before_lines = line_split(std::move(before));
    auto lcs = nway::longest_common_subsequence(after_lines, before_lines);
    std::string processed;
    for (size_t i = 0; i < after_lines.size(); i++) {
        if (lcs.find(i) == lcs.end()) {
            if (string_has_only_whitespace(after_lines[i])) continue;
            // auto-indent
            if (i > 0 && find_first_non_space(after_lines[i]) == after_lines[i].begin()) {
                auto prev_line = after_lines[i-1];
                // get indent from previous line
                auto new_indent = prev_line.substr(0, find_first_non_space(prev_line) - prev_line.begin());
                // increase indent if previous line has an open bracket
                if (bracket_balance(prev_line) > 0) {
                    new_indent += indentation;
                }
                // decrease indent if we have closing bracket
                if (bracket_balance(after_lines[i]) < 0) {
                    new_indent = new_indent.substr(std::min(indentation.size(), new_indent.size()));
                }
                after_lines[i] = new_indent + after_lines[i];
            }
        }
        processed += after_lines[i];
    }
    return processed;
}

std::map<std::string, std::string> get_results(const rewrite_collection& rewrites) {
    std::map<std::string, std::string> results;
    std::map<std::string, std::vector<std::string>> rewrites_per_file;
    for (auto& [filename, rule, after, is_accepted] : rewrites) {
        if (is_accepted) {
            rewrites_per_file[filename].emplace_back(after);
        }
    }
    for (auto [filename, rewrites] : rewrites_per_file) {
        std::string before = read_file(filename);
        auto diff = nway::diff(before, rewrites);
        assert(!nway::has_conflict(diff));
        results[filename] = post_process(before, nway::merge(diff));
    }
    return results;
}

int main(int argc, char** argv) {

    options_t options = parse_options(argc, argv);

    git_libgit2_init();

    std::mutex thread_mutex;
    std::vector<std::thread> thread_pool;
    std::vector<std::tuple<std::string,std::string,std::string, bool>> rewrites;

    std::vector<std::string> file_stack(options.files.begin(), options.files.end());

#ifdef PERF
    std::vector<std::pair<double, std::string>> file_time;
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
#endif

    std::cout << TTY_HIDE_CURSOR;

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
                        std::cerr << "\rAnalyzed " << options.files.size() - file_stack.size() << "/" <<  options.files.size() << " files";
                    }
#ifdef PERF
                    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
#endif
                    std::unique_ptr<logifix::program> program = std::make_unique<logifix::program>();
                    std::string input = read_file(file);
                    auto result = program->run(input, {});
                    program = nullptr;
#ifdef PERF
                    std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
                    auto diff = duration_cast<milliseconds>(end - start).count();
#endif
                    std::lock_guard<std::mutex> lock(thread_mutex);
#ifdef PERF
                    file_time.emplace_back(double(diff) / 1000.0f, file);
#endif
                    if (result.size() == 0) continue;
                    for (auto [output, rule] : result) {
                        if (output != input) {
                            rewrites.emplace_back(file, rule, output, false);
                        }
                    }
                }
            }));
    }

    for (auto& f : thread_pool) {
        f.join();
    }

    std::cout << TTY_SHOW_CURSOR;

#ifdef PERF
    std::sort(file_time.begin(), file_time.end());
    for (auto [t, f] : file_time) {
        printf("%.3f ", t);
        std::cerr << f << std::endl;
    }
#endif


    /* Sort rewrites by filename */
    std::sort(rewrites.begin(), rewrites.end(), [](const auto& a, const auto& b) -> bool {
        return std::get<0>(a) < std::get<0>(b);
    });

    auto review = [](auto& rw, size_t curr, size_t total) {
        auto& [filename, rule, after, accepted] = rw;
        std::cout << "-----------------------------------------------------------" << std::endl;
        fmt::print(fmt::emphasis::bold, "\nRewrite {}/{} • {}\n\n", curr, total, filename);
        std::string before = read_file(filename);
        print_patch(filename, before, post_process(before, after), {true, false, stdout});
        std::cout << std::endl;
        auto choice = multi_choice("What would you like to do?", {
            "Accept this rewrite",
            "Reject this rewrite"
        }, true);
        if (choice == -1) {
            return false;
        } else if (choice == 0) {
            accepted = true;
        } else if (choice == 1) {
            accepted = false;
        }
        return true;
    };

    if (!options.patch && !options.in_place) {

        while (true) {

            size_t selected_rewrites = std::count_if(rewrites.begin(), rewrites.end(), [](const auto& rewrite) {
                return std::get<3>(rewrite);
            });

            int selection;

            if (selected_rewrites > 0) {
                fmt::print(fmt::emphasis::bold,
                    "\nSelected {}/{} rewrites\n\n",
                    selected_rewrites,
                    rewrites.size());
                selection = multi_choice("What would you like to do?", {
                    "Review rewrites by rule",
                    "Review rewrites by file",
                    "Show a diff of the current changes",
                    "Apply changes to files on disk and exit",
                    "Discard changes and exit",
                });

                if (selection == 2) {
                    auto* fp = popen("less -R", "w");
                    if (fp != NULL) {
                        for (auto [filename, after] : get_results(rewrites)) {
                            std::string before = read_file(filename);
                            print_patch(filename, before, after, {true, true, fp});
                        }
                        pclose(fp);
                    }
                }
                if (selection == 3) {
                    options.in_place = true;
                    break;
                }
                if (selection == 4) break;
            } else {
                fmt::print(fmt::emphasis::bold,
                    "\nFound {} rewrites\n\n",
                    rewrites.size());
                selection = multi_choice("What would you like to do?", {
                    "Review rewrites by rule",
                    "Review rewrites by file",
                    "Exit without doing anything",
                });
                if (selection == 2) break;
            }

            while (true) {

                if (selection == 0) {
                    std::map<std::string, decltype(rewrites)> rules;
                    for (auto& rewrite : rewrites) {
                        rules[std::get<1>(rewrite)].emplace_back(rewrite);
                    }
                    std::vector<std::string> keys;
                    std::vector<std::string> options;
                    for (auto& [rule, rws] : rules) {
                        size_t accepted = 0;
                        for (auto rw : rws) {
                            if (std::get<3>(rw)) accepted++;
                        }
                        keys.emplace_back(rule);
                        std::string description = rule;
                        for (auto [squid, pmdid, desc] : rule_data) {
                            if (squid == rule) {
                                description = fmt::format("{} • {}", desc, squid);
                                break;
                            }
                        }
                        std::string status;
                        if (accepted > 0) {
                            status = fmt::format(fg(fmt::terminal_color::green), " ({}/{})", accepted, rws.size());
                        } else {
                            status = fmt::format(" ({}/{})", accepted, rws.size());
                        }
                        options.emplace_back(description + status);
                    }
                    auto rule_selection = multi_choice("Which rule would you like to review?", options, true);
                    if (rule_selection == -1) break;
                    auto rule = keys[rule_selection];
                    auto curr = 1;
                    auto total = rules[rule].size();
                    for (auto& rw : rewrites) {
                        if (std::get<1>(rw) == rule) {
                            if (!review(rw, curr, total)) break;
                            curr++;
                        }
                    }
                } else if (selection == 1) {
                    /*std::map<std::string, decltype(rewrites)> files;
                    for (auto rewrite : rewrites) {
                        files[std::get<0>(rewrite)].emplace_back(rewrite);
                    }
                    std::vector<std::string> keys;
                    std::vector<std::string> options;
                    for (auto& [fn, rws] : files) {
                        keys.emplace_back(fn);
                        options.emplace_back(fn + " (" + std::to_string(rws.size()) + ")");
                    }
                    auto file_selection = multi_choice("Which file would you like to review?", options, true);
                    if (file_selection == -1) break;
                    auto file = keys[file_selection];
                    review(files[file]);*/
                } else {
                    break;
                }

            }
        }

    } else {
        if (options.accept_all) {
            for (auto& [filename, rule, after, accepted] : rewrites) {
                accepted = true;
            }
        } else if (!options.accepted.empty()) {
            for (auto& [filename, rule, after, accepted] : rewrites) {
                if (options.accepted.find(rule) != options.accepted.end()) {
                    accepted = true;
                }
            }
        }
    }

    for (auto [filename, after] : get_results(rewrites)) {
        if (options.in_place) {
            std::ofstream f(filename);
            f << after;
            f.close();
        } else if (options.patch) {
            std::string before = read_file(filename);
            print_patch(filename, before, after, {false, true, stdout});
        }
    }

    return 0;
}
