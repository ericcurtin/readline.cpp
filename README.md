# Readline Library

A readline implementation providing an interactive line editing interface with history support.

Available in both **Go** and **C++17**.

## Features

- Interactive line editing
- Command history with navigation (up/down arrows)
- Word-based navigation (Alt+B/Alt+F)
- Line editing commands (Ctrl+A, Ctrl+E, Ctrl+K, etc.)
- Bracket paste support
- Customizable prompts
- History persistence

## C++17 Version

### Building

The C++17 version uses CMake. To build:

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run the example
./simple_example
```

### Requirements

- C++17 compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- CMake 3.14 or higher
- POSIX-compliant system (Linux, macOS, BSD)

### Using the Library

```cpp
#include "readline/readline.h"
#include "readline/errors.h"
#include <iostream>

int main() {
    readline::Prompt prompt;
    prompt.prompt = ">>> ";
    prompt.alt_prompt = "... ";
    prompt.placeholder = "Enter a command";

    try {
        readline::Readline rl(prompt);
        rl.history_enable();

        while (true) {
            try {
                std::string line = rl.readline();
                std::cout << "You entered: " << line << "\n";
            } catch (const readline::eof_error&) {
                break;
            } catch (const readline::interrupt_error&) {
                std::cout << "^C\n";
                continue;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
```

## Go Version

### Building

The project uses Go modules. To build:

```bash
# Install dependencies
go mod download

# Build the example application
cd examples
go build -o simple simple.go
```

### Running the Example

```bash
# From the examples directory
./simple
```

The example application demonstrates basic readline functionality:
- Type commands and press Enter
- Use arrow keys to navigate through command history
- Type `history` to view command history
- Type `exit` or `quit` to exit
- Press Ctrl+C to interrupt

### Using the Library

```go
package main

import (
    "fmt"
    "io"

    "github.com/ecurtin/readline/readline.go"
)

func main() {
    rl, err := readline.New(readline.Prompt{
        Prompt:      ">>> ",
        AltPrompt:   "... ",
        Placeholder: "Enter a command",
    })
    if err != nil {
        panic(err)
    }

    rl.HistoryEnable()

    for {
        line, err := rl.Readline()
        if err == io.EOF || err == readline.ErrInterrupt {
            break
        }

        fmt.Printf("You entered: %s\n", line)
    }
}
```

### Dependencies (Go)

- `github.com/emirpasic/gods/v2` - Data structures
- `github.com/mattn/go-runewidth` - Unicode text width calculation
- `golang.org/x/term` - Terminal handling

## Project Structure

```
readline.cpp/
├── include/readline/     # C++ headers
│   ├── buffer.h
│   ├── errors.h
│   ├── history.h
│   ├── readline.h
│   ├── terminal.h
│   └── types.h
├── src/                  # C++ implementation
│   ├── buffer.cpp
│   ├── history.cpp
│   ├── readline.cpp
│   └── terminal.cpp
├── examples_cpp/         # C++ examples
│   └── simple.cpp
├── readline.go/          # Go implementation
│   ├── buffer.go
│   ├── errors.go
│   ├── history.go
│   ├── readline.go
│   ├── term.go
│   └── types.go
├── examples/             # Go examples (binaries)
├── CMakeLists.txt        # C++ build configuration
└── go.mod               # Go dependencies
```

## License

See LICENSE file for details.
