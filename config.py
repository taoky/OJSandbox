import os
import json

g = {
    "max-processes": 128,
    "output-size": 16,
    "compile-time": 5000,
    "compile-memory": 256,
    "mem-bonus": {
    },
    "time-bonus": {
    },
    "enabled": ['.c', '.cpp', '.py'],
}

def loadConfig():
    global g
    try:
        with open(os.path.dirname(__file__) + '/config.json') as f:
            fileconf = json.loads(f.read())
            for key in fileconf:
                g[key] = fileconf[key]
        print('Config loaded from config.json')
    except FileNotFoundError:
        print('Warning: config.json not found. Default config used.')


def generateConfig():
    with open(os.path.dirname(__file__) + '/Backend/config.sh', "w") as f:
        f.write('RAMDISKSIZE={}\n'.format(g['output-size'] + 8))
