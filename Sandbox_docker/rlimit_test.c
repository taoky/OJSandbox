#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>

// #define NOBODY 65534
// Cannot assume that nobody's uid & gid are 65534.
#define RLERR "rlimit error"
#define GIERR "set gid error"
#define UIERR "set uid error"
#define RDERR "file descriptor redirect error"
#define FOERR "fork() error"
#define SGERR "sigaction() error"
#define EXERR "exec error"
#define TPERR "mkdtemp() error"
#define FIERR "file error"
#define FSERR "fstat() error"
#define CPERR "sendfile() error"
#define NBERR "nobody's gid or uid error"

pid_t son;
bool killedByAlarm = false;
char tmpDir[] = "/tmp/OJSandbox-XXXXXX";
uid_t nobodyUID;
gid_t nobodyGID;
char *execFile;

bool isRootUser() {
	return !(geteuid() || getegid());
}

void errorExit(char str[]) {
	perror(str);
	exit(-1);
}

void init() {
	struct passwd *nobody = getpwnam("nobody");
	nobodyUID = nobody->pw_uid;
	nobodyGID = nobody->pw_gid;
	if (!nobodyUID || !nobodyGID) {
		errorExit(NBERR);
	}
}

void setNonPrivilegeUser() {
	int status = 0;
	status = setgid(nobodyGID);
	if (status == -1) {
		errorExit(GIERR);
	}
	status = setuid(nobodyUID);
	if (status == -1) {
		errorExit(UIERR);
	}
}

void setrlimStruct(rlim_t num, struct rlimit * st) {
	st->rlim_cur = st->rlim_max = num;
}

void setLimit(rlim_t maxMemory, rlim_t maxCPUTime, rlim_t maxProcessNum, rlim_t maxFileSize, rlim_t maxStackSize) {
	/* The unit of some arguments:
	 * maxMemory (MB)
	 * maxCPUTime (s)
	 * maxFileSize (MB)
	 * maxStackSize (MB)
	 */
	maxMemory *= (1 << 20);
	maxFileSize *= (1 << 20);
	maxStackSize *= (1 << 20);
	struct rlimit max_memory, max_cpu_time, max_process_num, max_file_size, max_stack, nocore, nofile;
	setrlimStruct(maxMemory, &max_memory);
	setrlimStruct(maxCPUTime, &max_cpu_time);
	setrlimStruct(maxProcessNum, &max_process_num);
	setrlimStruct(maxFileSize, &max_file_size);
	setrlimStruct(maxStackSize, &max_stack);
	setrlimStruct(0, &nocore);
	// setrlimStruct(4, &nofile); // stdin, stdout & stderr
	if (setrlimit(RLIMIT_AS, &max_memory) != 0) {
		errorExit(RLERR);
	}
	if (setrlimit(RLIMIT_CPU, &max_cpu_time) != 0) {
		errorExit(RLERR);
	}	       
	if (setrlimit(RLIMIT_NPROC, &max_process_num) != 0) {
		errorExit(RLERR);
	}
	if (setrlimit(RLIMIT_FSIZE, &max_file_size) != 0) {
		errorExit(RLERR);
	}
	if (setrlimit(RLIMIT_STACK, &max_stack) != 0) {
		errorExit(RLERR);
	}
	// set no core file:
	if (setrlimit(RLIMIT_CORE, &nocore) != 0) {
		errorExit(RLERR);
	}
	// set num of file descriptor:
	//if (setrlimit(RLIMIT_NOFILE, &nofile) != 0) {
	//	errorExit(RLERR);
	//}
}

void fileRedirect(char inputpath[], char outputpath[]) {
	/*
	null_file = fopen("/dev/null", "w");
	if (dup2(fileno(null_file), fileno(stderr)) == -1) {
		perror(FRERR);
		exit(-1);
	} */
	FILE * input_file = fopen(inputpath, "r");
	FILE * output_file = fopen(outputpath, "w");
	if (dup2(fileno(input_file), fileno(stdin)) == -1) {
		errorExit(RDERR);
	}
	if (dup2(fileno(output_file), fileno(stdout)) == -1) {
		errorExit(RDERR);
	}
}

void killChild() {
	int res = kill(son, SIGKILL);
	if (res == -1 && errno == ESRCH) {
		fprintf(stderr, "Cannot find child process. Maybe it has exited.\n");
	}
	else if (res == -1) {
		fprintf(stderr, "Failed to kill child.\n");
	}
	killedByAlarm = true;
}

