PROJECT := app

LIBSQLITE3_LIBS = `pkg-config --libs sqlite3`
LIBSQLITE3_CFLAGS = `pkg-config --cflags sqlite3`

# LIBCJSON_LIBS = -lcjson
# LIBCJSON_CFLAGS =

CC       := gcc
CFLAGS   := -g -Wall -Wextra -pedantic ${LIBSQLITE3_CFLAGS} ${LIBCJSON_CFLAGS} -Wno-unused-parameter
LD       := $(CC)
LDFLAGS  := -std=c99 -g
LIBS     := -lm ${LIBSQLITE3_LIBS} ${LIBCJSON_LIBS}

SOURCE     := $(wildcard *.c)
OBJECTS    := $(SOURCE:%.c=%.o)
DEPENDS    := $(SOURCE:%.c=%.d)

DATABASE ?= tasks.db
PORT ?= 5000
