import os
import json

supported_extension = [".c", ".cpp"]

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
    return config

def getProblemFiles(problemName):
    ins = [i for i in os.listdir(getProblemDirectory(problemName)) if os.path.splitext(i)[1] == '.in']
    ins.sort()
    outs = [os.path.splitext(i)[0] + ".out" for i in ins]
    res = [(ins[i], outs[i]) for i in range(len(ins))]
    for i in outs:
        if not os.path.exists(getProblemDirectory(problemName) + i):
            print("Problem is not configured properly. %s cannot found." % (getProblemDirectory(problemName) + i))
            raise FileNotFoundError
    return res