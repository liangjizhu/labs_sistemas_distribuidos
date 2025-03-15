#!/bin/bash
# run_test.sh - Script para probar el servicio distribuido

# Limpiar compilación anterior y compilar el proyecto
echo "Limpiando compilación previa..."
make clean
echo "Compilando el proyecto..."
make

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

# Ejecutar el cliente
echo "Ejecutando el cliente..."
./app-cliente

# Esperar un momento para que se procese la petición
sleep 2

# Finalizar el servidor
echo "Terminando el servidor con PID: $SERVER_PID"
kill $SERVER_PID

echo "Prueba completada."
exit 0
