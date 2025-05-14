#!/bin/bash
set -euo pipefail

# -------------------------------------------------------------------
# Configuración
# -------------------------------------------------------------------
SERVER_PORT=12345
CLIENT="python3 client.py -s 127.0.0.1 -p $SERVER_PORT"

# -------------------------------------------------------------------
# Logs y temporales
# -------------------------------------------------------------------
RPC_LOG=rpc.log
SERVER_LOG=server.log
DT_LOG=datetime_service.log
ALICE_LOG=client_alice.log
BOB_LOG=client_bob.log

# -------------------------------------------------------------------
# Cleanup on exit
# -------------------------------------------------------------------
cleanup() {
    echo ">>> Deteniendo servicios..."
    kill ${SRV_PID:-} ${DT_PID:-} ${RPC_PID:-} 2>/dev/null || true
    lsof -ti tcp:5000 | xargs -r kill
}
trap cleanup EXIT

# -------------------------------------------------------------------
# 0) Limpieza inicial
# -------------------------------------------------------------------
rm -f $RPC_LOG $SERVER_LOG $DT_LOG $ALICE_LOG $BOB_LOG
rm -rf input/alice input/bob /tmp/alice_copy.txt /tmp/bob_copy.txt

# -------------------------------------------------------------------
# 1) Crear ficheros de prueba
# -------------------------------------------------------------------
mkdir -p input/alice input/bob
echo "Hola desde Alice" > input/alice/msg.txt
echo "Hola desde Bob"   > input/bob/msg.txt

# -------------------------------------------------------------------
# 2) Arrancar servicios
# -------------------------------------------------------------------
echo ">>> Levantando log_server (RPC)"
./log_server > $RPC_LOG 2>&1 &
RPC_PID=$!

echo ">>> Levantando datetime_service"
python3 datetime_service.py > $DT_LOG 2>&1 &
DT_PID=$!

export LOG_RPC_IP=127.0.0.1
echo ">>> Levantando servidor P2P"
./server -p $SERVER_PORT > $SERVER_LOG 2>&1 &
SRV_PID=$!

sleep 2   # dar tiempo a que arranquen todo

# -------------------------------------------------------------------
# 3) Crear scripts de comandos
# -------------------------------------------------------------------
cat > cmds_alice.txt <<'EOF'
UNREGISTER alice
sleep 0.2
REGISTER alice
sleep 0.2
REGISTER alice       # duplicado
sleep 0.2
UNREGISTER nobody    # error
sleep 0.2
CONNECT alice
sleep 0.5
PUBLISH input/alice/msg.txt "A1"
sleep 0.2
PUBLISH input/alice/msg.txt "dup A1"
sleep 0.2
LIST_USERS
sleep 0.2
LIST_CONTENT alice
sleep 0.2
# Alice sirve el fichero a Bob:
GET_FILE bob input/bob/msg.txt /tmp/alice_copy.txt
sleep 3              # ESPERA para dar tiempo a Bob a descargar
DELETE input/alice/msg.txt
sleep 0.2
DISCONNECT alice
sleep 0.2
UNREGISTER alice
QUIT
EOF

cat > cmds_bob.txt <<'EOF'
sleep 0.5
UNREGISTER bob
sleep 0.2
REGISTER bob
sleep 0.2
CONNECT bob
sleep 0.5
LIST_USERS
sleep 0.2
# primero falla, aún no ha publicado Alice
GET_FILE alice input/alice/msg.txt /tmp/bob_try1.txt
sleep 1
# ahora sí, después de que Alice publique
GET_FILE alice input/alice/msg.txt /tmp/bob_copy.txt
sleep 0.2
DELETE input/bob/msg.txt
sleep 0.2
GET_FILE bob input/bob/msg.txt /tmp/bob_try2.txt      # error tras delete
sleep 0.2
DISCONNECT bob
sleep 0.2
UNREGISTER bob
QUIT
EOF

# -------------------------------------------------------------------
# 4) Función de preprocesado (quitar comentarios y ejecutar sleep)
# -------------------------------------------------------------------
prep() {
  sed '/^\s*#/d;/^\s*$/d' "$1" | while IFS= read -r raw; do
    line="${raw%%#*}"
    line="$(echo "$line" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
    [[ -z "$line" ]] && continue
    if [[ $line =~ ^sleep[[:space:]]+([0-9]+(\.[0-9]+)?)$ ]]; then
      sleep "${BASH_REMATCH[1]}"
    else
      echo "$line"
    fi
  done
}

# -------------------------------------------------------------------
# 5) Lanzar Alice y Bob en paralelo
# -------------------------------------------------------------------
echo ">>> Lanzando Alice y Bob en paralelo"
prep cmds_alice.txt | $CLIENT &> $ALICE_LOG &
ALICE_PID=$!
prep cmds_bob.txt   | $CLIENT &> $BOB_LOG &
BOB_PID=$!

# -------------------------------------------------------------------
# 6) Esperar a los dos clientes
# -------------------------------------------------------------------
wait "$ALICE_PID" "$BOB_PID"

# -------------------------------------------------------------------
# 7) Mostrar logs
# -------------------------------------------------------------------
echo -e "\n--- server.log ---"
head -n 50 $SERVER_LOG

echo -e "\n--- rpc.log ---"
head -n 50 $RPC_LOG

echo -e "\n--- alice log ---"
sed '/^\s*#/d' $ALICE_LOG | sed 's/^/alice> /'

echo -e "\n--- bob log ---"
sed '/^\s*#/d' $BOB_LOG | sed 's/^/bob> /'

# -------------------------------------------------------------------
# 8) Verificar rpc.log (solo las llamadas realmente invocadas)
# -------------------------------------------------------------------
echo -e "\n>>> Verificando rpc.log…"
alice_ops=( UNREGISTER REGISTER CONNECT PUBLISH LIST_USERS LIST_CONTENT DELETE DISCONNECT UNREGISTER )
bob_ops=(   UNREGISTER REGISTER CONNECT LIST_USERS DELETE DISCONNECT UNREGISTER )

for op in "${alice_ops[@]}"; do
  if ! grep -qE "^alice[[:space:]]+${op}\b" "$RPC_LOG"; then
    echo "[FAIL] Falta RPC \"$op\" de alice"
    exit 1
  fi
done
for op in "${bob_ops[@]}"; do
  if ! grep -qE "^bob[[:space:]]+${op}\b" "$RPC_LOG"; then
    echo "[FAIL] Falta RPC \"$op\" de bob"
    exit 1
  fi
done
echo "[PASS] Todas las operaciones RPC esperadas están en rpc.log"

# -------------------------------------------------------------------
# 9) Verificar transferencias de archivos
# -------------------------------------------------------------------
echo -e "\n>>> Verificando archivos:"
if [[ "$(cat /tmp/alice_copy.txt)" == "Hola desde Bob" ]]; then
  echo "[PASS] /tmp/alice_copy.txt OK"
else
  echo "[FAIL] /tmp/alice_copy.txt no coincide"
  exit 1
fi
if [[ "$(cat /tmp/bob_copy.txt)" == "Hola desde Alice" ]]; then
  echo "[PASS] /tmp/bob_copy.txt OK"
else
  echo "[FAIL] /tmp/bob_copy.txt no coincide"
  exit 1
fi

echo -e "\n+++ TODOS LOS TESTS DE LA PARTE 3 SUPERADOS +++"
