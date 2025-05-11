#!/bin/bash
set -euo pipefail

# -------------------------------------------------------------------
# Configuración
# -------------------------------------------------------------------
SERVER_IP="127.0.0.1"
SERVER_PORT="12345"
CLIENT="python3 client.py -s $SERVER_IP -p $SERVER_PORT"
DATETIME_PORT=5000

# -------------------------------------------------------------------
# Función de limpieza en EXIT
# -------------------------------------------------------------------
cleanup() {
  echo ">>> Parando servicios..."
  kill ${SRV_PID:-}   2>/dev/null || true
  kill ${DT_PID:-}    2>/dev/null || true
  # Asegurarse de matar al worker de Flask si quedase colgado
  lsof -ti tcp:$DATETIME_PORT | xargs -r kill || true
}
trap cleanup EXIT

# -------------------------------------------------------------------
# 0) Preparar entorno
# -------------------------------------------------------------------
rm -f server.log datetime_service.log liang.log paolo.log
rm -f /tmp/paolo_copy.txt /tmp/liang_copy.txt
mkdir -p input/liang input/paolo

echo "Contenido original de Liang" > input/liang/file.txt
echo "Contenido original de Paolo" > input/paolo/file.txt

# -------------------------------------------------------------------
# 1) Arrancar datetime_service
# -------------------------------------------------------------------
echo "=== Levantando datetime_service ==="
python3 datetime_service.py &> datetime_service.log &
DT_PID=$!
sleep 1

# -------------------------------------------------------------------
# 2) Arrancar servidor P2P
# -------------------------------------------------------------------
echo "=== Levantando servidor P2P ==="
./server -p $SERVER_PORT &> server.log &
SRV_PID=$!
sleep 1

# -------------------------------------------------------------------
# 3) Verificar arranque correcto
# -------------------------------------------------------------------
if grep -Ei "address already in use|error" server.log; then
  echo "[FAIL] El servidor no arrancó correctamente:"
  head -n20 server.log
  exit 1
fi

# -------------------------------------------------------------------
# 4) Escritura de secuencias de comandos
# -------------------------------------------------------------------
cat > cmds_liang.txt <<EOF
REGISTER liang
REGISTER liang
UNREGISTER nobody
CONNECT liang
PUBLISH input/liang/file.txt "Liang publica"
PUBLISH input/liang/file.txt "Liang duplicado"
LIST_USERS
LIST_CONTENT liang
LIST_CONTENT paolo
GET_FILE paolo input/paolo/file.txt /tmp/liang_copy.txt
DELETE input/liang/file.txt
GET_FILE liang input/liang/file.txt /tmp/liang_copy.txt
DISCONNECT liang
UNREGISTER liang
QUIT
EOF

cat > cmds_paolo.txt <<EOF
sleep 0.2
REGISTER paolo
CONNECT paolo
PUBLISH input/paolo/file.txt "Paolo publica"
LIST_USERS
LIST_CONTENT liang
LIST_CONTENT paolo
GET_FILE liang input/liang/file.txt /tmp/paolo_copy.txt
DELETE input/paolo/file.txt
GET_FILE paolo input/paolo/file.txt /tmp/paolo_copy.txt
DISCONNECT paolo
UNREGISTER paolo
QUIT
EOF

# -------------------------------------------------------------------
# 5) Lanzar ambos clientes en paralelo y capturar sus PIDs
# -------------------------------------------------------------------
echo "=== Ejecutando clientes liang y paolo en paralelo ==="

# liang
(
  while IFS= read -r line; do
    # ignorar comentarios
    [[ "$line" =~ ^# ]] && continue
    echo "$line"
    sleep 0.5
  done < cmds_liang.txt
) | $CLIENT &> liang.log &
LIANG_PID=$!

# paolo
(
  while IFS= read -r line; do
    [[ "$line" =~ ^# ]] && continue
    sleep 0.2
    echo "$line"
    sleep 0.5
  done < cmds_paolo.txt
) | $CLIENT &> paolo.log &
PAOLO_PID=$!

# -------------------------------------------------------------------
# 6) Esperar SÓLO a los clientes
# -------------------------------------------------------------------
wait $LIANG_PID $PAOLO_PID

# -------------------------------------------------------------------
# 7) Mostrar resultados
# -------------------------------------------------------------------
echo
echo "----- server.log -----"
sed -n '1,200p' server.log

echo
echo "----- liang.log -----"
sed -n '1,200p' liang.log

echo
echo "----- paolo.log -----"
sed -n '1,200p' paolo.log

echo
echo "Contenido de /tmp/liang_copy.txt: $( [ -f /tmp/liang_copy.txt ] && cat /tmp/liang_copy.txt || echo "<no existe>" )"
echo "Contenido de /tmp/paolo_copy.txt: $( [ -f /tmp/paolo_copy.txt ] && cat /tmp/paolo_copy.txt || echo "<no existe>" )"

echo
echo "+++ FIN de las pruebas de concurrencia entre liang y paolo +++"
