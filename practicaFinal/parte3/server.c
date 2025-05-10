#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_USERS 100
#define MAX_FILES 100
#define MAX_NAME 256
#define MAX_DESC 256

typedef struct {
    char filename[MAX_NAME];
    char description[MAX_DESC];
} File;

typedef struct {
    char name[MAX_NAME];
    int connected;
    char ip[INET_ADDRSTRLEN];
    int port;
    File files[MAX_FILES];
    int file_count;
} User;

User users[MAX_USERS];
int user_count = 0;
pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_socket;


void handle_sigint(int sig) {
    printf("\ns> Shutting down server\n");
    close(server_socket);
    exit(0);
}

int read_string(int sock, char *buffer) {
    int i = 0;
    char c;
    while (recv(sock, &c, 1, 0) > 0 && c != '\0' && i < MAX_NAME - 1) {
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    return i > 0;
}

void *client_handler(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    char op[MAX_NAME];
    char ts[MAX_NAME];
    char user[MAX_NAME];
    if (!read_string(client_sock, op)) {
        close(client_sock);
        pthread_exit(NULL);
    }
    // Primero leo el timestamp que envía el cliente tras el opcode
    if (!read_string(client_sock, ts)) { 
        close(client_sock); 
        pthread_exit(NULL); 
    }
    // Luego leo el nombre de usuario
    if (!read_string(client_sock, user)) {
        close(client_sock);
        pthread_exit(NULL);
    }
    
    

    printf("s> OPERATION %s FROM %s AT %s\n", op, user, ts);
    // server.log
    fflush(stdout);
    pthread_mutex_lock(&user_mutex);

    // Buscar índice del usuario
    int user_idx = -1;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].name, user) == 0) {
            user_idx = i;
            break;
        }
    }

    if (strcmp(op, "REGISTER") == 0) {
        if (user_idx != -1) {
            unsigned char res = 1;
            send(client_sock, &res, 1, 0);
        } else if (user_count < MAX_USERS) {
            strncpy(users[user_count].name, user, MAX_NAME);
            users[user_count].connected = 0;
            users[user_count].file_count = 0;
            user_count++;
            unsigned char res = 0;
            send(client_sock, &res, 1, 0);
        } else {
            unsigned char res = 2;
            send(client_sock, &res, 1, 0);
        }
    }

    else if (strcmp(op, "UNREGISTER") == 0) {
        if (user_idx == -1) {
            unsigned char res = 1;
            send(client_sock, &res, 1, 0);
        } else {
            users[user_idx] = users[--user_count];
            unsigned char res = 0;
            send(client_sock, &res, 1, 0);
        }
    }

    else if (strcmp(op, "CONNECT") == 0) {
        char port_str[MAX_NAME];
        read_string(client_sock, port_str);

        if (user_idx == -1) {
            unsigned char res = 1;
            send(client_sock, &res, 1, 0);
        } else if (users[user_idx].connected) {
            unsigned char res = 2;
            send(client_sock, &res, 1, 0);
        } else {
            users[user_idx].connected = 1;

            // ← Nuevo código para IP real
            struct sockaddr_in peer;
            socklen_t peer_len = sizeof(peer);
            getpeername(client_sock, (struct sockaddr *)&peer, &peer_len);
            inet_ntop(AF_INET, &peer.sin_addr,
                    users[user_idx].ip, INET_ADDRSTRLEN);

            users[user_idx].port = atoi(port_str);
            unsigned char res = 0;
            send(client_sock, &res, 1, 0);
        }
    }


    else if (strcmp(op, "DISCONNECT") == 0) {
        if (user_idx == -1) {
            unsigned char res = 1;
            send(client_sock, &res, 1, 0);
        } else if (!users[user_idx].connected) {
            unsigned char res = 2;
            send(client_sock, &res, 1, 0);
        } else {
            users[user_idx].connected = 0;
            unsigned char res = 0;
            send(client_sock, &res, 1, 0);
        }
    }

    else if (strcmp(op, "PUBLISH") == 0) {
        char path[MAX_NAME];
        char desc[MAX_DESC];
        read_string(client_sock, path);
        read_string(client_sock, desc);

        if (user_idx == -1) {
            unsigned char res = 1;
            send(client_sock, &res, 1, 0);
        } else if (!users[user_idx].connected) {
            unsigned char res = 2;
            send(client_sock, &res, 1, 0);
        } else {
            for (int i = 0; i < users[user_idx].file_count; i++) {
                if (strcmp(users[user_idx].files[i].filename, path) == 0) {
                    unsigned char res = 3;
                    send(client_sock, &res, 1, 0);
                    goto END;
                }
            }
            File *f = &users[user_idx].files[users[user_idx].file_count++];
            strncpy(f->filename, path, MAX_NAME);
            strncpy(f->description, desc, MAX_DESC);
            unsigned char res = 0;
            send(client_sock, &res, 1, 0);
        }
    }

    else if (strcmp(op, "DELETE") == 0) {
        char path[MAX_NAME];
        read_string(client_sock, path);

        if (user_idx == -1) {
            unsigned char res = 1;
            send(client_sock, &res, 1, 0);
        } else if (!users[user_idx].connected) {
            unsigned char res = 2;
            send(client_sock, &res, 1, 0);
        } else {
            int found = 0;
            for (int i = 0; i < users[user_idx].file_count; i++) {
                if (strcmp(users[user_idx].files[i].filename, path) == 0) {
                    users[user_idx].files[i] = users[user_idx].files[--users[user_idx].file_count];
                    found = 1;
                    break;
                }
            }
            unsigned char res = found ? 0 : 3;
            send(client_sock, &res, 1, 0);
        }
    }

    else if (strcmp(op, "LIST USERS") == 0) {
        if (user_idx == -1) {
            unsigned char res = 1;
            send(client_sock, &res, 1, 0);
        } else if (!users[user_idx].connected) {
            unsigned char res = 2;
            send(client_sock, &res, 1, 0);
        } else {
            unsigned char res = 0;
            send(client_sock, &res, 1, 0);

            // 1) Contar sólo **otros** usuarios
            int count = 0;
            char num[10];
            for (int i = 0; i < user_count; i++)
                if (users[i].connected && i != user_idx)
                    count++;
            sprintf(num, "%d", count);
            send(client_sock, num, strlen(num) + 1, 0);

            // 2) Enviar sólo **otros** usuarios
            for (int i = 0; i < user_count; i++) {
                if (users[i].connected && i != user_idx) {
                    send(client_sock, users[i].name, strlen(users[i].name) + 1, 0);
                    send(client_sock, users[i].ip,   strlen(users[i].ip)   + 1, 0);
                    char port_str[10];
                    sprintf(port_str, "%d", users[i].port);
                    send(client_sock, port_str, strlen(port_str) + 1, 0);
                }
            }
        }
    }

    else if (strcmp(op, "LIST CONTENT") == 0) {
        char target[MAX_NAME];
        read_string(client_sock, target);
        int target_idx = -1;
        for (int i = 0; i < user_count; i++)
            if (strcmp(users[i].name, target) == 0)
                target_idx = i;

        if (user_idx == -1) {
            unsigned char res = 1;
            send(client_sock, &res, 1, 0);
        } else if (!users[user_idx].connected) {
            unsigned char res = 2;
            send(client_sock, &res, 1, 0);
        } else if (target_idx == -1) {
            unsigned char res = 3;
            send(client_sock, &res, 1, 0);
        } else {
            unsigned char res = 0;
            send(client_sock, &res, 1, 0);
            char num[10];
            sprintf(num, "%d", users[target_idx].file_count);
            send(client_sock, num, strlen(num) + 1, 0);
            for (int i = 0; i < users[target_idx].file_count; i++) {
                send(client_sock, users[target_idx].files[i].filename,
                     strlen(users[target_idx].files[i].filename) + 1, 0);
            }
        }
    }
    else if (strcmp(op, "GET USER INFO") == 0) {
        char target[MAX_NAME];
        read_string(client_sock, target);
    
        int target_idx = -1;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].name, target) == 0) {
                target_idx = i;
                break;
            }
        }
    
        if (user_idx == -1) {
            unsigned char res = 1;  // usuario que hace la petición no existe
            send(client_sock, &res, 1, 0);
        } else if (!users[user_idx].connected) {
            unsigned char res = 2;  // usuario que hace la petición no está conectado
            send(client_sock, &res, 1, 0);
        } else if (target_idx == -1 || !users[target_idx].connected) {
            unsigned char res = 3;  // usuario objetivo no existe o no está conectado
            send(client_sock, &res, 1, 0);
        } else {
            unsigned char res = 0;  // todo bien
            send(client_sock, &res, 1, 0);
            send(client_sock, users[target_idx].ip, strlen(users[target_idx].ip) + 1, 0);
            char port_str[10];
            sprintf(port_str, "%d", users[target_idx].port);
            send(client_sock, port_str, strlen(port_str) + 1, 0);
        }
    }
    

END:
    pthread_mutex_unlock(&user_mutex);
    close(client_sock);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    signal(SIGINT, handle_sigint);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("s> init server 0.0.0.0:%d\n", port);
    printf("s>\n");

    while (1) {
        int *client_sock = malloc(sizeof(int));
        *client_sock = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (*client_sock < 0) {
            perror("accept");
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_sock);
        pthread_detach(tid);
    }

    return 0;
}
