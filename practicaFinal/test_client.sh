#!/bin/bash

SERVER_IP="127.0.0.1"
SERVER_PORT="12345"
CLIENT="python3 client.py -s $SERVER_IP -p $SERVER_PORT"

# Comandos a ejecutar secuencialmente
COMMANDS=$(cat <<EOF
REGISTER paolo
REGISTER paolo
UNREGISTER nobody
UNREGISTER paolo
DISCONNECT paolo
REGISTER liang
CONNECT liang
PUBLISH /home/liang/file1.txt "Mi documento personal"
PUBLISH /home/liang/file1.txt "Intento duplicado"
LIST_USERS
LIST_CONTENT liang
DELETE /home/liang/file1.txt
LIST_CONTENT liang
DISCONNECT liang
QUIT
EOF
)

# Ejecutar cliente con input simulado
echo "$COMMANDS" | $CLIENT
