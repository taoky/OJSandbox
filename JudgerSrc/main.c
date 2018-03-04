#include "main.h"
#include "util.h"
#include "secrules.h"
#include "config.h"

#define TITLE "OJ sandbox backend for Docker (version " GITVERSION_STR ", commit " GITCOMMIT_STR ", " __DATE__ " at " __TIME__ ")"

pid_t son;
static volatile int son_exec = 0;
bool killedByTimer = false;
bool memLimKilled = false;

static int maxOutputFile = 16; // Unit: MiB

struct runArgs_t
{
    char **execCommand;      // after '--'
    char *inputFileName;     // --input
    char *outputFileName;    // --output
    char *logFileName;       // --log, optional
    char *execStderr;        // --exec-stderr, optional
    unsigned long timeLimit; // --time-limit
    unsigned long memLimit;  // --mem-limit
    bool isSeccompDisabled;  // --disable-seccomp, optional
    bool isLimitVM;          // --enable-vm-limit
} runArgs;

static const char * const optString = "+i:o:t:m:l:hv?";

static const struct option longOpts[] = {
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"log", required_argument, NULL, 'l'},
    {"time-limit", required_argument, NULL, 't'},
    {"mem-limit", required_argument, NULL, 'm'},
    {"disable-seccomp", no_argument, NULL, 0},
    {"exec-stderr", required_argument, NULL, 0},
    {"enable-vm-limit", no_argument, NULL, 0},
    {"output-file-size", required_argument, NULL, 0},
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {NULL, no_argument, NULL, 0}};

void display_version(const char *a0)
{
    // Strip everything before the last slash to get the base name
    const char * pname = a0 + strlen(a0);
    for (int i = strlen(a0); i > 0; i--)
    {
        if (*--pname == '/')
        {
            ++pname;
            break;
        }
    }
    log("%s, " TITLE "\n", pname);
    exit(0);
}

void display_help(const char *a0)
{
    log(TITLE "\n\n"
        "Usage: %s -i file -o file [--disable-seccomp] [--allow-multi-process] [--exec-stderr file] [-l file] [-t num] [-m num] [--enable-vm-limit] [-h|-v|-?] -- PROG [ARGS]\n"
        "       %s --input file --output file [--disable-seccomp] [--allow-multi-process] [--exec-stderr file] [--log file] [--time-limit num] [--mem-limit num] [--enable-vm-limit] [--help|--version|-?] -- PROG [ARGS]\n"
        "  -i  --input          The file that will be the input source.\n"
        "  -o  --output         The file that will be the output (stdout) of the\n"
        "                       program.\n"
        "  -l  --log            (Optional, stderr by default) The file that will be the\n"
        "                       output (stderr) of the sandbox and program.\n"
        "  -t  --time-limit     (Optional, unlimited by default) The time (ms) limit of\n"
        "                       the program.\n"
        "  -m  --mem-limit      (Optional, unlimited by default) The memory size (MiB)\n"
        "                       limit of the program.\n"
        "      --disable-seccomp\n"
        "                       (Optional) This will disable secure computing mode,\n"
        "                       the system call filter.\n"
        "      --exec-stderr    (Optional) This file will be the output (stderr) of the\n"
        "                       executed program.\n"
        "      --enable-vm-limit\n"
        "                       (Optional) Limit VM (Virtual Memory) only, if\n"
        "                       --mem-limit is on.\n"
        "      --output-file-size\n"
        "                       (Optional, 16MiB by default) Max output file size.\n"
        "\n"
        "  -h  --help           Show this help message and quit.\n"
        "  -v  --version        Show version information and quit.\n",
		a0, a0);
    exit(0);
}

void option_handle(int argc, char **argv)
{
    // init runArgs
    runArgs.timeLimit = runArgs.memLimit = 0;
    runArgs.inputFileName = runArgs.outputFileName = runArgs.logFileName = NULL;
    runArgs.isSeccompDisabled = runArgs.isLimitVM = false;
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
            if (strcmp("exec-stderr", longOpts[longIndex].name) == 0)
            {
                runArgs.execStderr = optarg;
            }
            if (strcmp("enable-vm-limit", longOpts[longIndex].name) == 0)
            {
                runArgs.isLimitVM = true;
            }
            if (strcmp("output-file-size", longOpts[longIndex].name) == 0)
            {
                errno = 0;
                val = strtol(optarg, NULL, 10);
                if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0) || (endptr == optarg) || val <= 0)
                {
                    log("Output file size error.\n");
                    exit(-1);
                }
                maxOutputFile = val;
            }
            break;
        default:
            break;
        }
    }
    if (runArgs.inputFileName == NULL || runArgs.outputFileName == NULL)
    {
        log("Missing argument(s).\nUse %s -h or %s --help to get help.\n", argv[0], argv[0]);
        exit(-1);
    }
    if (optind >= argc)
    {
        log("Missing command. Remember to add your command after '--'!\n");
        exit(-1);
    }
    runArgs.execCommand = argv + optind;
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

