#!/bin/bash
set -e

# -------------------------------------------------------------------
# Configuración
# -------------------------------------------------------------------
SERVER_PORT=12345
CLIENT="python3 client.py -s 127.0.0.1 -p $SERVER_PORT"

# -------------------------------------------------------------------
# Limpieza de logs previos
# -------------------------------------------------------------------
rm -f rpc.log server.log datetime_service.log client_alice.log client_bob.log

# -------------------------------------------------------------------
# 1) Crear ficheros de prueba
# -------------------------------------------------------------------
mkdir -p input/alice input/bob
echo "Hola desde Alice" > input/alice/msg.txt
echo "Hola desde Bob"   > input/bob/msg.txt

# -------------------------------------------------------------------
# 2) Arrancar servicios en background
# -------------------------------------------------------------------
./log_server    > rpc.log              2>&1 &
RPC_PID=$!

python3 datetime_service.py > datetime_service.log 2>&1 &
DT_PID=$!

export LOG_RPC_IP=127.0.0.1
./server -p $SERVER_PORT > server.log 2>&1 &
SRV_PID=$!

sleep 1  # Deja que arranquen

# -------------------------------------------------------------------
# 3) Preparar comandos de Alice y Bob
# -------------------------------------------------------------------
cat > cmds_alice.txt <<'EOF'
REGISTER alice
sleep 1
CONNECT alice
sleep 1
PUBLISH input/alice/msg.txt "primer mensaje"
sleep 1
LIST_USERS
sleep 1
GET_FILE bob input/bob/msg.txt /tmp/alice_copy.txt
sleep 1
DISCONNECT alice
sleep 1
QUIT
EOF

cat > cmds_bob.txt <<'EOF'
sleep 0.5
REGISTER bob
sleep 1
CONNECT bob
sleep 1
PUBLISH input/bob/msg.txt "respuesta de bob"
sleep 1
LIST_CONTENT alice
sleep 1
GET_FILE alice input/alice/msg.txt /tmp/bob_copy.txt
sleep 1
DISCONNECT bob
sleep 1
QUIT
EOF

# -------------------------------------------------------------------
# 4) Lanzar ambos clientes en paralelo y capturar sus PIDs
# -------------------------------------------------------------------
(
  while IFS= read -r cmd; do
    if [[ "$cmd" =~ ^sleep ]]; then
      sleep ${cmd#sleep }
    else
      echo "$cmd"
    fi
  done < cmds_alice.txt
) | $CLIENT &> client_alice.log &
ALICE_PID=$!

(
  while IFS= read -r cmd; do
    if [[ "$cmd" =~ ^sleep ]]; then
      sleep ${cmd#sleep }
    else
      echo "$cmd"
    fi
  done < cmds_bob.txt
) | $CLIENT &> client_bob.log &
BOB_PID=$!

# -------------------------------------------------------------------
# 5) Esperar **solo** a los clientes
# -------------------------------------------------------------------
wait $ALICE_PID $BOB_PID

# -------------------------------------------------------------------
# 6) Parar servicios
# -------------------------------------------------------------------
kill $SRV_PID $DT_PID $RPC_PID 2>/dev/null || true
wait $SRV_PID $DT_PID $RPC_PID 2>/dev/null || true

# -------------------------------------------------------------------
# 7) Mostrar y verificar resultados
# -------------------------------------------------------------------
echo -e "\n--- server.log ---"
sed -n '1,200p' server.log

echo -e "\n--- rpc.log ---"
sed -n '1,200p' rpc.log

# Verificar contenidos copiados
echo
echo "alice_copy.txt => $(cat /tmp/alice_copy.txt)"
echo "bob_copy.txt   => $(cat /tmp/bob_copy.txt)"

echo
echo "+++ TEST DE CONCURRENCIA COMPLETADO +++"
