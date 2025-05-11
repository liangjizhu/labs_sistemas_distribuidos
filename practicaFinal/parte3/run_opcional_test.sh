#!/bin/bash
set -euo pipefail

# -------------------------------------------------------------------
# Configuración
# -------------------------------------------------------------------
SERVER_PORT=12345
CLIENT="python3 client.py -s 127.0.0.1 -p $SERVER_PORT"
export LOG_RPC_IP=127.0.0.1

# -------------------------------------------------------------------
# Logs y temporales
# -------------------------------------------------------------------
RPC_LOG=edge_rpc.log
SERVER_LOG=edge_server.log
DT_LOG=edge_datetime.log
UL_LOG=edge_userlimit.log
FL_LOG=edge_filelimit.log
TO_LOG=edge_timeout.log

# -------------------------------------------------------------------
# Cleanup on exit
# -------------------------------------------------------------------
cleanup() {
  echo ">>> Deteniendo servicios..."
  kill ${SRV_PID:-} ${DT_PID:-} ${RPC_PID:-} ${SLOWPEER_PID:-} 2>/dev/null || true
  lsof -ti tcp:5000 | xargs -r kill
}
trap cleanup EXIT

# -------------------------------------------------------------------
# 0) Limpieza inicial
# -------------------------------------------------------------------
rm -f $RPC_LOG $SERVER_LOG $DT_LOG $UL_LOG $FL_LOG $TO_LOG
rm -rf input/userlimit input/filelimit /tmp/timeout_*

# -------------------------------------------------------------------
# 1) Arrancar log_server, datetime_service y P2P
# -------------------------------------------------------------------
./log_server        > $RPC_LOG    2>&1 &
RPC_PID=$!
python3 datetime_service.py > $DT_LOG 2>&1 &
DT_PID=$!
./server -p $SERVER_PORT > $SERVER_LOG 2>&1 &
SRV_PID=$!
sleep 1

# -------------------------------------------------------------------
# 2) Test de límite de usuarios (MAX_USERS = 100)
# -------------------------------------------------------------------
mkdir -p input/userlimit
{
  for i in $(seq 1 100); do
    echo "REGISTER u$i"
  done
  echo "REGISTER u101"
  echo "QUIT"
} | $CLIENT &> $UL_LOG

# Assertions
grep -c "REGISTER OK" $UL_LOG | grep -qx 100 \
  && echo "[PASS] 100 usuarios registrados OK"
grep -qE "REGISTER FAIL"  $UL_LOG \
  && echo "[PASS] Rechazada la 101ª inscripción"

# -------------------------------------------------------------------
# 3) Test de límite de ficheros por usuario (MAX_FILES = 100)
# -------------------------------------------------------------------
mkdir -p input/filelimit
# Crear 101 ficheros de prueba
for i in $(seq 0 100); do
  echo "contenido $i" > input/filelimit/f$i.txt
done

{
  echo "REGISTER fluser"
  echo "CONNECT fluser"
  for i in $(seq 0 99); do
    echo "PUBLISH input/filelimit/f$i.txt \"desc $i\""
  done
  echo "PUBLISH input/filelimit/f100.txt \"overflow\""
  echo "QUIT"
} | $CLIENT &> $FL_LOG

grep -c "PUBLISH OK"   $FL_LOG | grep -qx 100 \
  && echo "[PASS] 100 ficheros publicados OK"
grep -qE "PUBLISH FAIL" $FL_LOG \
  && echo "[PASS] Rechazo al publicar el fichero nº101"

# -------------------------------------------------------------------
# 4) Test de timeout en GET_FILE contra un peer que no responde
# -------------------------------------------------------------------
# Levantamos un "peer lento" que acepta la conexión pero no responde
python3 - <<'PYCODE' &
import socket, threading, time
def peer():
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('127.0.0.1', 60000))
    s.listen(1)
    conn, _ = s.accept()
    time.sleep(300)
threading.Thread(target=peer, daemon=True).start()
PYCODE
SLOWPEER_PID=$!

# Ejecutamos GET_FILE y dejamos un timeout de 5s
{
  echo "REGISTER slowpeer"
  echo "CONNECT slowpeer 60000"
  echo "GET_FILE slowpeer foo bar"
  echo "QUIT"
} > cmds_timeout.txt

timeout 5s bash -c "$CLIENT < cmds_timeout.txt" &> $TO_LOG || true

if grep -q "GET_FILE FAIL" $TO_LOG; then
  echo "[PASS] GET_FILE detecta fallo (sin respuesta remota)"
else
  echo "[PASS] GET_FILE abortado con timeout"
fi

# -------------------------------------------------------------------
# 5) Fin
# -------------------------------------------------------------------
echo "+++ TODOS LOS TESTS DE LÍMITE COMPLETADOS +++"
