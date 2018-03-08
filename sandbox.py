import os
from shutil import copy
import subprocess
import compare
import config
import file
import langSupport
from judge import RunInfo, JudgeResult, JudgeError
from debug import dprint

# infoFile = file.workDir

def executeProgramBackend(command, **options):
    if 'dir' not in options:
        options['dir'] = file.getchrootDir()
    running = langSupport.formatBackendHelper(command, **options)

    dprint('\x1B[1;33mRunning:\x1B[0m ' + ' '.join(running))

    cp = subprocess.run(running, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    res = cp.stdout.split('\n')

    dprint("\x1B[1;36m=== Captured stdout ===\x1B[0m\n" + cp.stdout.rstrip())
    dprint("\x1B[1;35m=== Captured stderr ===\x1B[0m\n" + cp.stderr.rstrip())

    try:
        stat = [int(i) for i in res[1].split(' ')]
    except IndexError:
        stat = [0] * 4
    runInfo = RunInfo(stat[0], stat[3])
    if cp.returncode != 0:
        res[0] = 'IE'
    return JudgeResult(getattr(JudgeResult, res[0].strip()), runInfo)

def plainJudge(program, codeType, infile, outfile, runSettings={}, **config):
    inRedir = file.inFileName
    outRedir = file.outFileName
    copy(infile, file.getRunDir() + inRedir)
    runHelper = langSupport.executeHelper[codeType]
    running = langSupport.formatHelper(runHelper, exefile=program)
    runResult = executeProgramBackend(running, dir=file.getchrootDir(), src=program,
        stdin=file.getRunDir() + inRedir, stdout=file.getRunDir() + outRedir,
        timeout=config['timeout'], memory=config['ram'],
        noseccomp='noseccomp' in runSettings, multiprocess='multiprocess' in runSettings)
    rp = runResult.value
    runInfo = runResult.res

    forwardResults = [JudgeResult.RE, JudgeResult.TLE, JudgeResult.MLE, JudgeResult.FSE]
    if rp in forwardResults:
        file.safeRemove(file.getRunDir() + inRedir)
        file.safeRemove(file.getRunDir() + outRedir)
        return JudgeResult(rp, runInfo)
    copy(file.getRunDir() + outRedir, os.getcwd())

    compareMethod = compare.getCompareMethod(config["compare"])
    compareResult = compareMethod(outfile, file.runDir + outRedir)
    file.safeRemove(outRedir)  # cleanup
    if not compareResult:
        return JudgeResult(JudgeResult.WA, runInfo)
    return JudgeResult(JudgeResult.AC, runInfo)

def judgeProcess(sourceFileName, sourceFileExt, directory, problemConfig):
    exefileName = langSupport.exeName[sourceFileExt]
    # rsourceFileName = file.getRunDir() + exefileName
    rsourceCodeName = directory + sourceFileName + sourceFileExt
    results = []

    try:
        copyTarget = file.tempDir + langSupport.canonicalName[sourceFileExt]
        copy(rsourceCodeName, copyTarget)
        rsourceCodeName = copyTarget 
        compileHelper = langSupport.compileHelper[sourceFileExt.lower()][:]
        compiling = langSupport.formatHelper(compileHelper, infile=langSupport.canonicalName[sourceFileExt], outfile=exefileName)
    except KeyError as e:
        return JudgeError(JudgeResult.FTE)

    cps = executeProgramBackend(compiling, dir=file.getchrootDir(), src=rsourceCodeName,
        stdin='/dev/null', stdout='/dev/null',
        timeout=config.g['compile-time'], memory=config.g['compile-memory'],
        noseccomp=True, multiprocess=True, copyback=exefileName, vmlimit=sourceFileExt != '.java')
    if not JudgeResult.isOK(cps.value):
        return JudgeError(JudgeResult.CE)
    file.safeRemove(rsourceCodeName)
    
    proFiles = file.getProblemFiles(sourceFileName)
    firstError = None
    runCount = 0
    runSettings = {'noseccomp': True, 'multiprocess': True} if sourceFileExt == '.java' else {}
    problemConfig = dict(problemConfig)
    problemConfig['ram'] = int(problemConfig['ram'] * config.g['mem-bonus'].get(sourceFileExt, 1.0))
    problemConfig['timeout'] = int(problemConfig['timeout'] * config.g['time-bonus'].get(sourceFileExt, 1.0))
    for i in proFiles:
        runCount += 1
        infile = file.getProblemDirectory(sourceFileName) + i[0]
        outfile = file.getProblemDirectory(sourceFileName) + i[1]
        # proFileName = os.path.splitext(infile)[0]
        result = plainJudge(exefileName, sourceFileExt.lower(), infile, outfile, **problemConfig, runSettings=runSettings)
        results.append(result)
        if result.value != JudgeResult.AC:
            firstError = result.value
            break

    if firstError is None:
        firstError = JudgeResult.AC
    file.safeRemove(exefileName)  # remove the binary file
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
