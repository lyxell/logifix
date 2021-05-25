#include <git2.h>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <future>
#include <mutex>
#include "../logifix.h"

static const char *colors[] = {
	"\033[m", /* reset */
	"\033[1m", /* bold */
	"\033[31m", /* red */
	"\033[32m", /* green */
	"\033[36m" /* cyan */
};

int diff_output(
	const git_diff_delta *d,
	const git_diff_hunk *h,
	const git_diff_line *l,
	void *p)
{
	FILE *fp = (FILE*)p;

	(void)d; (void)h;

	if (!fp)
		fp = stdout;

	if (l->origin == GIT_DIFF_LINE_CONTEXT ||
		l->origin == GIT_DIFF_LINE_ADDITION ||
		l->origin == GIT_DIFF_LINE_DELETION)
		fputc(l->origin, fp);

	fwrite(l->content, 1, l->content_len, fp);

	return 0;
}

static int color_printer(
	const git_diff_delta *delta,
	const git_diff_hunk *hunk,
	const git_diff_line *line,
	void *data)
{
	int *last_color = (int*) data, color = 0;

	(void)delta; (void)hunk;

	if (*last_color >= 0) {
		switch (line->origin) {
		case GIT_DIFF_LINE_ADDITION:  color = 3; break;
		case GIT_DIFF_LINE_DELETION:  color = 2; break;
		case GIT_DIFF_LINE_ADD_EOFNL: color = 3; break;
		case GIT_DIFF_LINE_DEL_EOFNL: color = 2; break;
		case GIT_DIFF_LINE_FILE_HDR:  color = 1; break;
		case GIT_DIFF_LINE_HUNK_HDR:  color = 4; break;
		default: break;
		}

		if (color != *last_color) {
			if (*last_color == 1 || color == 1)
				fputs(colors[0], stdout);
			fputs(colors[color], stdout);
			*last_color = color;
		}
	}

	return diff_output(delta, hunk, line, stdout);
}

std::string apply_rewrites(const std::string& input, std::vector<std::tuple<int,int,int,std::string,std::string>> rewrites) {
    /* expand rewrites that only does deletion to also remove whitespace at the beginning of the line */
    for (auto& [rule_number, start, end, replacement, mess] : rewrites) {
        if (replacement.empty()) {
            while (start > 1 && (input[start-1] == ' ' || input[start-1] == '\t')) {
                start--;
            }
            if (start > 2 && input[start-2] == '\r' && input[start-1] == '\n') start -= 2;
            else if (start > 1 && input[start-1]) start--;
        }
    }
    /* sort rewrites */
    std::sort(rewrites.begin(), rewrites.end(), [](auto x, auto y) {
        if (std::get<1>(x) == std::get<1>(y)) return std::get<2>(x) < std::get<2>(y);
        return std::get<1>(x) > std::get<1>(y);
    });
    size_t curr = 0;
    std::string result;
    while (curr < input.size()) {
        /* discard rewrites that happened before curr */
        while (rewrites.size() && std::get<1>(rewrites.back()) < curr) rewrites.pop_back();
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

int main(int argc, char** argv) {
    int rule_number = -1;
    bool in_place = false;
    std::vector<std::string> files;
    for (int i = 1; i < argc; i++) {
        std::string s(argv[i]);
        if (s == "--in-place") {
            in_place = true;
        } else if (s.substr(0, 8) == "--rules=") {
            rule_number = std::stoi(s.substr(8));
        } else {
            files.emplace_back(std::filesystem::path(std::move(s)).lexically_normal());
        }
    }

    if (rule_number == -1) {
        std::cerr << "No rule specified" << std::endl;
        return 1;
    }

    if (files.empty()) {
        std::cerr << "No files specified" << std::endl;
        return 1;
    }
    
    git_libgit2_init();

    /* perform analysis */
    std::mutex print_mutex;
    std::vector<std::future<void>> futures;
    for (const auto& file : files) {
        futures.emplace_back(std::async(std::launch::async, [&file,rule_number,in_place,&print_mutex] {
            logifix::program program;
            program.add_file(file.c_str());
            program.run();

            /* filter rewrites by their id */

            std::vector<std::tuple<int,int,int,std::string,std::string>> rewrites;
            for (auto r : program.get_possible_rewrites(file.c_str())) {
                if (std::get<0>(r) == rule_number) rewrites.emplace_back(std::move(r));
            }

            if (in_place) {
                std::cout << apply_rewrites(program.get_source_code(file.c_str()), std::move(rewrites));
            } else {
                auto input = program.get_source_code(file.c_str());
                auto output = apply_rewrites(input, std::move(rewrites));

                // create diff
                git_diff *diff;
                git_buf buf = {0};
                git_patch *patch = NULL;
                git_patch_from_buffers(&patch, input.c_str(), input.size(), file.c_str(), output.c_str(), output.size(), file.c_str(), NULL),
                git_patch_to_buf(&buf, patch);
                git_diff_from_buffer(&diff, buf.ptr, buf.size);
                git_patch_free(patch);
                git_buf_dispose(&buf);
                if (git_diff_num_deltas(diff) > 0) {
                    std::lock_guard<std::mutex> lock(print_mutex);
                    int color = isatty(fileno(stdout)) ? 0 : -1;
                    git_diff_print(diff, GIT_DIFF_FORMAT_PATCH, color_printer, &color);
                }
                git_diff_free(diff);
            }
        }));
    }

    for (auto& f : futures) f.wait();

    return 0;

}
