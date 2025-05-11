# Sistema P2P de Distribución de Ficheros con RPC y Timestamp
## Autores
Liang Ji Zhu 100495723

Paolo Michael Webb 100495955

Este proyecto implementa un sistema **peer-to-peer** de intercambio de ficheros, complementado con:
1. Un servicio REST en Python para obtener marcas temporales.  
2. Un servidor RPC (generado con `rpcgen`) que registra todas las operaciones.  
3. Un servidor central P2P en C que coordina el alta/baja de usuarios, publica/lista borra referencias, y delega la transferencia en conexiones directas cliente-cliente.  

---

## Prerrequisitos

- **Compilador C**: `gcc`  
- **rpcgen** (parte de las RPC Utilities)
- **make**  
- **Python 3** y pip:  
  ```bash
  pip install flask requests
``

---

## Estructura de ficheros

* `log.x`
* `log.h`, `log_clnt.c`, `log_svc.c`, `log_xdr.c` ← generados por `rpcgen`
* `log_server.c`        ← implementación del servicio RPC
* `server.c`            ← servidor central P2P en C (usa stubs RPC para trazar)
* `client.py`           ← cliente Python
* `datetime_service.py` ← microservicio Flask para timestamp
* `Makefile`            ← compila servidor P2P y log\_server
* `run_test.sh`, `run_concurrencia_test.sh`, `run_opcional_test.sh` ← scripts de prueba

---

## Compilación

1. **Generar los stubs RPC**

   ```bash
   rpcgen -NMa log.x
   ```

   Esto crea:

   ```
   log.h log_clnt.c log_xdr.c log_svc.c ...
   ```

2. **Compilar todo**

   ```bash
   make
   ```
   * `server`       ← servidor central P2P
   * `log_server`   ← servidor RPC de logging
   * **Nota**: Aunque en el Makefile se hace uso de rpcgen también, o sea, con `make` se podría saltar el paso previo.

---

## Despliegue / Ejecución

1. **Levantar el servicio de timestamp**

   ```bash
   python3 datetime_service.py
   ```

   Escucha por defecto en `http://127.0.0.1:5000/datetime`.

2. **Levantar el servidor RPC de logging**

   ```bash
   ./log_server
   ```

   Muestra por stdout cada llamada RPC recibida (usuario, operación, timestamp).

3. **Levantar el servidor P2P**

   ```bash
   ./server -p 12345
   ```

   Por cada operación de cliente:

   * Obtiene timestamp vía REST
   * Envía operación+timestmap al RPC
   * Ejecuta la lógica P2P

4. **Iniciar un cliente**

   ```bash
   python3 client.py -s 127.0.0.1 -p 12345
   ```

   Dispondrá de un prompt con comandos:

   ```
   REGISTER <userName>
   CONNECT  <userName>
   PUBLISH  <file> <description>
   LIST_USERS
   LIST_CONTENT <userName>
   GET_FILE <userName> <remote_fn> <local_fn>
   DELETE   <file>
   DISCONNECT <userName>
   UNREGISTER <userName>
   QUIT
   ```

---

## Pruebas Automatizadas

* **Parte 1**:
  `./run_test_parte1.sh`
  valida registro, conexión, publicación, lista, GET\_FILE, borrado…

* **Parte 2**:
  `./run_test_parte2.sh` (adaptado)
  asegura que cada operación lleve timestamp y quede reflejada en `server.log`.

* **Parte 3 (concurrencia + RPC)**:
  
  `./run_test_parte3.sh` (adaptado, un cliente)
  
  `./run_concurrencia_test.sh`
  lanza dos clientes en paralelo y comprueba:

  * Trazas en `server.log`
  * Mensajes en `rpc.log` (output de `log_server`)
  * Correcta transferencia P2P concurrente

* **Pruebas extra (límites y timeout)**:
  `./run_opcional_test.sh`
  comprueba límite de usuarios/ficheros y detección de peers no responsivos.

---

## Ejemplo de salida

```text
$ ./run_concurrencia_test.sh
>>> Levantando log_server (RPC)
>>> Levantando datetime_service
>>> Levantando servidor P2P
>>> Lanzando Alice y Bob en paralelo
...
alice REGISTER 11/05/2025 12:34:56
bob   REGISTER 11/05/2025 12:34:56
...
alice_copy.txt => Hola desde Bob
bob_copy.txt   => Hola desde Alice
+++ TEST DE CONCURRENCIA COMPLETADO +++
```
