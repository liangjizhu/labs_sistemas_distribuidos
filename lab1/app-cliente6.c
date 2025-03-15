#include <stdio.h>
#include "claves.h"

int main(void) {
    // Insertar algunas tuplas
    int err = set_value(500, "Tuple 500", 2, (double[]){10.1, 20.2}, (struct Coord){100, 200});
    if (err != 0) {
        printf("app-cliente_destroy: Error al insertar la tupla 500\n");
    } else {
        printf("app-cliente_destroy: Tupla 500 insertada.\n");
    }
    err = set_value(501, "Tuple 501", 1, (double[]){30.3}, (struct Coord){300, 400});
    if (err != 0) {
        printf("app-cliente_destroy: Error al insertar la tupla 501\n");
    } else {
        printf("app-cliente_destroy: Tupla 501 insertada.\n");
    }
    
    // Llamar a destroy para eliminar todas las tuplas
    err = destroy();
    if (err == 0) {
        printf("app-cliente_destroy: Todas las tuplas han sido destruidas.\n");
    } else if (err == -1) {
        printf("app-cliente_destroy: Error al destruir las tuplas\n");
    } else {
        printf("app-cliente_destroy: Error en la comunicación\n");
    }
    return 0;
}
