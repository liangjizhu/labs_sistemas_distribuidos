#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "claves.h"
#include "tuplas.h"  // generado por rpcgen desde tuplas.x

int *set_value_1_svc(Tupla *argp, struct svc_req *rqstp) {
    static int result;

    result = set_value(argp->key, argp->value1, argp->N_value2, argp->V_value2, argp->value3);
    return &result;
}

Tupla_Resultado *get_value_1_svc(int *key, struct svc_req *rqstp) {
    static Tupla_Resultado result;
    static double valores[32];  // para evitar problemas al retornar punteros a memoria temporal
    char value1[256];
    int N_value2;
    struct Coord coord;

    memset(&result, 0, sizeof(result));  // Limpia la estructura

    int status = get_value(*key, value1, &N_value2, valores, &coord);

    result.status = status;
    if (status == 0) {
        result.tupla.key = *key;
        strncpy(result.tupla.value1, value1, sizeof(result.tupla.value1));
        result.tupla.N_value2 = N_value2;
        result.tupla.V_value2 = valores;
        result.tupla.value3 = coord;
    }

    return &result;
}

int *modify_value_1_svc(Tupla *argp, struct svc_req *rqstp) {
    static int result;

    result = modify_value(argp->key, argp->value1, argp->N_value2, argp->V_value2, argp->value3);
    return &result;
}

int *delete_key_1_svc(int *key, struct svc_req *rqstp) {
    static int result;

    result = delete_key(*key);
    return &result;
}

int *exist_1_svc(int *key, struct svc_req *rqstp) {
    static int result;

    result = exist(*key);
    return &result;
}

int *destroy_1_svc(void *argp, struct svc_req *rqstp) {
    static int result;

    result = destroy();
    return &result;
}
