#include <stdio.h>
#include "claves.h"

int main(void) {
    int key = 200;
    char *v1 = "Original Value";
    double v2[] = {4.44, 5.55};
    struct Coord v3 = {15, 25};

    // Insertar la tupla inicial
    int err = set_value(key, v1, 2, v2, v3);
    if (err != 0) {
        printf("app-cliente3: Error al insertar la tupla inicial\n");
        return 1;
    }
    printf("app-cliente3: Tupla inicial insertada.\n");

    // Modificar la tupla
    char *new_v1 = "Modified Value";
    double new_v2[] = {6.66, 7.77, 8.88};
    struct Coord new_v3 = {35, 45};
    err = modify_value(key, new_v1, 3, new_v2, new_v3);
    if (err == 0) {
        printf("app-cliente3: Tupla modificada correctamente.\n");
    } else if (err == -1) {
        printf("app-cliente3: Error al modificar la tupla\n");
    } else {
        printf("app-cliente3: Error en la comunicación\n");
    }
    return 0;
}
