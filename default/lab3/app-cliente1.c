#include <stdio.h>
#include "claves.h"

int main(void) {
    const int total_tuples = 1000;

    for (int i = 0; i < total_tuples; i++) {
        int key = 100 + i;

        char value1[64];
        snprintf(value1, sizeof(value1), "User_%d", i);

        double v2[] = {1.1 * i, 2.2 * i, 3.3 * i};
        struct Coord v3 = {i * 10, i * 20};

        int err = set_value(key, value1, 3, v2, v3);
        if (err == 0) {
            printf("Tupla insertada (key=%d, value1=%s, Coord=(%d,%d))\n",
                   key, value1, v3.x, v3.y);
        } else if (err == -1) {
            printf("❌ Error al insertar la tupla (key=%d, error de aplicación)\n", key);
        } else {
            printf("❌ Error en la comunicación (key=%d)\n", key);
        }
    }

    return 0;
}