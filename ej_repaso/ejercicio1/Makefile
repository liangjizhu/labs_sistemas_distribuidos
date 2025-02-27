BIN_FILES  = p1 p2 p3 

CC = gcc

CFLAGS = -Wall

all: $(BIN_FILES)

p1: p1.o
	$(CC) $(CFLAGS) $^  -o $@

p2: p2.o
	$(CC) $(CFLAGS) $^  -o $@

p3: p3.o
	$(CC) $(CFLAGS) $^  -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(BIN_FILES) *.o

