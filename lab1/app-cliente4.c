#include <stdio.h>
#include "claves.h"

int main(void) {
    int key = 300;
    char *v1 = "To be deleted";
    double v2[] = {9.99, 8.88};
    struct Coord v3 = {55, 65};

    // Insertar la tupla
    int err = set_value(key, v1, 2, v2, v3);
    if (err != 0) {
        printf("app-cliente_delete: Error al insertar la tupla\n");
        return 1;
    }
    printf("app-cliente_delete: Tupla insertada.\n");

    // Eliminar la tupla
    err = delete_key(key);
    if (err == 0) {
        printf("app-cliente_delete: Tupla borrada correctamente.\n");
    } else if (err == -1) {
        printf("app-cliente_delete: Error al borrar la tupla (clave no encontrada)\n");
    } else {
        printf("app-cliente_delete: Error en la comunicación\n");
    }
    return 0;
}
