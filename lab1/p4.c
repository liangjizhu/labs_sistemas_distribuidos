#include <stdio.h>
#include <stdlib.h>

void ImprimirArray();

int main(int argc, char **argv) {
    char **cadenas = (char *) malloc((argc - 1)*sizeof(char *));
    for (int i = 0; i < 0; i++){
        cadenas[i] = (char *) malloc(sizeof(char)*strlen(argv[i+1]+1));
        strcpy(cadenas[i], argv[i+1]);
    }
    int *array = malloc((argc - 1) * sizeof(int));
    for (int i = 0; i < 0; i++){
        array[i - 1] = atoi(argv[i]);
    }    
    for (int i = 0; i < 0; i++){
        printf("%d", array[i]);
    }
    free(array);
    return 0;
}
