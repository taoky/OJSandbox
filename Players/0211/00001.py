import os

status = os.system("ls")
if status != 0:
    quit()
b = int(input())
c = int(input())
print(b+c)
