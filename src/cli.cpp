#include "logifix.h"
#include "tty.h"
#include "config.h"
#include "libgit.h"
#include <filesystem>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <stack>
#include <cctype>
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

extern std::unordered_map<std::string, std::tuple<std::string, std::string, std::string>> rule_data;

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

static const char* TTY_CLEAR_TO_EOL = "\033[K";
static const char* TTY_CURSOR_UP    = "\033[A";
static const char* TTY_HIDE_CURSOR  = "\033[?25l";
static const char* TTY_SHOW_CURSOR  = "\033[?25h";
static const char* TTY_MOVE_TO_BOTTOM = "\033[9999;1H";

std::string replace_tabs_with_spaces(std::string str) {
    std::string result;
    for (char c : str) {
        if (c == '\t') {
            result += "    ";
        } else {
            result += c;
        }
    }
    return result;
}

std::vector<std::string> prettify_patch(std::vector<std::string> lines) {
    std::vector<std::pair<std::string, std::string>> columns;
    // erase first line (git header)
    lines.erase(lines.begin());
    size_t line_number = 0;
    bool seen_header_before = false;
    for (auto& line : lines) {
        if (line.empty()) continue;
        auto marker = line[0];
        auto content = replace_tabs_with_spaces(line.substr(1));
        switch (marker) {
        case '-':
            columns.emplace_back("-", content);
            line_number++;
            break;
        case '+':
            columns.emplace_back("+", content);
            break;
        case '@':
            if (seen_header_before) {
                columns.emplace_back("...", "");
            }
            line_number = std::stoi(line.substr(line.find('-') + 1, line.find(',')));
            seen_header_before = true;
            break;
        default:
            columns.emplace_back(std::to_string(line_number), content);
            line_number++;
            break;
        }
    }
    size_t left_column_size = 0;
    for (const auto& [l, r] : columns) {
        left_column_size = std::max(left_column_size, l.size());
    }
    std::vector<std::string> result;
    auto format = "  {0:>{2}} {1}";
    for (auto& [l, r] : columns) {
        if (l == "+") {
            result.emplace_back(fmt::format(fg(fmt::terminal_color::green), format, l, r, left_column_size));
        } else if (l == "-") {
            result.emplace_back(fmt::format(fg(fmt::terminal_color::red), format, l, r, left_column_size));
        } else {
            result.emplace_back(fmt::format(format, l, r, left_column_size));
        }
    }
    return result;
}

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
    tty::enable_cbreak_mode();
#ifndef NDEBUG
    fmt::print(fg(fmt::terminal_color::red) | fmt::emphasis::bold, "DEBUG ");
#endif
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
            tty::disable_cbreak_mode();
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
        if (!lcs[i]) {
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

void at_signal(int signal) {
    std::exit(1);
}

void at_exit() {
    std::cerr << TTY_MOVE_TO_BOTTOM;
    std::cerr << TTY_SHOW_CURSOR << std::endl;
    tty::disable_cbreak_mode();
}

int main(int argc, char** argv) {

    setenv("SOUFFLE_ALLOW_SIGNALS","",1);
    std::signal(SIGINT, at_signal);
    std::atexit(at_exit);

    std::cerr << TTY_HIDE_CURSOR;

    options_t options = parse_options(argc, argv);

    std::vector<std::tuple<std::string,std::string,std::string, bool>> rewrites;

    for (const auto& file : options.files) {
        logifix::add_file(read_file(file));
    }

    size_t count = 0;

    logifix::run([&count, &options](size_t node) {
        count++;
        int progress = int((double(count) / double(options.files.size())) * 40);
        int progress_full = 40;
        fmt::print("\r[{2:=^{0}}{2: ^{1}}] {3}/{4}", progress, progress_full - progress, "", count, options.files.size());
    });

    //logifix::print_performance_metrics();

    for (const auto& file : options.files) {
        for (auto [rule, result] : logifix::get_rewrites_for_file(read_file(file))) {
            rewrites.emplace_back(file, rule, result, false);
        }
    }

    auto review = [](auto& rw, size_t curr, size_t total) {
        auto& [filename, rule, after, accepted] = rw;
        fmt::print(fmt::emphasis::bold, "\nRewrite {}/{} • {}\n\n", curr, total, filename);
        std::string before = read_file(filename);
        auto patch = libgit::create_patch(filename, before, post_process(before, after));
        if (patch.size()) {
        for (auto line : prettify_patch(patch)) {
            std::cout << line << std::endl;
        }
        }
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
                selection = multi_choice("What would you like to do?",
                {
                    "Review rewrites by rule",
                    "Show a diff of the current changes",
                    "Apply changes to files on disk and exit",
                    "Discard changes and exit",
                });

                if (selection == 1) {
                    auto* fp = popen("less -R", "w");
                    if (fp != NULL) {
                        for (auto [filename, after] : get_results(rewrites)) {
                            fmt::print(fp, "\n{}\n\n", filename);
                            std::string before = read_file(filename);
                            auto patch = libgit::create_patch(filename, before, after);
                            for (auto line : prettify_patch(patch)) {
                                fputs(line.c_str(), fp);
                                fputs("\n", fp);
                            }
                        }
                        pclose(fp);
                    }
                }
                if (selection == 2) {
                    options.in_place = true;
                    break;
                }
                if (selection == 3) break;
            } else {
                fmt::print(fmt::emphasis::bold,
                    "\nFound {} rewrites\n\n",
                    rewrites.size());

                if (rewrites.size() == 0) break;

                selection = multi_choice("What would you like to do?", {
                    "Review rewrites by rule",
                    "Exit without doing anything",
                });
                if (selection == 1) break;
            }

            while (true) {

                if (selection == 0) {
                    std::map<std::string, decltype(rewrites)> rules;
                    for (auto& rewrite : rewrites) {
                        rules[std::get<1>(rewrite)].emplace_back(rewrite);
                    }
                    std::vector<std::string> keys;
                    std::vector<std::tuple<std::string,std::string,std::string>> columns;
                    for (auto& [rule, rws] : rules) {
                        keys.emplace_back(rule);
                        auto description = std::get<2>(rule_data[rule]);
                        size_t accepted = std::count_if(rws.begin(), rws.end(), [](auto rw) {
                            return std::get<3>(rw);
                        });
                        std::string status;
                        if (accepted > 0) {
                            columns.emplace_back(description,
                                    std::get<0>(rule_data[rule]),
                                    fmt::format(fg(fmt::terminal_color::green), "{}/{}", accepted, rws.size()));
                        } else {
                            columns.emplace_back(description,
                                    std::get<0>(rule_data[rule]),
                                    fmt::format("{}/{}", accepted, rws.size()));
                        }
                    }
                    size_t left_column_width = 0;
                    for (auto [l, m, r] : columns) {
                        left_column_width = std::max(left_column_width, l.size());
                    }
                    std::vector<std::string> options;
                    for (auto [l, m, r] : columns) {
                        options.emplace_back(fmt::format("{0:<{3}}   {1:5}  {2}", l, m, r, left_column_width));
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
            auto patch = libgit::create_patch(filename, before, after);
            for (auto line : patch) {
                std::cout << line << std::endl;
            }
        }
    }

    return 0;
}
