from flask import Flask, request, jsonify
from flask_cors import CORS
import socket
import threading
import os
import struct

app = Flask(__name__)
CORS(app)

# Retrieve host and port from environment variables, with defaults
HOST = os.getenv('FLASK_RUN_HOST', '0.0.0.0')
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

def send_to_robot(data):
    for client_socket in client_sockets:
        try:
            client_socket.sendall(data)
            print(f"Sent to robot at {client_socket.getpeername()}: {data}")
        except Exception as e:
            print(f"Error sending to robot at {client_socket.getpeername()}: {e}")
            client_sockets.remove(client_socket)
            client_addresses.remove(client_socket.getpeername())
            client_socket.close()

def create_endpoint(setting_name, setting_id, min_val, max_val, fmt):
    @app.route(f'/update_{setting_name}', methods=['POST'], endpoint=f'update_{setting_name}')
    def update_setting():
        value = request.json.get(setting_name)
        if value is None or not (min_val <= value <= max_val):
            return jsonify({"error": f"Invalid {setting_name} value"}), 400
        packed_id = struct.pack('B', setting_id)  # Assuming each setting ID fits in one byte
        packed_data = struct.pack(fmt, value)
        full_message = packed_id + packed_data  # Concatenate setting ID and data
        send_to_robot(full_message)
        return jsonify({"message": f"{setting_name.capitalize()} updated"}), 200
    return update_setting

# Define the settings with their properties
settings_info = {
    'autoExposure': (0x01, 0, 1, 'B'),
    'autoExposureBrightness': (0x02, -255, 255, 'i'),
    'exposure': (0x03, 0, 1048575, 'I'),
    'gain': (0x04, 0, 1023, 'I'),
    'autoWhiteBalance': (0x05, 0, 1, 'B'),
    'autoFocus': (0x06, 0, 1, 'B'),
    'focus': (0x07, 0, 250, 'I'),
    'autoHue': (0x08, 0, 1, 'B'),
    'hue': (0x09, -180, 180, 'i'),
    'saturation': (0x0A, 0, 255, 'B'),
    'contrast': (0x0B, 0, 255, 'B'),
    'sharpness': (0x0C, 0, 9, 'B'),
    'redGain': (0x0D, 0, 4095, 'I'),
    'greenGain': (0x0E, 0, 4095, 'I'),
    'blueGain': (0x0F, 0, 4095, 'I')
}

for setting, (setting_id, min_val, max_val, fmt) in settings_info.items():
    create_endpoint(setting, setting_id, min_val, max_val, fmt)


if __name__ == "__main__":
    app.run(host=HOST, port=FLASK_PORT)