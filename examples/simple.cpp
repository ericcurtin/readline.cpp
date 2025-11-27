#include "readline/readline.h"
#include "readline/errors.h"
#include <iostream>
#include <string>
#include <algorithm>

int main() {
    readline::Prompt prompt;
    prompt.prompt = ">>> ";
    prompt.alt_prompt = "... ";
    prompt.placeholder = "Enter a command";

    try {
        readline::Readline rl(prompt);
        rl.history_enable();

        std::cout << "Welcome to the simple readline example!\n";
        std::cout << "Type 'history' to view command history\n";
        std::cout << "Type 'exit' or 'quit' to exit\n";
        std::cout << "Press Ctrl+C to interrupt\n";
        std::cout << "Press Ctrl+D on empty line to exit\n\n";

        while (true) {
            try {
                std::string line = rl.readline();

                // Trim whitespace
                line.erase(0, line.find_first_not_of(" \t\n\r"));
                line.erase(line.find_last_not_of(" \t\n\r") + 1);

                if (line.empty()) {
                    continue;
                }

                if (line == "exit" || line == "quit") {
                    std::cout << "Goodbye!\n";
                    break;
                }

                if (line == "history") {
                    auto* hist = rl.history();
                    std::cout << "Command history:\n";
                    for (size_t i = 0; i < hist->size(); ++i) {
                        std::cout << "  " << (i + 1) << ": ";
                        // Access history by going through prev/next
                        size_t old_pos = hist->pos;
                        hist->pos = i;
                        std::string hist_line = hist->prev();
                        hist->pos = old_pos;
                        std::cout << hist_line << "\n";
                    }
                    continue;
                }

                std::cout << "You entered: " << line << "\n";

            } catch (const readline::eof_error&) {
                std::cout << std::endl;
                break;
            } catch (const readline::interrupt_error&) {
                std::cout << "\n^C\n";
                continue;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
