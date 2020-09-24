include config.mk

all: build

prepare:
	@${CARGO_BIN} build

build:
	@${CARGO_BIN} build

run:
	@${CARGO_BIN} run ${DATABASE} ${PORT}

clean:
	@${CARGO_BIN} clean
