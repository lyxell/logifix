#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "tty.h"

static struct termios orig_termios;

void tty_disable_cbreak_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int tty_enable_cbreak_mode() {
    struct termios raw;
    if (!isatty(STDIN_FILENO)) return -1;
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) return -1;
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_lflag |= ISIG;
    raw.c_iflag &= ~ICRNL;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw) < 0) return -1;
    return 0;
}
