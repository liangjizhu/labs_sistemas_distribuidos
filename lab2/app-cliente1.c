#include <stdio.h>
#include "claves.h"

int main(void) {
    int key = 100;
    char *v1 = "SetValue Test";
    double v2[] = {1.11, 2.22, 3.33};
    struct Coord v3 = {5, 10};

    int err = set_value(key, v1, 3, v2, v3);
    if (err == 0) {
        printf("app-cliente1: Tupla insertada correctamente (key=%d)\n", key);
    } else if (err == -1) {
        printf("app-cliente1: Error al insertar la tupla (error de aplicación)\n");
    } else {
        printf("app-cliente1: Error en la comunicación\n");
    }
    return 0;
}
