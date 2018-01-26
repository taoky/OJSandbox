#include "main.h"
#include "util.h"
#include "init.h"

pid_t son;
static int son_exec = 0;
bool killedByTimer = false;

void ready(int sig)
{
    if (sig == SIGUSR1)
        son_exec = 1;
}

void killChild()
{
    int res = kill(son, SIGKILL);
    if (res == -1 && errno == ESRCH)
    {
        fprintf(stderr, "Cannot find child process. Maybe it has exited.\n");
    }
    else if (res == -1)
    {
        fprintf(stderr, "Failed to kill child.\n");
    }
    killedByTimer = true;
}

void setLimit(rlim_t maxMemory, rlim_t maxCPUTime, rlim_t maxProcessNum, rlim_t maxFileSize, rlim_t maxStackSize)
{
    /* The unit of some arguments:
	 * maxMemory (MB)
	 * maxCPUTime (s)
	 * maxFileSize (MB)
	 * maxStackSize (MB)
	 */
    maxMemory *= (1 << 20);
    maxFileSize *= (1 << 20);
    maxStackSize *= (1 << 20);
    struct rlimit max_memory, max_cpu_time, max_process_num, max_file_size, max_stack, nocore /*, nofile*/;
    setrlimStruct(maxMemory, &max_memory);
    setrlimStruct(maxCPUTime, &max_cpu_time);
    setrlimStruct(maxProcessNum, &max_process_num);
    setrlimStruct(maxFileSize, &max_file_size);
    setrlimStruct(maxStackSize, &max_stack);
    setrlimStruct(0, &nocore);
    // setrlimStruct(4, &nofile); // stdin, stdout & stderr
    if (setrlimit(RLIMIT_AS, &max_memory) != 0)
    {
        errorExit(RLERR);
    }
    if (setrlimit(RLIMIT_CPU, &max_cpu_time) != 0)
    {
        errorExit(RLERR);
    }
    if (setrlimit(RLIMIT_NPROC, &max_process_num) != 0)
    {
        errorExit(RLERR);
    }
    if (setrlimit(RLIMIT_FSIZE, &max_file_size) != 0)
    {
        errorExit(RLERR);
    }
    if (setrlimit(RLIMIT_STACK, &max_stack) != 0)
    {
        errorExit(RLERR);
    }
    // set no core file:
    if (setrlimit(RLIMIT_CORE, &nocore) != 0)
    {
        errorExit(RLERR);
    }
    // set num of file descriptor:
    //if (setrlimit(RLIMIT_NOFILE, &nofile) != 0) {
    //	errorExit(RLERR);
    //}
}

void fileRedirect(char inputpath[], char outputpath[])
{
    /* redirect stdin & stdout */
    FILE *input_file = fopen(inputpath, "r");
    FILE *output_file = fopen(outputpath, "w");
    if (dup2(fileno(input_file), fileno(stdin)) == -1)
    {
        errorExit(RDERR);
    }
    if (dup2(fileno(output_file), fileno(stdout)) == -1)
    {
        errorExit(RDERR);
    }
}

int main(int argc, char **argv)
{
    if (!isRootUser())
    {
        fprintf(stderr, "This program should be run only in root user!\n");
        exit(-1);
    }
    if (argc != 7)
    {
        fprintf(stderr, "Usage: %s CHROOT_DIR EXEC_FILE INPUT_FILE OUTPUT_FILE TIME_LIMIT (ms) MEMORY_LIMIT (MB)\n", argv[0]);
        fprintf(stderr, "Example: %s /tmp/ojs-123456 exec input output 1000 128\n", argv[0]);
        fprintf(stderr, "Please make sure to ./init before running this sandbox.\n");
        exit(-1);
    }
    char *chroot_dir = argv[1];
    char *exec_prog = argv[2];
    char *input_file = argv[3];
    char *output_file = argv[4];
    long time_limit, memory_limit;
    errno = 0;
    time_limit = strtol(argv[5], NULL, 10);
    if (errno != 0)
    {
        errorExit("time_limit error");
    }
    memory_limit = strtol(argv[6], NULL, 10);
    if (errno != 0)
    {
        errorExit("memory_limit error");
    }

    signal(SIGUSR1, ready);
    son_exec = 0;
    son = fork();
    if (son < 0)
    {
        // fork failed
        errorExit(FOERR);
    }
    if (son == 0)
    {
        // child
        // 1. copy prog
        char *copyprogTo = pathCat(chroot_dir, exec_prog);
        copyFile(exec_prog, copyprogTo);
        // char *copyinputTo = pathCat(chroot_dir, input_file);
        // copyFile(input_file, copyinputTo);
        // 2. set rlimit
        setLimit(memory_limit, (int)((time_limit + 1000) / 1000), 1, 16, memory_limit); // allow 1 process, 16 MB file size, rough time limit
        // 3. redirect stdin & stdout
        fileRedirect(input_file, output_file);
        // 4. chroot & setuid!
        chroot(chroot_dir);
        chdir("/");
        // 5. set uid & gid to nobody
        setNonPrivilegeUser();
        // 6. exec
        char *f_argv[] = {NULL}, *f_envp[] = {NULL};
        while (!son_exec)
            ;
        execve(exec_prog, f_argv, f_envp);
        fprintf(stderr, "%s error\n", exec_prog);
        perror("exec error");
    }
    else
    {
        // parent
        // 1. init cgroup
        writeFileInt(CGROUP_DIR "cpuacct/" CNAME "/tasks", son, true);
        writeFileInt(CGROUP_DIR "memory/" CNAME "/tasks", son, true);
        writeFileInt(CGROUP_DIR "pids/" CNAME "/tasks", son, true);
        writeFileInt(CGROUP_DIR "memory/" CNAME "/memory.limit_in_bytes", memory_limit * (1 << 20), true);
        writeFileInt(CGROUP_DIR "pids/" CNAME "/pids.max", 1, true); // allow 1 process only

        // 2. set timer
        struct itimerval itval;
        itval.it_interval.tv_sec = itval.it_interval.tv_usec = 0; // only once
        itval.it_value.tv_sec = time_limit / 1000;
        itval.it_value.tv_usec = time_limit % 1000 + 500;
        if (setitimer(ITIMER_REAL, &itval, NULL) == -1)
        {
            perror("setitimer error");
        }
        // 3. call son to exec
        kill(son, SIGUSR1);
        // 4. wait & cleanup
        struct rusage sonUsage;
        int status;
        wait3(&status, WUNTRACED, &sonUsage);
        int rusage_total_time = timevalms(&sonUsage.ru_utime) + timevalms(&sonUsage.ru_stime);
        int rusage_memory = sonUsage.ru_minflt * (getpagesize() >> 10);
        int cgroup_total_time = readFileLL(CGROUP_DIR "cpuacct/" CNAME "/cpuacct.usage") / 1000000;
        int cgroup_memory = readFileLL(CGROUP_DIR "memory/" CNAME "/memory.max_usage_in_bytes") / (1 << 20);
        printf("ru_time: %d, ru_mem: %d, cg_time: %d, cg_mem: %d\n", rusage_total_time, rusage_memory, cgroup_total_time, cgroup_memory);
        if (WIFEXITED(status))
        {
            puts("The program terminated.");
            printf("Exit code: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            puts("The program was terminated by a signal.");
            printf("Signal code: %d\n", WTERMSIG(status));
        }
        else if (WIFSTOPPED(status))
        {
            puts("The program was stopped by a signal. (WUNTRACED or being traced).");
            printf("Signal code: %d\n", WSTOPSIG(status));
        }
    }

    return 0;
}