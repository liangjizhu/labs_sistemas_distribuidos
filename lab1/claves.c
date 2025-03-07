/* Gestiona el almacenamiento y procesamiento de las tuplas*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>
#include "claves.h"

typedef struct {
    int key;
    char value1[256];
    int N_value2;
    int V_value2[32];
    struct Coord value3;
}Tupla;

#define MAX_TUPLAS 100
static Tupla tuplas[MAX_TUPLAS];
static int num_tuplas = 0;

mqd_t mq_cliente, mq_servidor;


int destroy() {
    num_tuplas = 0;
    return 0;
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (strlen(value1) > 255 || N_value2 < 1 || N_value2 > 32) {
        return -1;
    }

    // Verificar si la clave ya existe
    for (int i = 0; i < num_tuplas; i++) {
        if (tuplas[i].key == key) {
            return -1;  // Clave ya existente
        }
    }

    // Insertar la nueva tupla
    if (num_tuplas < MAX_TUPLAS) {
        tuplas[num_tuplas].key = key;
        strncpy(tuplas[num_tuplas].value1, value1, 255);
        tuplas[num_tuplas].N_value2 = N_value2;
        memcpy(tuplas[num_tuplas].V_value2, V_value2, sizeof(double) * N_value2);
        tuplas[num_tuplas].value3 = value3;
        num_tuplas++;
        return 0;  // Éxito
    }

    return -1;  // Error: lista llena
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    for (int i = 0; i < num_tuplas; i++) {
        if (tuplas[i].key == key) {
            strncpy(value1, tuplas[i].value1, 255);
            *N_value2 = tuplas[i].N_value2;
            memcpy(V_value2, tuplas[i].V_value2, sizeof(double) * tuplas[i].N_value2);
            *value3 = tuplas[i].value3;
            return 0;  // Éxito
        }
    }
    return -1;  // Error: clave no encontrada
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    for (int i = 0; i < num_tuplas; i++) {
        if (tuplas[i].key == key) {
            strncpy(tuplas[i].value1, value1, 255);
            tuplas[i].N_value2 = N_value2;
            memcpy(tuplas[i].V_value2, V_value2, sizeof(double) * N_value2);
            tuplas[i].value3 = value3;
            return 0;
        }
    }
    return -1;
}

int delete_key(int key) {
    for (int i = 0; i < num_tuplas; i++) {
        if (tuplas[i].key == key) {
            // Hay que borras la tupla desplazando los otros 
            for (int j = i; j < num_tuplas - 1; j++) {
                tuplas[j] = tuplas[j + 1];
            }
            num_tuplas--;
            return 0;  
        }
    }
    return -1;  // clave no encontrada
}


int exist(int key) {
    for (int i = 0; i < num_tuplas; i++) {
        if (tuplas[i].key == key) {
            return 1;  // Existe :)
        }
    }
    return 0;  // No existe :(
}

int init_queues() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(int);
    attr.mq_curmsgs = 0;

    mq_cliente = mq_open("/mq_cliente", O_CREAT | O_RDONLY, 0644, &attr);
    if (mq_cliente == (mqd_t) -1) {
        perror("Error al abrir la cola de cliente");
        return -1;
    }

    mq_servidor = mq_open("/mq_servidor", O_CREAT | O_WRONLY, 0644, &attr);
    if (mq_servidor == (mqd_t) -1) {
        perror("Error al abrir la cola del servidor");
        return -1;
    }

    return 0;  // Éxito
}



