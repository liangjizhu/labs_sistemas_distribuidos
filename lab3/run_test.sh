#!/usr/bin/env bash
set -euo pipefail

LOGFILE="resultado.txt"
exec > >(tee -a "$LOGFILE") 2>&1

echo "=== Iniciando pruebas ONC RPC: $(date) ==="

# 0) Limpiar y construir todo con Make
echo "[make] Limpiando y compilando..."
make clean all

# 1) Levantar el servidor RPC
echo "[8] Levantando servidor RPC..."
./servidor &
SERVER_PID=$!
sleep 2
echo "Servidor RPC en PID=$SERVER_PID listo."

# 2) Ejecutar las pruebas

echo
echo "[9.1] Inserciones (app-cliente1)"
./app-cliente1
echo "   ✓ app-cliente1 completado"
echo

echo "[9.2] Clientes 2–5 en paralelo (get/modify/delete/exist)"
pids=()
for i in {2..5}; do
  "./app-cliente${i}" >"c${i}.log" 2>&1 &
  pids+=($!)
done

# Tolerar segfaults sin interrumpir el script
set +e
for pid in "${pids[@]}"; do
  if wait "$pid"; then
    echo "   → Cliente PID $pid finalizó con éxito"
  else
    echo "   Cliente PID $pid falló con código de salida $?"
  fi
done
set -e
echo "   ✓ Clientes 2–5 completados (con posibles fallos reportados)"
echo

# 3) Esperar fin de todos los clientes 1–5 antes de lanzar el Destroy
echo "[9.3] Esperando a que terminen todos los procesos de app-cliente1–5..."
# Mientras haya algún cliente 1-5 activo, esperamos
while pgrep -f '\./app-cliente[1-5]' >/dev/null; do
  sleep 1
done
echo "   ✓ Todos los procesos de app-cliente1–5 han finalizado"
echo

# 4) Destroy (app-cliente6)
echo "[9.4] Destroy (app-cliente6)"
./app-cliente6 >c6.log 2>&1
echo "   ✓ app-cliente6 completado"
echo

# 5) Detener el servidor
echo "[10] Deteniendo servidor RPC (PID=$SERVER_PID)..."
kill "$SERVER_PID"

echo "=== Pruebas ONC RPC finalizadas: $(date) ==="
