"""
client.py

Cliente P2P que:
 1. Se comunica con un servidor central para registrar/desregistrar,
    conectar/desconectar, publicar/borrar referencias a ficheros y obtener
    información de pares (IP/puerto).
 2. Lanza un microservicio HTTP para obtener marcas temporales (timestamp)
    antes de cada operación.
 3. Transfiere ficheros directamente P2P entre clientes.
"""
from enum import Enum
import argparse
import requests
import socket
import os

class client :

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    _user = None

    # ******************** METHODS *******************


    @staticmethod
    def register(user):
        """
        REGISTRAR un nuevo usuario:
         1) Conecta al servidor central.
         2) Envía opcode 'REGISTER' + timestamp.
         3) Envía el nombre de usuario.
         4) Lee y procesa el código de retorno.
        """
        try:
            print(f"Trying to connect to {client._server}:{client._port}")
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))

                # 1) Enviar opcode y timestamp
                client._send_op(s, "REGISTER")
                # 2) Enviar nombre de usuario
                print(f"Sending user: {user}")
                s.sendall(user.encode('utf-8') + b'\0')

                # 3) Leer respuesta
                response = s.recv(1)
                if not response:
                    print("REGISTER FAIL")
                    return client.RC.ERROR
                code = response[0]
                print(f"Received code: {code}")
                # 4) Interpretar código
                if code == 0:
                    print("REGISTER OK")
                    client._user = user
                    return client.RC.OK
                elif code == 1:
                    print("USERNAME IN USE")
                    return client.RC.USER_ERROR
                else:
                    print("REGISTER FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print("Exception in register:", e)
            print("REGISTER FAIL")
            return client.RC.ERROR


    @staticmethod
    def unregister(user):
        """
        DAR DE BAJA un usuario:
         - Similar a register, pero con opcode 'UNREGISTER'.
         - Si coincide con el usuario local, lo borra de _user.
        """
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar UNREGISTER + timestamp
                client._send_op(s, "UNREGISTER")
                s.sendall(user.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    if code == 0:
                        if client._user == user:
                            client._user = None # Quita el usuario
                    print("UNREGISTER OK")
                    return client.RC.OK
                elif code == 1:
                    print("USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                else:
                    print("UNREGISTER FAIL")
                    return client.RC.ERROR
        except:
            print("UNREGISTER FAIL")
            return client.RC.ERROR


    
    @staticmethod
    def connect(user):
        """
        CONECTAR un usuario:
         1) Abre un socket de escucha en un puerto efímero para servir GET_FILE.
         2) Envía opcode 'CONNECT'+timestamp + usuario + puerto al servidor.
         3) Activa hilo que atiende conexiones P2P entrantes.
        """
        try:
            # 1) Preparar socket P2P de servidor local
            server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_socket.bind(('', 0))
            server_socket.listen(1)
            port = server_socket.getsockname()[1]

            # Hilo que atenderá peticiones GET FILE de otros peers
            def listen_thread():
                while True:
                    try:
                        conn, addr = server_socket.accept()
                    except OSError:
                        # socket cerrado -> salir del hilo
                        break

                    # 1) Leer operación
                    op = client.read_string(conn)
                    if op == "GET FILE":
                        # 2) Leer nombre de fichero
                        remote_file = client.read_string(conn)
                        # 3) Comprobar existencia
                        if not os.path.isfile(remote_file):
                            conn.sendall(b'\1')           # código = 1 → no existe
                        else:
                            conn.sendall(b'\0')           # código = 0 → existe
                            size = os.path.getsize(remote_file)
                            # enviar tamaño
                            conn.sendall(str(size).encode() + b'\0')
                            # enviar contenido en bloques
                            with open(remote_file, 'rb') as f:
                                while True:
                                    chunk = f.read(1024)
                                    if not chunk:
                                        break
                                    conn.sendall(chunk)
                    # Cerrar siempre la conexión
                    conn.close()

            import threading
            threading.Thread(target=listen_thread, daemon=True).start()
            
            # 2) Conectar al servidor central
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar CONNECT + timestamp
                client._send_op(s, "CONNECT")
                s.sendall(user.encode('utf-8') + b'\0')
                s.sendall(str(port).encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("CONNECT OK")
                    client._user = user
                    return client.RC.OK
                elif code == 1:
                    print("CONNECT FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif code == 2:
                    print("USER ALREADY CONNECTED")
                    return client.RC.USER_ERROR
                else:
                    print("CONNECT FAIL")
                    return client.RC.ERROR
        except:
            print("CONNECT FAIL")
            return client.RC.ERROR


    
    @staticmethod
    def disconnect(user) :
        """
        DESCONECTAR un usuario:
         - Envía opcode 'DISCONNECT' + timestamp y usuario.
         - Libera estado en el servidor central.
        """
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar DISCONNECT + timestamp
                client._send_op(s, "DISCONNECT")
                s.sendall(user.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("DISCONNECT OK")
                    return client.RC.OK
                elif code == 1:
                    print("DISCONNECT FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif code == 2:
                    print("DISCONNECT FAIL, USER NOT CONNECTED")
                    return client.RC.USER_ERROR
                else:
                    print("DISCONNECT FAIL")
                    return client.RC.ERROR
        except:
            print("DISCONNECT FAIL")
            return client.RC.ERROR
        
    @staticmethod
    def publish(fileName, description):
        """
        PUBLICAR un fichero:
         - Envía opcode 'PUBLISH' + timestamp + usuario + ruta + descripción.
         - El servidor guarda sólo la referencia, no el contenido.
        """
        try:
            if client._user is None:
                print("ERROR: No user is connected.")
                return client.RC.ERROR
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar PUBLISH + timestamp
                client._send_op(s, "PUBLISH")
                s.sendall(client._user.encode('utf-8') + b'\0')
                s.sendall(fileName.encode('utf-8') + b'\0')
                s.sendall(description.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("PUBLISH OK")
                    return client.RC.OK
                elif code == 1:
                    print("PUBLISH FAIL, USER DOES NOT EXIST")
                elif code == 2:
                    print("PUBLISH FAIL, USER NOT CONNECTED")
                elif code == 3:
                    print("PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
                else:
                    print("PUBLISH FAIL")
                return client.RC.ERROR
        except:
            print("PUBLISH FAIL")
            return client.RC.ERROR

    @staticmethod
    def delete(fileName):
        """
        BORRAR una publicación previa:
        """
        try:
            if client._user is None:
                print("ERROR: No user is connected.")
                return client.RC.ERROR
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar DELETE + timestamp
                client._send_op(s, "DELETE")
                s.sendall(client._user.encode('utf-8') + b'\0')
                s.sendall(fileName.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("DELETE OK")
                    return client.RC.OK
                elif code == 1:
                    print("DELETE FAIL, USER DOES NOT EXIST")
                elif code == 2:
                    print("DELETE FAIL, USER NOT CONNECTED")
                elif code == 3:
                    print("DELETE FAIL, CONTENT NOT PUBLISHED")
                else:
                    print("DELETE FAIL")
                return client.RC.ERROR
        except:
            print("DELETE FAIL")
            return client.RC.ERROR

    @staticmethod
    def listusers():
        """
        LISTAR usuarios conectados (excepto yo):
        """
        try:
            if client._user is None:
                print("ERROR: No user is connected.")
                return client.RC.ERROR
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar LIST USERS + timestamp
                client._send_op(s, "LIST USERS")
                s.sendall(client._user.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("LIST_USERS OK")
                    count = int(client.read_string(s))
                    for _ in range(count):
                        uname = client.read_string(s)
                        ip = client.read_string(s)
                        port = client.read_string(s)
                        print(f"{uname} {ip} {port}")
                    return client.RC.OK
                elif code == 1:
                    print("LIST_USERS FAIL, USER DOES NOT EXIST")
                elif code == 2:
                    print("LIST_USERS FAIL, USER NOT CONNECTED")
                else:
                    print("LIST_USERS FAIL")
                return client.RC.ERROR
        except Exception as e:
            print("Exception in LIST_USERS:", e)
            print("LIST_USERS FAIL")
            return client.RC.ERROR



    @staticmethod
    def listcontent(user):
        """
        LISTAR contenido publicado por otro usuario:
        """
        try:
            if client._user is None:
                print("ERROR: No user is connected.")
                return client.RC.ERROR
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar LIST CONTENT + timestamp
                client._send_op(s, "LIST CONTENT")
                s.sendall(client._user.encode('utf-8') + b'\0')
                s.sendall(user.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("LIST_CONTENT OK")
                    count = int(client.read_string(s))
                    for _ in range(count):
                        fname = client.read_string(s)
                        print(fname)
                    return client.RC.OK
                elif code == 1:
                    print("LIST_CONTENT FAIL, USER DOES NOT EXIST")
                elif code == 2:
                    print("LIST_CONTENT FAIL, USER NOT CONNECTED")
                elif code == 3:
                    print("LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST")
                else:
                    print("LIST_CONTENT FAIL")
                return client.RC.ERROR
        except:
            print("LIST_CONTENT FAIL")
            return client.RC.ERROR

    @staticmethod
    def getfile(user, remote_FileName, local_FileName):
        """
        DESCARGAR un archivo desde otro peer:
         1) Solicita IP/puerto al servidor central.
         2) Conecta directamente al peer.
         3) Descarga tamaño + contenido.
        """
        # Obtener IP y puerto del usuario desde el servidor
        if client._user is None:
            print("GET_FILE FAIL")
            return client.RC.ERROR
        # 1) Consultar info al servidor
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s_info:
                s_info.connect((client._server, client._port))
                # Enviar GET USER INFO + timestamp
                client._send_op(s_info, "GET USER INFO")
                s_info.sendall(client._user.encode('utf-8') + b'\0')
                s_info.sendall(user.encode('utf-8') + b'\0')

                response = s_info.recv(1)
                if not response or response[0] != 0:
                    print("GET_FILE FAIL")
                    return client.RC.ERROR

                remote_ip = client.read_string(s_info)
                remote_port = int(client.read_string(s_info))
        except Exception as e:
            print("GET_FILE FAIL")
            return client.RC.ERROR
        
        # 2) Conectar P2P y transferir
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((remote_ip, remote_port))
                s.sendall(b"GET FILE\0")
                s.sendall(remote_FileName.encode('utf-8') + b'\0')

                response = s.recv(1)
                if not response:
                    print("GET_FILE FAIL")
                    return client.RC.ERROR
                code = response[0]
                if code == 0:
                    size_str = client.read_string(s)
                    size = int(size_str)
                    with open(local_FileName, 'wb') as f:
                        remaining = size
                        while remaining > 0:
                            data = s.recv(min(1024, remaining))
                            if not data:
                                break
                            f.write(data)
                            remaining -= len(data)
                        if remaining > 0:
                            f.close()
                            os.remove(local_FileName)
                            print("GET_FILE FAIL")
                            return client.RC.ERROR
                    print("GET_FILE OK")
                    return client.RC.OK
                elif code == 1:
                    print("GET_FILE FAIL, FILE NOT EXIST")
                else:
                    print("GET_FILE FAIL")
                return client.RC.ERROR
        except Exception as e:
            if os.path.exists(local_FileName):
                os.remove(local_FileName)
            print("GET_FILE FAIL")
            return client.RC.ERROR
    
    # ******************** MÉTODOS AUXILIARES *******************
    @staticmethod
    def read_string(sock):
        """
        Lee del socket hasta encontrar '\0' y devuelve la cadena sin él.
        """
        buf = bytearray()
        while True:
            b = sock.recv(1)
            if not b or b == b'\0':
                break
            buf.extend(b)
        return buf.decode('utf-8')
    @staticmethod
    def _send_op(s, op_str):
        """
        Envía el opcode (terminado en '\0') y a continuación obtiene
        un timestamp del microservicio Flask en /datetime, también
        NUL-terminated.
        """
        # 1) Opcode
        s.sendall(op_str.encode('utf-8') + b'\0')
        # 2) Timestamp desde el servicio web
        try:
            ts = requests.get('http://127.0.0.1:5000/datetime').text
        except Exception:
            ts = ""
        s.sendall(ts.encode('utf-8') + b'\0')

    # *
    # **
    # * @brief Command interpreter for the client. It calls the protocol functions.
    @staticmethod
    def shell():
        """
        Bucle de interacción por línea de comandos.
        Reconoce comandos REGISTER, CONNECT, PUBLISH, DELETE, LIST_USERS,
        LIST_CONTENT, GET_FILE, DISCONNECT, UNREGISTER y QUIT.
        """
        while (True) :
            try :
                command = input("c> ")
                line = command.split(" ")
                if (len(line) > 0):
                    line[0] = line[0].upper()

                    if (line[0]=="REGISTER") :
                        if (len(line) == 2) :
                            client.register(line[1])
                        else :
                            print("Syntax error. Usage: REGISTER <userName>")

                    elif(line[0]=="UNREGISTER") :
                        if (len(line) == 2) :
                            client.unregister(line[1])
                        else :
                            print("Syntax error. Usage: UNREGISTER <userName>")

                    elif(line[0]=="CONNECT") :
                        if (len(line) == 2) :
                            client.connect(line[1])
                        else :
                            print("Syntax error. Usage: CONNECT <userName>")
                    
                    elif(line[0]=="PUBLISH") :
                        if (len(line) >= 3) :
                            #  Remove first two words
                            description = ' '.join(line[2:])
                            client.publish(line[1], description)
                        else :
                            print("Syntax error. Usage: PUBLISH <fileName> <description>")

                    elif(line[0]=="DELETE") :
                        if (len(line) == 2) :
                            client.delete(line[1])
                        else :
                            print("Syntax error. Usage: DELETE <fileName>")

                    elif(line[0]=="LIST_USERS") :
                        if (len(line) == 1) :
                            client.listusers()
                        else :
                            print("Syntax error. Use: LIST_USERS")

                    elif(line[0]=="LIST_CONTENT") :
                        if (len(line) == 2) :
                            client.listcontent(line[1])
                        else :
                            print("Syntax error. Usage: LIST_CONTENT <userName>")

                    elif(line[0]=="DISCONNECT") :
                        if (len(line) == 2) :
                            client.disconnect(line[1])
                        else :
                            print("Syntax error. Usage: DISCONNECT <userName>")

                    elif(line[0]=="GET_FILE") :
                        if (len(line) == 4) :
                            client.getfile(line[1], line[2], line[3])
                        else :
                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")

                    elif(line[0]=="QUIT") :
                        if (len(line) == 1) :
                            break
                        else :
                            print("Syntax error. Use: QUIT")
                    else :
                        print("Error: command " + line[0] + " not valid.")
            except Exception as e:
                print("Exception: " + str(e))

    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        """Muestra ayuda de uso."""
        print("Usage: python3 client.py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        """Parsea y valida argumentos -s <server> -p <port>."""
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 client.py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535");
            return False;
        
        client._server = args.s
        client._port = args.p

        return True


    # ******************** MAIN *********************
    @staticmethod
    def main(argv) :
        """Punto de entrada: parsea args y arranca shell."""
        if (not client.parseArguments(argv)) :
            client.usage()
            return

        #  Write code here
        client.shell()
        print("+++ FINISHED +++")
    

if __name__=="__main__":
    import sys
    client.main([])