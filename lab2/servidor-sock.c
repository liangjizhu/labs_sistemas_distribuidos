#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "claves.h"

#define OP_SET_VALUE 1
#define OP_GET_VALUE 2
#define OP_MODIFY_VALUE 3
#define OP_EXIST 4
#define OP_DELETE_KEY 5
#define OP_DESTROY 6

char *recv_string(int sc) {
    int32_t len_net;
    if (recv(sc, &len_net, sizeof(int32_t), MSG_WAITALL) <= 0) return NULL;
    int32_t len = ntohl(len_net);

    char *str = malloc(len + 1);
    if (!str) return NULL;

    if (recv(sc, str, len, MSG_WAITALL) <= 0) {
        free(str);
        return NULL;
    }
    str[len] = '\0';
    return str;
}

void *atender_cliente(void *arg) {
    int sc = *((int *)arg);
    free(arg);

    struct sockaddr_in client_addr;
    socklen_t size = sizeof(client_addr);
    getpeername(sc, (struct sockaddr *)&client_addr, &size);
    printf("Conexión aceptada de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    char op_code;
    if (recv(sc, &op_code, sizeof(char), MSG_WAITALL) <= 0) {
        perror("recv op_code");
        close(sc);
        return NULL;
    }

    if (op_code == OP_SET_VALUE) {
        int32_t key_net, n_val2_net;
        int32_t x_net, y_net;
        int key, N_value2;
        double *V_value2 = NULL;
        struct Coord value3;
        char *value1 = NULL;
        char status = 1;

        if (recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL) <= 0) goto end;
        key = ntohl(key_net);
        value1 = recv_string(sc);
        if (!value1) goto end;
        if (recv(sc, &n_val2_net, sizeof(int32_t), MSG_WAITALL) <= 0) goto end;
        N_value2 = ntohl(n_val2_net);

        V_value2 = malloc(sizeof(double) * N_value2);
        if (!V_value2) goto end;
        for (int i = 0; i < N_value2; i++) {
            uint64_t d_net;
            if (recv(sc, &d_net, sizeof(uint64_t), MSG_WAITALL) <= 0) goto end;
            uint64_t d_host = be64toh(d_net);
            memcpy(&V_value2[i], &d_host, sizeof(double));
        }

        if (recv(sc, &x_net, sizeof(int32_t), MSG_WAITALL) <= 0) goto end;
        if (recv(sc, &y_net, sizeof(int32_t), MSG_WAITALL) <= 0) goto end;
        value3.x = ntohl(x_net);
        value3.y = ntohl(y_net);

        if (set_value(key, value1, N_value2, V_value2, value3) == 0) status = 0;

end:
        send(sc, &status, sizeof(char), 0);
        free(value1);
        free(V_value2);
    }

    else if (op_code == OP_GET_VALUE) {
        int32_t key_net;
        if (recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL) <= 0) return NULL;
        int key = ntohl(key_net);

        char value1[256];
        int N_value2;
        double V_value2[32];
        struct Coord value3;
        int res = get_value(key, value1, &N_value2, V_value2, &value3);

        char status = (res == 0) ? 0 : 1;
        send(sc, &status, sizeof(char), 0);

        if (res == 0) {
            int32_t len = strlen(value1);
            int32_t len_net = htonl(len);
            send(sc, &len_net, sizeof(int32_t), 0);
            send(sc, value1, len, 0);
            int32_t n_net = htonl(N_value2);
            send(sc, &n_net, sizeof(int32_t), 0);
            for (int i = 0; i < N_value2; i++) {
                uint64_t d;
                memcpy(&d, &V_value2[i], sizeof(double));
                d = htobe64(d);
                send(sc, &d, sizeof(uint64_t), 0);
            }
            int32_t x_net = htonl(value3.x);
            int32_t y_net = htonl(value3.y);
            send(sc, &x_net, sizeof(int32_t), 0);
            send(sc, &y_net, sizeof(int32_t), 0);
        }
    }

    else if (op_code == OP_MODIFY_VALUE) {
        int32_t key_net;
        recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL);
        int key = ntohl(key_net);

        int32_t len_net;
        recv(sc, &len_net, sizeof(int32_t), MSG_WAITALL);
        int len = ntohl(len_net);
        char *value1 = malloc(len + 1);
        recv(sc, value1, len, MSG_WAITALL);
        value1[len] = '\0';

        int32_t nval_net;
        recv(sc, &nval_net, sizeof(int32_t), MSG_WAITALL);
        int N_value2 = ntohl(nval_net);

        double *V_value2 = malloc(sizeof(double) * N_value2);
        for (int i = 0; i < N_value2; i++) {
            uint64_t d_net;
            recv(sc, &d_net, sizeof(uint64_t), MSG_WAITALL);
            uint64_t d_host = be64toh(d_net);
            memcpy(&V_value2[i], &d_host, sizeof(double));
        }

        int32_t x_net, y_net;
        recv(sc, &x_net, sizeof(int32_t), MSG_WAITALL);
        recv(sc, &y_net, sizeof(int32_t), MSG_WAITALL);
        struct Coord value3 = { ntohl(x_net), ntohl(y_net) };

        int res = modify_value(key, value1, N_value2, V_value2, value3);
        char status = (res == 0) ? 0 : 1;
        send(sc, &status, sizeof(char), 0);

        free(value1);
        free(V_value2);
    }

    else if (op_code == OP_EXIST) {
        int32_t key_net;
        recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL);
        int key = ntohl(key_net);
        int res = exist(key);
        char status = (char)res;
        send(sc, &status, sizeof(char), 0);
    }

    else if (op_code == OP_DELETE_KEY) {
        int32_t key_net;
        recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL);
        int key = ntohl(key_net);
        int res = delete_key(key);
        char status = (res == 0) ? 0 : -1;
        send(sc, &status, sizeof(char), 0);
    }

    else if (op_code == OP_DESTROY) {
        int res = destroy();
        char status = (res == 0) ? 0 : -1;
        send(sc, &status, sizeof(char), 0);
    }

    close(sc);
    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t size;
    int sd, err;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    int val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4500);

    if (bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return -1;
    }

    if (listen(sd, SOMAXCONN) < 0) {
        perror("listen");
        return -1;
    }

    printf("Servidor escuchando en el puerto 4500...\n");
    size = sizeof(client_addr);

    while (1) {
        int sc = accept(sd, (struct sockaddr *)&client_addr, &size);
        if (sc < 0) {
            perror("accept");
            continue;
        }

        int *sock_ptr = malloc(sizeof(int));
        *sock_ptr = sc;

        pthread_t tid;
        if (pthread_create(&tid, NULL, atender_cliente, sock_ptr) != 0) {
            perror("pthread_create");
            close(sc);
            free(sock_ptr);
        } else {
            pthread_detach(tid);
        }
    }

    close(sd);
    return 0;
}