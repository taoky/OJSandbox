#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

rlim_t maxMemory = 128 * (1 << 20);
rlim_t maxCPUTime = 2;
rlim_t maxProcessNum = 1;
rlim_t maxFileSize = 5 * (1 << 20);

void setrlimStruct(rlim_t num, struct rlimit *st) {
	st->rlim_cur = st->rlim_max = num;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s FILENAME\n", argv[0]);
		exit(-1);
	}
	pid_t son = fork();
	if (son < 0) {
		perror("fork error");
	}
	else if (son == 0) {
		// child
		struct rlimit max_memory, max_cpu_time, max_process_num, max_file_size, nocore;
		setrlimStruct(maxMemory, &max_memory);
		setrlimStruct(maxCPUTime, &max_cpu_time);
		setrlimStruct(maxProcessNum, &max_process_num);
		setrlimStruct(maxFileSize, &max_file_size);
		setrlimStruct(0, &nocore);
		if (setrlimit(RLIMIT_AS, &max_memory) != 0) {
			perror("rlimit error");
		}
		if (setrlimit(RLIMIT_CPU, &max_cpu_time) != 0) {
			perror("rlimit error");
		}
		if (setrlimit(RLIMIT_NPROC, &max_process_num) != 0) {
			perror("rlimit error");
		}
		if (setrlimit(RLIMIT_FSIZE, &max_file_size) != 0) {
			perror("rlimit error");
		}
		if (setrlimit(RLIMIT_STACK, &max_memory) != 0) { // max stack = max memory
			perror("rlimit error");
		}
		// set no core file:
		if (setrlimit(RLIMIT_CORE, &nocore) != 0) {
			perror("rlimit error");
		}
		char *cargv[] = {NULL};
		char *cenv[] = {NULL};
		execve(argv[1], cargv, cenv);
		perror("exec error");
	}
	else {
		// parent
		wait(NULL);
	}
	return 0;
}
