.PHONY: all build clean run test deps

all: build

deps:
	go mod download

build: deps
	go build -o examples/simple.go examples/simple.go

run: build
	examples/simple.go

clean:
	rm -f examples/simple.go

test:
	go test ./...
