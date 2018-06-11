import os, sys

status = os.system("ls")
if status != 0:
    sys.exit()
b = int(input())
c = int(input())
print(b+c)
