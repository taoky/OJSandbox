#!/usr/bin/env python3
import os
import sys
import sandbox
import config
import file
import langSupport
from debug import dprint

def OJRun():
    global config
    config.loadConfig()
    config.generateConfig()

    lPlayers = file.listOfPlayers()
    lProblems = file.listOfProblems()

    for thisPlayer in lPlayers:
        relaPath = file.getPlayerDirectory(thisPlayer)
        if not os.path.isdir(relaPath):
            print("Ignored %s: Not a directory." % thisPlayer)
            continue
        pInfo = file.getPlayerInfo(thisPlayer)
        if pInfo is None:
            print("No information registered for player {}, skipping".format(thisPlayer))
            continue
        lSources = file.listOfPlayerSources(thisPlayer)
        for thisSource in lSources:
            sourceRelaPath = relaPath + thisSource
            filename, fileExtension = os.path.splitext(thisSource)
            if not os.path.isfile(sourceRelaPath):
                print("Ignored %s: Not a file." % sourceRelaPath)
                continue
            elif not langSupport.langType(fileExtension.lower()):
                if thisSource != 'info.json':
                    print("Ignored %s: Unsupported file extension." % sourceRelaPath)
                continue
            elif filename not in lProblems:
                print("Ignored %s: Cannot find Problem %s." % (sourceRelaPath, filename))
                continue
            config = file.loadProblemConfig(filename)
            res = sandbox.safeJudge(filename, fileExtension, relaPath, config)
            print('\x1B[1m{0}\x1B[0m on \x1B[1m{1}\x1B[0m: {2} ({3})'.format(pInfo['name'], config['title'], res.pretty(), res.res))

def OJReset():
    file.cleanupWorkspace()

if __name__ == '__main__':
    os.chdir(file.tempDir)
    routine = OJRun
    for opt in sys.argv[1:]:
        if opt == 'cleanup':
            routine = OJReset
        elif opt == 'debug':
            dprint.enable()
    routine()
    exit(0)
