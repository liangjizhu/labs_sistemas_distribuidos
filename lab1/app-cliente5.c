#include <stdio.h>
#include "claves.h"

int main(void) {
    int key = 400;
    char *v1 = "Existence test";
    double v2[] = {1.23, 4.56};
    struct Coord v3 = {20, 30};

    // Insertar la tupla
    int err = set_value(key, v1, 2, v2, v3);
    if (err != 0) {
        printf("app-cliente5: Error al insertar la tupla\n");
        return 1;
    }
    printf("app-cliente5: Tupla insertada.\n");

    // Comprobar existencia
    int exists = exist(key);
    if (exists == 1) {
        printf("app-cliente5: La clave %d existe.\n", key);
    } else if (exists == 0) {
        printf("app-cliente5: La clave %d no existe.\n", key);
    } else {
        printf("app-cliente5: Error en la comunicación\n");
    }
    return 0;
}
