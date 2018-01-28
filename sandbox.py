import os
import subprocess
import compare
import file
import langSupport
from judgeResult import JudgeResult, JudgeError

def writeResult(res, fee, reason, i):
    res.append((i, reason))
    if fee is None:
        fee = reason
    return fee

def judgeProcessC(sourceFileName, sourceFileExt, directory, problemConfig):
    # WARNING: IT IS UNSAFE NOW!
    rsourceFileName = file.runDir + sourceFileName
    rsourceCodeName = directory + sourceFileName + sourceFileExt
    
    # os.system(compileHelper[sourceFileExt] % (rsourceCodeName, rsourceFileName))
    try:
        compileHelper = langSupport.compileHelper[sourceFileExt.lower()]
        compiling = langSupport.formatHelper(compileHelper, infile=rsourceCodeName, outfile=rsourceFileName)
    except KeyError as e:
        raise JudgeError(JudgeResult.FTE, [])
    
    cps = subprocess.run(compiling, bufsize=0, timeout=10)
    if cps.returncode != 0:
        raise JudgeError(JudgeResult.CE, [])
    proFiles = file.getProblemFiles(sourceFileName)
    outfile = file.outFileName
    res = []
    firstEncounterError = None
    for i in proFiles:
        istream = open(file.getProblemDirectory(sourceFileName) + i[0])
        ostream = open(outfile, "w")
        proFileName = os.path.splitext(i[0])[0]
        runHelper = langSupport.executeHelper[sourceFileExt.lower()]
        running = langSupport.formatHelper(runHelper, exefile=rsourceFileName)
        try:
            sp = subprocess.run(running, stdin=istream, stdout=ostream, timeout=problemConfig["timeout"] / 1000.0)
        except subprocess.TimeoutExpired:
            res = {'exe': rsourceFileName,
                   'out': outfile}
            raise JudgeError(JudgeError.TLE, res)
        finally:
            istream.close()
            ostream.close()

        if sp.returncode != 0:
            res = {'exe': rsourceFileName,
                   'out': outfile}
            raise JudgeError(JudgeError.RE, res)

        compareMethod = compare.getCompareMethod(problemConfig["compare"])
        cp = compareMethod(outfile, file.getProblemDirectory(sourceFileName) + i[1])
        os.remove(outfile) # cleanup
        if sp.returncode != 0:
            firstEncounterError = writeResult(res, firstEncounterError, (JudgeResult.RE), proFileName)
        elif cp == False:
            firstEncounterError = writeResult(res, firstEncounterError, (JudgeResult.WA), proFileName)
        else:
            firstEncounterError = writeResult(res, firstEncounterError, (JudgeResult.AC), proFileName)

    if firstEncounterError is None:
        firstEncounterError = JudgeResult.WA
    os.remove(rsourceFileName) # remove the compiler file
    return JudgeResult(firstEncounterError, res)

def safeJudge(sourceFileName, sourceFileExt, directory, problemConfig):
    try:
        # Forward everything
        return judgeProcessC(sourceFileName, sourceFileExt, directory, problemConfig)
    except JudgeError as e:
        # Cleanup
        keys = ['exe', 'out']
        for i in keys:
            try:
                os.remove(e.res[i])
            except KeyError:
                pass
        return e
