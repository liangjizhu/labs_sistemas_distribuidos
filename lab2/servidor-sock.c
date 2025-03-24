#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "claves.h"

#define OP_SET_VALUE 1


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


int main(int argc, char *argv []) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t size;
    int sd, sc;
    int val;
    char op;
    int32_t a, b, res;
    int err;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) <0) {
        printf ("SERVIDOR: Error en el socket");
        return (0);
    }
    val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4200);

    err = bind(sd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    if (err == -1) {
		printf("Error en bind\n");
		return -1;
	}

    	err = listen(sd, SOMAXCONN);
	if (err == -1) {
		printf("Error en listen\n");
		return -1;
	}

    size = sizeof(client_addr);

    while (1) {
        printf("Esperando conexión...\n");
        sc = accept(sd, (struct sockaddr *)&client_addr, &size);
        if (sc == -1) {
            perror("accept");
            continue;
        }

        printf("Conexión aceptada de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Leer el op_code
        char op_code;
        if (recv(sc, &op_code, sizeof(char), MSG_WAITALL) <= 0) {
            perror("recv op_code");
            close(sc);
            continue;
        }

        if (op_code == OP_SET_VALUE) {
            int32_t key_net, n_val2_net;
            int32_t x_net, y_net;
            int key, N_value2;
            double *V_value2 = NULL;
            struct Coord value3;
            char *value1 = NULL;
            char status = 1;
        
            // 1. Recibir key
            if (recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL) <= 0) goto end;
            key = ntohl(key_net);
        
            // 2. Recibir value1
            value1 = recv_string(sc);
            if (!value1) goto end;
        
            // 3. Recibir N_value2
            if (recv(sc, &n_val2_net, sizeof(int32_t), MSG_WAITALL) <= 0) goto end;
            N_value2 = ntohl(n_val2_net);
        
            // 4. Recibir array de N_value2 doubles
            V_value2 = malloc(sizeof(double) * N_value2);
            if (!V_value2) goto end;
            for (int i = 0; i < N_value2; i++) {
                uint64_t d_net;
                if (recv(sc, &d_net, sizeof(uint64_t), MSG_WAITALL) <= 0) goto end;
                uint64_t d_host = be64toh(d_net);
                memcpy(&V_value2[i], &d_host, sizeof(double));
            }
        
            // 5. Recibir struct Coord (x, y)
            if (recv(sc, &x_net, sizeof(int32_t), MSG_WAITALL) <= 0) goto end;
            if (recv(sc, &y_net, sizeof(int32_t), MSG_WAITALL) <= 0) goto end;
            value3.x = ntohl(x_net);
            value3.y = ntohl(y_net);
        
            // 6. Llamar la función
            if (set_value(key, value1, N_value2, V_value2, value3) == 0)
                status = 0;
        
        end:
            send(sc, &status, sizeof(char), 0);
            free(value1);
            free(V_value2);
            close(sc);
            continue;
        }
        close(sc);
    }

    close(sd);
    return 0;
}