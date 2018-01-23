#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

rlim_t maxMemory = 128 * (1 << 20);
rlim_t maxCPUTime = 2;
rlim_t maxProcessNum = 1;
rlim_t maxFileSize = 5 * (1 << 20);

char *cpu_cgroup = "/sys/fs/cgroup/cpuacct/ojs"
char *mem_cgroup = "/sys/fs/cgroup/memory/ojs"
char *pid_cgroup = "/sys/fs/cgroup/pids/ojs"

bool isRootUser() {
	return !(geteuid() || getegid());
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s FILENAME\n", argv[0]);
		exit(-1);
	}
	if (!isRootUser()) {
		printf("This program should be run only in root user!\n");
		exit(-1);
	}

	pid_t son = fork();
	if (son < 0) {
		perror("fork error");
	}
	else if (son == 0) {
		// child
		mkdir(cpu_cgroup, 0755);
		mkdir(mem_cgroup, 0755);
		mkdir(pid_cgroup, 0755);
		//TODO
		char *cargv[] = {argv[1], NULL};
		char *cenv[] = {NULL};
		execve(argv[1], cargv, cenv);
		perror("exec error");
	}
	else {
		// parent
		wait(NULL);
		rmdir(cpu_cgroup);
		rmdir(mem_cgroup);
		rmdir(pid_cgroup);
	}
	return 0;
}
