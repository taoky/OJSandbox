import os
import json

g = {
    "max-processes": 128,
    "output-size": 16,
    "compile-time": 5000,
    "compile-memory": 128
}

def loadConfig():
    global g
    try:
        with open('config.json') as f:
            g = json.loads(f.read())
        print('Config loaded from config.json')
    except FileNotFoundError:
        print('Warning: config.json not found. Default config used.')


def generateConfig():
    with open('Backend/config.sh', "w") as f:
        f.write('RAMDISKSIZE={}\n'.format(g['output-size'] + 8))

