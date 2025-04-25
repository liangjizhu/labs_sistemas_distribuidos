#include <stdio.h>
#include "claves.h"

int main(void) {
    const int total_mods = 100;

    for (int i = 0; i < total_mods; i++) {
        int key = 100 + i;  // Claves metidas por appcliente1 :0

        char new_value1[64];
        snprintf(new_value1, sizeof(new_value1), "Modified_User_%d", i);

        double new_v2[] = {9.99 + i, 8.88 + i, 7.77 + i};
        struct Coord new_coord = {100 + i, 200 + i};

        int err = modify_value(key, new_value1, 3, new_v2, new_coord);
        if (err == 0) {
            printf("Tupla modificada: key=%d, value1=%s, Coord=(%d,%d))\n",
                   key, new_value1, new_coord.x, new_coord.y);
        } else if (err == -1) {
            printf("app-cliente3: Error al modificar la tupla (key=%d)\n", key);
        } else {
            printf("app-cliente3: Error en la comunicación (key=%d)\n", key);
        }
    }

    return 0;
}
