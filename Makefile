CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc

ifdef MSYSTEM
	CFLAGS += -D_XOPEN_SOURCE_EXTENDED=1
	LIBS    = -lncursesw
else ifeq ($(OS),Windows_NT)
	LIBS = -lpdcurses
else
	CFLAGS += -D_XOPEN_SOURCE_EXTENDED=1
	LIBS    = -lncursesw
endif

SRCS   = src/main.c src/board.c src/moves.c src/engine.c src/render.c src/input.c src/game.c
OBJS   = $(SRCS:.c=.o)
TARGET = tuichess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean
