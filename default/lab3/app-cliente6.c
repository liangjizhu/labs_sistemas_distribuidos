#include <stdio.h>
#include "claves.h"

int main(void) {
    // Llamar a destroy para eliminar todas las tuplas
    int err = destroy();
    if (err == 0) {
        printf("app-cliente6: Todas las tuplas han sido destruidas.\n");
    } else if (err == -1) {
        printf("app-cliente6: Error al destruir las tuplas\n");
    } else {
        printf("app-cliente6: Error en la comunicación\n");
    }
    return 0;
}
