#!/usr/bin/env python3

import socket
import pickle
HOST = '127.0.0.1'
PORT = 8001

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

# while True:
# cmd = "/test/main -i /dev/null -o /dev/null -- /bin/ls"
cmd_array = ["-i", "/dev/null", "-o", "/dev/null", "--", "/bin/ls"]
cmd = pickle.dumps(cmd_array)
s.send(cmd)
data_raw = s.recv(1024)
data = pickle.loads(data_raw)
print(data)