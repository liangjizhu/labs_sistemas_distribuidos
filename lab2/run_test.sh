#!/bin/bash
# run_test.sh - Script para probar el servicio distribuido utilizando sockets TCP (Ejercicio 2)

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

# Iniciar el servidor-sock en background (el puerto se pasa como argumento)
echo "Iniciando el servidor..."
./servidor-sock 4500 &
SERVER_PID=$!
echo "Servidor iniciado con PID: $SERVER_PID"

# Esperar un poco para que el servidor se inicie correctamente
sleep 2

# Array de nombres de clientes (app-cliente1 a app-cliente6)
CLIENTS=("app-cliente1" "app-cliente2" "app-cliente3" "app-cliente4" "app-cliente5" "app-cliente6")

# Ejecutar todos los clientes concurrentemente
echo "Ejecutando clientes concurrentemente..."
for client in "${CLIENTS[@]}"; do
    echo "Lanzando $client..."
    ./$client &
done

# Esperar a que todos los clientes finalicen
wait

# Finalizar el servidor
echo "Terminando el servidor con PID: $SERVER_PID"
kill $SERVER_PID

echo "Prueba de todos los clientes completada."
exit 0
