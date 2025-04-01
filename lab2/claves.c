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
    double V_value2[32];
    struct Coord value3;
}Tupla;

Tupla *tuplas = NULL;
int num_tuplas = 0;
int capacidad = 0;

extern mqd_t mq_cliente;
extern mqd_t mq_servidor;



int destroy() {
    free(tuplas);
    tuplas = NULL;
    num_tuplas = 0;
    capacidad = 0;
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

    // Redimensionar si es necesario
    if (num_tuplas == capacidad) {
        int nueva_capacidad = (capacidad == 0) ? 10 : capacidad * 2;
        Tupla *nueva_memoria = realloc(tuplas, nueva_capacidad * sizeof(Tupla));
        if (!nueva_memoria) {
            return -1; // Error al reservar memoria
        }
        tuplas = nueva_memoria;
        capacidad = nueva_capacidad;
    }

    // Insertar la nueva tupla
    tuplas[num_tuplas].key = key;
    strncpy(tuplas[num_tuplas].value1, value1, 255);
    tuplas[num_tuplas].value1[255] = '\0';  // Por seguridad
    tuplas[num_tuplas].N_value2 = N_value2;
    memcpy(tuplas[num_tuplas].V_value2, V_value2, sizeof(double) * N_value2);
    tuplas[num_tuplas].value3 = value3;
    num_tuplas++;
    return 0;
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