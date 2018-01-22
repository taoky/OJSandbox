#!/usr/bin/env python3
import os
import sys

def trim(l):
    while l[-1] == '':
        l.pop()
    return l

def strictCompare(file1, file2):
    with open(file1, 'r') as a, open(file2, 'r') as b:
        linesA, linesB = trim([l.rstrip() for l in a]), trim([l.rstrip() for l in b])
        return linesA == linesB


if __name__ == "__main__":
    try:
        if len(sys.argv) == 3:
            if os.path.isfile(sys.argv[1]) and os.path.isfile(sys.argv[2]):
                print(strictCompare(sys.argv[1], sys.argv[2]))
            else:
                raise ValueError('Not files')
        else:
            raise ValueError('Bad arguments')
    except ValueError as e:
        sys.exit(e)

