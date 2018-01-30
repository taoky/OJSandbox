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
    running = langSupport.formatDockerHelper(command, **options)
    options['encoding'] = 'utf-8'
    cp = subprocess.run(command, **options)
    res = cp.stdout.split('\n')
    return JudgeResult(getattr(JudgeResult, res[0].strip()))

def plainJudge(program, codeType, infile, outfile, **config):
    inRedir = file.inFileName
    outRedir = file.outFileName
    copy(infile, inRedir)
    istream = open(inRedir, 'r')
    ostream = open(outRedir, 'w')
    files = {'in': inRedir, 'out': outRedir}
    #proFileName = os.path.splitext(i[0])[0]
    runHelper = langSupport.executeHelper[codeType]
    running = langSupport.formatHelper(runHelper, exefile=program)
    runResult = executeProgram(running, stdin=istream, stdout=ostream, timeout=config["timeout"] / 1000.0)
    rp = runResult.value
    istream.close()
    ostream.close()

    forwardResults = [JudgeResult.RE, JudgeResult.TLE, JudgeResult.MLE, JudgeResult.FSE]
    if rp in forwardResults:
        os.remove(inRedir)
        os.remove(outRedir)
        return JudgeResult(rp)

    compareMethod = compare.getCompareMethod(config["compare"])
    cp = compareMethod(outfile, outRedir)
    os.remove(outRedir) # cleanup
    if cp == False:
        return JudgeResult(JudgeResult.WA)
    return JudgeResult(JudgeResult.AC)

def judgeProcess(sourceFileName, sourceFileExt, directory, problemConfig):
    # WARNING: IT IS UNSAFE NOW!
    rsourceFileName = file.runDir + sourceFileName
    rsourceCodeName = directory + sourceFileName + sourceFileExt
    results = []
    
    # os.system(compileHelper[sourceFileExt] % (rsourceCodeName, rsourceFileName))
    try:
        compileHelper = langSupport.compileHelper[sourceFileExt.lower()]
        compiling = langSupport.formatHelper(compileHelper, infile=rsourceCodeName, outfile=rsourceFileName)
    except KeyError as e:
        return JudgeError(JudgeResult.FTE)
    
    cps = subprocess.run(compiling, bufsize=0, timeout=10)
    if cps.returncode != 0:
        return JudgeError(JudgeResult.CE, res)
    proFiles = file.getProblemFiles(sourceFileName)
    firstError = None
    for i in proFiles:
        infile = file.getProblemDirectory(sourceFileName) + i[0]
        outfile = file.getProblemDirectory(sourceFileName) + i[1]
        proFileName = os.path.splitext(infile)[0]
        result = plainJudge(rsourceFileName, sourceFileExt.lower(), infile, outfile, **problemConfig)
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
