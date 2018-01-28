#include "secrules.h"
#include "util.h"

static int trusted_syscalls[] = {
    SCMP_SYS(uname), SCMP_SYS(brk), SCMP_SYS(arch_prctl),
    SCMP_SYS(readlink), SCMP_SYS(access), SCMP_SYS(fstat),
    SCMP_SYS(write), SCMP_SYS(exit_group), SCMP_SYS(mmap),
    SCMP_SYS(close), SCMP_SYS(mprotect), SCMP_SYS(munmap),
    SCMP_SYS(read), SCMP_SYS(lseek), // SCMP_SYS(open),
    SCMP_SYS(stat)};

void nativeProgRules(char *progExec)
{
    // native program (C/C++): whitelist
    int trusted_len = sizeof(trusted_syscalls) / sizeof(int);
    int blocked_syscalls[] = {
        // ...
    };
    char lib[] = "/lib/x86_64-linux-gnu/libc.so.6";
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
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1, SCMP_A1(SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR, 0));
    if (seccomp_load(ctx))
    {
        errorExit(SCERR);
    }
    seccomp_release(ctx);
}

void runnerRules(char *progExec)
{
}