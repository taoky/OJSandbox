import file

COMPILED = 1
INTERPRETED = 2
MIXED = 3
UNKNOWN = 0

# Just leave the classfication here in case they're needed later
langs = {
    '.c': COMPILED,
    '.cpp': COMPILED,
    #'.py': INTERPRETED,
}

# '%i' is input file
# '%o' is output file
# '%e' is executable

compileHelper = {
    '.c': ['/usr/bin/gcc', '-Wall', '-O3', '%i', '-o', '%o'],
    '.cpp': ['/usr/bin/g++', '-Wall', '-O3', '%i', '-o', '%o'],
    '.py': ['/bin/cp', '%i', '%o'],
    '.java': ['javac', '%i', '-o', '%o']
}

executeHelper = {
    '.c': ['%e'],
    '.cpp': ['%e'],
    '.py': ['python3', '%e'],
    '.java': ['javaw', '%e']
}

dockerExe = [file.backendExe]
# dockerHelper has a ddifferrennt format from other helpers!
dockerHelper = {
    # 'dir': ['-c', '%'],
    # 'src': ['-e', '%'],
    'stdin': ['-i', '%'],
    'stdout': ['-o', '%'],
    'stderr': [], # This is not implemented yet
    'timeout': ['-t', '%'],
    'memory': ['-m', '%'],
    'noseccomp': ['--disable-seccomp'],
    'multiprocess': ['--allow-multi-process'],
    # 'copyback': ['--copy-back', '%']
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
    # try:
    #     while args['dir'][-1] == '/':
    #         args['dir'] = args['dir'][:-1]
    # except KeyError:
    #     pass

    for key in args:
        try:
            arg = [i if i != '%' else str(args[key]) for i in dockerHelper[key]]
            res += arg
        except KeyError:
            pass
    if type(command) is str:
        command = [command]
    res += ['--'] + command
    return res

