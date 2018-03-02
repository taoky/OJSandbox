import file
import config

COMPILED = 1
INTERPRETED = 2
MIXED = 3
UNKNOWN = 0

# Just leave the classfication here in case they're needed later
langs = {
    '.c': COMPILED,
    '.cpp': COMPILED,
    '.py': INTERPRETED,
    '.java': MIXED,
}

# '%i' is input file
# '%o' is output file
# '%e' is executable

compileHelper = {
    '.c': ['/usr/bin/gcc', '-Wall', '-O3', '%i', '-o', '%o'],
    '.cpp': ['/usr/bin/g++', '-Wall', '-O3', '%i', '-o', '%o'],
    '.py': ['/bin/cp', '%i', '%o'],
    '.java': ['javac', '%i', '-o', '%o'],
}

executeHelper = {
    '.c': ['%e'],
    '.cpp': ['%e'],
    '.py': ['python3', '%e'],
    '.java': ['javaw', '%e'],
}

backendExe = ['sudo', file.backendExe]
# backendHelper has a ddifferrennt format from other helpers!
backendHelper = {
    'dir': ['-c', '%'],
    'src': ['-e', '%'],
    'stdin': ['-i', '%'],
    'stdout': ['-o', '%'],
    'stderr': [], # This is not implemented yet
    'timeout': ['-t', '%'],
    'memory': ['-m', '%'],
    'noseccomp': ['--disable-seccomp'],
    'multiprocess': ['--allow-multi-process'],
    'maxproc': ['--max-processes', '%'],
    'outsize': ['--output-file-size', '%'],
    'copyback': ['--copy-back', '%'],
    'rssmem': ['--mem-rss-only']
    #'command': ['--exec-command', '--', '%'] # Special handling
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

def formatBackendHelper(command, **args):
    res = backendExe[:]
    try:
        while args['dir'][-1] == '/':
            args['dir'] = args['dir'][:-1]
    except KeyError:
        pass

    if 'config' in args:
        del args['config']
        args['maxproc'] = config.g['max-processes']
        args['outsize'] = config.g['out-size']

    args['rssmem'] = None
    for key in args:
        try:
            arg = [i if i != '%' else str(args[key]) for i in backendHelper[key]]
            res += arg
        except KeyError:
            pass

    if not command is None:
        res += ['--exec-command', '--'] + command
    return res