void setLimit(rlim_t maxMemory, rlim_t maxCPUTime,
                 rlim_t maxFileSize, rlim_t maxStackSize)
{
    /* The unit of some arguments:
	 * maxMemory (MiB)
	 * maxCPUTime (s)
	 * maxFileSize (MiB)
	 * maxStackSize (MiB)
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
    struct rlimit max_memory, max_cpu_time,
                 max_file_size, max_stack, nocore;
    if (maxMemory != 0)
    {
        setrlimStruct(maxMemory, &max_memory);
    }
    if (maxCPUTime != 0)
    {
        setrlimStruct(maxCPUTime, &max_cpu_time);
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
    if (!isPrivilege())
    {
        log("This program requires root user or enough capabilities.\n");
        exit(-1);
    }
    logRedirect(runArgs.logFileName);
    default_signal(SIGUSR1, ready);

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
        setLimit(runArgs.memLimit == 0 || !runArgs.isLimitVM ? 0 : (runArgs.memLimit * 1.5),
                 runArgs.timeLimit == 0 ? 0 : (int)((runArgs.timeLimit + 1000) / 1000),
                //  runArgs.isMultiProcess ? 128 : 1,
                 maxOutputFile,
                 runArgs.memLimit == 0 || !runArgs.isLimitVM ? 0 : (runArgs.memLimit * 1.5)); // allow 1 process, 16 MB file size, rough time & memory limit
        // redirect stdin & stdout
        fileRedirect(runArgs.inputFileName, runArgs.outputFileName);

        logRedirect(runArgs.execStderr);
        // set uid & gid to user 'ojs'
        setNonPrivilegeUser();

        char **f_argv = runArgs.execCommand;
        char *execProg = runArgs.execCommand[0];

        while (!son_exec)
            ;

        // load seccomp rule
        if (!runArgs.isSeccompDisabled)
            whiteListProgRules(execProg);
        // exec
        log("=== PROG START ===\n");
        execv(execProg, f_argv);
        perror("exec error"); // unreachable normally
        return -1;
    }
    else
    {
        // parent
        char procStat[13 + 10] = {};
        snprintf(procStat, sizeof(procStat),  "/proc/%d/statm", son);
        // set timer
        default_signal(SIGALRM, killChild);
        struct itimerval itval;
        if (runArgs.timeLimit != 0)
        {
            itval.it_interval.tv_sec = itval.it_interval.tv_usec = 0; // only once
            itval.it_value.tv_sec = (runArgs.timeLimit + TL_MARGIN) / 1000;
            itval.it_value.tv_usec = ((runArgs.timeLimit + TL_MARGIN) % 1000) * 1000; // Allow up to 50 ms of systematic error
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
        unsigned long memory_max = 0, memory_now = 0;         // rss mem
        int pagesize = getpagesize();
        while (wait3(&status, WUNTRACED | WNOHANG, &sonUsage) == 0)
        {
            FILE *procFile = fopen(procStat, "r");
            if (procFile)
            {
                // prevent segfault
                if (fscanf(procFile, "%*u %lu", &memory_now) < 1)
				{
					memory_now = 0;
				}
                fclose(procFile);
            }
			memory_now *= pagesize / (1 << 10);
			if (memory_now > memory_max)
			{
				memory_max = memory_now;
				if (runArgs.memLimit != 0 && memory_max > runArgs.memLimit * (1 << 10))
				{
					killChild(SIGUSR1);
				}
			}
        }
        int cpuTime = timevalms(&sonUsage.ru_utime) + timevalms(&sonUsage.ru_stime);
        // long maxrss = sonUsage.ru_maxrss;
        gettimeofday(&progEnd, NULL);
        timersub(&progEnd, &progStart, &useTime);
        int actualTime = timevalms(&useTime);
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
        printf("%d %lu %d %lu\n", actualTime, memory_max, cpuTime, memory_max);
    }

    return 0;
}
