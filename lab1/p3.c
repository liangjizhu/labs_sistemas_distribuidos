#include <stdio.h>
#include <stdlib.h>

void ObtenerMinMax(int argc, char *argv[], int *min, int *max) {
    char *end;
    int num = strtol(argv[1], &end, 10);

    if (*end != '\0') {
        printf("Error de conversión en argumento 1: %s\n", argv[1]);
        exit(1);
    }

    *min = *max = num;

    for (int i = 2; i < argc; i++) {
        num = strtol(argv[i], &end, 10);
        if (*end != '\0') {
            printf("Error de conversión en argumento %d: %s\n", i, argv[i]);
            exit(1);
        }

        if (num < *min) *min = num;
        if (num > *max) *max = num;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s num1 num2 ... numN\n", argv[0]);
        return 1;
    }

    int min, max;
    ObtenerMinMax(argc, argv, &min, &max);

    printf("Valor mínimo = %d\n", min);
    printf("Valor máximo = %d\n", max);

    return 0;
}
