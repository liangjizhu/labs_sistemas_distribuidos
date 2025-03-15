# Makefile para el Ejercicio 1: Sistema Distribuido con colas de mensajes POSIX

CC = gcc
CFLAGS = -Wall -g -fPIC
LDFLAGS = -lrt -lpthread

# Objetivos principales
all: servidor-mq libclaves.so app-cliente1 app-cliente2 app-cliente3 app-cliente4 app-cliente5 app-cliente6

# Compilación del servidor (servidor-mq.c y claves.c)
servidor-mq: servidor-mq.o claves.o
	$(CC) -o $@ servidor-mq.o claves.o $(LDFLAGS)

# Generación de la biblioteca dinámica a partir de proxy-mq.c
libclaves.so: proxy-mq.o
	$(CC) -shared -o $@ proxy-mq.o $(LDFLAGS)

# Compilación de los clientes, que se enlazan con libclaves.so
app-cliente1: app-cliente1.o
	$(CC) -o $@ app-cliente1.o -L. -lclaves $(LDFLAGS)

app-cliente2: app-cliente2.o
	$(CC) -o $@ app-cliente2.o -L. -lclaves $(LDFLAGS)

app-cliente3: app-cliente3.o
	$(CC) -o $@ app-cliente3.o -L. -lclaves $(LDFLAGS)

app-cliente4: app-cliente4.o
	$(CC) -o $@ app-cliente4.o -L. -lclaves $(LDFLAGS)

app-cliente5: app-cliente5.o
	$(CC) -o $@ app-cliente5.o -L. -lclaves $(LDFLAGS)

app-cliente6: app-cliente6.o
	$(CC) -o $@ app-cliente6.o -L. -lclaves $(LDFLAGS)

# Reglas de compilación de cada archivo objeto
servidor-mq.o: servidor-mq.c claves.h
	$(CC) $(CFLAGS) -c servidor-mq.c

proxy-mq.o: proxy-mq.c claves.h
	$(CC) $(CFLAGS) -c proxy-mq.c

claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c claves.c

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

# Limpieza de archivos generados
clean:
	rm -f *.o servidor-mq libclaves.so app-cliente1 app-cliente2 app-cliente3 app-cliente4 app-cliente5 app-cliente6

.PHONY: all clean
