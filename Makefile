SOURCES=main.c
LDFLAGS=-lSDL2
CFLAGS=-Wall -Werror -Wextra -g
CC=gcc

OBJECTS=$(SOURCES:.c=.o)
TARGET=FreeCandy

.PHONY: clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) $(TARGET) $(OBJECTS)

