#include <stdio.h>
#include <stdlib.h>

void ObtenerMinMax(int *array, int size, int *min, int *max) {
    if (size <= 0) return;

    *min = *max = array[0];

    for (int i = 1; i < size; i++) {
        if (array[i] < *min) *min = array[i];
        if (array[i] > *max) *max = array[i];
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s num1 num2 ... numN\n", argv[0]);
        return 1;
    }

    int *numeros = malloc((argc - 1) * sizeof(int));
    if (!numeros) {
        printf("Error: No se pudo asignar memoria.\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        char *end;
        numeros[i - 1] = strtol(argv[i], &end, 10);
        if (*end != '\0') {
            printf("Error de conversión en argumento %d: %s\n", i, argv[i]);
            free(numeros);
            return 1;
        }
    }

    int min, max;
    ObtenerMinMax(numeros, argc - 1, &min, &max);

    printf("Valor mínimo = %d\n", min);
    printf("Valor máximo = %d\n", max);

    free(numeros);
    return 0;
}
