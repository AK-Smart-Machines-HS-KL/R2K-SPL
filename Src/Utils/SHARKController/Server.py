import socket
import threading

clients = []

def handle_client(conn):
    clients.append(conn)
    while True:
        try:
            data = conn.recv(1024)
            if not data:
                break
            # Send data to all other clients
            for client in clients:
                if client != conn:
                    client.sendall(data)	
        except:
            break
    conn.close()
    clients.remove(conn)

def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(('localhost', 5000))
    server.listen()
    print("Server listening on port 5000")
    while True:
        conn, addr = server.accept()
        threading.Thread(target=handle_client, args=(conn,), daemon=True).start()

start_server()
