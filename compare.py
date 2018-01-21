import os
import sys
import filecmp

def strictCompare(file1, file2):
    return filecmp.cmp(file1, file2, shallow=False)


if __name__ == "__main__":
    if len(sys.argv) == 3:
        if os.path.isfile(sys.argv[1]) and os.path.isfile(sys.argv[2]):
            print(strictCompare(sys.argv[1], sys.argv[2]))
