import os
import subprocess as sub
import json
import shutil
from debug import dprint

dataDir = os.path.dirname(__file__) + '/'
workDir = os.getcwd() + '/'
fnull = open(os.devnull, 'w')
runDir = None
chrootDir = None
resourceDir = os.getcwd() + '/Backend/'
initExe = resourceDir + 'init.sh'
backendExe = resourceDir + 'safeJudger'
inFileName = 'in.tmp'
outFileName = 'out.tmp'
errFileName = 'err.tmp'
tempDir = '/tmp/'

def createWorkspace():
    global runDir, chrootDir
    try:
        cp = sub.run(['sudo', initExe], stdout=sub.PIPE, universal_newlines=True)
    except FileNotFoundError:
        raise
    except:
        # Not implemented yet
        raise NotImplementedError
    if cp.returncode != 0:
        # This may be a bit confusing but just use as a wordaround
        raise FileNotFoundError("Failed to create workspace")
    runDir = chrootDir = cp.stdout.split(':')[-1].strip()
    runDir += "/tmp/"
    if not os.path.isdir(runDir):
        raise FileNotFoundError("Failed to create workspace")


def cleanupWorkspace():
    # global runDir
    try:
        cp = sub.run(['sudo', initExe, 'cleanup'], stdout=sub.PIPE, stderr=sub.PIPE, universal_newlines=True)
    except FileNotFoundError:
        raise
    except:
        # Not implemented yet
        raise NotImplementedError
    if cp.returncode != 0:
        # This may be a bit confusing but just use as a wordaround
        raise FileNotFoundError('Failed to cleanup workspace')
    return cp

def getRunDir():
    global runDir
    if runDir is None:
        createWorkspace()
    if runDir[-1] != '/':
        runDir += '/'
    return runDir

def getchrootDir():
    global chrootDir
    if chrootDir is None:
        createWorkspace()
    if chrootDir[-1] != '/':
        chrootDir += '/'
    return chrootDir


def safeRemove(f):
    try:
        os.remove(f)
        return True
    except (FileNotFoundError, PermissionError, IsADirectoryError):
        return False

def cleanupRunDir():
    # os.system("find " + getRunDir() + " -delete >/dev/null 2>&1")
    shutil.rmtree(getRunDir(), ignore_errors=True)
    return True

def removeHiddenFiles(li):
    return [i for i in li if not i.startswith('.')]

def listOfPlayers():
    return sorted(removeHiddenFiles(os.listdir(dataDir + "Players")))

def listOfProblems():
    return sorted(removeHiddenFiles(os.listdir(dataDir + "Problems")))

def listOfPlayerSources(playerName):
    return sorted(removeHiddenFiles(os.listdir(getPlayerDirectory(playerName))))

def getProblemDirectory(problemName):
    return dataDir + "Problems/" + problemName + "/"

def getPlayerDirectory(playerName):
    return dataDir + "Players/" + playerName + "/"

def getPlayerInfo(playerName):
    try:
        with open(getPlayerDirectory(playerName) + "info.json") as f:
            return json.loads(f.read())
    except FileNotFoundError:
        return None

def loadProblemConfig(problemName):
    with open(getProblemDirectory(problemName) + "config.json", "r") as f:
        config = json.loads(f.read())
        f.close()
    return config

def getProblemFiles(problemName):
    ins = [i for i in os.listdir(getProblemDirectory(problemName)) if os.path.splitext(i)[-1] == '.in']
    ins.sort()
    outs = [os.path.splitext(i)[0] + ".out" for i in ins]
    res = [(ins[i], outs[i]) for i in range(len(ins))]
    for i in outs:
        if not os.path.exists(getProblemDirectory(problemName) + i):
            raise FileNotFoundError("Problem is not configured properly. {} not found.".format(getProblemDirectory(problemName) + i))
    return res
