#include <stdio.h>
#include "claves.h"

int main(void) {
    const int total_checks = 100;

    for (int i = 0; i < total_checks; i++) {
        int key = 100 + i;

        int exists = exist(key);
        if (exists == 1) {
            printf("La clave %d EXISTE ✅\n", key);
        } else if (exists == 0) {
            printf("La clave %d NO EXISTE\n", key);
        } else {
            printf("Error en la comunicación con el servidor (key=%d)\n", key);
        }
    }

    return 0;
}
