# Makefile para el Ejercicio 2: Sockets TCP

CC = gcc
CFLAGS = -Wall -g -fPIC
LDFLAGS = -lpthread

# Objetivo all: compila el servidor, la biblioteca y todos los clientes
all: servidor-sock libclaves.so app-cliente1 app-cliente2 app-cliente3 app-cliente4 app-cliente5 app-cliente6

# Compilación del servidor (servidor-sock.c y claves.c)
servidor-sock: servidor-sock.o claves.o
	$(CC) -o $@ servidor-sock.o claves.o $(LDFLAGS)

# Generación de la biblioteca dinámica a partir de proxy-sock.c
libclaves.so: proxy-sock.o
	$(CC) -shared -o $@ proxy-sock.o $(LDFLAGS)

# Compilación de los clientes, que se enlazan con libclaves.so
app-cliente1: app-cliente1.o
	$(CC) -o $@ app-cliente1.o -L. -lclaves -Wl,-rpath=. $(LDFLAGS)

app-cliente2: app-cliente2.o
	$(CC) -o $@ app-cliente2.o -L. -lclaves -Wl,-rpath=. $(LDFLAGS)

app-cliente3: app-cliente3.o
	$(CC) -o $@ app-cliente3.o -L. -lclaves -Wl,-rpath=. $(LDFLAGS)

app-cliente4: app-cliente4.o
	$(CC) -o $@ app-cliente4.o -L. -lclaves -Wl,-rpath=. $(LDFLAGS)

app-cliente5: app-cliente5.o
	$(CC) -o $@ app-cliente5.o -L. -lclaves -Wl,-rpath=. $(LDFLAGS)

app-cliente6: app-cliente6.o
	$(CC) -o $@ app-cliente6.o -L. -lclaves -Wl,-rpath=. $(LDFLAGS)

# Reglas de compilación de archivos objeto
servidor-sock.o: servidor-sock.c claves.h
	$(CC) $(CFLAGS) -c servidor-sock.c

claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c claves.c

proxy-sock.o: proxy-sock.c claves.h
	$(CC) $(CFLAGS) -c proxy-sock.c

app-cliente1.o: app-cliente1.c claves.h
	$(CC) $(CFLAGS) -c app-cliente1.c

app-cliente2.o: app-cliente2.c claves.h
	$(CC) $(CFLAGS) -c app-cliente2.c

app-cliente3.o: app-cliente3.c claves.h
	$(CC) $(CFLAGS) -c app-cliente3.c

app-cliente4.o: app-cliente4.c claves.h
	$(CC) $(CFLAGS) -c app-cliente4.c

app-cliente5.o: app-cliente5.c claves.h
	$(CC) $(CFLAGS) -c app-cliente5.c

app-cliente6.o: app-cliente6.c claves.h
	$(CC) $(CFLAGS) -c app-cliente6.c

# Objetivo clean para eliminar archivos generados
clean:
	rm -f *.o servidor-sock libclaves.so app-cliente1 app-cliente2 app-cliente3 app-cliente4 app-cliente5 app-cliente6

.PHONY: all clean
