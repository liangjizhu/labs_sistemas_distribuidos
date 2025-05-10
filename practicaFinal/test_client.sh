#!/bin/bash

SERVER_IP="127.0.0.1"
SERVER_PORT="12345"
CLIENT="python3 client.py -s $SERVER_IP -p $SERVER_PORT"

# Crear archivos de prueba en directorio local
mkdir -p input/paolo
mkdir -p input/liang
echo "Contenido de prueba de Paolo" > input/paolo/file1.txt
echo "Contenido de prueba de Liang" > input/liang/file1.txt

# Comandos a ejecutar secuencialmente
COMMANDS=$(cat <<EOF
REGISTER paolo
REGISTER paolo
UNREGISTER nobody
UNREGISTER paolo
DISCONNECT paolo
REGISTER liang
CONNECT liang
PUBLISH input/liang/file1.txt Mi documento personal
PUBLISH input/liang/file1.txt Intento duplicado
LIST_USERS
LIST_CONTENT liang
LIST_CONTENT nobody
REGISTER paolo
CONNECT paolo
LIST_USERS
GET_FILE liang input/liang/file1.txt /tmp/copied_file.txt
DELETE input/liang/file1.txt
LIST_CONTENT paolo
DISCONNECT liang
DISCONNECT paolo
QUIT
EOF
)

# Ejecutar cliente con input simulado
echo "$COMMANDS" | $CLIENT