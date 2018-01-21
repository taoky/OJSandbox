#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <stdbool.h>

#define NOBODY 65534
#define RLERR "rlimit error"
#define NBERR "set gid & uid error"
#define RDERR "file descriptor redirect error"

bool isRootUser() {
	return !getuid();
}

void errorExit(char str[]) {
	perror(str);
	exit(-1);
}

void setNonPrivilegeUser() {
	int status = 0;
	status = setgid(NOBODY);
	status = setuid(NOBODY);
	if (status == -1) {
		errorExit(NBERR);
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

int main(int argc, char *argv[]) {
	if (!isRootUser()) {
		fprintf(stderr, "This program should be run only in root user!\n");
		exit(-1);
	}
	fileRedirect("testinput", "testoutput");
	setLimit(5, 1, 1, 5, 5);
	setNonPrivilegeUser();
	execv("test", argv);
	return 0;
}
