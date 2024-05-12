from flask import Flask, request, jsonify
from flask_cors import CORS
import socket
import threading
import os

app = Flask(__name__)
CORS(app)

# Retrieve host and port from environment variables, with defaults
HOST = os.getenv('FLASK_RUN_HOST', '127.0.0.1')
FLASK_PORT = int(os.getenv('FLASK_RUN_PORT', 5000))
LISTENER_PORT = int(os.getenv('LISTENER_PORT', 4242))

# Global variables
server_socket = None
client_sockets = []
client_addresses = []  # Store client addresses

# Function to handle a connected client
def handle_client(client_socket):
    with client_socket:
        client_address = client_socket.getpeername()
        print(f"Connected to {client_address}")
        client_sockets.append(client_socket)
        client_addresses.append(client_address)  # Store client address
        try:
            while True:
                data = client_socket.recv(1024)
                if not data:
                    break
        finally:
            client_sockets.remove(client_socket)
            client_addresses.remove(client_address)
            print(f"Disconnected from {client_address}")

# Thread to handle incoming connections
def server_thread():
    global server_socket
    try:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((HOST, LISTENER_PORT))
        server_socket.listen()
        print(f"Listening on {HOST}:{LISTENER_PORT}")
        while True:
            client_socket, client_address = server_socket.accept()
            print(f"Accepted connection from {client_address}")
            threading.Thread(target=handle_client, args=(client_socket,)).start()
    except Exception as e:
        print(f"Server error: {e}")

@app.route('/start', methods=['POST'])
def start_server():
    if not server_socket:
        threading.Thread(target=server_thread).start()
        return jsonify({"message": "Server started", "clients": client_addresses}), 200
    else:
        return jsonify({"message": "Server already running", "clients": client_addresses}), 200

@app.route('/stop', methods=['POST'])
def stop_server():
    global server_socket
    if server_socket:
        for client_socket in client_sockets:
            client_socket.close()
        server_socket.close()
        server_socket = None
        client_sockets.clear()
        client_addresses.clear()
        return jsonify({"message": "Server stopped"}), 200
    return jsonify({"error": "Server not running"}), 400

@app.route('/update_camera_setting', methods=['POST'])
def update_camera_setting():
    data = request.json
    setting = data.get('setting')
    value = data.get('value')

    if setting and isinstance(value, int):
        print(f"Updating {setting} to {value}")
        send_to_robot(setting, value)
        return jsonify({"message": f"{setting} updated to {value}"}), 200
    else:
        return jsonify({"error": "Invalid setting or value"}), 400

# Function to send data to all connected robots
def send_to_robot(setting, value):
    for client_socket in client_sockets:
        try:
            message = f"{setting}:{value}".encode('utf-8')
            client_socket.sendall(message)
            print(f"Sent {setting} value {value} to robot at {client_socket.getpeername()}")
        except Exception as e:
            print(f"Error sending to robot at {client_socket.getpeername()}: {e}")
            client_sockets.remove(client_socket)
            client_addresses.remove(client_socket.getpeername())
            client_socket.close()

if __name__ == "__main__":
    app.run(host=HOST, port=FLASK_PORT)