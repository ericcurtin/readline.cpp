#include "readline/terminal.h"
#include "readline/errors.h"
#include <stdexcept>
#include <iostream>
#include <signal.h>
#include <cstdio>
#include <pthread.h>

namespace {
    // Dummy signal handler to interrupt blocking reads
    void sigusr1_handler(int) {
        // Do nothing - just interrupt the syscall
    }

    // Install signal handler on first use
    void install_signal_handler() {
        static bool installed = false;
        if (!installed) {
            struct sigaction sa;
            sa.sa_handler = sigusr1_handler;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sigaction(SIGUSR1, &sa, nullptr);
            installed = true;
        }
    }
}

namespace readline {

Terminal::Terminal()
    : fd_(STDIN_FILENO), raw_mode_(false), stop_io_loop_(false) {

    if (!is_terminal(fd_)) {
        throw std::runtime_error("stdin is not a terminal");
    }

    // Don't start I/O thread yet - will be started when needed
}

Terminal::~Terminal() {
    stop_io_loop_ = true;
    queue_cv_.notify_all();

    // Send a signal to unblock the read() call in the I/O thread
    if (io_thread_.joinable()) {
        // Give the thread a chance to exit naturally
        pthread_kill(io_thread_.native_handle(), SIGUSR1);
        io_thread_.join();
    }

    if (raw_mode_) {
        unset_raw_mode();
    }
}

void Terminal::set_raw_mode() {
    if (raw_mode_) {
        return;
    }

    // Get current terminal settings
    if (tcgetattr(fd_, &original_termios_) < 0) {
        throw std::runtime_error("Failed to get terminal attributes");
    }

    struct termios raw = original_termios_;

    // Set raw mode flags
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

    // Disable stdout buffering for immediate character display
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // Start I/O thread now that raw mode is set
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
