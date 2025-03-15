#include <stdio.h>
#include "claves.h"

int main(void) {
    int key = 100;  // Se asume que la tupla con key=100 ya fue insertada.
    char buffer[256];
    int n;
    double v2[32];
    struct Coord v3;
    
    int err = get_value(key, buffer, &n, v2, &v3);
    if (err == 0) {
        printf("app-cliente_get: Tupla recuperada:\n");
        printf("  value1: %s\n", buffer);
        printf("  N_value2: %d\n", n);
        printf("  v2: ");
        for (int i = 0; i < n; i++) {
            printf("%lf ", v2[i]);
        }
        printf("\n  Coord: (%d, %d)\n", v3.x, v3.y);
    } else if (err == -1) {
        printf("app-cliente_get: Error al obtener la tupla (clave no encontrada)\n");
    } else {
        printf("app-cliente_get: Error en la comunicación\n");
    }
    return 0;
}
