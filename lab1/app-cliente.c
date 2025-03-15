#include <stdio.h>
#include "claves.h"

int main(void) {
    int key = 5;
    char *v1 = "ejemplo de valor 1";
    double v2[] = {2.3, 0.5, 23.45};
    struct Coord v3 = {10, 5};

    int err = set_value(key, v1, 3, v2, v3);
    if (err == -1) {
        printf("Error al insertar la tupla\n");
    } else if(err == -2) {
        printf("Error en la comunicación\n");
    } else {
        printf("Tupla insertada correctamente\n");
    }
    return 0;
}
