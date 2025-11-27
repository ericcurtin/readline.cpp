#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>

int main() {
    struct termios orig, raw;

    // Get current terminal settings
    if (tcgetattr(STDIN_FILENO, &orig) < 0) {
        std::cerr << "Failed to get terminal attributes\n";
        return 1;
    }

    raw = orig;

    // Set raw mode
    raw.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    raw.c_cflag &= ~(CSIZE | PARENB);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0) {
        std::cerr << "Failed to set raw mode\n";
        return 1;
    }

    // Disable stdout buffering
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::cout << "Raw mode test. Type characters (press 'q' to quit):\n";
    std::cout << "Each character you type should appear immediately.\n\n";

    while (true) {
        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);

        if (n <= 0) {
            break;
        }

        if (c == 'q') {
            break;
        }

        if (c == 3) {  // Ctrl+C
            break;
        }

        // Echo the character
        printf("Got: '%c' (0x%02x)\n", c >= 32 && c < 127 ? c : '?', (unsigned char)c);
    }

    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
    std::cout << "\nDone!\n";

    return 0;
}
