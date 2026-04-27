CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -O2
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS := $(shell sdl2-config --libs)

TARGET := pong
SOURCES := pong.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ $^ $(SDL_LIBS)

clean:
	rm -f $(TARGET)
