import os
from shutil import copy
import subprocess
import compare
import config
import file
import langSupport
from judge import RunInfo, JudgeResult, JudgeError
from debug import dprint

infoFile = file.workDir

def executeProgramBackend(command, **options):
    if not 'dir' in options:
        options['dir'] = file.getRunDir()
    running = langSupport.formatBackendHelper(command, **options)
    pwd = os.getcwd()
    cp = subprocess.run(running, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    res = cp.stdout.split('\n')

    dprint("stdout: {}".format(res))
    dprint("stderr: {}".format(cp.stderr.split('\n')))

    try:
        stat = [int(i) for i in res[1].split(' ')]
    except IndexError:
        stat = [0] * 4
    runInfo = RunInfo(stat[0], stat[3])
    if cp.returncode != 0:
        res[0] = 'IE'
    return JudgeResult(getattr(JudgeResult, res[0].strip()), runInfo)

def plainJudge(program, codeType, infile, outfile, **config):
    inRedir = file.inFileName
    outRedir = file.outFileName
    copy(infile, file.getRunDir() + inRedir)
    runHelper = langSupport.executeHelper[codeType]
    running = langSupport.formatHelper(runHelper, exefile=program)
    runResult = executeProgramBackend(None, dir=file.getRunDir(), src=program,
        stdin=file.getRunDir() + inRedir, stdout=file.getRunDir() + outRedir,
        timeout=config['timeout'], memory=config['ram'])
    rp = runResult.value
    runInfo = runResult.res

    forwardResults = [JudgeResult.RE, JudgeResult.TLE, JudgeResult.MLE, JudgeResult.FSE]
    if rp in forwardResults:
        file.safeRemove(file.getRunDir() + inRedir)
        file.safeRemove(file.getRunDir() + outRedir)
        return JudgeResult(rp, runInfo)
    copy(file.getRunDir() + outRedir, os.getcwd())

    compareMethod = compare.getCompareMethod(config["compare"])
    cp = compareMethod(outfile, file.runDir + outRedir)
    file.safeRemove(outRedir) # cleanup
    if cp == False:
        return JudgeResult(JudgeResult.WA, runInfo)
    return JudgeResult(JudgeResult.AC, runInfo)

def judgeProcess(sourceFileName, sourceFileExt, directory, problemConfig):
    exefileName = 'out'
    rsourceFileName = file.getRunDir() + exefileName
    rsourceCodeName = directory + sourceFileName + sourceFileExt
    results = []
    
    try:
        compileHelper = langSupport.compileHelper[sourceFileExt.lower()][:]
        compiling = langSupport.formatHelper(compileHelper, infile=sourceFileName+sourceFileExt, outfile=exefileName)
    except KeyError as e:
        return JudgeError(JudgeResult.FTE)
    
    cps = executeProgramBackend(compiling, dir=file.getRunDir(), src=rsourceCodeName,
        stdin='/dev/null', stdout='/dev/null',
        timeout=config.g['compile-time'], memory=config.g['compile-memory'],
        noseccomp=None, multiprocess=None, copyback=exefileName)
    if not JudgeResult.isOK(cps.value):
        return JudgeError(JudgeResult.CE)
    proFiles = file.getProblemFiles(sourceFileName)
    firstError = None
    runCount = 0
    for i in proFiles:
        runCount += 1
        infile = file.getProblemDirectory(sourceFileName) + i[0]
        outfile = file.getProblemDirectory(sourceFileName) + i[1]
        proFileName = os.path.splitext(infile)[0]
        result = plainJudge(exefileName, sourceFileExt.lower(), infile, outfile, **problemConfig)
        results.append(result)
        if result.value != JudgeResult.AC:
            firstError = result.value
            break

    if firstError is None:
        firstError = JudgeResult.AC
    os.remove(exefileName) # remove the binary file
    sumInfo = RunInfo()
    for i in results:
        sumInfo += i.res
    avgInfo = sumInfo / runCount
    return JudgeResult(firstError, avgInfo)

def safeJudge(sourceFileName, sourceFileExt, directory, problemConfig):
    try:
        # Forward everything
        return judgeProcess(sourceFileName, sourceFileExt, directory, problemConfig)
    except JudgeError as e:
        # Cleanup
        keys = ['exe', 'in', 'out']
        for i in keys:
            try:
                os.remove(e.res[i])
            except KeyError:
                pass
        return e, None
