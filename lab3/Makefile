# Makefile

# -----------------------------------
# Variables de compilación (tal como en tu .sh)
# -----------------------------------
CC      := cc
CFLAGS  := -D_REENTRANT -g -I/usr/include/tirpc
LDLIBS  := -lnsl -lpthread -ldl -ltirpc
RPCGENFLAGS := -N -M

IDL         := tuplas.x
GEN_SRCS    := tuplas.h tuplas_clnt.c tuplas_svc.c tuplas_xdr.c

# proxy + librería compartida
PROXY_SRC   := proxy-rpc.c
LIB_NAME    := libclaves.so

# clientes de aplicación
CLIENT_APPS := app-cliente1 app-cliente2 app-cliente3 app-cliente4 app-cliente5 app-cliente6

# -----------------------------------
# Reglas principales
# -----------------------------------
.PHONY: all clean

all: stubs server libclaves $(CLIENT_APPS)

# 1) Generar stubs XDR/RPC
$(GEN_SRCS): $(IDL)
	rpcgen $(RPCGENFLAGS) $(IDL)

stubs: $(GEN_SRCS)

# 2) Servidor RPC
tuplas_xdr.o: tuplas_xdr.c tuplas.h
	$(CC) $(CFLAGS) -c $<

tuplas_svc.o: tuplas_svc.c tuplas.h
	$(CC) $(CFLAGS) -c $<

tuplas_server.o: tuplas_server.c tuplas.h
	$(CC) $(CFLAGS) -c $<

server: tuplas_server.o tuplas_svc.o tuplas_xdr.o
	$(CC) -o tuplas_server tuplas_server.o tuplas_svc.o tuplas_xdr.o $(LDLIBS)

# 3) Librería compartida libclaves.so
tuplas_clnt.o: tuplas_clnt.c tuplas.h
	$(CC) $(CFLAGS) -c $<

proxy-rpc.o: $(PROXY_SRC) tuplas_clnt.o tuplas_xdr.o
	$(CC) $(CFLAGS) -fPIC -c $(PROXY_SRC) -o proxy-rpc.o

libclaves: proxy-rpc.o tuplas_clnt.o tuplas_xdr.o
	$(CC) -shared -o $(LIB_NAME) proxy-rpc.o tuplas_clnt.o tuplas_xdr.o $(LDLIBS)

# 4) Clientes de aplicación
app-cliente%.o: app-cliente%.c tuplas_clnt.o tuplas_xdr.o
	$(CC) $(CFLAGS) -c $< -o $@

app-cliente%: app-cliente%.o libclaves
	$(CC) -o $@ $< -L. -lclaves -Wl,-rpath=. $(LDLIBS)

# 5) Limpiar todo
clean:
	rm -f core \
	      $(GEN_SRCS) \
	      *.o \
	      tuplas_server \
	      $(LIB_NAME) \
	      $(CLIENT_APPS)
