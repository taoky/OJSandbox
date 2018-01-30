COMPILED = 1
INTERPRETED = 2
MIXED = 3
UNKNOWN = 0

# Just leave the classfication here in case they're needed later
langs = {
    '.c': COMPILED,
    '.cpp': COMPILED,
    '.py': INTERPRETED,
}

# '%i' is input file
# '%o' is output file
# '%e' is executable

compileHelper = {
    '.c': ['gcc', '-Wall', '-O3', '%i', '-o', '%o'],
    '.cpp': ['g++', '-Wall', '-O3', '%i', '-o', '%o'],
    '.py': ['cp', '%i', '%o'],
    '.java': ['javac', '%i', '-o', '%o']
}

executeHelper = {
    '.c': ['%e'],
    '.cpp': ['%e'],
    '.py': ['python3', '%e'],
    '.java': ['javaw', '%e']
}

dockerExe = ['sudo', './main']
# dockerHelper has a ddifferrennt format from other helpers!
dockerHelper = {
    'dir': ['-c', '%'],
    'src': ['-e', '%'],
    'stdin': ['-i', '%'],
    'stdout': ['-o', '%'],
    'stderr': [], # This is not implemented yet
    'timeout': ['-t', '%'],
    'memory': ['-m', '%'],
    'noseccomp': ['--disable-seccomp'],
    'multiprocess': ['--allow-multi-process'],
    'copyback': ['--copy-back', '%']
    #'command': ['--exec-command', '--', '%']
}

def langType(lang):
    try:
        return langs[lang]
    except KeyError:
        return UNKNOWN

def formatHelper(helper, **args):
    fdict = {}
    mapper = [('%i', 'infile'), ('%o', 'outfile'), ('%e', 'exefile')]
    for fkey, akey in mapper:
        try:
            fdict[fkey] = args[akey]
        except KeyError:
            pass
    
    return [fdict.get(key, key) for key in helper]

def formatDockerHelper(command, **args):
    res = dockerExe[:]
    for key in args:
        try:
            arg = [i if i != '%' else arg[key] for i in dockerHelper[key]]
            res += arg
        except KeyError:
            pass
    res += ['--exec-command', '--'] + command
    return res

