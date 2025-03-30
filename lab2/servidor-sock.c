#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
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

    // bind() -> vincular el socket con la dirección y puerto configurados
    err = bind(sd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    if (err == -1) {
		printf("Error en bind\n");
		return -1;
	}

    // listen() -> para poner al servidor a la espera de conexiones
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
            if (set_value(key, value1, N_value2, V_value2, value3) == 0) {
                status = 0;
            }
        
        
        end:
            send(sc, &status, sizeof(char), 0);
            free(value1);
            free(V_value2);
            close(sc);
            continue;
        }

        if (op_code == OP_GET_VALUE) {
            int32_t key_net;
            if (recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
                perror("recv key");
                close(sc);
                continue;
            }
            int key = ntohl(key_net);
        
            // Llamar a la función get_value para obtener la tupla
            char value1[256];
            int N_value2;
            double V_value2[32];
            struct Coord value3;
            int res = get_value(key, value1, &N_value2, V_value2, &value3);
        
            // Enviar status: 0 si get_value fue exitoso, 1 si error
            char status = (res == 0) ? 0 : 1;
            if (send(sc, &status, sizeof(char), 0) < 0) {
                perror("send status");
                close(sc);
                continue;
            }
        
            if (res == 0) {
                // Enviar longitud de value1
                int32_t len = strlen(value1);
                int32_t len_net = htonl(len);
                if (send(sc, &len_net, sizeof(int32_t), 0) < 0) {
                    perror("send value1 length");
                    close(sc);
                    continue;
                }
                // Enviar la cadena value1
                if (send(sc, value1, len, 0) < 0) {
                    perror("send value1");
                    close(sc);
                    continue;
                }
                // Enviar N_value2
                int32_t n_net = htonl(N_value2);
                if (send(sc, &n_net, sizeof(int32_t), 0) < 0) {
                    perror("send N_value2");
                    close(sc);
                    continue;
                }
                // Enviar cada elemento del vector V_value2
                for (int i = 0; i < N_value2; i++) {
                    uint64_t d;
                    memcpy(&d, &V_value2[i], sizeof(double));
                    d = htobe64(d);
                    if (send(sc, &d, sizeof(uint64_t), 0) < 0) {
                        perror("send V_value2 element");
                        close(sc);
                        continue;
                    }
                }
                // Enviar las coordenadas
                int32_t x_net = htonl(value3.x);
                int32_t y_net = htonl(value3.y);
                if (send(sc, &x_net, sizeof(int32_t), 0) < 0 ||
                    send(sc, &y_net, sizeof(int32_t), 0) < 0) {
                    perror("send value3");
                    close(sc);
                    continue;
                }
            }
        }

        if (op_code == OP_MODIFY_VALUE) {
            int32_t key_net;
            if (recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
                perror("recv key");
                close(sc);
                continue;
            }
            int key = ntohl(key_net);
        
            // 1. Recibir value1: primero la longitud (int32_t) y luego la cadena
            int32_t len_net;
            if (recv(sc, &len_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
                perror("recv value1 length");
                close(sc);
                continue;
            }
            int len = ntohl(len_net);
            char *value1 = malloc(len + 1);
            if (!value1) {
                perror("malloc value1");
                close(sc);
                continue;
            }
            if (recv(sc, value1, len, MSG_WAITALL) <= 0) {
                perror("recv value1");
                free(value1);
                close(sc);
                continue;
            }
            value1[len] = '\0';
        
            // 2. Recibir N_value2 (int32_t)
            int32_t nval_net;
            if (recv(sc, &nval_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
                perror("recv N_value2");
                free(value1);
                close(sc);
                continue;
            }
            int N_value2 = ntohl(nval_net);
        
            // 3. Recibir el vector V_value2 (cada elemento se envía como un uint64_t en orden de red)
            double *V_value2 = malloc(sizeof(double) * N_value2);
            if (!V_value2) {
                perror("malloc V_value2");
                free(value1);
                close(sc);
                continue;
            }
            for (int i = 0; i < N_value2; i++) {
                uint64_t d_net;
                if (recv(sc, &d_net, sizeof(uint64_t), MSG_WAITALL) <= 0) {
                    perror("recv V_value2 element");
                    free(value1);
                    free(V_value2);
                    close(sc);
                    continue;
                }
                uint64_t d_host = be64toh(d_net);
                memcpy(&V_value2[i], &d_host, sizeof(double));
            }
        
            // 4. Recibir value3: dos int32_t (x e y)
            int32_t x_net, y_net;
            if (recv(sc, &x_net, sizeof(int32_t), MSG_WAITALL) <= 0 ||
                recv(sc, &y_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
                perror("recv value3");
                free(value1);
                free(V_value2);
                close(sc);
                continue;
            }
            struct Coord value3;
            value3.x = ntohl(x_net);
            value3.y = ntohl(y_net);
        
            // Llamar a modify_value (implementada en claves.c)
            int res = modify_value(key, value1, N_value2, V_value2, value3);
        
            // Enviar respuesta al cliente: 0 para éxito, 1 para error
            char status = (res == 0) ? 0 : 1;
            if (send(sc, &status, sizeof(char), 0) < 0) {
                perror("send status");
            }
        
            free(value1);
            free(V_value2);
            close(sc);
            continue;
        }

        if (op_code == OP_EXIST) {
            int32_t key_net;
            if (recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
                perror("recv key");
                close(sc);
                continue;
            }
            int key = ntohl(key_net);
            int res = exist(key);
        
            // Enviar status: enviar directamente el valor devuelto por exist
            char status = (char) res;
            if (send(sc, &status, sizeof(char), 0) < 0) {
                perror("send status");
                close(sc);
                continue;
            }
        }

        if (op_code == OP_DELETE_KEY) {
            int32_t key_net;
            if (recv(sc, &key_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
                perror("recv key");
                close(sc);
                continue;
            }
            int key = ntohl(key_net);
            int res = delete_key(key);

            // Enviar status: 0 si la eliminación fue exitosa, -1 en caso de error.
            char status = (res == 0) ? 0 : -1;
            if (send(sc, &status, sizeof(char), 0) < 0) {
                perror("send status");
                close(sc);
                continue;
            }
        }

        if (op_code == OP_DESTROY) {
            int res = destroy();
            // Enviar status: 0 si se destruyeron las tuplas correctamente, -1 en caso de error.
            char status = (res == 0) ? 0 : -1;
            if (send(sc, &status, sizeof(char), 0) < 0) {
                perror("send status");
                close(sc);
                continue;
            }
        }
        close(sc);
    }

    close(sd);
    return 0;
}