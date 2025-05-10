#!/bin/bash
set -e

# -------------------------------------------------------------------
# Configuración
# -------------------------------------------------------------------
SERVER_IP="127.0.0.1"
SERVER_PORT="12345"
CLIENT="python3 client.py -s $SERVER_IP -p $SERVER_PORT"

# -------------------------------------------------------------------
# Iniciar servicio web de timestamp
# -------------------------------------------------------------------
echo "=== Iniciando servicio web de datetime ==="
python3 datetime_service.py &> datetime_service.log &
DATETIME_PID=$!
sleep 1

# -------------------------------------------------------------------
# Iniciar servidor P2P (Parte 1 modificado para leer timestamps)
# -------------------------------------------------------------------
echo "=== Iniciando servidor P2P ==="
./server -p $SERVER_PORT &> server.log &
SERVER_PID=$!
sleep 1

# -------------------------------------------------------------------
# Comprobamos que el servidor arrancó sin errores de bind
# -------------------------------------------------------------------
if grep -qEi "address already in use|error" server.log; then
    echo
    echo "[FAIL] El servidor no arrancó correctamente:"
    sed -n '1,20p' server.log
    kill $DATETIME_PID 2>/dev/null || true
    kill $SERVER_PID   2>/dev/null || true
    exit 1
fi

# -------------------------------------------------------------------
# 1) Test del servicio web de timestamp
# -------------------------------------------------------------------
echo "=== TEST: formato de timestamp devuelto ==="
TS=$(curl -s http://127.0.0.1:5000/datetime)
if [[ $TS =~ ^[0-9]{2}/[0-9]{2}/[0-9]{4}\ [0-9]{2}:[0-9]{2}:[0-9]{2}$ ]]; then
    echo "[PASS] Timestamp válido: $TS"
else
    echo "[FAIL] Timestamp inválido: '$TS'"
    kill $DATETIME_PID 2>/dev/null || true
    kill $SERVER_PID   2>/dev/null || true
    exit 1
fi

# -------------------------------------------------------------------
# 2) Ejecutar secuencia de comandos cliente con retardo
# -------------------------------------------------------------------
echo "=== Ejecutando operaciones P2P con client.py (1s de retardo) ==="
cat > cmds_part2.txt <<EOF
UNREGISTER liang
REGISTER liang
CONNECT liang
PUBLISH input/liang/file1.txt "Doc prueba"
LIST_USERS
DISCONNECT liang
QUIT
EOF

# Enviamos cada línea y luego esperamos 1 segundo
(
  while IFS= read -r cmd; do
    echo "$cmd"
    sleep 1
  done < cmds_part2.txt
) | $CLIENT &> client_part2.log

# -------------------------------------------------------------------
# 3) Parar servicios
# -------------------------------------------------------------------
kill $DATETIME_PID 2>/dev/null || true
kill $SERVER_PID   2>/dev/null || true
wait $DATETIME_PID $SERVER_PID 2>/dev/null || true

# -------------------------------------------------------------------
# 4) Aserciones sobre server.log
# -------------------------------------------------------------------
assert_server() {
    local desc="$1"; local pat="$2"
    if grep -qE "$pat" server.log; then
        echo "[PASS] $desc"
    else
        echo
        echo "[FAIL] $desc"
        echo "  Esperado patrón: $pat"
        echo "  --- server.log ---"
        sed -n '1,200p' server.log
        exit 1
    fi
}

echo "=== Verificando logs del servidor ==="
assert_server "REGISTER log"    "^s> OPERATION REGISTER FROM liang AT [0-9]{2}/[0-9]{2}/[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}$"
assert_server "CONNECT log"     "^s> OPERATION CONNECT FROM liang AT [0-9]{2}/[0-9]{2}/[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}$"
assert_server "PUBLISH log"     "^s> OPERATION PUBLISH FROM liang AT [0-9]{2}/[0-9]{2}/[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}$"
assert_server "LIST_USERS log"  "^s> OPERATION LIST USERS FROM liang AT [0-9]{2}/[0-9]{2}/[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}$"
assert_server "DISCONNECT log"  "^s> OPERATION DISCONNECT FROM liang AT [0-9]{2}/[0-9]{2}/[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}$"

echo
echo "+++ TODOS LOS TESTS DE LA PARTE 2 SUPERADOS +++"
