#!/usr/bin/env python3
import os
import sandbox
import file

if __name__ == '__main__':
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
            filename, file_extension = os.path.splitext(thisSource)
            if not os.path.isfile(sourceRelaPath):
                print("Ignored %s: Not a file." % sourceRelaPath)
                continue
            elif not file_extension.lower() in file.supported_extension:
                print("Ignored %s: Unsupported file extension." % sourceRelaPath)
                continue
            elif not filename in lProblems:
                print("Ignored %s: Cannot find Problem %s." % (sourceRelaPath, filename))
                continue
            config = file.loadProblemConfig(filename)
            res = sandbox.safe_judge(filename, file_extension, relaPath, config)
            print(thisPlayer, res)
