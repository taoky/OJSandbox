#include "main.h"
#include "util.h"
#include "init.h"
#include "secrules.h"

pid_t son;
static int son_exec = 0;
bool killedByTimer = false;

struct runArgs_t {
    char *chrootDir; // --chroot-dir
    char *execFileName; // --exec-file
    char *execProfile; // --exec-argument, optional
    char *inputFileName; // --input
    char *outputFileName; // --output
    char *logFileName; // --log, optional
    int timeLimit; // --time-limit
    int memLimit; // --mem-limit
    bool isSeccompDisabled; // --disable-seccomp, optional
} runArgs;

static const char *optString = "cepiotmlh?";

static const struct option longOpts[] = {
    { "chroot-dir", required_argument, NULL, 'c'},
    { "exec-file", required_argument, NULL, 'e'},
    { "exec-profile", required_argument, NULL, 'p'},
    { "input", required_argument, NULL, 'i'},
    { "output", required_argument, NULL, 'o'},
    { "log", required_argument, NULL, 'l'},
    { "time-limit", required_argument, NULL, 't'},
    { "mem-limit", required_argument, NULL, 'm'},
    { "disable-seccomp", no_argument, NULL, 0},
    { "help", no_argument, NULL, 'h'},
    { NULL, no_argument, NULL, 0}
};

void display_help(char *a0) {
    log("This is the backend of the sandbox for oj.\n");
    log("Usage: %s -c path -e file -i file -o file [--disable-seccomp] [-p name] [-l file] [-t num] [-m num] [-h]\n", a0);
    log("or: %s --chroot-dir path --exec-file file --input file --output file [--disable-seccomp] [--exec-profile name] [--log file] [--time-limit num] [--mem-limit num] [--help]\n", a0);
    log("--chroot-dir or -c: The directory that will be chroot(2)ed\n");
    log("--exec-file or -e: The program (or source file) that will be executed or interpreted.\n");
    log("--exec-profile or -p: (Optional) The profile for a explicit program language (such as python, java)\n");
    log("--input or -i: The file that will be the input source.\n");
    log("--output or -o: The file that will be the output (stdout) of the program.\n");
    log("--log or -l: (Optional) The file that will be the output (stderr) of the sandbox & program.\n");
    log("--time-limit or -t: (Optional) The time limit of the program.\n");
    log("--mem-limit or -m: (Optional) The memory size limit of the program.\n");
    log("--disable-seccomp: (Optional) This will disable system call filter.\n");
    log("--help or -h: (Optional) This will show this message.\n");
    exit(0);
}

void option_handle(int argc, char **argv) {
    // init runArgs
    runArgs.timeLimit = runArgs.memLimit = -1;
    runArgs.chrootDir = runArgs.execFileName = runArgs.execProfile = runArgs.inputFileName = runArgs.outputFileName = runArgs.logFileName = NULL;
    runArgs.isSeccompDisabled = false;
    int longIndex;
    int opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while (opt != -1) {
        switch(opt) {
            case 'c':
                runArgs.chrootDir = optarg;
                break;
            case 'e':
                runArgs.execFileName = optarg;
                break;
            case 'p':
                runArgs.execProfile = optarg;
                break;
            case 'i':
                runArgs.inputFileName = optarg;
                break;
            case 'o':
                runArgs.outputFileName = optarg;
                break;
            case 'l':
                runArgs.logFileName = optarg;
                break;
            case 't':
                runArgs.timeLimit = *optarg;
                break;
            case 'm':
                runArgs.memLimit = *optarg;
                break;
            case 'h':
            case '?':
                display_help(argv[0]);
                break;
            case 0:
                if (strcmp("disable-seccomp", longOpts[longIndex].name) == 0) {
                    runArgs.isSeccompDisabled = true;
                }
                break;
            default:
                break;
        }
        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }
    // check 
    if (runArgs.chrootDir == NULL || runArgs.execFileName == NULL || runArgs.inputFileName == NULL || runArgs.outputFileName == NULL) {
        log("Missing argument(s).\nUse %s -h or %s --help to get help.\n", argv[0], argv[0]);
        exit(-1);
    }
}

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
    // if (argc != 7)
    // {
    //     fprintf(stderr, "Usage: %s CHROOT_DIR EXEC_FILE INPUT_FILE OUTPUT_FILE TIME_LIMIT (ms) MEMORY_LIMIT (MB)\n", argv[0]);
    //     fprintf(stderr, "Example: %s /tmp/ojs-123456 exec input output 1000 128\n", argv[0]);
    //     fprintf(stderr, "Please make sure to ./init before running this sandbox.\n");
    //     exit(-1);
    // }
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

        while (!son_exec)
            ;

        // 6. load seccomp rule
        nativeProgRules(exec_prog);
        // 7. exec
        char *f_argv[] = {NULL}, *f_envp[] = {NULL};

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