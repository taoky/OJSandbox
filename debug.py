# Debug support

import sys

class DebugPrinter(object):
    def __init__(self, enable=False):
        self.enabled = enable

    def __call__(self, *args, **kwargs):
        if self.enabled:
            print(*args, **kwargs, file=sys.stderr)

    def enable(self):
        self.enabled = True

    def disable(self):
        self.enabled = False

dprint = DebugPrinter()
