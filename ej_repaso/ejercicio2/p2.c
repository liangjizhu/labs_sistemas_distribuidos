#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 2
#define ITER 10

pthread_mutex_t m;
pthread_cond_t cond;
int turno = 0; // Comienza el hilo 0

void *funcion(void *arg) {
    int mid = *(int *)arg;
    free(arg);  // Liberar memoria

    for (int j = 0; j < ITER; j++) {
        pthread_mutex_lock(&m);
        while (turno != mid) {  
            pthread_cond_wait(&cond, &m);  // Esperar turno
        }

        printf("Ejecuta el thread %d iteración %d\n", mid, j);

        turno = 1 - turno;  // Cambia turno
        pthread_cond_signal(&cond);  // Despertar al otro hilo
        pthread_mutex_unlock(&m);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t thid[NUM_THREADS];

    // Inicializar mutex y variable de condición
    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&cond, NULL);

    for (int j = 0; j < NUM_THREADS; j++) {
        int *id = malloc(sizeof(int));
        *id = j;
        pthread_create(&thid[j], NULL, funcion, id);
    }

    for (int j = 0; j < NUM_THREADS; j++) {
        pthread_join(thid[j], NULL);
    }

    // Destruir mutex y variable de condición
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&cond);

    return 0;
}
