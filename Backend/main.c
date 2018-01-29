#include "main.h"
#include "util.h"
#include "init.h"
#include "secrules.h"

pid_t son;
static int son_exec = 0;
bool killedByTimer = false;

struct runArgs_t
{
    char *chrootDir;        // --chroot-dir
    char *execFileName;     // --exec-file
    char *execProfile;      // --exec-argument, optional
    char *inputFileName;    // --input
    char *outputFileName;   // --output
    char *logFileName;      // --log, optional
    long timeLimit;         // --time-limit
    long memLimit;          // --mem-limit
    bool isSeccompDisabled; // --disable-seccomp, optional
} runArgs;

static const char *optString = "c:e:p:i:o:t:m:l:h?";

static const struct option longOpts[] = {
    {"chroot-dir", required_argument, NULL, 'c'},
    {"exec-file", required_argument, NULL, 'e'},
    {"exec-profile", required_argument, NULL, 'p'},
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"log", required_argument, NULL, 'l'},
    {"time-limit", required_argument, NULL, 't'},
    {"mem-limit", required_argument, NULL, 'm'},
    {"disable-seccomp", no_argument, NULL, 0},
    {"help", no_argument, NULL, 'h'},
    {NULL, no_argument, NULL, 0}};

void display_help(char *a0)
{
    log("This is the backend of the sandbox for oj.\n");
    log("Usage: %s -c path -e file -i file -o file [--disable-seccomp] [-p name] [-l file] [-t num] [-m num] [-h]\n", a0);
    log("or: %s --chroot-dir path --exec-file file --input file --output file [--disable-seccomp] [--exec-profile name] [--log file] [--time-limit num] [--mem-limit num] [--help]\n", a0);
    log("--chroot-dir or -c: The directory that will be chroot(2)ed in.\n");
    log("--exec-file or -e: The program (or source file) that will be executed or interpreted.\n");
    log("--exec-profile or -p: (Optional) The profile for a explicit program language (such as python, java)\n");
    log("--input or -i: The file that will be the input source.\n");
    log("--output or -o: The file that will be the output (stdout) of the program.\n");
    log("--log or -l: (Optional, stderr by default) The file that will be the output (stderr) of the sandbox & program.\n");
    log("--time-limit or -t: (Optional, unlimited by default) The time (ms) limit of the program.\n");
    log("--mem-limit or -m: (Optional, unlimited by default) The memory size (MB) limit of the program.\n");
    log("--disable-seccomp: (Optional) This will disable system call filter.\n");
    log("--help or -h: (Optional) This will show this message.\n");
    exit(0);
}

void option_handle(int argc, char **argv)
{
    // init runArgs
    runArgs.timeLimit = runArgs.memLimit = -1;
    runArgs.chrootDir = runArgs.execFileName = runArgs.execProfile = runArgs.inputFileName = runArgs.outputFileName = runArgs.logFileName = NULL;
    runArgs.isSeccompDisabled = false;
    int longIndex;
    char *endptr;
    long val;
    int opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while (opt != -1)
    {
        switch (opt)
        {
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
            errno = 0;
            val = strtol(optarg, &endptr, 10);
            if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0) || (endptr == optarg) || val <= 0)
            {
                log("Time limit error.\n");
                exit(-1);
            }
            runArgs.timeLimit = val;
            break;
        case 'm':
            errno = 0;
            val = strtol(optarg, &endptr, 10);
            if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0) || (endptr == optarg) || val <= 0)
            {
                log("Memory limit error.\n");
                exit(-1);
            }
            runArgs.memLimit = val;
            break;
        case 'h':
        case '?':
            display_help(argv[0]);
            break;
        case 0:
            if (strcmp("disable-seccomp", longOpts[longIndex].name) == 0)
            {
                runArgs.isSeccompDisabled = true;
            }
            break;
        default:
            break;
        }
        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }
    // check
    if (runArgs.chrootDir == NULL || runArgs.execFileName == NULL || runArgs.inputFileName == NULL || runArgs.outputFileName == NULL)
    {
        log("Missing argument(s).\nUse %s -h or %s --help to get help.\n", argv[0], argv[0]);
        exit(-1);
    }
}

void ready(int sig)
{
    if (sig == SIGUSR1)
        son_exec = 1;
}

