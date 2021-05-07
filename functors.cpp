#include <cstdlib>
#include <cstring>

extern "C" {

// TODO O(n^2) -> O(n)

const char* format2(const char* fmt, const char* x, const char* y) {
    static char memory[256];
    memory[0] = '\0';
    bool seen_left_bracket = false;
    bool written_x = false;
    for (size_t i = 0; i < strlen(fmt); i++) {
        if (fmt[i] == '{') {
            seen_left_bracket = true;
        } else if (fmt[i] == '}' && seen_left_bracket) {
            if (!written_x) {
                strcat(memory, x);
                written_x = true;
            } else {
                strcat(memory, y);
                // copy rest of string
                strcat(memory, fmt + i + 1);
                break;
            }
            seen_left_bracket = false;
        } else {
            size_t len = strlen(memory);
            if (seen_left_bracket) {
                memory[len] = '{';
                memory[len+1] = fmt[i];
                memory[len+2] = '\0';
            } else {
                memory[len] = fmt[i];
                memory[len+1] = '\0';
            }
            seen_left_bracket = false;
        }
    }
    return memory;
}

}
