<<<<<<< HEAD:ej_repaso/ejercicio1/Makefile
BIN_FILES  = p1 p2 p3 
=======
BIN_FILES  = p1 p2
>>>>>>> b1de124 (lab2? xd):lab1/Makefile

CC = gcc

CFLAGS = -Wall

all: $(BIN_FILES)

p1: p1.o
	$(CC) $(CFLAGS) $^  -o $@

p2: p2.o
	$(CC) $(CFLAGS) $^  -o $@

<<<<<<< HEAD:ej_repaso/ejercicio1/Makefile
p3: p3.o
	$(CC) $(CFLAGS) $^  -o $@

=======
>>>>>>> b1de124 (lab2? xd):lab1/Makefile
%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(BIN_FILES) *.o

