import os
import subprocess as sub
import json
import shutil as sh

import docker
from pathlib import Path

workDir = os.getcwd() + '/'
runDir = None
resourceDir = os.getcwd() + '/Backend/'
# initExe = resourceDir + 'init.sh'
backendExe = "/Backend/judger"
inFileName = 'in.tmp'
outFileName = 'out.tmp'

client = docker.from_env()
container = None

def createWorkspace():
    global runDir
    global container
    try:
    #     cp = sub.run([initExe], stdout=sub.PIPE, universal_newlines=True)
    # except FileNotFoundError:
    #     raise
        # build ojs-alpha image
        client.images.build(path=".", tag="ojs-alpha")
        print("Build ojs-alpha image done.")
        volume_path = str(Path("volume").resolve())
        runDir = volume_path
        volumes = {volume_path: {"bind": "/volume", "mode": "rw"}}
        container = client.containers.run("ojs-alpha", detach=True, volumes=volumes, stdin_open=True)
        print("Container {0} is running...".format(container))
    except:
        # Not implemented yet
        print("An error occurred. Is Docker configured correctly?")
        raise
    # if cp.returncode != 0:
    #     # This may be a bit confusing but just use as a wordaround
    #     raise FileNotFoundError("Failed to create workspace")
    # runDir = cp.stdout.split(':')[-1].strip()
    # if not os.path.isdir(runDir):
    #     raise FileNotFoundError("Failed to create workspace")
    # sh.copy(backendExe, runDir)

def cleanupWorkspace():
    # global runDir
    try:
    #     cp = sub.run([initExe, 'cleanup'], stdout=sub.PIPE, universal_newlines=True)
    # except FileNotFoundError:
    #     raise
        if not container == None:
            container.stop()
        else:
            containerList = client.containers.list(filters={"status":"running", "ancestor":"ojs-alpha"})
            for i in containerList:
                i.stop()
        print("Container stopped successfully.")
    except:
        # Not implemented yet
        print("Failed to stop container.")
        raise
    # if cp.returncode != 0:
    #     # This may be a bit confusing but just use as a wordaround
    #     raise FileNotFoundError('Failed to cleanup workspace')

def getRunDir():
    global runDir
    if runDir is None:
        createWorkspace()
    if runDir[-1] != '/':
        runDir += '/'
    return runDir

def safeRemove(f):
    try:
        os.remove(f)
        return True
    except FileNotFoundError:
        return False

def removeHiddenFiles(li):
    return [i for i in li if not i.startswith('.')]

def listOfPlayers():
    return sorted(removeHiddenFiles(os.listdir("Players")))

def listOfProblems():
    return sorted(removeHiddenFiles(os.listdir("Problems")))

def listOfPlayerSources(playerName):
    return sorted(removeHiddenFiles(os.listdir(getPlayerDirectory(playerName))))

def getProblemDirectory(problemName):
    return "Problems/" + problemName + "/"

def getPlayerDirectory(playerName):
    return "Players/" + playerName + "/"

def loadProblemConfig(problemName):
    with open(getProblemDirectory(problemName) + "config.json", "r") as f:
        config = json.loads(f.read())
        f.close()
    return config

def getProblemFiles(problemName):
    ins = [i for i in os.listdir(getProblemDirectory(problemName)) if os.path.splitext(i)[1] == '.in']
    ins.sort()
    outs = [os.path.splitext(i)[0] + ".out" for i in ins]
    res = [(ins[i], outs[i]) for i in range(len(ins))]
    for i in outs:
        if not os.path.exists(getProblemDirectory(problemName) + i):
            raise FileNotFoundError("Problem is not configured properly. {} not found.".format(getProblemDirectory(problemName) + i))
    return res
