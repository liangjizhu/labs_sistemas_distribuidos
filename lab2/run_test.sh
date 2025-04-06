#!/bin/bash
# run_test.sh - Script para probar el servicio distribuido utilizando sockets TCP (Ejercicio 2)

# Matar instancias previas del servidor y clientes, si las hay
pkill -f servidor-sock 2>/dev/null
pkill -f app-cliente 2>/dev/null

echo "Limpiando compilación previa..."
make clean

echo "Compilando el proyecto..."
make

if [ $? -ne 0 ]; then
    echo "Error en la compilación."
    exit 1
fi

# Configurar variables de entorno para el cliente
export IP_TUPLAS=127.0.0.1
export PORT_TUPLAS=4500

# Iniciar el servidor-sock en background
echo "Iniciando el servidor..."
./servidor-sock 4500 &
SERVER_PID=$!
echo "Servidor iniciado con PID: $SERVER_PID"

# Esperar un poco para que el servidor se inicie correctamente
sleep 2

# Insertar tuplas iniciales (cliente1)
echo "Insertando tuplas iniciales..."
./app-cliente1
echo "✓ app-cliente1 finalizado"

# Ejecutar los demás clientes en background y guardar sus PIDs
CLIENTS=("app-cliente2" "app-cliente3" "app-cliente4" "app-cliente5")
CLIENT_PIDS=()

echo "Lanzando los demás clientes..."
for client in "${CLIENTS[@]}"; do
    echo "→ Ejecutando $client..."
    ./$client &
    CLIENT_PIDS+=($!)
done

# Esperar a que terminen los clientes 2–5
for pid in "${CLIENT_PIDS[@]}"; do
    wait $pid
done
echo "Todos los clientes (2–5) han terminado"

# Ejecutar app-cliente6 al final (reseteo)
echo "Ejecutando app-cliente6 (destroy)..."
./app-cliente6
echo "app-cliente6 finalizado"

# Finalizar el servidor
echo "Terminando el servidor con PID: $SERVER_PID"
kill $SERVER_PID

echo "Prueba de todos los clientes completada."
exit 0
