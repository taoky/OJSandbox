#include "main.h"
#include "util.h"
#include "secrules.h"
#include "gitversion.h"

pid_t son;
static volatile int son_exec = 0;
bool killedByTimer = false;
bool memLimKilled = false;

static int maxProcesses = 128;
static int maxOutputFile = 16; // Unit: MiB

struct runArgs_t
{
    char *chrootDir;         // --chroot-dir
    char *execFileName;      // --exec-file
    char **execCommand;      // after '--'
    char *inputFileName;     // --input
    char *outputFileName;    // --output
    char *logFileName;       // --log, optional
    char *copyBackFileName;  // --copy-back, optional (compile)
    char *execStderr;        // --exec-stderr, optional
    unsigned long timeLimit; // --time-limit
    unsigned long memLimit;  // --mem-limit
    bool isSeccompDisabled;  // --disable-seccomp, optional
    bool isCommandEnabled;   // --exec-command
    bool isMultiProcess;     // --allow-multi-process
    bool isMemLimitRSS;      // --mem-rss-only
} runArgs;

static const char * const optString = "+c:e:i:o:t:m:l:h?";

static const struct option longOpts[] = {
    {"chroot-dir", required_argument, NULL, 'c'},
    {"exec-file", required_argument, NULL, 'e'},
    {"exec-command", no_argument, NULL, 0},
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"log", required_argument, NULL, 'l'},
    {"time-limit", required_argument, NULL, 't'},
    {"mem-limit", required_argument, NULL, 'm'},
    {"disable-seccomp", no_argument, NULL, 0},
    {"copy-back", required_argument, NULL, 0},
    {"allow-multi-process", no_argument, NULL, 0},
    {"exec-stderr", required_argument, NULL, 0},
    {"mem-rss-only", no_argument, NULL, 0},
    {"max-processes", required_argument, NULL, 0},
    {"output-file-size", required_argument, NULL, 0},
    {"help", no_argument, NULL, 'h'},
    {NULL, no_argument, NULL, 0}};

void display_title(void){
    log("OJ sandbox backend (version " GITVERSION_STR ", commit " GITCOMMIT_STR ")\n\n");
}

void display_help(const char *a0)
{
    display_title();
    log("Usage: %s -c path -e file -i file -o file [--disable-seccomp] [--allow-multi-process] [--copy-back file] [--exec-stderr file] [-l file] [-t num] [-m num] [--mem-rss-only] [-h] [--exec-command] [-- PROG [ARGS]]\n"
        "       %s --chroot-dir path --exec-file file --input file --output file [--disable-seccomp] [--allow-multi-process] [--copy-back file] [--exec-stderr file] [--log file] [--time-limit num] [--mem-limit num] [--mem-rss-only] [--help] [--exec-command] [-- PROG [ARGS]]\n"
        "  -c  --chroot-dir     The directory that will be chroot(2)ed in.\n"
        "  -e  --exec-file      The program (or source file) that will be executed or\n"
        "                       interpreted.\n"
        "      --exec-command   Enable the function to run command after '--'.\n"
        "  -i  --input          The file that will be the input source.\n"
        "  -o  --output         The file that will be the output (stdout) of the\n"
        "                       program.\n"
        "  -l  --log            (Optional, stderr by default) The file that will be the\n"
        "                       output (stderr) of the sandbox and program.\n"
        "  -t  --time-limit     (Optional, unlimited by default) The time (ms) limit of\n"
        "                       the program.\n"
        "  -m  --mem-limit      (Optional, unlimited by default) The memory size (MB)"
        "                       limit of the program.\n"
        "      --disable-seccomp\n"
        "                       (Optional) This will disable secure computing mode,\n"
        "                       the system call filter.\n"
        "      --copy-back      (Optional, usually required when compiling)"
        "                       The following argument will be copied back to the\n"
        "                       working directory.\n"
        "      --allow-multi-process\n"
        "                       (Optional) Allow up to 128 processes to run.\n"
        "      --exec-stderr    (Optional) This file will be the output (stderr) of the\n"
        "                       executed program.\n"
        "      --mem-rss-only   (Optional) Limit RSS (Resident Set Size) memory only, if\n"
        "                       --mem-limit is on.\n"
        "  -h  --help           (Optional) This will show this message.\n",
		a0, a0);
    exit(0);
}

