# Porting Notes: Go to C++17

This document describes the porting process from Go to C++17 for the readline library.

## Architecture

The C++17 port maintains the same modular architecture as the Go version:

- **types.h**: Constants and ANSI escape sequences
- **errors.h**: Exception classes (interrupt_error, eof_error)
- **history.h/cpp**: Command history management with persistence
- **terminal.h/cpp**: Raw mode terminal handling using POSIX termios
- **buffer.h/cpp**: Line editing buffer with Unicode support
- **readline.h/cpp**: Main interface coordinating all components

## Key Design Decisions

### 1. Unicode Handling
- Go's `rune` type → C++ `char32_t` and `std::u32string`
- Custom UTF-8 ↔ UTF-32 conversion functions
- Simplified CJK character width detection (full ICU would be ideal)

### 2. Threading
- Go's goroutines and channels → C++ `std::thread` with `std::queue` and `std::mutex`
- Terminal I/O loop runs in separate thread
- Thread-safe character queue with condition variables

### 3. Memory Management
- Go's garbage collection → C++17 smart pointers (`std::unique_ptr`)
- RAII for terminal state management
- Automatic cleanup in destructors

### 4. Error Handling
- Go's multiple return values → C++ exceptions
- `std::optional` for nullable values
- Clear exception types for EOF and interrupt

### 5. Standard Library
- Go's `arraylist` → C++ `std::vector`
- Go's `os` package → C++ `<filesystem>` (C++17)
- Go's `term` package → POSIX `termios`

## Dependencies

### Go Version
- `github.com/emirpasic/gods/v2` - Data structures
- `github.com/mattn/go-runewidth` - Unicode width
- `golang.org/x/term` - Terminal handling

### C++ Version
- C++17 standard library only
- POSIX termios for terminal control
- No external dependencies

## Features

Both versions support:
- Interactive line editing
- Command history with persistence
- Arrow key navigation (up/down/left/right)
- Word-based navigation (Alt+B/Alt+F)
- Ctrl commands (Ctrl+A, Ctrl+E, Ctrl+K, etc.)
- Bracket paste mode
- Customizable prompts
- Multi-line editing with word wrap

## Platform Support

- **Go version**: Cross-platform (Linux, macOS, BSD, Windows)
- **C++ version**: POSIX systems (Linux, macOS, BSD)
  - Windows support would require separate implementation

## Build System

- **Go**: Native `go build` with modules
- **C++**: CMake-based build system with proper installation support

## Future Improvements

1. Full Unicode width calculation using ICU library
2. Windows support via Windows Console API
3. Tab completion support
4. Syntax highlighting hooks
5. Better multi-line editing support
6. History search (Ctrl+R)

## API Comparison

### Go
```go
rl, err := readline.New(readline.Prompt{
    Prompt: ">>> ",
})
rl.HistoryEnable()
line, err := rl.Readline()
```

### C++
```cpp
readline::Prompt prompt;
prompt.prompt = ">>> ";
readline::Readline rl(prompt);
rl.history_enable();
std::string line = rl.readline();
```

## Testing

Build and run the example:
```bash
mkdir build && cd build
cmake ..
cmake --build .
./simple_example
```
