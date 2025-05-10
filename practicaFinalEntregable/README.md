# Sistema Distribuido P2P con Servicio HTTP y RPC de Logs

Este repositorio contiene la solución completa de la práctica de Sistemas Distribuidos, incluyendo:

- **Parte 1**: Servicio P2P básico (REGISTER, CONNECT, PUBLISH, LIST, GET_FILE, DELETE, DISCONNECT).  
- **Parte 2**: Integración de un microservicio HTTP que provee un timestamp (`datetime_service.py`).  
- **Parte 3**: Envío de logs a un servidor ONC-RPC (`log_server`) tras cada operación.

---

## Requisitos

1. **Sistema operativo**: Linux (se ha probado en Ubuntu 22.04).  
2. **Python 3** con los paquetes:
   ```bash
   pip install flask requests
