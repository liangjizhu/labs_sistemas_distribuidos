#!/bin/bash
set -e

# -------------------------------------------------------------------
# Configuración de servidor y cliente
# -------------------------------------------------------------------
SERVER_IP="127.0.0.1"
SERVER_PORT="12345"
CLIENT="python3 client.py -s $SERVER_IP -p $SERVER_PORT"

# -------------------------------------------------------------------
# Prepara directorios y ficheros de prueba
# -------------------------------------------------------------------
mkdir -p input/paolo input/liang
echo "Contenido de prueba de Paolo" > input/paolo/file1.txt
echo "Contenido de prueba de Liang" > input/liang/file1.txt

# -------------------------------------------------------------------
# Función de aserción: comprueba patrón en el log
# -------------------------------------------------------------------
assert() {
    local logfile="$1"; shift
    local desc="$1"; shift
    local pat="$1"
    if grep -qF "$pat" "$logfile"; then
        echo "[PASS] $desc"
    else
        echo
        echo "[FAIL] $desc"
        echo "  Esperado: $pat"
        echo "  --- Contenido de $logfile ---"
        sed -n '1,200p' "$logfile"
        echo "--------------------------------"
        exit 1
    fi
}

################################################################################
# 1) Test de operaciones del usuario 'liang'
################################################################################
LOG1="test_liang.log"
cat > cmds_liang.txt <<EOF
UNREGISTER liang
REGISTER liang
CONNECT liang
PUBLISH input/liang/file1.txt "Documento de prueba"
PUBLISH input/liang/file1.txt "Intento duplicado"
LIST_USERS
LIST_CONTENT liang
LIST_CONTENT nobody
DISCONNECT liang
QUIT
EOF

echo "=== Ejecutando tests de 'liang' ==="
cat cmds_liang.txt | $CLIENT &> "$LOG1"

assert "$LOG1" "REGISTER liang (OK)"                  "REGISTER OK"
assert "$LOG1" "CONNECT liang (OK)"                   "CONNECT OK"
assert "$LOG1" "PUBLISH por primera vez (OK)"         "PUBLISH OK"
assert "$LOG1" "PUBLISH duplicado (FAIL)"             "PUBLISH FAIL, CONTENT ALREADY PUBLISHED"
assert "$LOG1" "LIST_USERS (OK)"                      "LIST_USERS OK"
assert "$LOG1" "LIST_CONTENT liang (OK)"              "LIST_CONTENT OK"
assert "$LOG1" "LIST_CONTENT nobody (FAIL)"           "LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST"
assert "$LOG1" "DISCONNECT liang (OK)"                "DISCONNECT OK"

################################################################################
# 2) Test de 'paolo' + GET_FILE OK (necesita 'liang' conectado en segundo plano)
################################################################################
LOG2="test_paolo.log"
cat > cmds_liang_bg.txt <<EOF
CONNECT liang
EOF

echo "=== Iniciando sesión de escucha de 'liang' en background ==="
( cat cmds_liang_bg.txt; tail -f /dev/null ) | $CLIENT &> liang_bg.log &
LIANG_PID=$!
sleep 1  # espera a que arranque el hilo de escucha

cat > cmds_paolo.txt <<EOF
UNREGISTER paolo
REGISTER paolo
CONNECT paolo
LIST_USERS
GET_FILE liang input/liang/file1.txt /tmp/copied1.txt
LIST_CONTENT paolo
DISCONNECT paolo
QUIT
EOF

rm -f /tmp/copied1.txt

echo "=== Ejecutando tests de 'paolo' (incluye GET_FILE OK) ==="
cat cmds_paolo.txt | $CLIENT &> "$LOG2"

kill $LIANG_PID
wait $LIANG_PID 2>/dev/null || true
echo -e "DISCONNECT liang\nQUIT" | $CLIENT &>/dev/null

assert "$LOG2" "REGISTER paolo (OK)"                   "REGISTER OK"
assert "$LOG2" "CONNECT paolo (OK)"                    "CONNECT OK"
assert "$LOG2" "LIST_USERS (OK)"                       "LIST_USERS OK"
assert "$LOG2" "GET_FILE (OK)"                         "GET_FILE OK"
if diff -u input/liang/file1.txt /tmp/copied1.txt &>/dev/null; then
    echo "[PASS] Contenido de GET_FILE coincide"
else
    echo "[FAIL] Contenido de GET_FILE NO coincide"
    diff -u input/liang/file1.txt /tmp/copied1.txt || true
    exit 1
fi
assert "$LOG2" "LIST_CONTENT paolo (OK)"               "LIST_CONTENT OK"
assert "$LOG2" "DISCONNECT paolo (OK)"                 "DISCONNECT OK"

################################################################################
# 3) Test de DELETE por parte de 'liang' y GET_FILE fallo
################################################################################
LOG3="test_delete.log"
cat > cmds_delete.txt <<EOF
CONNECT liang
DELETE input/liang/file1.txt
GET_FILE liang input/liang/file1.txt /tmp/copied2.txt
DISCONNECT liang
QUIT
EOF

rm -f /tmp/copied2.txt
rm -f input/liang/file1.txt   # <<<<— borramos el fichero físico para forzar el FAIL

echo "=== Ejecutando tests de DELETE y GET_FILE FAIL ==="
cat cmds_delete.txt | $CLIENT &> "$LOG3"

assert "$LOG3" "CONNECT liang para DELETE (OK)"        "CONNECT OK"
assert "$LOG3" "DELETE existing content (OK)"          "DELETE OK"
assert "$LOG3" "GET_FILE tras DELETE (FAIL)"           "GET_FILE FAIL, FILE NOT EXIST"
if [ -e /tmp/copied2.txt ]; then
    echo "[FAIL] GET_FILE fallo no debería crear fichero"
    exit 1
else
    echo "[PASS] GET_FILE fallo no creó fichero"
fi
assert "$LOG3" "DISCONNECT liang (OK)"                 "DISCONNECT OK"

################################################################################
# 4) Limpieza final: desregistrar usuarios
################################################################################
echo -e "UNREGISTER paolo\nUNREGISTER liang\nQUIT" | $CLIENT &>/dev/null

echo
echo "+++ TODOS LOS TESTS SUPERADOS +++"