void option_handle(int argc, char **argv)
{
    // init runArgs
    runArgs.timeLimit = runArgs.memLimit = 0;
    runArgs.chrootDir = runArgs.execFileName = runArgs.copyBackFileName = runArgs.inputFileName = runArgs.outputFileName = runArgs.logFileName = NULL;
    runArgs.isSeccompDisabled = runArgs.isCommandEnabled = runArgs.isMultiProcess = runArgs.isMemLimitRSS = false;
    runArgs.execCommand = (char **)NULL;
    runArgs.execStderr = NULL;
    int longIndex;
    char *endptr;
    long val;
    int opt;
    while ((opt = getopt_long(argc, argv, optString, longOpts, &longIndex)) != EOF)
    {
        switch (opt)
        {
        case 'c':
            runArgs.chrootDir = optarg;
            break;
        case 'e':
            runArgs.execFileName = optarg;
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
            if (strcmp("copy-back", longOpts[longIndex].name) == 0)
            {
                runArgs.copyBackFileName = optarg;
            }
            if (strcmp("exec-command", longOpts[longIndex].name) == 0)
            {
                runArgs.isCommandEnabled = true;
            }
            if (strcmp("allow-multi-process", longOpts[longIndex].name) == 0)
            {
                runArgs.isMultiProcess = true;
            }
            if (strcmp("exec-stderr", longOpts[longIndex].name) == 0)
            {
                runArgs.execStderr = optarg;
            }
            if (strcmp("mem-rss-only", longOpts[longIndex].name) == 0)
            {
                runArgs.isMemLimitRSS = true;
            }
            if (strcmp("max-processes", longOpts[longIndex].name) == 0)
            {
                maxProcesses = strtol(optarg, NULL, 10);
            }
            if (strcmp("output-file-size", longOpts[longIndex].name) == 0)
            {
                maxOutputFile = strtol(optarg, NULL, 10);
            }
            break;
        default:
            break;
        }
    }
    if (runArgs.isCommandEnabled)
    {
        if (optind >= argc)
        {
            log("Missing command. Remember to add your command after '--'!\n");
            exit(-1);
        }
        runArgs.execCommand = argv + optind;
    }
    // check
    if (runArgs.chrootDir == NULL || runArgs.execFileName == NULL || runArgs.inputFileName == NULL || runArgs.outputFileName == NULL)
    {
        display_title();
        log("Missing argument(s).\nUse %s -h or %s --help to get help.\n", argv[0], argv[0]);
        exit(-1);
    }
    if (runArgs.isMemLimitRSS && runArgs.memLimit == 0)
    {
        log("Missing --mem-limit when --mem-rss-only is on.\n");
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
    else if (sig == SIGUSR1)
        memLimKilled = true;
}

void setLimit(rlim_t maxMemory, rlim_t maxCPUTime, rlim_t maxProcessNum, rlim_t maxFileSize, rlim_t maxStackSize)
{
    /* The unit of some arguments:
	 * maxMemory (MB)
	 * maxCPUTime (s)
	 * maxFileSize (MB)
	 * maxStackSize (MB)
	 */
    if (maxMemory != 0)
    {
        maxMemory *= (1 << 20);
    }
    maxFileSize *= (1 << 20);
    if (maxStackSize != 0)
    {
        maxStackSize *= (1 << 20);
    }
    struct rlimit max_memory, max_cpu_time, max_process_num, max_file_size, max_stack, nocore;
    if (maxMemory != 0)
    {
        setrlimStruct(maxMemory, &max_memory);
    }
    if (maxCPUTime != 0)
    {
        setrlimStruct(maxCPUTime, &max_cpu_time);
    }
    if (maxProcessNum != 0)
    {
        setrlimStruct(maxProcessNum, &max_process_num);
    }
    setrlimStruct(maxFileSize, &max_file_size);
    if (maxStackSize != 0)
    {
        setrlimStruct(maxStackSize, &max_stack);
    }
    setrlimStruct(0, &nocore);

    if (maxMemory != 0)
    {
        if (setrlimit(RLIMIT_AS, &max_memory) != 0)
        {
            errorExit(RLERR);
        }
    }
    if (maxCPUTime != 0)
    {
        if (setrlimit(RLIMIT_CPU, &max_cpu_time) != 0)
        {
            errorExit(RLERR);
        }
    }
    if (maxProcessNum != 0)
    {
        if (setrlimit(RLIMIT_NPROC, &max_process_num) != 0)
        {
            errorExit(RLERR);
        }
    }
    if (setrlimit(RLIMIT_FSIZE, &max_file_size) != 0)
    {
        errorExit(RLERR);
    }
    if (maxStackSize != 0)
    {
        if (setrlimit(RLIMIT_STACK, &max_stack) != 0)
        {
            errorExit(RLERR);
        }
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
    if (input_file == NULL || output_file == NULL)
    {
        errorExit(FIERR);
    }
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
    /* redirect stderr */
    if (logpath != NULL)
    {
        FILE *log_file = fopen(logpath, "w");
        if (log_file == NULL) 
        {
            errorExit(FIERR);
        }
        if (dup2(fileno(log_file), fileno(stderr)) == -1)
        {
            errorExit(RDERR);
        }
    }
}

int main(int argc, char **argv)
{
    option_handle(argc, argv);
    if (!isRootUser())
    {
        log("This program requires root user.\n");
        exit(-1);
    }
    logRedirect(runArgs.logFileName);
    signal(SIGUSR1, ready);

/*     char *current_dir = get_current_dir_name();
    log("%s\n", current_dir);
    free(current_dir); */

    char *execFileBaseName = basename(runArgs.execFileName);
    // 1. copy prog
    char *chrootTmp = pathCat(runArgs.chrootDir, "/tmp");
    char *copyprogTo = pathCat(chrootTmp, execFileBaseName);
    copyFile(runArgs.execFileName, copyprogTo);
    char *chrootProg = execFileBaseName;
    initUser();
    son_exec = 0;
    son = fork();
    if (son < 0)
    {
        // fork failed
        errorExit(FOERR);
        return -1;
    }
    if (son == 0)
    {
        // child

        // set rlimit
        setLimit(runArgs.memLimit == 0 || runArgs.isMemLimitRSS ? 0 : (runArgs.memLimit * 1.5),
                 runArgs.timeLimit == 0 ? 0 : (int)(runArgs.timeLimit / 1000.0 + 1),
                 runArgs.isMultiProcess ? maxProcesses : 1,
                 maxOutputFile,
                 runArgs.memLimit == 0 || runArgs.isMemLimitRSS ? 0 : (runArgs.memLimit * 1.5)); // allow 1 process, 16 MB file size, rough time & memory limit
        // redirect stdin & stdout
        fileRedirect(runArgs.inputFileName, runArgs.outputFileName);

        logRedirect(runArgs.execStderr);
        // chroot
        chroot(runArgs.chrootDir);
        chdir("/tmp");
        // set uid & gid to nobody
        setNonPrivilegeUser();

        char /* **f_envp = {NULL},*/ **f_argv = {NULL};
        if (runArgs.isCommandEnabled)
        {
            f_argv = runArgs.execCommand;
            chrootProg = runArgs.execCommand[0];
        }

        while (!son_exec)
            ;

        // load seccomp rule
        if (!runArgs.isSeccompDisabled)
            whiteListProgRules(chrootProg);
        // exec
        log("=== PROG START ===\n");
        execv(chrootProg, f_argv);
        perror("exec error"); // unreachable normally
        return -1;
    }
    else
    {
        // parent
        char procStat[12 + 10] = {};
        sprintf(procStat, "/proc/%d/stat", son);
        // set timer
        signal(SIGALRM, killChild);
        struct itimerval itval;
        if (runArgs.timeLimit != 0)
        {
            itval.it_interval.tv_sec = itval.it_interval.tv_usec = 0; // only once
            itval.it_value.tv_sec = runArgs.timeLimit / 1000;
            itval.it_value.tv_usec = ((runArgs.timeLimit + 50) % 1000) * 1000; // Allow up to 50 ms of systematic error
            if (setitimer(ITIMER_REAL, &itval, NULL) == -1)
            {
                perror("setitimer error");
            }
        }
        struct timeval progStart, progEnd, useTime;
        gettimeofday(&progStart, NULL);
        // call son to exec
        kill(son, SIGUSR1);
        // wait & cleanup
        struct rusage sonUsage;
        int status;
        unsigned long memory_max = 0, memory_now = 0;         // virt mem
        unsigned long rss_memory_max = 0, rss_memory_now = 0; // rss mem
        int pagesize = getpagesize();
        while (wait3(&status, WUNTRACED | WNOHANG, &sonUsage) == 0)
        {
            FILE *procFile = fopen(procStat, "r");
            if (procFile)
            {
                // prevent segfault
                fscanf(procFile, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %lu %lu", &memory_now, &rss_memory_now);
                fclose(procFile);
            }
            if (!runArgs.isMemLimitRSS)
            {
                if (memory_now > memory_max)
                {
                    memory_max = memory_now;
                    if (runArgs.memLimit != 0 && memory_max > runArgs.memLimit * (1 << 20))
                    {
                        killChild(SIGUSR1); // mem > limit
                    }
                }
            }
            else
            {
                rss_memory_now *= pagesize / (1 << 10);
                if (rss_memory_now > rss_memory_max)
                {
                    rss_memory_max = rss_memory_now;
                    if (runArgs.memLimit != 0 && rss_memory_max > runArgs.memLimit * (1 << 10))
                    {
                        killChild(SIGUSR1);
                    }
                }
            }
        }
        memory_max /= (1 << 10); // accurate virt usage
        int cpuTime = timevalms(&sonUsage.ru_utime) + timevalms(&sonUsage.ru_stime);
        // long maxrss = sonUsage.ru_maxrss;
        gettimeofday(&progEnd, NULL);
        timersub(&progEnd, &progStart, &useTime);
        int actualTime = timevalms(&useTime);
        remove(copyprogTo);
        if (runArgs.copyBackFileName != NULL)
        {
            char *copyFrom = pathCat(chrootTmp, runArgs.copyBackFileName);
            // copyto TODO
            copyFile(copyFrom, runArgs.copyBackFileName);
            free(copyFrom);
        }
        itval.it_value.tv_sec = itval.it_value.tv_usec = 0; // stop timer
        if (WIFEXITED(status))
        {
            int ret = WEXITSTATUS(status);
            if (ret == 0)
            {
                puts(RES_OK);
            }
            else
            {
                puts(RES_RE);
            }
        }
        else if (WIFSIGNALED(status))
        {
            int sig = WTERMSIG(status);
            if (killedByTimer || sig == SIGXCPU)
            {
                puts(RES_TLE);
            }
            else if (sig == SIGXFSZ)
            {
                puts(RES_FSE);
            }
            else if (memLimKilled || (runArgs.memLimit != 0 && memory_max > runArgs.memLimit * (1 << 10)))
            {
                puts(RES_MLE);
            }
            else
            {
                puts(RES_RE);
            }
        }
        else if (WIFSTOPPED(status))
        {
            killChild(WSTOPSIG(status));
            puts(RES_RE);
        }
        printf("%d %lu %d %lu\n", actualTime, memory_max, cpuTime, rss_memory_max);

        free(chrootTmp);
        free(copyprogTo);
    }

    return 0;
}
