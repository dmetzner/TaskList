include config.mk

GOPATH=$(shell pwd)
GOBIN=$(shell pwd)/bin

all: build

prepare:
	@GOPATH=${GOPATH} GOBIN=${GOBIN} go get

build:
	@GOPATH=${GOPATH} GOBIN=${GOBIN} go build -o ${PROJECT} ${GOFILES}

run:
	@./$(PROJECT) ${DATABASE} ${PORT}

clean:
	@GOPATH=${GOPATH} GOBIN=${GOBIN} go clean
