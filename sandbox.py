import os
from shutil import copy
import subprocess
import compare
import file
import langSupport
from judge import JudgeResult, JudgeError

infoFile = file.workDir

def writeResult(res, fe, reason, i):
    res.append((i, reason))
    if fe is None:
        fe = reason
    return fe

def executeProgram(command, **options):
    try:
        cp = subprocess.run(command, **options)
    except subprocess.TimeoutExpired:
        return JudgeResult(JudgeResult.TLE)
    if cp.returncode != 0:
        return JudgeResult(JudgeResult.RE)
    return JudgeResult(JudgeResult.OK)

def executeProgramDocker(command, **options):
    # if not 'dir' in options:
    #     options['dir'] = file.getRunDir()
    running = langSupport.formatDockerHelper(command, **options)
    pwd = os.getcwd()
    # cp = subprocess.run(running, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

    #print(running)

    cp = file.container.exec_run(running, stderr=False)
    res = cp[1].decode().split('\n')
    if cp[0] != 0:
        #print(cp.stderr)
        # print(cp[0])
        res[0] = 'IE'

    #print(res)
    
    return JudgeResult(getattr(JudgeResult, res[0].strip()))

def plainJudge(program, codeType, infile, outfile, **config):
    inRedir = file.inFileName
    outRedir = file.outFileName
    copy(infile, file.getRunDir() + inRedir)

    #print("Copy file from {0} to {1}".format(infile, file.getRunDir() + inRedir))

    #istream = open(inRedir, 'r')
    #ostream = open(outRedir, 'w')
    #proFileName = os.path.splitext(i[0])[0]
    runHelper = langSupport.executeHelper[codeType]
    running = langSupport.formatHelper(runHelper, exefile=program)
    #runResult = executeProgram(running, stdin=istream, stdout=ostream, timeout=config['timeout'] / 1000.0)
    #runResult = executeProgramDocker(running, src=program, stdin=inRedir, stdout=outRedir,
    runResult = executeProgramDocker(program,
        stdin=inRedir, stdout=outRedir,
        timeout=config['timeout'], memory=config['ram'])
    rp = runResult.value
    #istream.close()
    #ostream.close()

    forwardResults = [JudgeResult.RE, JudgeResult.TLE, JudgeResult.MLE, JudgeResult.FSE]
    if rp in forwardResults:
        file.safeRemove(file.getRunDir() + inRedir)
        file.safeRemove(file.getRunDir() + outRedir)
        return JudgeResult(rp)
    copy(file.getRunDir() + outRedir, os.getcwd())

    compareMethod = compare.getCompareMethod(config["compare"])
    cp = compareMethod(outfile, file.runDir + outRedir)
    file.safeRemove(outRedir) # cleanup
    if cp == False:
        return JudgeResult(JudgeResult.WA)
    return JudgeResult(JudgeResult.AC)

def judgeProcess(sourceFileName, sourceFileExt, directory, problemConfig):
    # WARNING: IT IS UNSAFE NOW!
    exefileName = 'out'
    rsourceFileName = file.getRunDir() + exefileName
    rsourceCodeName = directory + sourceFileName + sourceFileExt
    results = []
    #print(rsourceCodeName, rsourceFileName)
    copy(rsourceCodeName, file.getRunDir() + sourceFileName + sourceFileExt)
    try:
        compileHelper = langSupport.compileHelper[sourceFileExt.lower()][:]
        #compiling = langSupport.formatHelper(compileHelper, infile=rsourceCodeName, outfile=rsourceFileName)
        compiling = langSupport.formatHelper(compileHelper, infile=sourceFileName+sourceFileExt, outfile=exefileName)
    except KeyError as e:
        return JudgeError(JudgeResult.FTE)
    
    #cps = subprocess.run(compiling, bufsize=0, timeout=10)
    
    #print(compiling)

    cps = executeProgramDocker(compiling, # src=rsourceCodeName,
        stdin='/dev/null', stdout='/dev/null',
        timeout=5000, memory=128, noseccomp=None, multiprocess=None)
    if not JudgeResult.isOK(cps.value):
        return JudgeError(JudgeResult.CE, results)
    proFiles = file.getProblemFiles(sourceFileName)
    firstError = None
    for i in proFiles:
        infile = file.getProblemDirectory(sourceFileName) + i[0]
        outfile = file.getProblemDirectory(sourceFileName) + i[1]
        proFileName = os.path.splitext(infile)[0]
        result = plainJudge(exefileName, sourceFileExt.lower(), infile, outfile, **problemConfig)
        firstError = writeResult(results, firstError, result.value, proFileName)
        if not firstError is None:
            break

    if firstError is None:
        firstError = JudgeResult.WA
    os.remove(rsourceFileName) # remove the compiler file
    return JudgeResult(firstError, results)

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
        return e.value, None
