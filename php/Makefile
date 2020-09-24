include config.mk

all: build

prepare:
	@composer install

build:
	php -l ${PROJECT}.php

run: build
	php -S localhost:${PORT} ${PROJECT}.php

clean:
	@rm -rf vendor
