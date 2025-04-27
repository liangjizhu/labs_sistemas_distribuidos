/* servidor-rpc.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tuplas.h" // Archivo generado por rpcgen

#define MAX_TUPLAS 1024

// Estructura de almacenamiento de las tuplas
typedef struct {
    int key;
    char value1[256];
    int N_value2;
    double V_value2[256];
    struct Coord value3;
    int ocupado; // 1 si ocupado, 0 si libre
} Tupla;

static Tupla base_datos[MAX_TUPLAS];

// Buscar índice de key
static int buscar_key(int key) {
    for (int i = 0; i < MAX_TUPLAS; i++) {
        if (base_datos[i].ocupado && base_datos[i].key == key)
            return i;
    }
    return -1;
}

// Buscar primer hueco libre
static int buscar_hueco() {
    for (int i = 0; i < MAX_TUPLAS; i++) {
        if (!base_datos[i].ocupado)
            return i;
    }
    return -1;
}

// Implementaciones de las funciones RPC

bool_t set_value_1_svc(set_args args, int *result, struct svc_req *req) {
    int idx = buscar_key(args.key);
    if (idx != -1) {
        *result = -1; // Ya existe
        return TRUE;
    }
    idx = buscar_hueco();
    if (idx == -1) {
        *result = -1; // No hay espacio
        return TRUE;
    }
    base_datos[idx].key = args.key;
    strncpy(base_datos[idx].value1, args.value1, sizeof(base_datos[idx].value1)-1);
    base_datos[idx].value1[255] = '\0';
    base_datos[idx].N_value2 = args.N_value2;
    memcpy(base_datos[idx].V_value2, args.V_value2.V_value2_val, sizeof(double) * args.N_value2);
    base_datos[idx].value3 = args.value3;
    base_datos[idx].ocupado = 1;
    *result = 0;
    return TRUE;
}

bool_t get_value_1_svc(int key, get_result *result, struct svc_req *req) {
    int idx = buscar_key(key);
    if (idx == -1) {
        result->status = -1;
        return TRUE;
    }
    result->status = 0;
    result->value1 = strdup(base_datos[idx].value1);
    result->N_value2 = base_datos[idx].N_value2;
    result->V_value2.V_value2_len = base_datos[idx].N_value2;
    result->V_value2.V_value2_val = malloc(sizeof(double) * base_datos[idx].N_value2);
    memcpy(result->V_value2.V_value2_val, base_datos[idx].V_value2, sizeof(double) * base_datos[idx].N_value2);
    result->value3 = base_datos[idx].value3;
    return TRUE;
}

bool_t modify_value_1_svc(modify_args args, int *result, struct svc_req *req) {
    int idx = buscar_key(args.key);
    if (idx == -1) {
        *result = -1; // No existe
        return TRUE;
    }
    strncpy(base_datos[idx].value1, args.value1, sizeof(base_datos[idx].value1)-1);
    base_datos[idx].value1[255] = '\0';
    base_datos[idx].N_value2 = args.N_value2;
    memcpy(base_datos[idx].V_value2, args.V_value2.V_value2_val, sizeof(double) * args.N_value2);
    base_datos[idx].value3 = args.value3;
    *result = 0;
    return TRUE;
}

bool_t delete_key_1_svc(int key, int *result, struct svc_req *req) {
    int idx = buscar_key(key);
    if (idx == -1) {
        *result = -1;
        return TRUE;
    }
    base_datos[idx].ocupado = 0;
    *result = 0;
    return TRUE;
}

bool_t exist_1_svc(int key, int *result, struct svc_req *req) {
    int idx = buscar_key(key);
    *result = (idx != -1) ? 1 : 0;
    return TRUE;
}

bool_t destroy_1_svc(int *result, struct svc_req *req) {
    for (int i = 0; i < MAX_TUPLAS; i++) {
        base_datos[i].ocupado = 0;
    }
    *result = 0;
    return TRUE;
}

bool_t tuplas_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result) {
    (void) transp;

    if (xdr_result == (xdrproc_t)xdr_get_result) {
        get_result *res = (get_result *)result;
        if (res != NULL) {
            if (res->value1 != NULL) {
                free(res->value1);
                res->value1 = NULL;
            }
            if (res->V_value2.V_value2_val != NULL) {
                free(res->V_value2.V_value2_val);
                res->V_value2.V_value2_val = NULL;
            }
        }
    }
    return TRUE;
}


