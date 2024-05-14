from flask import Flask, request, jsonify
from flask_cors import CORS
import socket
import threading
import os
from struct import pack

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

@app.route('/update_camera_settings', methods=['POST'])
def update_camera_settings():
    data = request.json
    settings = data.get('settings')

    # Define the ranges and data types
    ranges = [
        (0, 1),           # autoExposure (1 byte)
        (-255, 255),      # autoExposureBrightness (4 bytes, int32)
        (0, 1048575),     # exposure (4 bytes, int32)
        (0, 1023),        # gain (4 bytes, int32)
        (0, 1),           # autoWhiteBalance (1 byte)
        (0, 1),           # autoFocus (1 byte)
        (0, 250),         # focus (4 bytes, int32)
        (0, 1),           # autoHue (1 byte)
        (-180, 180),      # hue (4 bytes, int32)
        (0, 255),         # saturation (1 byte)
        (0, 255),         # contrast (1 byte)
        (0, 9),           # sharpness (1 byte)
        (0, 4095),        # redGain (4 bytes, int32)
        (0, 4095),        # greenGain (4 bytes, int32)
        (0, 4095)         # blueGain (4 bytes, int32)
    ]

    # Ensure all settings are within the correct ranges
    for i, (min_val, max_val) in enumerate(ranges):
        if not (min_val <= settings[i] <= max_val):
            return jsonify({"error": f"Setting {i} out of range"}), 400


    def pack_data(settings, ranges):
        packed_data = bytearray()
        for i, (min_val, max_val) in enumerate(ranges):
            if min_val >= -128 and max_val <= 127:  # 1 byte signed
                packed_data.extend(pack('b', settings[i]))
            elif min_val >= 0 and max_val <= 255:  # 1 byte unsigned
                packed_data.extend(pack('B', settings[i]))
            else:  # 4 bytes signed (int32)
                packed_data.extend(pack('i', settings[i]))
        return packed_data
    
    packed_data = pack_data(settings, ranges)

    print(f"Packed data: {list(packed_data)}")
    send_to_robot(packed_data)
    return jsonify({"message": "Settings updated"}), 200

# Function to send data to all connected robots
def send_to_robot(packed_data):
    for client_socket in client_sockets:
        try:
            client_socket.sendall(packed_data)
            print(f"Sent settings to robot at {client_socket.getpeername()}")
        except Exception as e:
            print(f"Error sending to robot at {client_socket.getpeername()}: {e}")
            client_sockets.remove(client_socket)
            client_addresses.remove(client_socket.getpeername())
            client_socket.close()


if __name__ == "__main__":
    app.run(host=HOST, port=FLASK_PORT)