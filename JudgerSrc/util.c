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
		if (errno != 0)
			errorExit(USERR);
		else {
			log("Cannot find user 'ojs'.\n");
			exit(-1);
		}
	}
	ojsUID = ojsuser->pw_uid;
	ojsGID = ojsuser->pw_gid;
	if (!ojsUID || !ojsGID) {
		errorExit(USERR);
	}
	// ojsUID = ojsGID = 65534; // alpine's nobody's uid & gid are surely 65534.
}

void setNonPrivilegeUser(void) {
	int status = 0;
	if (ojsGID == 0 || ojsUID == 0) {
		initUser();
	}
	status = setregid(ojsGID, ojsGID);
	if (status == -1) {
		errorExit(GIERR);
	}
	status = setreuid(ojsUID, ojsUID);
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

int timevalms(const struct timeval *timev) {
	return timev->tv_sec * 1000 + timev->tv_usec / 1000;
}

void setrlimStruct(rlim_t num, struct rlimit * st) {
	st->rlim_cur = st->rlim_max = num;
}

// char *unstandard_basename(const char *str) {
// 	/* port from glibc's prototype to musl libc */
// 	char *p = strrchr(str, '/');
// 	if (p == NULL)
// 		return (char *)str;
// 	else
// 		return p + 1;
// }

void default_signal(int signum, sighandler_t handler) {
	struct sigaction act = {};
	act.sa_flags = SA_RESTART;
	act.sa_handler = handler;
	if (sigaction(signum, &act, NULL) == -1) {
		errorExit(SGERR);
	}
}