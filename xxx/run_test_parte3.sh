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
rm -f rpc.log server.log datetime_service.log client3.log

# -------------------------------------------------------------------
# 1) Arrancar servidor RPC de logs
# -------------------------------------------------------------------
echo "=== Iniciando log_server (RPC) ==="
./log_server > rpc.log 2>&1 &
RPC_PID=$!
sleep 1

# -------------------------------------------------------------------
# 2) Arrancar servicio web datetime
# -------------------------------------------------------------------
echo "=== Iniciando servicio web datetime ==="
python3 datetime_service.py > datetime_service.log 2>&1 &
DT_PID=$!
sleep 1

# -------------------------------------------------------------------
# 3) Arrancar servidor P2P con RPC cliente embebido
# -------------------------------------------------------------------
echo "=== Iniciando servidor P2P ==="
export LOG_RPC_IP=127.0.0.1
./server -p $SERVER_PORT > server.log 2>&1 &
SRV_PID=$!
sleep 1

# -------------------------------------------------------------------
# 4) Enviar operaciones desde el cliente, con 1s de retardo
# -------------------------------------------------------------------
echo "=== Ejecutando cliente con operaciones RPC ==="
cat > cmds3.txt <<'EOF'
REGISTER liang
sleep 1
CONNECT liang
sleep 1
PUBLISH input/liang/file1.txt "RPC test"
sleep 1
LIST_USERS
sleep 1
DISCONNECT liang
sleep 1
QUIT
EOF

( while IFS= read -r cmd; do
    if [[ "$cmd" =~ ^sleep ]]; then
      # extrae segundos y duerme
      secs=${cmd#sleep }
      sleep "$secs"
    else
      echo "$cmd"
    fi
  done < cmds3.txt
) | $CLIENT &> client3.log

# Espera a que lleguen todos los RPC
sleep 2

# -------------------------------------------------------------------
# 5) Detener todos los servicios
# -------------------------------------------------------------------
kill $SRV_PID $DT_PID $RPC_PID 2>/dev/null || true
wait $SRV_PID $DT_PID $RPC_PID 2>/dev/null || true

# -------------------------------------------------------------------
# 6) Aserciones sobre rpc.log
# -------------------------------------------------------------------
assert_rpc() {
  local desc="$1"; local pat="$2"
  if grep -qE "$pat" rpc.log; then
    echo "[PASS] $desc"
  else
    echo
    echo "[FAIL] $desc"
    echo "  Esperado patrón: $pat"
    echo "  --- rpc.log ---"
    sed -n '1,200p' rpc.log
    exit 1
  fi
}

echo "=== Verificando rpc.log ==="
assert_rpc "RPC REGISTER"    "^liang REGISTER [0-9]{2}/[0-9]{2}/2025 [0-9]{2}:[0-9]{2}:[0-9]{2}$"
assert_rpc "RPC CONNECT"     "^liang CONNECT [0-9]{2}/[0-9]{2}/2025 [0-9]{2}:[0-9]{2}:[0-9]{2}$"
assert_rpc "RPC PUBLISH"     "^liang PUBLISH input/liang/file1\\.txt [0-9]{2}/[0-9]{2}/2025 [0-9]{2}:[0-9]{2}:[0-9]{2}$"
assert_rpc "RPC LIST_USERS"  "^liang LIST_USERS [0-9]{2}/[0-9]{2}/2025 [0-9]{2}:[0-9]{2}:[0-9]{2}$"
assert_rpc "RPC DISCONNECT"  "^liang DISCONNECT [0-9]{2}/[0-9]{2}/2025 [0-9]{2}:[0-9]{2}:[0-9]{2}$"

echo
echo "+++ TODOS LOS TESTS DE LA PARTE 3 SUPERADOS +++"
