#include <sys/ptrace.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/reg.h>

#define NUMSIZE 4000000
// ~ 16 MB

bool available_syscalls[512] = {};
bool is_executed = false;

int trusted_syscalls[] = {
    SYS_uname, SYS_brk, SYS_arch_prctl,
    SYS_readlink, SYS_access, SYS_fstat,
    SYS_write, SYS_exit_group, SYS_mmap,
    SYS_close, SYS_mprotect, SYS_munmap,
    SYS_read, SYS_lseek,
    SYS_stat, // native

    SYS_futex, SYS_getrandom, SYS_getdents,
    SYS_fcntl, SYS_openat, SYS_open,

    SYS_ioctl, SYS_dup // python3
};                     // the same as OJSandbox

void init_syscall(void)
{
    for (int i = 0; i < sizeof(trusted_syscalls) / sizeof(int); i++)
    {
        available_syscalls[trusted_syscalls[i]] = true;
    }
}

int main(void)
{
    init_syscall();
    pid_t son;
    son = fork();
    if (son < 0)
    {
        fprintf(stderr, "fork() error\n");
        return -1;
    }
    if (son == 0)
    {
        // child
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1)
        {
            fprintf(stderr, "error init ptrace\n");
            return -1;
        }
        execve("./exe", NULL, NULL);
        // raise(SIGCONT);
        // int *nums = malloc(NUMSIZE * sizeof(int));
        // int tmp;
        // int pos = 0;
        // while (scanf("%d", &tmp) != -1)
        // {
        //     nums[pos++] = tmp;
        //     // input test
        // }
        // for (int i = 0; i < pos; i++)
        // {
        //     printf("%d ", nums[i]);
        //     // output test
        // }
        // free(nums);
        // remove("/tmp/aaa");
        // return 0;
    }
    else
    {
        // parent
        int wstatus;
        while (1)
        {
            waitpid(-1, &wstatus, WUNTRACED);
            if (WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) == SIGTRAP)
            {
                int sc = ptrace(PTRACE_PEEKUSER, son, 8 * ORIG_RAX, NULL);
                if (sc == SYS_execve && !is_executed) {
                    is_executed = true;
                }
                else if (!available_syscalls[sc])
                {
                    fprintf(stderr, "syscall %d violation.\n", sc);
                    kill(son, SIGKILL);
                    return -1;
                }
            }
            else if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus))
            {
                break; // exited
            }
            if (ptrace(PTRACE_SYSCALL, son, NULL, NULL) == -1)
            {
                fprintf(stderr, "error ptrace son\n");
                kill(son, SIGKILL);
                return -1;
            }
        }
        return 0;
    }
}