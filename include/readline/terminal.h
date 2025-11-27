#pragma once

#include <termios.h>
#include <unistd.h>
#include <optional>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace readline {

class Terminal {
public:
    Terminal();
    ~Terminal();

    void set_raw_mode();
    void unset_raw_mode();
    bool is_raw_mode() const { return raw_mode_; }
    std::optional<char> read();
    bool is_terminal(int fd);

private:
    void io_loop();

    int fd_;
    bool raw_mode_;
    struct termios original_termios_;
    std::thread io_thread_;
    std::queue<char> char_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> stop_io_loop_;
};

} // namespace readline
