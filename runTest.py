import os
import sandbox
import file
from judgeResult import JudgeResult

def runTest(player, problem):
    relaPath = file.getPlayerDirectory(thisPlayer)
    if not os.path.isdir(relaPath):
        print("Ignored %s: Not a directory." % thisPlayer)
        continue
    lSources = file.listOfPlayerSources(thisPlayer)
    for thisSource in lSources:
        sourceRelaPath = relaPath + thisSource
        filename, fileExtension = os.path.splitext(thisSource)
        if not os.path.isfile(sourceRelaPath):
            print("Ignored %s: Not a file." % sourceRelaPath)
            continue
        elif not fileExtension.lower() in file.supported_extension:
            print("Ignored %s: Unsupported file extension." % sourceRelaPath)
            continue
        elif not filename in lProblems:
            print("Ignored %s: Cannot find Problem %s." % (sourceRelaPath, filename))
            continue
        config = file.loadProblemConfig(filename)
        res = sandbox.safeJudge(filename, fileExtension, relaPath, config)
        print('{}: {}'.format(thisPlayer, res))
