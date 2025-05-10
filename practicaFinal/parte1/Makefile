CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pthread
TARGET  = server

.PHONY: all clean

all: $(TARGET)

$(TARGET): server.c
	$(CC) $(CFLAGS) -o $(TARGET) server.c

clean:
	rm -f $(TARGET)
