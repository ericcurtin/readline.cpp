# Readline Library for Go

A readline implementation in Go that provides an interactive line editing interface with history support.

## Features

- Interactive line editing
- Command history with navigation (up/down arrows)
- Word-based navigation (Alt+B/Alt+F)
- Line editing commands (Ctrl+A, Ctrl+E, Ctrl+K, etc.)
- Bracket paste support
- Customizable prompts
- History persistence

## Building

The project uses Go modules. To build:

```bash
# Install dependencies
go mod download

# Build the example application
cd examples
go build -o simple simple.go
```

## Running the Example

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

## Using the Library

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

## Dependencies

- `github.com/emirpasic/gods/v2` - Data structures
- `github.com/mattn/go-runewidth` - Unicode text width calculation
- `golang.org/x/term` - Terminal handling
