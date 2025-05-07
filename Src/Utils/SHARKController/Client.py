import socket

n=1
m=1
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('localhost', 5000))

while m<=n
 message = m
 client.sendall(message.encode())
 m++

client.close()
