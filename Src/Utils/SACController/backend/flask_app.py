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
LISTENER_PORT = int(os.getenv('LISTENER_PORT', 5050))

# Global variables
server_socket = None
client_sockets = []
client_addresses = []  # Store client addresses

# Mapping of behaviors to integers
behavior_mapping = {
    "InitialCard": 0,
    "SearchForBallCard": 1,
    "SACCard": 2,
    "GoToBallPassToMateCard": 3
}

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

@app.route('/behavior', methods=['POST'])
def update_behavior():
    behavior = request.json.get('behavior')
    if not behavior or behavior not in behavior_mapping:
        return jsonify({"error": "Invalid or missing behavior"}), 400
    
    behavior_int = behavior_mapping[behavior]
    
    # Pack the behavior integer to send to the robot
    packed_id = struct.pack('B', 0x02)  # Assuming 0x02 is the ID for behavior commands
    packed_data = struct.pack('B', behavior_int)
    full_message = packed_id + packed_data  # Concatenate ID and data
    
    send_to_robot(full_message)
    return jsonify({"message": f"Behavior updated to {behavior} ({behavior_int})"}), 200

@app.route('/mode', methods=['POST'])
def update_mode():
    mode = request.json.get('mode')
    if mode not in [0, 1]:
        return jsonify({"error": "Invalid mode value"}), 400
    
    # Pack the mode integer to send to the robot
    packed_id = struct.pack('B', 0x03)  # Assuming 0x03 is the ID for mode commands
    packed_data = struct.pack('B', mode)
    full_message = packed_id + packed_data  # Concatenate ID and data
    
    send_to_robot(full_message)
    return jsonify({"message": f"Mode updated to {mode}"}), 200

@app.route('/direction', methods=['POST'])
def update_direction():
    direction = request.json.get('direction')
    if direction not in [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]:
        return jsonify({"error": "Invalid direction value"}), 400
    
    # Pack the direction integer to send to the robot
    packed_id = struct.pack('B', 0x04)  # Assuming 0x04 is the ID for direction commands
    packed_data = struct.pack('B', direction)
    full_message = packed_id + packed_data  # Concatenate ID and data
    
    send_to_robot(full_message)
    return jsonify({"message": f"Direction updated to {direction}"}), 200

if __name__ == "__main__":
    app.run(host=HOST, port=FLASK_PORT)