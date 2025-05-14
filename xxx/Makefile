CC      = gcc
CFLAGS  = -Wall -pthread -I/usr/include/tirpc
RPCLIB  = -lnsl -ltirpc

# RPC generated files
RPC_HDR     = log.h
RPC_CLNT    = log_clnt.c
RPC_SVC     = log_svc.c
RPC_XDR     = log_xdr.c

# P2P server
SRV_SRC     = server.c
SRV_OBJS    = server.o log_clnt.o log_xdr.o

# RPC log server
RPC_SVC_OBJS= log_svc.o log_xdr.o log_server.o

all: server log_server

# 1) genera los stubs
$(RPC_HDR) $(RPC_CLNT) $(RPC_SVC) $(RPC_XDR): log.x
	rpcgen log.x

# 2) Compila todos los .o
%.o: %.c $(RPC_HDR)
	$(CC) $(CFLAGS) -c $< -o $@

# 3) Enlaza el P2P server (cliente RPC)
server: $(SRV_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(RPCLIB)

# 4) Enlaza el log_server RPC
log_server: $(RPC_SVC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(RPCLIB)

clean:
	rm -f *.o server log_server $(RPC_HDR) $(RPC_CLNT) $(RPC_SVC) $(RPC_XDR)
