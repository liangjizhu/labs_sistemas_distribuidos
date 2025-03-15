# Makefile para el Ejercicio 1: Sistema Distribuido con colas de mensajes POSIX

CC = gcc
CFLAGS = -Wall -g -fPIC
LDFLAGS = -lrt -lpthread

# Objetivos principales
all: servidor-mq libclaves.so app-cliente

# Compilación del servidor (servidor-mq.c y claves.c)
servidor-mq: servidor-mq.o claves.o
	$(CC) -o $@ servidor-mq.o claves.o $(LDFLAGS)

# Generación de la biblioteca dinámica a partir de proxy-mq.c y claves.c
libclaves.so: proxy-mq.o
	$(CC) -shared -o $@ proxy-mq.o $(LDFLAGS)

# Compilación del cliente, que se enlaza con libclaves.so
app-cliente: app-cliente.o
	$(CC) -o $@ app-cliente.o -L. -lclaves $(LDFLAGS)

# Reglas de compilación de cada archivo objeto
servidor-mq.o: servidor-mq.c claves.h
	$(CC) $(CFLAGS) -c servidor-mq.c

proxy-mq.o: proxy-mq.c claves.h
	$(CC) $(CFLAGS) -c proxy-mq.c

claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c claves.c

app-cliente.o: app-cliente.c claves.h
	$(CC) $(CFLAGS) -c app-cliente.c

# Limpieza de archivos generados
clean:
	rm -f *.o servidor app-cliente libclaves.so

.PHONY: all clean
