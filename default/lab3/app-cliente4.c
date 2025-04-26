#include <stdio.h>
#include "claves.h"

int main(void) {
    const int total_deletes = 100;

    for (int i = 0; i < total_deletes; i++) {
        int key = 400 + i;  // Debe existir previamente

        int err = delete_key(key);
        if (err == 0) {
            printf("Tupla borrada (key=%d)\n", key);
        } else if (err == -1) {
            printf("app-cliente4: Tupla no encontrada (key=%d)\n", key);
        } else {
            printf("app-cliente4: Error en (key=%d)\n", key);
        }
    }

    return 0;
}
