import os

def removeHiddenFiles(li):
    return [i for i in li if not i.startswith('.')]

# def filterDir(li):
#     return [i for i in li if os.path.isdir(i)]

def getAllProblems():
    return sorted(next(os.walk('/problems'))[1])