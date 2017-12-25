import filecmp

def strictCompare(file1, file2):
    return filecmp.cmp(file1, file2, shallow=False)