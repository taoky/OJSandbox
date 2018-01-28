class JudgeResult:
    """This class serves as a constants provider"""
    # 0: No exceptions
    AC = 0
    WA = 1

    # 100: Runtime exceptions
    RE = 100
    TLE = 101
    MLE = 102
    
    # 200: Compilation exceptions
    CE = 200
    FTE = 201

    # 800: Internal exceptions
    IE = 800

    # 900: Miscellaneous
    UNKNOWN = 999

    INFO = {
        AC: "Accepted",
        WA: "Wrong answer",
        RE: "Runtime error",
        MLE: "Memory limit exceeded",
        TLE: "Time limit exceeded",
        CE: "Compile error",
        FTE: "Invalid file type",
        IE: "Internal error",
        UNKNOWN: "Unknown error"
    }

    @staticmethod
    def stringBase(res):
        return JudgeResult.INFO.get(res, "Unknown result")

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
