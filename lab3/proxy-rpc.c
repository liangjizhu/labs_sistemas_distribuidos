/* proxy-rpc.c
 * Biblioteca cliente "libclaves_rpc.so" que envuelve llamadas RPC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "tuplas.h"    /* rpcgen-generated */

/* Prototipos de la API original */
int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3);
int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3);
int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3);
int delete_key(int key);
int exist(int key);
int destroy(void);


static const char *get_server_host(void) {
    char *h = getenv("IP_TUPLAS");
    return h ? h : "localhost";
}


/* set_value */
int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    CLIENT *cl;
    enum clnt_stat stat;
    int result;

    cl = clnt_create(get_server_host(), TUPLAS_PROG, TUPLAS_VERS, "tcp");
    if (!cl) {
        clnt_pcreateerror("set_value: clnt_create");
        return -1;
    }

    /* Preparar argumentos RPC */
    set_args args;
    args.key      = key;
    args.value1   = value1;
    args.N_value2 = N_value2;
    args.value3   = value3;
    args.V_value2.V_value2_len = N_value2;
    args.V_value2.V_value2_val = V_value2;

    /* CORRECCIÓN: pasar args por valor, no &args */
    stat = set_value_1(args, &result, cl);
    if (stat != RPC_SUCCESS) {
        clnt_perror(cl, "set_value RPC failed");
        result = -1;
    }

    clnt_destroy(cl);
    return result;
}


/* get_value */
int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    CLIENT *cl;
    enum clnt_stat stat;
    get_result gres;

    cl = clnt_create(get_server_host(), TUPLAS_PROG, TUPLAS_VERS, "tcp");
    if (!cl) {
        clnt_pcreateerror("get_value: clnt_create");
        return -1;
    }

    if (stat != RPC_SUCCESS) {
        clnt_perror(cl, "get_value RPC failed");
        clnt_destroy(cl);
        return -2;  // Error de comunicación
    }
    
    if (gres.status != 0) {
        clnt_destroy(cl);
        return -1;  // Error de aplicación (key no encontrada)
    }
    
    /* Copiar resultados a la API, con seguridad */
    if (gres.value1 != NULL) {
        strncpy(value1, gres.value1, 256);
        value1[255] = '\0';
    } else {
        value1[0] = '\0';
    }
    
    *N_value2 = gres.N_value2;
    if (gres.V_value2.V_value2_val != NULL && gres.N_value2 > 0) {
        memcpy(V_value2, gres.V_value2.V_value2_val, sizeof(double) * gres.N_value2);
    } else {
        *N_value2 = 0;
    }
    
    if (value3 != NULL) {
        *value3 = gres.value3;
    }

    xdr_free((xdrproc_t)xdr_get_result, (char *)&gres);
    clnt_destroy(cl);
    return 0;
}


/* modify_value */
int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    CLIENT *cl;
    enum clnt_stat stat;
    int result;

    cl = clnt_create(get_server_host(), TUPLAS_PROG, TUPLAS_VERS, "tcp");
    if (!cl) {
        clnt_pcreateerror("modify_value: clnt_create");
        return -1;
    }

    modify_args args;
    args.key      = key;
    args.value1   = value1;
    args.N_value2 = N_value2;
    args.value3   = value3;
    args.V_value2.V_value2_len = N_value2;
    args.V_value2.V_value2_val = V_value2;

    /* CORRECCIÓN: pasar args por valor, no &args */
    stat = modify_value_1(args, &result, cl);
    if (stat != RPC_SUCCESS) {
        clnt_perror(cl, "modify_value RPC failed");
        result = -1;
    }

    clnt_destroy(cl);
    return result;
}


/* delete_key */
int delete_key(int key) {
    CLIENT *cl;
    enum clnt_stat stat;
    int result;

    cl = clnt_create(get_server_host(), TUPLAS_PROG, TUPLAS_VERS, "tcp");
    if (!cl) {
        clnt_pcreateerror("delete_key: clnt_create");
        return -1;
    }

    stat = delete_key_1(key, &result, cl);
    if (stat != RPC_SUCCESS) {
        clnt_perror(cl, "delete_key RPC failed");
        result = -1;
    }

    clnt_destroy(cl);
    return result;
}


/* exist */
int exist(int key) {
    CLIENT *cl;
    enum clnt_stat stat;
    int result;

    cl = clnt_create(get_server_host(), TUPLAS_PROG, TUPLAS_VERS, "tcp");
    if (!cl) {
        clnt_pcreateerror("exist: clnt_create");
        return -1;
    }

    stat = exist_1(key, &result, cl);
    if (stat != RPC_SUCCESS) {
        clnt_perror(cl, "exist RPC failed");
        result = -1;
    }

    clnt_destroy(cl);
    return result;
}


/* destroy */
int destroy(void) {
    CLIENT *cl;
    enum clnt_stat stat;
    int result;

    cl = clnt_create(get_server_host(), TUPLAS_PROG, TUPLAS_VERS, "tcp");
    if (!cl) {
        clnt_pcreateerror("destroy: clnt_create");
        return -1;
    }

    /* CORRECCIÓN: la firma es destroy_1(int *result, CLIENT *clnt) */
    stat = destroy_1(&result, cl);
    if (stat != RPC_SUCCESS) {
        clnt_perror(cl, "destroy RPC failed");
        result = -1;
    }

    clnt_destroy(cl);
    return result;
}
