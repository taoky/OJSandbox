#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define FIERR "file error"
#define FSERR "fstat() error"
#define CPERR "sendfile() error"

char tmp[] = "/tmp/ojs-XXXXXX";

void errorExit(char str[]) {
	perror(str);
	exit(-1);
}

bool isRootUser() {
	return !(geteuid() || getegid());
}

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
	 */
	int plen = strlen(path), flen = strlen(fileName);
	char *res = malloc(plen + flen);
	strcpy(res, path);
	res[plen] = '/';
	strcpy(res + plen + 1, fileName);
	return res;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s FILENAME\n", argv[0]);
		exit(-1);
	}
	if (!isRootUser()) {
		printf("This program must be run in root user!\n");
		exit(-1);
	}
	char *oriPro = argv[1];
	char *res = mkdtemp(tmp);
	if (res == NULL) {
		perror("mkdtemp error");
		exit(-1);
	}
	char *tarPro = pathCat(tmp, oriPro);
	copyFile(oriPro, tarPro);
	chmod(tarPro, 0777);
	pid_t son = fork();
	if (son < 0) {
		perror("fork error");
		exit(-1);
	}
	else if (son == 0) {
		// Child
		printf("%s", argv[1]);
		int res = chroot(tmp);
		if (res == -1) {
			perror("chroot error");
			exit(-1);
		}
		res = chdir("/");
		if (res == -1) {
			perror("chdir error");
			exit(-1);
		}
		setuid(65534);
		setgid(65534);
		char *cargv[] = {NULL};
		char *cenv[] = {NULL};
		execve(argv[1], cargv, cenv); // static link only
		perror("exec error");
	}
	else {
		// Parent
		wait(NULL);
	}
	return 0;
}
