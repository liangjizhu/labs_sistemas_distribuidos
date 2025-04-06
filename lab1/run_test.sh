#!/bin/bash
# run_test.sh - Script para probar el servicio distribuido con múltiples clientes concurrentemente

echo "Limpiando compilación previa..."
make clean
echo "Compilando el proyecto..."
make

echo "Matando instancias anteriores..."
pkill servidor-sock
pkill app-cliente1
pkill app-cliente2
pkill app-cliente3
pkill app-cliente4
pkill app-cliente5
pkill app-cliente6


if [ $? -ne 0 ]; then
    echo "Error en la compilación."
    exit 1
fi

# Configurar LD_LIBRARY_PATH para que se encuentre libclaves.so
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

# Iniciar el servidor en background
echo "Iniciando el servidor..."
./servidor-mq &
SERVER_PID=$!
echo "Servidor iniciado con PID: $SERVER_PID"

# Esperar un poco para que el servidor se inicie correctamente
sleep 2

# Array de nombres de clientes
CLIENTS=("app-cliente1" "app-cliente2" "app-cliente3" "app-cliente4" "app-cliente5" "app-cliente6")
CLIENT_PIDS=()

# Ejecutar todos los clientes concurrentemente y almacenar sus PIDs
echo "Ejecutando clientes concurrentemente..."
for client in "${CLIENTS[@]}"; do
    echo "Lanzando $client..."
    ./$client &
    CLIENT_PIDS+=($!)
done

# Esperar a que todos los clientes finalicen
for pid in "${CLIENT_PIDS[@]}"; do
    wait $pid
done

# Finalizar el servidor
echo "Terminando el servidor con PID: $SERVER_PID"
kill $SERVER_PID

echo "Prueba de todos los clientes completada."
exit 0
