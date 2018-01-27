import os
import subprocess
import compare
import file
from judge_errors import JudgeError

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

def judge_process(sourceFileName, sourceFileExt, directory, problemConfig):
    # WARNING: IT IS UNSAFE NOW!
    rsourceFileName = directory + sourceFileName
    rsourceCodeName = rsourceFileName + sourceFileExt
    
    # os.system(compileHelper[sourceFileExt] % (rsourceCodeName, rsourceFileName))
    try:
        compileCommand = compileHelper[sourceFileExt.lower()]
        compile = [{'%i': rsourceCodeName, '%o': rsourceFileName}.get(i, i) for i in compileCommand]
    except KeyError as e:
        raise JudgeError("File type not supported", [])
    cps = subprocess.run(compile, bufsize=0, timeout=10)
    if cps.returncode != 0:
        raise JudgeError("Compilation Error", [])
    proFiles = file.getProblemFiles(sourceFileName)
    tmpfile = "tmp.out"
    res = []
    firstEncounterError = None
    for i in proFiles:
        input = open(file.getProblemDirectory(sourceFileName) + i[0])
        output = open(tmpfile, "w")
        proFileName = os.path.splitext(i[0])[0]
        sp = subprocess.run(rsourceFileName, stdin=input, stdout=output, timeout=problemConfig["timeout"] / 1000.0)
        input.close()
        output.close()
        cp = compare.lineCompare(tmpfile, file.getProblemDirectory(sourceFileName) + i[1])
        os.remove(tmpfile) # cleanup
        if sp.returncode != 0:
            firstEncounterError = writeResult(res, firstEncounterError, ("Runtime Error"), proFileName)
        elif cp == False:
            firstEncounterError = writeResult(res, firstEncounterError, ("Wrong Answer"), proFileName)
        else:
            firstEncounterError = writeResult(res, firstEncounterError, ("Accepted"), proFileName)

    if firstEncounterError is None:
        firstEncounterError = "Accepted"
    os.remove(rsourceFileName) # remove the compiler file
    return (firstEncounterError, res)

def safe_judge(sourceFileName, sourceFileExt, directory, problemConfig):
    try:
        # Forward everything
        return judge_process(sourceFileName, sourceFileExt, directory, problemConfig)
    except JudgeError as e:
        return (e.value, e.info)
