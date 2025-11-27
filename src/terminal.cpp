#include "readline/terminal.h"
#include "readline/errors.h"
#include <stdexcept>
#include <iostream>
#include <cstdio>

#ifdef _WIN32
#include <conio.h>
#else
#include <signal.h>
#include <cerrno>
#include <unistd.h>
#endif

namespace readline {

#ifdef _WIN32

Terminal::Terminal()
    : input_handle_(GetStdHandle(STD_INPUT_HANDLE)),
      output_handle_(GetStdHandle(STD_OUTPUT_HANDLE)),
      original_input_mode_(0),
      original_output_mode_(0),
      raw_mode_(false),
      stop_io_loop_(false) {

    if (!is_terminal(input_handle_)) {
        throw std::runtime_error("stdin is not a terminal");
    }
}

Terminal::~Terminal() {
    if (raw_mode_) {
        unset_raw_mode();
    }

    stop_io_loop_ = true;
    queue_cv_.notify_all();

    if (io_thread_.joinable()) {
        io_thread_.detach();
    }
}

void Terminal::set_raw_mode() {
    if (raw_mode_) {
        return;
    }

    // Save original console modes
    if (!GetConsoleMode(input_handle_, &original_input_mode_)) {
        throw std::runtime_error("Failed to get console input mode");
    }
    if (!GetConsoleMode(output_handle_, &original_output_mode_)) {
        throw std::runtime_error("Failed to get console output mode");
    }

    // Set raw mode for input
    DWORD new_input_mode = original_input_mode_;
    new_input_mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    new_input_mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;

    if (!SetConsoleMode(input_handle_, new_input_mode)) {
        throw std::runtime_error("Failed to set console input mode");
    }

    // Enable virtual terminal processing for output (ANSI escape sequences)
    DWORD new_output_mode = original_output_mode_;
    new_output_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(output_handle_, new_output_mode)) {
        // Restore input mode if output mode fails
        SetConsoleMode(input_handle_, original_input_mode_);
        throw std::runtime_error("Failed to set console output mode");
    }

    raw_mode_ = true;

    // Disable stdout buffering for immediate character display
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // Start I/O thread
    if (!io_thread_.joinable()) {
        io_thread_ = std::thread(&Terminal::io_loop, this);
    }
}

void Terminal::unset_raw_mode() {
    if (!raw_mode_) {
        return;
    }

    SetConsoleMode(input_handle_, original_input_mode_);
    SetConsoleMode(output_handle_, original_output_mode_);

    raw_mode_ = false;
}

bool Terminal::is_terminal(void* handle) {
    DWORD mode;
    return GetConsoleMode(static_cast<HANDLE>(handle), &mode) != 0;
}

void Terminal::io_loop() {
    auto push_escape_sequence = [this](const char* seq) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        for (const char* p = seq; *p; ++p) {
            char_queue_.push(*p);
        }
        queue_cv_.notify_one();
    };

    while (!stop_io_loop_) {
        INPUT_RECORD ir;
        DWORD events_read;

        if (!ReadConsoleInput(input_handle_, &ir, 1, &events_read)) {
            break;
        }

        if (events_read == 0) {
            continue;
        }

        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
            char c = ir.Event.KeyEvent.uChar.AsciiChar;

            // Handle special keys
            WORD vk = ir.Event.KeyEvent.wVirtualKeyCode;

            if (vk == VK_UP) {
                push_escape_sequence("\x1b[A");
                continue;
            } else if (vk == VK_DOWN) {
                push_escape_sequence("\x1b[B");
                continue;
            } else if (vk == VK_RIGHT) {
                push_escape_sequence("\x1b[C");
                continue;
            } else if (vk == VK_LEFT) {
                push_escape_sequence("\x1b[D");
                continue;
            } else if (vk == VK_DELETE) {
                push_escape_sequence("\x1b[3~");
                continue;
            } else if (vk == VK_HOME) {
                push_escape_sequence("\x1b[H");
                continue;
            } else if (vk == VK_END) {
                push_escape_sequence("\x1b[F");
                continue;
            }

            if (c != 0) {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                char_queue_.push(c);
                queue_cv_.notify_one();
            }
        }
    }
}

#else // Unix implementation

Terminal::Terminal()
    : fd_(STDIN_FILENO), raw_mode_(false), stop_io_loop_(false) {

    if (!is_terminal(fd_)) {
        throw std::runtime_error("stdin is not a terminal");
    }
}

Terminal::~Terminal() {
    if (raw_mode_) {
        unset_raw_mode();
    }

    stop_io_loop_ = true;
    queue_cv_.notify_all();

    if (io_thread_.joinable()) {
        io_thread_.detach();
    }
}

void Terminal::set_raw_mode() {
    if (raw_mode_) {
        return;
    }

    if (tcgetattr(fd_, &original_termios_) < 0) {
        throw std::runtime_error("Failed to get terminal attributes");
    }

    struct termios raw = original_termios_;

    raw.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    raw.c_cflag &= ~(CSIZE | PARENB);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(fd_, TCSAFLUSH, &raw) < 0) {
        throw std::runtime_error("Failed to set terminal to raw mode");
    }

    raw_mode_ = true;

    std::setvbuf(stdout, nullptr, _IONBF, 0);

    if (!io_thread_.joinable()) {
        io_thread_ = std::thread(&Terminal::io_loop, this);
    }
}

void Terminal::unset_raw_mode() {
    if (!raw_mode_) {
        return;
    }

    if (tcsetattr(fd_, TCSANOW, &original_termios_) < 0) {
        throw std::runtime_error("Failed to restore terminal settings");
    }

    raw_mode_ = false;
}

bool Terminal::is_terminal(int fd) {
    return isatty(fd) != 0;
}

void Terminal::io_loop() {
    while (!stop_io_loop_) {
        char c;
        ssize_t n = ::read(fd_, &c, 1);

        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            break;
        }

        if (n == 0) {
            break;
        }

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            char_queue_.push(c);
        }
        queue_cv_.notify_one();
    }
}

#endif // _WIN32

std::optional<char> Terminal::read() {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    queue_cv_.wait(lock, [this] {
        return !char_queue_.empty() || stop_io_loop_;
    });

    if (stop_io_loop_ && char_queue_.empty()) {
        return std::nullopt;
    }

    char c = char_queue_.front();
    char_queue_.pop();
    return c;
}

} // namespace readline
