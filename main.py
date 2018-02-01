#!/usr/bin/env python3
import os
import sys
import sandbox
import file
import langSupport
from judge import JudgeResult

def OJRun():
    lPlayers = file.listOfPlayers()
    lProblems = file.listOfProblems()

    for thisPlayer in lPlayers:
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
            elif not langSupport.langType(fileExtension.lower()):
                print("Ignored %s: Unsupported file extension." % sourceRelaPath)
                continue
            elif not filename in lProblems:
                print("Ignored %s: Cannot find Problem %s." % (sourceRelaPath, filename))
                continue
            config = file.loadProblemConfig(filename)
            res = sandbox.safeJudge(filename, fileExtension, relaPath, config)
            print('{} on {}: {}'.format(thisPlayer, config['title'], res))

def OJReset():
    file.cleanupWorkspace()

if __name__ == '__main__':
    if len(sys.argv) >= 2:
        if sys.argv[1] == 'cleanup':
            OJReset()
    OJRun()