/*
 * The COW copy is not suitable here, because it won't work if files are not on the same mounted filesystem.

static loff_t copy_file_range(int fd_in, loff_t *off_in, int fd_out,
                       loff_t *off_out, size_t len, unsigned int flags)
{
	// the implementation of copy_file_range()
	// reference: the manpage of copy_file_range
	return syscall(__NR_copy_file_range, fd_in, off_in, fd_out, 
			off_out, len, flags);
}


void copyFile(char *from, char *to) {
	// require Linux 4.5+, COW (Copy On Write)
	// reference: the manpage of copy_file_range
	int fd_in = open(from, O_RDONLY), fd_out;
	struct stat stat;
	loff_t len, ret;
	if (fd_in == -1) {
		errorExit(FIERR);
	}
	if (fstat(fd_in, &stat) == -1) {
		errorExit(FSERR);
	}
	len = stat.st_size;
	fd_out = open(to, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd_out == -1) {
		errorExit(FIERR);
	}
	do {
		ret = copy_file_range(fd_in, NULL, fd_out, NULL, len, 0);
		if (ret == -1) {
			errorExit(CPERR);
		}
		len -= ret;
	} while (len > 0);

	close(fd_in);
	close(fd_out);
}
*/

void copyFile(char *from, char *to) {
	int fd_in, fd_out;
	struct stat stat;
	off_t offset = 0;
	fd_in = open(from, O_RDONLY);
	if (fd_in == -1) {
		errorExit(FIERR);
	}
	if (fstat(fd_in, &stat) == -1) {
		errorExit(FSERR);
	}
	fd_out = open(to, O_CREAT | O_WRONLY, stat.st_mode);
	if (fd_out == -1) {
		errorExit(FIERR);
	}
	if (sendfile(fd_out, fd_in, &offset, stat.st_size) == -1) {
		errorExit(CPERR);
	}
	close(fd_in);
	close(fd_out);
}

char *pathCat(char *path, char *fileName) {
	/*
	 * dynamic alloc memory
	 * need to be freed
	 */
	int plen = strlen(path), flen = strlen(fileName);
	char *res = malloc(plen + flen);
	strcpy(res, path);
	res[plen] = '/';
	strcpy(res + plen + 1, fileName);
	return res;
}

void initTmp(char *progName) {
	char *tmp = mkdtemp(tmpDir);
	if (tmp == NULL) {
		errorExit(TPERR);
	}
	chown(tmp, nobodyUID, nobodyGID);
	execFile = pathCat(tmp, progName);
	copyFile(progName, execFile);
	chown(execFile, nobodyUID, nobodyGID);
	chmod(execFile, S_IRUSR | S_IWUSR | S_IXUSR);
}

int main(int argc, char *argv[]) {
	if (!isRootUser()) {
		fprintf(stderr, "This program should be run only in root user!\n");
		exit(-1);
	}
	init();

	int timeLimit = 2; // s
	int memoryLimit = 128; // MB
	char progName[] = "test";

	initTmp(progName);

	son = fork();
	if (son == -1) {
		// fork failed
		errorExit(FOERR);
	}
	if (son == 0) {
		// child process
		fileRedirect("testinput", "testoutput");
		setLimit(memoryLimit, timeLimit, 10, 5, memoryLimit);
		setNonPrivilegeUser();
		execv(execFile, argv);
		errorExit(EXERR); // unreachable normally
	}
	else {
		// parent process
		struct sigaction alarmKill;
		struct rusage sonUsage; // TODO
		int status;
		alarmKill.sa_handler = killChild;
		if (sigaction(SIGALRM, &alarmKill, NULL) == -1) {
			errorExit(SGERR);
		}
		alarm(timeLimit);
		while (1) {
			int res = waitpid(-1, &status, WUNTRACED | WNOHANG);
			getrusage(RUSAGE_CHILDREN, &sonUsage);
			if (res) {
				printf("%d, %d\n", status, res);
				if (WIFEXITED(status)) {
					puts("The program terminated.");
					printf("Exit code: %d\n", WEXITSTATUS(status));
				}
				else if (WIFSIGNALED(status)) {
					puts("The program was terminated by a signal.");
					printf("Signal code: %d\n", WTERMSIG(status));
				}
				else if (WIFSTOPPED(status)) {
					puts("The program was stopped by a signal. (WUNTRACED or being traced).");
					printf("Signal code: %d\n", WSTOPSIG(status));
				}
				return 0;
			}
		}
	}
	return 0;
}
