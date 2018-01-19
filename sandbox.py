import os
import subprocess
import compare
import file

compileHelper = {
    ".c": ["gcc", "-Wall", "-O3", "%s", "-o", "%s"],
    ".cpp": ["g++", "-Wall", "-O3", "%s", "-o", "%s"]
}

def writeResult(res, fee, reason, i):
    res.append((i, reason))
    if fee == False:
        fee = reason
    return fee

def safe_judge(sourceFileName, sourceFileExt, directory, problemConfig):
    # WARNING: IT IS UNSAFE NOW!
    rsourceFileName = directory + sourceFileName

    # os.system(compileHelper[sourceFileExt] % (rsourceFileName + sourceFileExt, rsourceFileName))
    compile = compileHelper[sourceFileExt.lower()]
    cps = subprocess.run(compile[0], compile[1:], timeout=10) #FIXME
    proFiles = file.getProblemFiles(sourceFileName)
    tmpfile = "tmp.out"
    res = []
    firstEncounterError = False
    for i in proFiles:
        input = open(file.getProblemDirectory(sourceFileName) + i[0])
        output = open(tmpfile, "w")
        proFileName = os.path.splitext(i[0])[0]
        sp = subprocess.run(rsourceFileName, stdin=input, stdout=output, timeout=problemConfig["timeout"] / 1000.0)
        input.close()
        output.close()
        cp = compare.strictCompare(tmpfile, file.getProblemDirectory(sourceFileName) + i[1])
        os.remove(tmpfile) # cleanup
        if sp.returncode != 0:
            firstEncounterError = writeResult(res, firstEncounterError, ("Runtime Error"), proFileName)
        elif cp == False:
            firstEncounterError = writeResult(res, firstEncounterError, ("Wrong Answer"), proFileName)
        else:
            firstEncounterError = writeResult(res, firstEncounterError, ("Accepted"), proFileName)

    if firstEncounterError == False:
        firstEncounterError = "Accepted"
    os.remove(rsourceFileName) # remove the compiler file
    return (firstEncounterError, res)