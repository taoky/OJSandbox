#include "secrules.h"
#include "util.h"

void nativeProgRules(char *progExec) {
    // native program (C/C++): whitelist
    int whitelist_syscalls[] = {
        SCMP_SYS(uname), SCMP_SYS(brk), SCMP_SYS(arch_prctl),
        SCMP_SYS(readlink), SCMP_SYS(access), SCMP_SYS(fstat),
        SCMP_SYS(write), SCMP_SYS(exit_group), SCMP_SYS(mmap),
        SCMP_SYS(close), SCMP_SYS(mprotect), SCMP_SYS(munmap),
        SCMP_SYS(read), SCMP_SYS(lseek), SCMP_SYS(open), 
        SCMP_SYS(stat)
    };
    int len = sizeof(whitelist_syscalls) / sizeof(int);
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
    if (ctx == NULL) {
        errorExit(SCERR);
    }
    for (int i = 0; i < len; i++) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, whitelist_syscalls[i], 0)) {
            errorExit(SCERR);
        }
    }
    // allow execve partly to execute user's program
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 1, SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)progExec));
    // ...
    if (seccomp_load(ctx)) {
        errorExit(SCERR);
    }
    seccomp_release(ctx);
}