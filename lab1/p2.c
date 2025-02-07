#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *end;
        int num = strtol(argv[i], &end, 10);

        if (*end != '\0') {
            printf("Argumento %d = Error de conversión.\n", i);
        } else {
            printf("Argumento %d = %d\n", i, num);
        }
    }
    return 0;
}
