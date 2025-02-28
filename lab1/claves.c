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