void killChild(int sig)
{
    int res = kill(son, SIGKILL);
    if (res == -1 && errno == ESRCH)
    {
        // nothing~ The child has exited.
    }
    else if (res == -1)
    {
        log("Failed to kill child.\n");
    }
    if (sig == SIGALRM)
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
    if (maxMemory != -1)
        maxMemory *= (1 << 20);
    maxFileSize *= (1 << 20);
    if (maxStackSize != -1)
        maxStackSize *= (1 << 20);
    struct rlimit max_memory, max_cpu_time, max_process_num, max_file_size, max_stack, nocore;
    if (maxMemory != -1)
        setrlimStruct(maxMemory, &max_memory);
    if (maxCPUTime != -1)
        setrlimStruct(maxCPUTime, &max_cpu_time);
    setrlimStruct(maxProcessNum, &max_process_num);
    setrlimStruct(maxFileSize, &max_file_size);
    if (maxStackSize != -1)
        setrlimStruct(maxStackSize, &max_stack);
    setrlimStruct(0, &nocore);
    // setrlimStruct(4, &nofile); // stdin, stdout & stderr
    if (maxMemory != -1)
        if (setrlimit(RLIMIT_AS, &max_memory) != 0)
        {
            errorExit(RLERR);
        }
    if (maxCPUTime != -1)
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
    if (maxStackSize != -1)
        if (setrlimit(RLIMIT_STACK, &max_stack) != 0)
        {
            errorExit(RLERR);
        }
    // set no core file:
    if (setrlimit(RLIMIT_CORE, &nocore) != 0)
    {
        errorExit(RLERR);
    }
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

void logRedirect(char logpath[])
{
    /* redirect stderr (global) */
    if (logpath != NULL)
    {
        FILE *log_file = fopen(logpath, "w");
        if (dup2(fileno(log_file), fileno(stderr)) == -1)
        {
            errorExit(RDERR);
        }
    }
}

int main(int argc, char **argv)
{
    if (!isRootUser())
    {
        log("This program requires root user!\n");
        exit(-1);
    }
    option_handle(argc, argv);
    logRedirect(runArgs.logFileName);
    signal(SIGUSR1, ready);
    son_exec = 0;
    son = fork();

    char *execFileBaseName = basename(runArgs.execFileName);
    // 1. copy prog
    char *chrootTmp = pathCat(runArgs.chrootDir, "/tmp");
    char *copyprogTo = pathCat(chrootTmp, execFileBaseName);
    copyFile(runArgs.execFileName, copyprogTo);
    char *chrootProg = pathCat("/tmp/", execFileBaseName);

    if (son < 0)
    {
        // fork failed
        errorExit(FOERR);
    }
    if (son == 0)
    {
        // child

        // 2. set rlimit
        setLimit(runArgs.memLimit,
                 runArgs.timeLimit == -1 ? -1 : (int)((runArgs.timeLimit + 1000) / 1000),
                 1, 16, runArgs.memLimit); // allow 1 process, 16 MB file size, rough time limit
        // 3. redirect stdin & stdout
        fileRedirect(runArgs.inputFileName, runArgs.outputFileName);
        // 4. chroot & setuid!
        chroot(runArgs.chrootDir);
        chdir("/");
        // 5. set uid & gid to nobody
        setNonPrivilegeUser();

        while (!son_exec)
            ;

        // 6. load seccomp rule
        if (!runArgs.isSeccompDisabled)
            nativeProgRules(chrootProg);
        // 7. exec
        char *f_argv[] = {NULL}, *f_envp[] = {NULL};
        execve(chrootProg, f_argv, f_envp);
        perror("exec error"); // unreachable normally
    }
    else
    {
        // parent
        char procStat[12 + 10] = {};
        sprintf(procStat, "/proc/%d/stat", son);
        // 2. set timer
        signal(SIGALRM, killChild);
        struct itimerval itval;
        itval.it_interval.tv_sec = itval.it_interval.tv_usec = 0; // only once
        itval.it_value.tv_sec = runArgs.timeLimit / 1000;
        itval.it_value.tv_usec = (runArgs.timeLimit % 1000 + 500) * 1000;
        if (setitimer(ITIMER_REAL, &itval, NULL) == -1)
        {
            perror("setitimer error");
        }
        struct timeval progStart, progEnd, useTime;
        gettimeofday(&progStart, NULL);
        // 3. call son to exec
        kill(son, SIGUSR1);
        // 4. wait & cleanup
        struct rusage sonUsage;
        int status;
        unsigned long memory_max = 0, memory_now = 0;
        while (wait3(&status, WUNTRACED | WNOHANG, &sonUsage) == 0)
        {
            FILE *procFile = fopen(procStat, "r");
            fscanf(procFile, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %lu", &memory_now);
            fclose(procFile);
            if (memory_now > memory_max)
            {
                memory_max = memory_now;
            }
        }
        memory_max /= (1 << 10); // accurate virt usage
        // int rusage_total_time = timevalms(&sonUsage.ru_utime) + timevalms(&sonUsage.ru_stime);
        // long rusage_memory2 = sonUsage.ru_maxrss;
        gettimeofday(&progEnd, NULL);
        timersub(&progEnd, &progStart, &useTime);
        int actualTime = timevalms(&useTime);
        printf("Time: %d, Memory: %lu\n", actualTime, memory_max);
        remove(copyprogTo);
        itval.it_value.tv_sec = itval.it_value.tv_usec = 0; // stop timer
        if (WIFEXITED(status))
        {
            int ret = WEXITSTATUS(status);
            if (ret == 0)
            {
                puts("Success.");
            }
            else
            {
                printf("Runtime Error, returns %d\n", ret);
            }
        }
        else if (WIFSIGNALED(status))
        {
            int sig = WTERMSIG(status);
            if (killedByTimer || sig == SIGXCPU)
            {
                printf("Time Limit Exceeded\n");
            }
            else if (sig == SIGXFSZ)
            {
                printf("File Size Limit Exceeded\n");
            }
            else if (memory_max > runArgs.memLimit * (1 << 10))
            {
                printf("Memory Limit Exceeded\n");
            }
            else
            {
                printf("Runtime Error. Signal: %d\n", sig);
            }
        }
        else if (WIFSTOPPED(status))
        {
            killChild(WSTOPSIG(status));
            printf("System Error: Strangely being stopped.\n");
        }
    }

    return 0;
}