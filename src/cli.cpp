#include "config.h"
#include "logifix.h"
#include "tty.h"
#include <cctype>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <mutex>
#include <nway.h>
#include <set>
#include <stack>
#include <thread>
#include <tuple>

using namespace std::string_literals;
namespace fs = std::filesystem;

using patch_collection =
    std::vector<std::tuple<std::string, std::string, std::string, bool>>;

extern std::unordered_map<std::string,
                          std::tuple<std::string, std::string, std::string>>
    rule_data;

struct options_t {
    bool accept_all;
    bool in_place;
    bool patch;
    bool verbose;
    std::set<std::string> files;
    std::set<std::string> accepted;
};

struct printer_opts {
    bool color;
    bool print_file_header;
    FILE* fp;
};

enum class key { down, left, ret, right, up, unknown };

static const char* TTY_CLEAR_TO_EOL = "\033[K";
static const char* TTY_CURSOR_UP = "\033[A";
static const char* TTY_HIDE_CURSOR = "\033[?25l";
static const char* TTY_SHOW_CURSOR = "\033[?25h";
static const char* TTY_MOVE_TO_BOTTOM = "\033[9999;1H";

std::string replace_tabs_with_spaces(const std::string& str) {
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

static void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](auto c) { return !std::isspace(c); })
                .base(),
            s.end());
}

std::vector<std::string> line_split(std::string str) {
    std::vector<std::string> result;
    while (!str.empty()) {
        size_t i;
        for (i = 0; i < str.size() - 1; i++) {
            if (str[i] == '\n')
                break;
        }
        result.emplace_back(str.substr(0, i + 1));
        str = str.substr(i + 1);
    }
    return result;
};

