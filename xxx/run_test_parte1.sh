#!/bin/bash
set -euo pipefail

# -------------------------------------------------------------------
# Configuración de servidor y cliente
# -------------------------------------------------------------------
SERVER_IP="127.0.0.1"
SERVER_PORT="12345"
CLIENT="python3 client.py -s $SERVER_IP -p $SERVER_PORT"

# -------------------------------------------------------------------
# Prepara directorios y ficheros de prueba
# -------------------------------------------------------------------
rm -rf input test_*.log /tmp/copied_*
mkdir -p input/paolo input/liang
echo "Contenido de prueba de Paolo" > input/paolo/file1.txt
echo "Contenido de prueba de Liang"  > input/liang/file1.txt

# -------------------------------------------------------------------
# Función de aserción: comprueba patrón en el log
# -------------------------------------------------------------------
assert() {
    local logfile="$1" desc="$2" pat="$3"
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
# 1) Test del usuario 'liang'
################################################################################
LOG1="test_liang.log"
cat > cmds_liang.txt <<EOF
UNREGISTER liang
REGISTER liang
CONNECT liang
PUBLISH input/liang/file1.txt "Doc prueba"
PUBLISH input/liang/file1.txt "Intento duplicado"
LIST_USERS
LIST_CONTENT liang
LIST_CONTENT nobody
DELETE input/liang/file1.txt
DELETE input/liang/file1.txt
GET_FILE paolo input/paolo/file1.txt /tmp/copied_liang_fail.txt
DISCONNECT liang
QUIT
EOF

echo "=== Ejecutando tests de 'liang' ==="
cat cmds_liang.txt | $CLIENT &> "$LOG1"

assert "$LOG1" "REGISTER liang (OK)"                      "REGISTER OK"
assert "$LOG1" "CONNECT liang (OK)"                       "CONNECT OK"
assert "$LOG1" "PUBLISH por primera vez (OK)"             "PUBLISH OK"
assert "$LOG1" "PUBLISH duplicado (FAIL)"                 "PUBLISH FAIL, CONTENT ALREADY PUBLISHED"
assert "$LOG1" "LIST_USERS (OK)"                          "LIST_USERS OK"
assert "$LOG1" "LIST_CONTENT liang (OK)"                  "LIST_CONTENT OK"
assert "$LOG1" "LIST_CONTENT nobody (FAIL)"               "LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST"
assert "$LOG1" "DELETE exist (OK)"                        "DELETE OK"
assert "$LOG1" "DELETE duplicado (FAIL)"                  "DELETE FAIL, CONTENT NOT PUBLISHED"
assert "$LOG1" "GET_FILE remoto inexistente (FAIL)"       "GET_FILE FAIL"
assert "$LOG1" "DISCONNECT liang (OK)"                    "DISCONNECT OK"

################################################################################
# 2) Test del usuario 'paolo' (necesita 'liang' escuchando para GET_FILE)
################################################################################
LOG2="test_paolo.log"
# Arranca liang en background para servir GET_FILE
cat > cmds_liang_bg.txt <<EOF
CONNECT liang
EOF
( cat cmds_liang_bg.txt; tail -f /dev/null ) | $CLIENT &> liang_bg.log &
LIANG_BG_PID=$!
sleep 1

cat > cmds_paolo.txt <<EOF
UNREGISTER paolo
REGISTER paolo
CONNECT paolo
PUBLISH input/paolo/file1.txt "Doc prueba"
PUBLISH input/paolo/file1.txt "Intento duplicado"
LIST_USERS
LIST_CONTENT paolo
LIST_CONTENT nobody
GET_FILE liang input/liang/file1.txt /tmp/copied_paolo.txt
GET_FILE nobody input/liang/file1.txt /tmp/copied_paolo_fail.txt
DELETE input/paolo/file1.txt
DELETE input/paolo/file1.txt
GET_FILE paolo input/paolo/file1.txt /tmp/copied_paolo2_fail.txt
DISCONNECT paolo
QUIT
EOF

rm -f /tmp/copied_paolo*.txt

echo
echo "=== Ejecutando tests de 'paolo' ==="
cat cmds_paolo.txt | $CLIENT &> "$LOG2"

# Detiene el listener de liang y limpia estado
kill $LIANG_BG_PID 2>/dev/null || true
echo -e "DISCONNECT liang\nQUIT" | $CLIENT &>/dev/null

assert "$LOG2" "REGISTER paolo (OK)"                      "REGISTER OK"
assert "$LOG2" "CONNECT paolo (OK)"                       "CONNECT OK"
assert "$LOG2" "PUBLISH paolo primera vez (OK)"            "PUBLISH OK"
assert "$LOG2" "PUBLISH paolo duplicado (FAIL)"            "PUBLISH FAIL, CONTENT ALREADY PUBLISHED"
assert "$LOG2" "LIST_USERS desde paolo (OK)"               "LIST_USERS OK"
assert "$LOG2" "LIST_CONTENT paolo (OK)"                   "LIST_CONTENT OK"
assert "$LOG2" "LIST_CONTENT nobody paolo (FAIL)"         "LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST"
assert "$LOG2" "GET_FILE liang → paolo (OK)"               "GET_FILE OK"
if diff -u input/liang/file1.txt /tmp/copied_paolo.txt &>/dev/null; then
    echo "[PASS] Contenido GET_FILE coincide"
else
    echo "[FAIL] Contenido GET_FILE NO coincide"
    diff -u input/liang/file1.txt /tmp/copied_paolo.txt
    exit 1
fi
assert "$LOG2" "GET_FILE nadie paolo (FAIL)"               "GET_FILE FAIL"
assert "$LOG2" "DELETE paolo exist (OK)"                   "DELETE OK"
assert "$LOG2" "DELETE paolo duplicado (FAIL)"             "DELETE FAIL, CONTENT NOT PUBLISHED"
assert "$LOG2" "GET_FILE tras delete (FAIL)"               "GET_FILE FAIL"
assert "$LOG2" "DISCONNECT paolo (OK)"                     "DISCONNECT OK"

################################################################################
# 3) Limpieza final: desregistrar usuarios
################################################################################
echo -e "UNREGISTER paolo\nUNREGISTER liang\nQUIT" | $CLIENT &>/dev/null

echo
echo "+++ TODOS LOS TESTS DE LA PARTE 1 SUPERADOS +++"
