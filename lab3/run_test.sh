#!/usr/bin/env bash
set -euo pipefail

# ------------------------------------------------------------
# run_test.sh - Pruebas para el servicio ONC RPC (Ejercicio 3)
# Captura todo el output en resultado.txt y tolera segfaults en clientes
# ------------------------------------------------------------

LOGFILE="resultado.txt"
# Iniciar registro (stdout y stderr) a la vez en consola y en LOGFILE
exec > >(tee -a "$LOGFILE") 2>&1

echo "=== Iniciando pruebas ONC RPC: $(date) ==="

# 0) Limpiar posibles instancias previas
echo "[0] Limpiando procesos anteriores..."
pkill -f tuplas_server   2>/dev/null || true
pkill -f app-cliente     2>/dev/null || true

# 1) Asegurarse de que rpcbind esté arriba
echo "[1] Verificando rpcbind..."
if ! rpcinfo -p localhost &>/dev/null; then
  echo "rpcbind no arrancado, iniciando..."
  sudo rpcbind
  sleep 1
else
  echo "rpcbind ya está funcionando."
fi

# 2) Generar stubs XDR & RPC
echo "[2] Ejecutando rpcgen..."
rpcgen -N -M tuplas.x

# 3) Compilar stubs XDR y dispatch
echo "[3] Compilando tuplas_xdr.c & tuplas_svc.c..."
cc -D_REENTRANT -g -I/usr/include/tirpc -c tuplas_xdr.c
cc -D_REENTRANT -g -I/usr/include/tirpc -c tuplas_svc.c

# 4) Compilar servidor RPC
echo "[4] Compilando tuplas_server..."
cc -D_REENTRANT -g -I/usr/include/tirpc -c tuplas_server.c -o tuplas_server.o
cc -D_REENTRANT -g -o tuplas_server \
    tuplas_server.o tuplas_svc.o tuplas_xdr.o \
    -lnsl -lpthread -ldl -ltirpc

# 5) Compilar stub cliente
echo "[5] Compilando tuplas_clnt.c..."
cc -D_REENTRANT -g -I/usr/include/tirpc -c tuplas_clnt.c -o tuplas_clnt.o

# 6) Compilar proxy-rpc y generar libclaves.so
echo "[6] Compilando proxy-rpc y libclaves.so..."
cc -D_REENTRANT -g -I/usr/include/tirpc -fPIC -c proxy-rpc.c -o proxy-rpc.o
cc -shared -o libclaves.so \
    proxy-rpc.o tuplas_clnt.o tuplas_xdr.o \
    -lnsl -lpthread -ldl -ltirpc

# 7) Compilar los clientes app-cliente1..6
echo "[7] Compilando clientes app-cliente1..6..."
for i in {1..6}; do
  src=app-cliente${i}.c
  obj=app-cliente${i}.o
  bin=app-cliente${i}
  echo "   - $src -> $bin"
  cc -D_REENTRANT -g -I/usr/include/tirpc -c "$src" -o "$obj"
  cc -o "$bin" "$obj" -L. -lclaves -Wl,-rpath=. \
     -lnsl -lpthread -ldl -ltirpc
done

# 8) Levantar el servidor RPC
echo "[8] Levantando servidor RPC..."
./tuplas_server &
SERVER_PID=$!
sleep 2
echo "Servidor RPC en PID=$SERVER_PID listo."

# 9) Ejecutar las pruebas
echo
echo "[9.1] Inserciones (app-cliente1)"
./app-cliente1
echo "   ✓ app-cliente1 completado"
echo

echo "[9.2] Clientes 2–5 en paralelo (get/modify/delete/exist)"
CLIENTS=(app-cliente2 app-cliente3 app-cliente4 app-cliente5)
PIDS=()
for cl in "${CLIENTS[@]}"; do
  echo "   → Lanzando $cl en background"
  ./$cl &
  PIDS+=($!)
done

# Aquí quitamos temporalmente el ‘set -e’ para que no nos mate el script si un cliente se segfaulta
set +e
for pid in "${PIDS[@]}"; do
  if wait "$pid"; then
    echo "   → Cliente PID $pid finalizó con éxito"
  else
    echo "   ⚠️ Cliente PID $pid falló con código de salida $?"
  fi
done
set -e
echo "   ✓ Clientes 2–5 completados (con posibles fallos reportados)"
echo

echo "[9.3] Destroy (app-cliente6)"
./app-cliente6
echo "   ✓ app-cliente6 completado"
echo

# 10) Detener el servidor
echo "[10] Deteniendo servidor RPC (PID=$SERVER_PID)..."
kill "$SERVER_PID"

echo "=== Pruebas ONC RPC finalizadas: $(date) ==="
