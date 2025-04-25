#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "claves.h"
#include "tuplas.h" 

CLIENT *cliente = NULL;

// Inicializa el cliente RPC
int init() {
    char *ip = getenv("IP_TUPLAS");
    if (!ip) {
        fprintf(stderr, "❌ Error: variable IP_TUPLAS no definida.\n");
        return -1;
    }

    cliente = clnt_create(ip, TUPLAS_PROG, TUPLAS_VERS, "tcp");
    if (!cliente) {
        clnt_pcreateerror("❌ Error al crear cliente RPC");
        return -1;
    }

    return 0;
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (!cliente && init() != 0) return -1;

    set_args args;
    args.key = key;
    args.value1 = value1;
    args.N_value2 = N_value2;

    // Copiar el array de valores
    for (int i = 0; i < N_value2 && i < 32; i++) {
        args.V_value2[i] = V_value2[i];
    }

    args.coord = value3;

    int *res = set_value_1(&args, cliente);
    return (res ? *res : -1);
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    if (!cliente && init() != 0) return -1;

    int arg = key;
    get_res *res = get_value_1(&arg, cliente);

    if (!res || res->status != 0) return -1;

    strcpy(value1, res->value1);
    *N_value2 = res->N_value2;
    for (int i = 0; i < *N_value2 && i < 32; i++) {
        V_value2[i] = res->V_value2[i];
    }
    *value3 = res->coord;

    return 0;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (!cliente && init() != 0) return -1;

    modify_args args;
    args.key = key;
    args.value1 = value1;
    args.N_value2 = N_value2;
    for (int i = 0; i < N_value2 && i < 32; i++) {
        args.V_value2[i] = V_value2[i];
    }
    args.coord = value3;

    int *res = modify_value_1(&args, cliente);
    return (res ? *res : -1);
}

int delete_key(int key) {
    if (!cliente && init() != 0) return -1;

    int *res = delete_key_1(&key, cliente);
    return (res ? *res : -1);
}

int exist(int key) {
    if (!cliente && init() != 0) return -1;

    int *res = exist_1(&key, cliente);
    return (res ? *res : -1);
}

int destroy() {
    if (!cliente && init() != 0) return -1;

    int *res = destroy_1(NULL, cliente);
    return (res ? *res : -1);
}
