#include <seccomp.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

const char *const FOERR = "fork() error";
const char *const SCERR = "seccomp error";

#define NUMSIZE 4000000
// ~ 16 MB

static int trusted_syscalls[] = {
    SCMP_SYS(uname), SCMP_SYS(brk), SCMP_SYS(arch_prctl),
    SCMP_SYS(readlink), SCMP_SYS(access), SCMP_SYS(fstat),
    SCMP_SYS(write), SCMP_SYS(exit_group), SCMP_SYS(mmap),
    SCMP_SYS(close), SCMP_SYS(mprotect), SCMP_SYS(munmap),
    SCMP_SYS(read), SCMP_SYS(lseek),
    SCMP_SYS(stat), // native

    SCMP_SYS(futex), SCMP_SYS(getrandom), SCMP_SYS(getdents),
    SCMP_SYS(fcntl), 
    
    SCMP_SYS(ioctl), SCMP_SYS(dup) // python3
}; // the same as OJSandbox

void errorExit(const char *str) {
	perror(str);
	exit(-1);
}

void whiteListProgRules(const char *progExec)
{
    // whitelist
    int trusted_len = sizeof(trusted_syscalls) / sizeof(int);
    // scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ERRNO(1));
    if (ctx == NULL)
    {
        errorExit(SCERR);
    }
    for (int i = 0; i < trusted_len; i++)
    {
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, trusted_syscalls[i], 0))
        {
            errorExit(SCERR);
        }
    }
    // allow execve partly to execute user's program
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 1, SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)progExec));
    // restrict IO
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1, SCMP_A1(SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR | O_APPEND | O_CREAT, 0));
    if (seccomp_load(ctx))
    {
        errorExit(SCERR);
    }
    seccomp_release(ctx);
}

int main(void) {
    pid_t son;
    son = fork();
    if (son < 0) {
        errorExit(FOERR);
        return -1;
    }
    if (son == 0) {
        // child
        whiteListProgRules("./exe");
        execve("./exe", NULL, NULL);
        // int *nums = malloc(NUMSIZE * sizeof(int));
        // int tmp; int pos = 0;
        // while (scanf("%d", &tmp) != -1) {
        //     nums[pos++] = tmp;
        //     // input test
        // }
        // for (int i = 0; i < pos; i++) {
        //     printf("%d ", nums[i]);
        //     // output test
        // }
        // free(nums);
        // return 0;
    }
    else {
        // parent
        int wstatus;
        wait(&wstatus);
        return 0;
    }
}