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
REGISTER liang
DISCONNECT liang
QUIT
EOF
)

# Ejecutar cliente con input simulado
echo "$COMMANDS" | $CLIENT
