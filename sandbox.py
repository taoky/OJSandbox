import os
import subprocess
import compare
import file
from judgeResult import JudgeResult, JudgeError

compileHelper = {
    # '%i' for input file, '%o' for output file
    ".c": ["gcc", "-Wall", "-O3", "%i", "-o", "%o"],
    ".cpp": ["g++", "-Wall", "-O3", "%i", "-o", "%o"]
}

def writeResult(res, fee, reason, i):
    res.append((i, reason))
    if fee is None:
        fee = reason
    return fee

def judgeProcess(sourceFileName, sourceFileExt, directory, problemConfig):
    # WARNING: IT IS UNSAFE NOW!
    rsourceFileName = directory + sourceFileName
    rsourceCodeName = rsourceFileName + sourceFileExt
    
    # os.system(compileHelper[sourceFileExt] % (rsourceCodeName, rsourceFileName))
    try:
        compileCommand = compileHelper[sourceFileExt.lower()]
        compile = [{'%i': rsourceCodeName, '%o': rsourceFileName}.get(i, i) for i in compileCommand]
    except KeyError as e:
        raise JudgeError(JudgeResult.FTE, [])
    cps = subprocess.run(compile, bufsize=0, timeout=10)
    if cps.returncode != 0:
        raise JudgeError(JudgeResult.CE, [])
    proFiles = file.getProblemFiles(sourceFileName)
    tmpfile = "tmp.out"
    res = []
    firstEncounterError = None
    for i in proFiles:
        input = open(file.getProblemDirectory(sourceFileName) + i[0])
        output = open(tmpfile, "w")
        proFileName = os.path.splitext(i[0])[0]
        try:
            sp = subprocess.run(rsourceFileName, stdin=input, stdout=output, timeout=problemConfig["timeout"] / 1000.0)
        except subprocess.TimeoutExpired:
            res = {'exe': rsourceFileName,
                   'out': tmpfile}
            raise JudgeError(JudgeError.TLE, res)
        finally:
            input.close()
            output.close()

        if sp.returncode != 0:
            res = {'exe': rsourceFileName,
                   'out': tmpfile}
            raise JudgeError(JudgeError.RE, res)
        cp = compare.lineCompare(tmpfile, file.getProblemDirectory(sourceFileName) + i[1])
        os.remove(tmpfile) # cleanup
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
        return judgeProcess(sourceFileName, sourceFileExt, directory, problemConfig)
    except JudgeError as e:
        # Cleanup
        keys = ['exe', 'out']
        for i in keys:
            try:
                os.remove(e.res[i])
            except KeyError:
                pass
        return e
