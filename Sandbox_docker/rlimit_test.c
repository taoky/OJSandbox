#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <stdbool.h>

#define NOBODY 65534
#define RLERR "rlimit error"
#define NBERR "set gid & uid error"

bool isRootUser() {
	return !getuid();
}

void setNonPrivilegeUser() {
	int status = 0;
	status = setgid(NOBODY);
	status = setuid(NOBODY);
	if (status == -1) {
		perror(NBERR);
		exit(-1);
	}
}

void setrlimStruct(rlim_t num, struct rlimit * st) {
	st->rlim_cur = st->rlim_max = num;
}

void setLimit(rlim_t maxMemory, rlim_t maxCPUTime, rlim_t maxProcessNum, rlim_t maxFileSize, rlim_t maxStackSize) {
	/* The unit of some arguments:
	 * maxMemory (MB)
	 * maxCPUTime (ms)
	 * maxFileSize (MB)
	 * maxStackSize (MB)
	 */
	maxMemory *= (1 << 20);
	//maxCPUTime *= 1000;
	maxFileSize *= (1 << 20);
	maxStackSize *= (1 << 20);
	struct rlimit max_memory, max_cpu_time, max_process_num, max_file_size, max_stack;
	setrlimStruct(maxMemory, &max_memory);
	setrlimStruct(maxCPUTime, &max_cpu_time);
	setrlimStruct(maxProcessNum, &max_process_num);
	setrlimStruct(maxFileSize, &max_file_size);
	setrlimStruct(maxStackSize, &max_stack);
	if (setrlimit(RLIMIT_AS, &max_memory) != 0) {
		perror(RLERR);
		exit(-1);
	}
	if (setrlimit(RLIMIT_CPU, &max_cpu_time) != 0) {
	        perror(RLERR);
		exit(-1);
	}	       
	if (setrlimit(RLIMIT_NPROC, &max_process_num) != 0) {
		perror(RLERR);
		exit(-1);
	}
	if (setrlimit(RLIMIT_FSIZE, &max_file_size) != 0) {
		perror(RLERR);
		exit(-1);
	}
	if (setrlimit(RLIMIT_STACK, &max_stack) != 0) {
		perror(RLERR);
		exit(-1);
	}
}

int main(int argc, char *argv[]) {
	if (!isRootUser()) {
		puts("This program should be run only in root user!");
		exit(-1);
	}
	setLimit(5, 1, 1, 5, 5);
	setNonPrivilegeUser();
	execv("test", argv);
	return 0;
}
