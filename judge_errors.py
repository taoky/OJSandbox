class JudgeError(Exception):
    def __init__(self, value, info = None):
        self.value = value
        self.info = info
    
    def __str__(self):
        return repr(self.value)
