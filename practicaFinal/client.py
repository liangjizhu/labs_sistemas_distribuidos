from enum import Enum
import argparse
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

    # ******************** METHODS *******************


    @staticmethod
    def register(user):
        try:
            print(f"Trying to connect to {client._server}:{client._port}")
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))

                s.sendall(b"REGISTER\0")

                print(f"Sending user: {user}")
                s.sendall(user.encode('utf-8') + b'\0')

                response = s.recv(1)
                if not response:
                    print("c> REGISTER FAIL")
                    return client.RC.ERROR

                code = response[0]
                print(f"Received code: {code}")
                if code == 0:
                    print("c> REGISTER OK")
                    return client.RC.OK
                elif code == 1:
                    print("c> USERNAME IN USE")
                    return client.RC.USER_ERROR
                else:
                    print("c> REGISTER FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print("Exception in register:", e)
            print("c> REGISTER FAIL")
            return client.RC.ERROR


    @staticmethod
    def unregister(user) :
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b"UNREGISTER\0")
                s.sendall(user.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("c> UNREGISTER OK")
                    return client.RC.OK
                elif code == 1:
                    print("c> USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                else:
                    print("c> UNREGISTER FAIL")
                    return client.RC.ERROR
        except:
            print("c> UNREGISTER FAIL")
            return client.RC.ERROR


    
    @staticmethod
    def connect(user) :
        try:
            server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_socket.bind(('', 0))
            server_socket.listen(1)
            port = server_socket.getsockname()[1]

            def listen_thread():
                while True:
                    conn, addr = server_socket.accept()
                    conn.close()  # Placeholder

            import threading
            threading.Thread(target=listen_thread, daemon=True).start()

            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b"CONNECT\0")
                s.sendall(user.encode('utf-8') + b'\0')
                s.sendall(str(port).encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("c> CONNECT OK")
                    return client.RC.OK
                elif code == 1:
                    print("c> CONNECT FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif code == 2:
                    print("c> USER ALREADY CONNECTED")
                    return client.RC.USER_ERROR
                else:
                    print("c> CONNECT FAIL")
                    return client.RC.ERROR
        except:
            print("c> CONNECT FAIL")
            return client.RC.ERROR


    
    @staticmethod
    def disconnect(user) :
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b"DISCONNECT\0")
                s.sendall(user.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("c> DISCONNECT OK")
                    return client.RC.OK
                elif code == 1:
                    print("c> DISCONNECT FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif code == 2:
                    print("c> DISCONNECT FAIL, USER NOT CONNECTED")
                    return client.RC.USER_ERROR
                else:
                    print("c> DISCONNECT FAIL")
                    return client.RC.ERROR
        except:
            print("c> DISCONNECT FAIL")
            return client.RC.ERROR
        
    @staticmethod
    def publish(fileName, description):
        try:
            username = "liang" 
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b"PUBLISH\0")
                s.sendall(username.encode('utf-8') + b'\0')
                s.sendall(fileName.encode('utf-8') + b'\0')
                s.sendall(description.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("c> PUBLISH OK")
                    return client.RC.OK
                elif code == 1:
                    print("c> PUBLISH FAIL, USER DOES NOT EXIST")
                elif code == 2:
                    print("c> PUBLISH FAIL, USER NOT CONNECTED")
                elif code == 3:
                    print("c> PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
                else:
                    print("c> PUBLISH FAIL")
                return client.RC.ERROR
        except:
            print("c> PUBLISH FAIL")
            return client.RC.ERROR

    @staticmethod
    def delete(fileName):
        try:
            username = "liang" 
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b"DELETE\0")
                s.sendall(username.encode('utf-8') + b'\0')
                s.sendall(fileName.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("c> DELETE OK")
                    return client.RC.OK
                elif code == 1:
                    print("c> DELETE FAIL, USER DOES NOT EXIST")
                elif code == 2:
                    print("c> DELETE FAIL, USER NOT CONNECTED")
                elif code == 3:
                    print("c> DELETE FAIL, CONTENT NOT PUBLISHED")
                else:
                    print("c> DELETE FAIL")
                return client.RC.ERROR
        except:
            print("c> DELETE FAIL")
            return client.RC.ERROR

    @staticmethod
    def listusers():
        try:
            username = "liang"
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b"LIST USERS\0")
                s.sendall(username.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("c> LIST_USERS OK")
                    count = int(s.recv(256).decode().strip('\0'))
                    for _ in range(count):
                        uname = s.recv(256).decode().strip('\0')
                        ip = s.recv(256).decode().strip('\0')
                        port = s.recv(256).decode().strip('\0')
                        print(f"{uname} {ip} {port}")
                    return client.RC.OK
                elif code == 1:
                    print("c> LIST_USERS FAIL, USER DOES NOT EXIST")
                elif code == 2:
                    print("c> LIST_USERS FAIL, USER NOT CONNECTED")
                else:
                    print("c> LIST_USERS FAIL")
                return client.RC.ERROR
        except:
            print("c> LIST_USERS FAIL")
            return client.RC.ERROR

    @staticmethod
    def listcontent(user):
        try:
            username = "liang"
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b"LIST CONTENT\0")
                s.sendall(username.encode('utf-8') + b'\0')
                s.sendall(user.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    print("c> LIST_CONTENT OK")
                    count = int(s.recv(256).decode().strip('\0'))
                    for _ in range(count):
                        fname = s.recv(256).decode().strip('\0')
                        print(fname)
                    return client.RC.OK
                elif code == 1:
                    print("c> LIST_CONTENT FAIL, USER DOES NOT EXIST")
                elif code == 2:
                    print("c> LIST_CONTENT FAIL, USER NOT CONNECTED")
                elif code == 3:
                    print("c> LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST")
                else:
                    print("c> LIST_CONTENT FAIL")
                return client.RC.ERROR
        except:
            print("c> LIST_CONTENT FAIL")
            return client.RC.ERROR

    @staticmethod
    def getfile(user, remote_FileName, local_FileName):
        try:
            # El cliente  obtiene IP/puerto del usuario con LIST_USERS
            remote_ip = "liang"
            remote_port = 9999

            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((remote_ip, remote_port))
                s.sendall(b"GET FILE\0")
                s.sendall(remote_FileName.encode('utf-8') + b'\0')

                code = s.recv(1)[0]
                if code == 0:
                    size_str = s.recv(256).decode().strip('\0')
                    size = int(size_str)
                    with open(local_FileName, 'wb') as f:
                        remaining = size
                        while remaining > 0:
                            data = s.recv(min(1024, remaining))
                            if not data:
                                break
                            f.write(data)
                            remaining -= len(data)
                    print("c> GET_FILE OK")
                    return client.RC.OK
                elif code == 1:
                    print("c> GET_FILE FAIL, FILE NOT EXIST")
                else:
                    print("c> GET_FILE FAIL")
                return client.RC.ERROR
        except:
            if os.path.exists(local_FileName):
                os.remove(local_FileName)
            print("c> GET_FILE FAIL")
            return client.RC.ERROR

    # *
    # **
    # * @brief Command interpreter for the client. It calls the protocol functions.
    @staticmethod
    def shell():

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
        print("Usage: python3 client.py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
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
        if (not client.parseArguments(argv)) :
            client.usage()
            return

        #  Write code here
        client.shell()
        print("+++ FINISHED +++")
    

if __name__=="__main__":
    client.main([])