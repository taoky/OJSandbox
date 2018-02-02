#include "util.h"

uid_t ojsUID = 0;
gid_t ojsGID = 0;

const char *const RLERR = "rlimit error";
const char *const GIERR = "set gid error";
const char *const UIERR = "set uid error";
const char *const RDERR = "file descriptor redirect error";
const char *const FOERR = "fork() error";
const char *const SGERR = "sigaction() error";
const char *const EXERR = "exec error";
const char *const TPERR = "mkdtemp() error";
const char *const FIERR = "file error";
const char *const FSERR = "fstat() error";
const char *const CPERR = "sendfile() error";
const char *const USERR = "(User) ojs's gid or uid error";
const char *const CGERR = "cgroup error";
const char *const SCERR = "seccomp error";

const char *const RES_OK = "OK";
const char *const RES_RE = "RE";
const char *const RES_TLE = "TLE";
const char *const RES_FSE = "FSE";
const char *const RES_MLE = "MLE";

bool isRootUser(void) {
	return !(geteuid() || getegid());
}

void errorExit(const char *str) {
	perror(str);
	exit(-1);
}

void copyFile(const char *from, const char *to) {
	int fd_in, fd_out;
	struct stat st;
	off_t offset = 0;
	fd_in = open(from, O_RDONLY);
	if (fd_in == -1) {
		errorExit(FIERR);
	}
	if (fstat(fd_in, &st) == -1) {
		errorExit(FSERR);
	}
	fd_out = open(to, O_CREAT | O_WRONLY, st.st_mode);
	if (fd_out == -1) {
		errorExit(FIERR);
	}
	if (sendfile(fd_out, fd_in, &offset, st.st_size) == -1) {
		errorExit(CPERR);
	}
	close(fd_in);
	close(fd_out);
}

void initUser(void) {
    struct passwd *ojsuser = getpwnam("ojs");
	if (ojsuser == NULL) {
		errorExit(USERR);
	}
	ojsUID = ojsuser->pw_uid;
	ojsGID = ojsuser->pw_gid;
	if (!ojsUID || !ojsGID) {
		errorExit(USERR);
	}
	// nobodyUID = nobodyGID = 65534;
}

void setNonPrivilegeUser(void) {
	int status = 0;
	if (ojsGID == 0 || ojsUID == 0) {
		initUser();
	}
	status = setgid(ojsGID);
	if (status == -1) {
		errorExit(GIERR);
	}
	status = setuid(ojsUID);
	if (status == -1) {
		errorExit(UIERR);
	}
}

char *pathCat(const char *path, const char *fileName) {
	/*
	 * dynamic alloc memory
     * free when necessary
	 */
	int plen = strlen(path), flen = strlen(fileName);
	char *res = malloc(plen + flen + 2);
	strcpy(res, path);
	res[plen] = '/';
	strcpy(res + plen + 1, fileName);
	return res;
}

/* bool isPathLink(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        printf("%s\n", path);
        errorExit(FSERR);
    }
    if ((st.st_mode & S_IFMT) == S_IFLNK) return true;
    else return false;
}

bool isPathDir(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        printf("%s\n", path);
        errorExit(FSERR);
    }
    if ((st.st_mode & S_IFMT) == S_IFDIR) return true;
    else return false;
}

long long readFileLL(const char *path) {
	long long res;
	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "%s error\n", path);
		perror("unable to open file");
		return -1;
	}
	fscanf(f, "%lld", &res);
	fclose(f);
	return res;
}

int writeFileInt(const char *path, int value, bool isOverWrite) {
	FILE *f;
	if (isOverWrite) {
		f = fopen(path, "w");
	}
	else {
		f = fopen(path, "a");
	}
	if (!f) {
		fprintf(stderr, "%s error\n", path);
		perror("unable to open file");
		return -1;
	}
	fprintf(f, "%d\n", value);
	fclose(f);
	return value;
}

bool writeFileStr(const char *path, const char *value, bool isOverWrite) {
	FILE *f;
	if (isOverWrite) {
		f = fopen(path, "w");
	}
	else {
		f = fopen(path, "a");
	}
	if (!f) {
		fprintf(stderr, "%s error\n", path);
		perror("unable to open file");
		return false;
	}
	fprintf(f, "%s\n", value);
	fclose(f);
	return true;
}

bool clearFile(const char *path) {
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "%s error\n", path);
		perror("unable to open file");
		return false;
	}
	fclose(f);
	return true;
} */

int timevalms(const struct timeval *timev) {
	return timev->tv_sec * 1000 + timev->tv_usec / 1000;
}

void setrlimStruct(rlim_t num, struct rlimit * st) {
	st->rlim_cur = st->rlim_max = num;
}
