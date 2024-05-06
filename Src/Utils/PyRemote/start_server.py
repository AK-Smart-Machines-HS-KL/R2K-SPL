import socket
import signal
import sys

# Server configuration
HOST = '0.0.0.0'  # Listen on all network interfaces (adjust as needed)
PORT = 4242       # Port to listen on

server = None

def stop_server(signum, frame):
    global server
    if server:
        server.close()
    print("Server stopped gracefully.")
    sys.exit(0)

# Bind the signal handler
signal.signal(signal.SIGINT, stop_server)
signal.signal(signal.SIGTERM, stop_server)

# Function to handle a connected client
def handle_client(client_socket):
    with client_socket:
        print(f"Connected to {client_socket.getpeername()}")
        while True:
            data = client_socket.recv(4)  # Integer size (4 bytes)
            if not data:
                break

            # Unpack the integer (heartbeat value)
            heartbeat_value = int.from_bytes(data, byteorder='little', signed=True)
            print(f"Received heartbeat value: {heartbeat_value}")
            
            # Respond with a heartbeat value
            heartbeat_value = 40
            heartbeat_bytes = heartbeat_value.to_bytes(4, byteorder='little', signed=True)
            client_socket.sendall(heartbeat_bytes)
            print(f"Sent heartbeat value: {heartbeat_value}")

# Main server function
def start_server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.bind((HOST, PORT))
        server.listen()
        print(f"Listening on {HOST}:{PORT}")
        while True:
            client_socket, client_address = server.accept()
            print(f"Accepted connection from {client_address}")
            handle_client(client_socket)

if __name__ == "__main__":
    start_server()