std::vector<std::string> create_patch(const std::string& filename, std::string before,
                                      std::string after) {
    auto a = line_split(before);
    auto b = line_split(after);
    for (auto& x : a) {
        rtrim(x);
    }
    for (auto& x : b) {
        rtrim(x);
    }
    std::vector<std::tuple<bool, char, std::string>> changes;
    std::vector<std::optional<size_t>> lcs =
        nway::longest_common_subsequence(a, b);
    size_t a_pos = 0;
    size_t b_pos = 0;
    while (a_pos < a.size() || b_pos < b.size()) {
        /* a and b agree */
        while (a_pos < a.size() && lcs[a_pos] && *lcs[a_pos] == b_pos) {
            changes.emplace_back(false, ' ', a[a_pos]);
            a_pos++;
            b_pos++;
        }
        /* a has no matching position */
        while (a_pos < a.size() && !lcs[a_pos]) {
            changes.emplace_back(false, '-', a[a_pos]);
            a_pos++;
        }
        /* a has matching position but it is not that of b_pos */
        while (a_pos < a.size() && lcs[a_pos] && *lcs[a_pos] != b_pos) {
            changes.emplace_back(false, '+', b[b_pos]);
            b_pos++;
        }
    }
    /* 3 lines of context */
    int context = 3;
    for (int i = 0; i < changes.size(); i++) {
        auto& [keep, marker, line] = changes[i];
        for (int j = std::max(0, i - context);
             j < std::min(int(changes.size()), i + context + 1); j++) {
            if (std::get<1>(changes[j]) != ' ') {
                keep = true;
            }
        }
    }
    /* create patch output */
    std::vector<std::string> result;
    result.emplace_back(
        fmt::format("diff --git a/{} b/{}", filename, filename));
    size_t a_pos_output = 1;
    size_t b_pos_output = 1;
    for (int i = 0; i < changes.size(); i++) {
        auto& [keep, marker, line] = changes[i];
        if (!keep) {
            a_pos_output++;
            b_pos_output++;
            continue;
        }
        size_t a_start = a_pos_output;
        size_t b_start = b_pos_output;
        size_t a_lines = 0;
        size_t b_lines = 0;
        size_t hunk_end = i;
        while (hunk_end < changes.size() && std::get<0>(changes[hunk_end])) {
            switch (std::get<1>(changes[hunk_end])) {
            case ' ':
                a_pos_output++;
                b_pos_output++;
                a_lines++;
                b_lines++;
                break;
            case '-':
                a_pos_output++;
                a_lines++;
                break;
            case '+':
                b_pos_output++;
                b_lines++;
                break;
            default:
                break;
            }
            hunk_end++;
        }
        /* output header */
        result.emplace_back(fmt::format("@@ -{},{} +{},{} @@", a_start, a_lines,
                                        b_start, b_lines));
        while (i < changes.size() && std::get<0>(changes[i])) {
            result.emplace_back(fmt::format("{}{}", std::get<1>(changes[i]),
                                            std::get<2>(changes[i])));
            i++;
        }
        i--;
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
        if (line.empty())
            continue;
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
            line_number =
                std::stoi(line.substr(line.find('-') + 1, line.find(',')));
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
    const auto* format = "  {0:>{2}} {1}";
    for (auto& [l, r] : columns) {
        if (l == "+") {
            result.emplace_back(fmt::format(fg(fmt::terminal_color::green),
                                            format, l, r, left_column_size));
        } else if (l == "-") {
            result.emplace_back(fmt::format(fg(fmt::terminal_color::red),
                                            format, l, r, left_column_size));
        } else {
            result.emplace_back(fmt::format(format, l, r, left_column_size));
        }
    }
    return result;
}

static std::string read_file(std::string_view path) {
    constexpr auto read_size = std::size_t{4096};
    auto stream = std::ifstream{path.data()};
    stream.exceptions(std::ios_base::badbit);
    auto out = std::string{};
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}

static key get_keypress() {
    switch (getchar()) {
    case 0x0d:
        return key::ret;
    case 'k':
        return key::up;
    case 'j':
        return key::down;
    case 'h':
        return key::left;
    case 'l':
        return key::right;
    case 0x1b:
        switch (getchar()) {
        case 0x5b:
            switch (getchar()) {
            case 0x41:
                return key::up;
            case 0x42:
                return key::down;
            case 0x44:
                return key::left;
            case 0x43:
                return key::right;
            default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return key::unknown;
}

static options_t parse_options(int argc, char** argv) {

    options_t options = {
        .accept_all = false,
        .in_place = false,
        .patch = false,
        .verbose = false,
        .files = {},
        .accepted = {},
    };

    std::vector<
        std::tuple<std::string, std::function<void(std::string)>, std::string>>
        opts;

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
            std::cout << " " << std::setw(19) << std::left << option
                      << description << std::endl;
        }
        std::cout << std::endl;
    };

    auto print_examples = []() {
        fmt::print(fmt::emphasis::bold, "EXAMPLES\n\n");
        fmt::print("  {} src/main src/test\n\n", PROJECT_NAME);
        fmt::print(
            "  {} src/main --in-place --accept=S1125,S1155 Test.java\n\n",
            PROJECT_NAME);
    };

    auto parse_accepted = [&](const std::string& str) {
        std::stringstream ss(str);
        while (ss.good()) {
            std::string substr;
            std::getline(ss, substr, ',');
            options.accepted.emplace(substr);
        }
    };

    opts = {
        {"--accept-all", [&](std::string str) { options.accept_all = true; },
         "Accept all patches without asking"},
        {"--accept=<rules>", [&](std::string str) { parse_accepted(str); },
         "Comma-separated list of rules to accept"},
        {"--in-place", [&](std::string str) { options.in_place = true; },
         "Disable interaction, rewrite files on disk"},
        {"--patch", [&](std::string str) { options.patch = true; },
         "Disable interaction, output a patch to stdout"},
        {"--help",
         [&](std::string str) {
             print_usage();
             print_flags();
             print_examples();
             std::exit(0);
         },
         "Print this information and exit"},
        {"--verbose", [&](std::string str) { options.verbose = true; },
         "Print debugging information"},
        {"--version", [&](std::string str) { print_version_and_exit(); },
         "Print version information and exit"},
    };

    std::vector<std::string> arguments;
    for (auto i = 1; i < argc; i++) {
        arguments.emplace_back(argv[i]);
    }
    std::reverse(arguments.begin(), arguments.end());

    while (arguments.size()) {
        auto argument = arguments.back();
        if (argument.size() < 2 || argument.substr(0, 2) != "--")
            break;
        auto found = false;
        for (const auto [option, fn, description] : opts) {
            if (option.find("=") != std::string::npos) {
                auto opt_part = option.substr(0, option.find("=") + 1);
                if (argument.size() >= opt_part.size() &&
                    argument.substr(0, opt_part.size()) == opt_part) {
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
            for (const auto& entry :
                 fs::recursive_directory_iterator(argument)) {
                if (!entry.is_regular_file())
                    continue;
                if (entry.path().extension() != ".java")
                    continue;
                std::string s = entry.path().lexically_normal();
                options.files.emplace(entry.path().lexically_normal());
            }
        } else {
            options.files.emplace(
                fs::path(std::move(argument)).lexically_normal());
        }
        arguments.pop_back();
    }

    return options;
}

static int multi_choice(std::string question,
                        std::vector<std::string> alternatives,
                        bool exit_on_left = false) {
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
        for (auto i = scroll;
             i < std::min(alternatives.size(), scroll + height); i++) {
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
        for (auto i = scroll;
             i < std::min(alternatives.size(), scroll + height); i++) {
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

static std::string post_process(std::string before, std::string after,
                                const options_t& options) {
    auto find_first_non_space = [](const std::string& str) {
        return std::find_if(str.begin(), str.end(),
                            [](char c) { return std::isspace(c) == 0; });
    };
    auto bracket_balance = [](const std::string& str) {
        int result = 0;
        for (char c : str) {
            if (c == '(' || c == '{') {
                result++;
            } else if (c == ')' || c == '}') {
                result--;
            }
        }
        return result;
    };
    auto detect_indentation = [find_first_non_space](std::string str) {
        auto lines = line_split(str);
        std::vector<std::string> indentations;
        for (auto line : lines) {
            auto first_non_space_at = find_first_non_space(line);
            if (first_non_space_at == line.end() ||
                first_non_space_at == line.begin()) {
                continue;
            }
            auto indent = line.substr(0, first_non_space_at - line.begin());
            /* Check that indentation has only one type of char */
            if (std::any_of(indent.begin(), indent.end(),
                            [indent](char c) { return c != indent[0]; })) {
                continue;
            }
            if (!indentations.empty() && indentations.back() == indent) {
                continue;
            }
            indentations.push_back(indent);
        }
        std::map<std::string, double> multiplier = {
            {"  ", 0.5}, {"    ", 0.25}, {"\t", 0.5}};
        std::map<std::string, double> candidates;
        for (size_t i = 1; i < indentations.size(); i++) {
            auto a = indentations[i - 1];
            auto b = indentations[i];
            if (a.size() > b.size()) {
                std::swap(a, b);
            }
            if (b.find(a) != 0) {
                continue;
            }
            auto candidate = b.substr(a.size());
            candidates[candidate] += 1 + multiplier[candidate];
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

    if (options.verbose) {
        std::cerr << "Detected indentation ";
        std::cerr << indentation.size() << " ";
        std::cerr << (indentation[0] == ' ' ? "space" : "tab");
        std::cerr << std::endl;
    }

    auto string_has_only_whitespace = [](const auto& str) {
        return std::all_of(str.begin(), str.end(),
                           [](char c) { return std::isspace(c) != 0; });
    };

    auto after_lines = line_split(std::move(after));
    auto before_lines = line_split(std::move(before));
    auto lcs = nway::longest_common_subsequence(after_lines, before_lines);
    std::string processed;
    for (size_t i = 0; i < after_lines.size(); i++) {
        if (!lcs[i]) {
            if (string_has_only_whitespace(after_lines[i])) {
                continue;
            }
            // auto-indent
            if (i > 0 && find_first_non_space(after_lines[i]) ==
                             after_lines[i].begin()) {
                auto prev_line = after_lines[i - 1];
                // get indent from previous line
                auto new_indent = prev_line.substr(
                    0, find_first_non_space(prev_line) - prev_line.begin());
                // increase indent if previous line has an open bracket
                if (bracket_balance(prev_line) > 0) {
                    new_indent += indentation;
                }
                // decrease indent if we have closing bracket
                if (bracket_balance(after_lines[i]) < 0) {
                    new_indent = new_indent.substr(
                        std::min(indentation.size(), new_indent.size()));
                }
                after_lines[i] = new_indent + after_lines[i];
            }
        }
        processed += after_lines[i];
    }
    return processed;
}

std::map<std::string, std::string> get_results(const patch_collection& patches,
                                               const options_t& options) {
    std::map<std::string, std::string> results;
    std::map<std::string, std::vector<std::string>> patches_per_file;
    for (const auto& [filename, rule, after, is_accepted] : patches) {
        if (is_accepted) {
            patches_per_file[filename].emplace_back(after);
        }
    }
    for (auto [filename, patches] : patches_per_file) {
        std::string before = read_file(filename);
        auto diff = nway::diff(before, patches);
        assert(!nway::has_conflict(diff));
        results[filename] = post_process(before, nway::merge(diff), options);
    }
    return results;
}

void at_signal(int signal) { std::exit(1); }

void at_exit() {
    std::cerr << TTY_MOVE_TO_BOTTOM;
    std::cerr << TTY_SHOW_CURSOR << std::endl;
    tty::disable_cbreak_mode();
}

int main(int argc, char** argv) {

    setenv("SOUFFLE_ALLOW_SIGNALS", "", 1);
    std::signal(SIGINT, at_signal);
    std::atexit(at_exit);

    std::cerr << TTY_HIDE_CURSOR;

    options_t options = parse_options(argc, argv);

    /**
     * Sort patches first by filename and then by the length of the prefix
     * that they share with the original file
     */
    auto patch_cmp = [](const auto& a, const auto& b) {
        const auto& [a_file, a_rule, a_result] = a;
        const auto& [b_file, b_rule, b_result] = b;
        if (a_file != b_file) {
            return a_file < b_file;
        }
        if (a_rule != b_rule) {
            return a_rule < b_rule;
        }
        auto before = read_file(a_file);
        size_t a_pos = std::numeric_limits<size_t>::max();
        size_t b_pos = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < before.size(); i++) {
            if (i < a_result.size() && before[i] != a_result[i]) {
                a_pos = std::min(a_pos, i);
            }
            if (i < b_result.size() && before[i] != b_result[i]) {
                b_pos = std::min(b_pos, i);
            }
        }
        if (a_pos == b_pos) {
            return a_result < b_result;
        }
        return a_pos < b_pos;
    };
    std::set<std::tuple<std::string, std::string, std::string>,
             decltype(patch_cmp)>
        patch_set(patch_cmp);

    for (const auto& file : options.files) {
        logifix::add_file(read_file(file));
    }

    size_t count = 0;

    logifix::run([&count, &options](size_t node) {
        count++;
        int progress = int((double(count) / double(options.files.size())) * 40);
        int progress_full = 40;
        fmt::print(stderr, "\r[{2:=^{0}}{2: ^{1}}] {3}/{4}", progress,
                   progress_full - progress, "", count, options.files.size());
    });

    // logifix::print_performance_metrics();

    for (const auto& file : options.files) {
        for (auto [rule, result] :
             logifix::get_patches_for_file(read_file(file))) {
            patch_set.emplace(file, rule, result);
        }
    }

    std::vector<std::tuple<std::string, std::string, std::string, bool>>
        patches;

    patches.resize(patch_set.size());

    for (const auto& [file, rule, result] : patch_set) {
        patches.emplace_back(file, rule, result, false);
    }

    // std::sort(patches.begin(), patches.end());

    auto review = [&options](auto& rw, size_t curr, size_t total) {
        auto& [filename, rule, after, accepted] = rw;
        fmt::print(fmt::emphasis::bold, "\nPatch {}/{} • {}\n\n", curr, total,
                   filename);
        std::string before = read_file(filename);
        auto patch = create_patch(filename, before,
                                  post_process(before, after, options));
        if (!patch.empty()) {
            for (const auto& line : prettify_patch(patch)) {
                std::cout << line << std::endl;
            }
        }
        std::cout << std::endl;
        auto choice =
            multi_choice("What would you like to do?",
                         {"Accept this patch", "Reject this patch"}, true);
        if (choice == -1) {
            return false;
        }
        if (choice == 0) {
            accepted = true;
        } else if (choice == 1) {
            accepted = false;
        }
        return true;
    };

    if (!options.patch && !options.in_place) {

        while (true) {

            size_t selected_patches = std::count_if(
                patches.begin(), patches.end(),
                [](const auto& patch) { return std::get<3>(patch); });

            int selection;

            if (selected_patches > 0) {
                fmt::print(fmt::emphasis::bold, "\nSelected {}/{} patches\n\n",
                           selected_patches, patches.size());
                selection =
                    multi_choice("What would you like to do?",
                                 {
                                     "Review patches by rule",
                                     "Show a diff of the current changes",
                                     "Apply changes to files on disk and exit",
                                     "Discard changes and exit",
                                 });

                if (selection == 1) {
                    auto* fp = popen("less -R", "w");
                    if (fp != nullptr) {
                        for (auto [filename, after] :
                             get_results(patches, options)) {
                            fmt::print(fp, "\n{}\n\n", filename);
                            std::string before = read_file(filename);
                            auto patch = create_patch(filename, before, after);
                            for (const auto& line : prettify_patch(patch)) {
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
                if (selection == 3) {
                    break;
                }
            } else {
                fmt::print(fmt::emphasis::bold,
                           "\n\nAnalyzed {} files and found {} patches\n\n",
                           options.files.size(), patches.size());

                if (patches.empty()) {
                    break;
                }

                selection = multi_choice("What would you like to do?",
                                         {
                                             "Review patches by rule",
                                             "Exit without doing anything",
                                         });
                if (selection == 1) {
                    break;
                }
            }

            while (true) {

                if (selection == 0) {
                    std::map<std::string, decltype(patches)> rules;
                    for (auto& patch : patches) {
                        rules[std::get<1>(patch)].emplace_back(patch);
                    }
                    std::vector<std::string> keys;
                    std::vector<
                        std::tuple<std::string, std::string, std::string>>
                        columns;
                    for (auto& [rule, rws] : rules) {
                        keys.emplace_back(rule);
                        auto description = std::get<2>(rule_data[rule]);
                        size_t accepted =
                            std::count_if(rws.begin(), rws.end(), [](auto rw) {
                                return std::get<3>(rw);
                            });
                        std::string status;
                        if (accepted > 0) {
                            columns.emplace_back(
                                description, std::get<0>(rule_data[rule]),
                                fmt::format(fg(fmt::terminal_color::green),
                                            "{}/{}", accepted, rws.size()));
                        } else {
                            columns.emplace_back(
                                description, std::get<0>(rule_data[rule]),
                                fmt::format("{}/{}", accepted, rws.size()));
                        }
                    }
                    size_t left_column_width = 0;
                    for (auto [l, m, r] : columns) {
                        left_column_width =
                            std::max(left_column_width, l.size());
                    }
                    std::vector<std::string> options;
                    options.resize(columns.size());
                    for (auto [l, m, r] : columns) {
                        options.emplace_back(fmt::format("{0:<{2}}    {1}", l,
                                                         r, left_column_width));
                    }
                    auto rule_selection = multi_choice(
                        "Which rule would you like to review?", options, true);
                    if (rule_selection == -1) {
                        break;
                    }
                    auto rule = keys[rule_selection];
                    auto curr = 1;
                    auto total = rules[rule].size();
                    for (auto& rw : patches) {
                        if (std::get<1>(rw) == rule) {
                            if (!review(rw, curr, total)) {
                                break;
                            }
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
            for (auto& [filename, rule, after, accepted] : patches) {
                accepted = true;
            }
        } else if (!options.accepted.empty()) {
            for (auto& [filename, rule, after, accepted] : patches) {
                if (options.accepted.find(rule) != options.accepted.end()) {
                    accepted = true;
                }
            }
        }
    }

    for (auto [filename, after] : get_results(patches, options)) {
        if (options.in_place) {
            std::ofstream f(filename);
            f << after;
            f.close();
        } else if (options.patch) {
            std::string before = read_file(filename);
            auto patch = create_patch(filename, before, after);
            for (const auto& line : patch) {
                std::cout << line << std::endl;
            }
        }
    }

    return 0;
}
