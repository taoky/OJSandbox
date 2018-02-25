class RunInfo:
    def __init__(self, time=0, mem=0):
        self.time = time
        self.mem = mem

    def __str__(self):
        return 'Time: {time}s, Mem: {mem} KiB'.format(
            time=self.time/1000.0, mem=self.mem
        )

    def __add__(self, n):
        return RunInfo(self.time+n.time, self.mem+n.mem)

    def __iadd__(self, n):
        self.time += n.time
        self.mem += n.mem
        return self

    def __truediv__(self, n):
        return RunInfo(self.time/n, self.mem//n)

class JudgeResult:
    # 0: No exceptions
    AC = 0
    WA = 1
    OK = 99

    # 100: Runtime exceptions
    RE = 100
    TLE = 101
    MLE = 102
    FSE = 103
    
    # 200: Compilation exceptions
    CE = 200
    FTE = 201

    # 800: Internal exceptions
    IE = 800
    AV = 801

    # 900: Miscellaneous
    UNKNOWN = 999

    INFO = {
        AC: 'Accepted',
        WA: 'Wrong answer',
        OK: 'OK',
        RE: 'Runtime error',
        MLE: 'Memory limit exceeded',
        TLE: 'Time limit exceeded',
        FSE: 'File size error',
        CE: 'Compile error',
        FTE: 'Invalid file type',
        IE: 'Internal error',
        #AV: 'Access violation', don't tell this
        AV: 'Runtime error',
        UNKNOWN: 'Unknown error'
    }

    @staticmethod
    def stringBase(res):
        return JudgeResult.INFO.get(res, 'Unknown result')

    def isOK(s):
        OKcode = [JudgeResult.AC, JudgeResult.WA, JudgeResult.OK]
        try:
            return s.value in OKcode
        except AttributeError:
            return s in OKcode

    def __init__(self, value, res = None):
        self.value = value
        self.res = res

    def __str__(self):
        return JudgeResult.stringBase(self.value)


class JudgeError(Exception, JudgeResult):
    def __init__(self, value, res = None):
        self.value = value
        self.res = res
    
    def __str__(self):
        return JudgeResult.stringBase(self.value)
