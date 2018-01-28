import os
import sandbox
import file
from judgeResult import JudgeResult

def runTest(player, problem):
    relaPath = file.getPlayerDirectory(thisPlayer)
    lSources = file.listOfPlayerSources(thisPlayer)
    for thisSource in lSources:
        sourceRelaPath = relaPath + thisSource
        filename, fileExtension = os.path.splitext(thisSource)
        config = file.loadProblemConfig(filename)
        res = sandbox.safeJudge(filename, fileExtension, relaPath, config)
        return thisPlayer, res
