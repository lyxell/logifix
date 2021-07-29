#include "tty.h"
#include <cstdlib>
#include <termios.h>
#include <unistd.h>

namespace {
struct termios orig_termios;
bool cbreak_is_enabled = false;
} // namespace

namespace tty {

void disable_cbreak_mode() {
    if (cbreak_is_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        cbreak_is_enabled = false;
    }
}

int enable_cbreak_mode() {
    if (!cbreak_is_enabled) {
        cbreak_is_enabled = true;
        struct termios raw;
        if (!isatty(STDIN_FILENO)) {
            return -1;
        }
        if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
            return -1;
        }
        raw = orig_termios;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_lflag |= ISIG;
        raw.c_iflag &= ~ICRNL;
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0) {
            return -1;
        }
    }
    return 0;
}

} // namespace tty
