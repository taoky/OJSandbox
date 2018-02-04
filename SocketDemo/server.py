#!/usr/bin/env python3

import socket
import pickle
import os
import subprocess

HOST = '0.0.0.0'
PORT = 8001

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(5)

print('Server start at: %s:%s' % (HOST, PORT))
print('wait for connection...')

while True:
    conn, addr = s.accept()
    print('Connected by ', addr)

    while True:
        data = conn.recv(1024)
        data_array = pickle.loads(data)
        # os.system(data) # it's a demo only. NOT SAFE
        print(data_array)
        cp = subprocess.run(["/test/main"] + data_array, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        res = cp.stdout.split('\n')
        if cp.returncode != 0:
            res[0] = 'IE'
        send_res = pickle.dumps(res)
        conn.send(send_res)