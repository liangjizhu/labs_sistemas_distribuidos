#include <stdio.h>
#include "claves.h"

int main(void) {
    const int total_tuples = 100;

    for (int i = 0; i < total_tuples; i++) {
        int key = 100 + i;
        char value1[256];
        int N_value2;
        double V_value2[32];
        struct Coord v3;

        int err = get_value(key, value1, &N_value2, V_value2, &v3);
        if (err == 0) {
            printf("Tupla leída: key = %d \n", key);
            printf("   - value1: %s\n", value1);
            printf("   - N_value2: %d\n", N_value2);
            printf("   - V_value2: ");
            for (int j = 0; j < N_value2; j++) {
                printf("%.2lf ", V_value2[j]);
            }
            printf("\n   - Coord: (%d, %d)\n\n", v3.x, v3.y);
        } else if (err == -1) {
            printf("❌ app-cliente2: Tupla no encontrada (key=%d)\n", key);
        } else {
            printf("❌ app-cliente2: Error en la comunicación (key=%d)\n", key);
        }
    }

    return 0;
}
