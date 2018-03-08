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
const char *const CHERR = "chroot error";
const char *const CAERR = "capability error";

const char *const RES_OK = "OK";
const char *const RES_RE = "RE";
const char *const RES_TLE = "TLE";
const char *const RES_FSE = "FSE";
const char *const RES_MLE = "MLE";

bool isPrivilege(void) {
	// return !(geteuid() || getegid());
	if (!getuid() && !getgid() && !geteuid() && !getegid())
		return true; // is root user
	else {
		cap_t cap_p = cap_get_proc();
		if (cap_p == NULL) {
			errorExit(CAERR);
		}
		/* required capabilities by this program:
		CAP_KILL
		CAP_SYS_RESOURCE
		CAP_SYS_CHROOT
		CAP_SETUID
		CAP_SETGID
		*/
		cap_value_t cap_list[] = {CAP_KILL, CAP_SYS_RESOURCE, CAP_SYS_CHROOT, CAP_SETUID, CAP_SETGID};
		for (int i = 0; i < sizeof(cap_list) / sizeof(cap_value_t); i++) {
			cap_flag_value_t value_p;
			if (cap_get_flag(cap_p, cap_list[i], CAP_EFFECTIVE, &value_p) == -1) {
				cap_free(cap_p);
				errorExit(CAERR);
			}
			if (value_p == CAP_CLEAR) {
				char *cap_name = cap_to_name(cap_list[i]);
				log("Missing capability: %s\n", cap_name);
				cap_free(cap_name); cap_free(cap_p);
				return false;
			}
		}
		cap_free(cap_p);
		return true;
	}
	// return false;
}

void errorExit(const char *str) {
	perror(str);
	exit(-1);
}

int copyFile(const char *from, const char *to) {
	int fd_in, fd_out;
	struct stat st;
	off_t offset = 0;
	fd_in = open(from, O_RDONLY);
	if (fd_in == -1) {
		perror(FIERR);
		return -1;
	}
	if (fstat(fd_in, &st) == -1) {
		perror(FSERR);
		return -1;
	}
	fd_out = open(to, O_CREAT | O_WRONLY | O_TRUNC, st.st_mode);
	if (fd_out == -1) {
		perror(FIERR);
		return -1;
	}
	if (sendfile(fd_out, fd_in, &offset, st.st_size) == -1) {
		perror(CPERR);
		return -1;
	}
	close(fd_in);
	close(fd_out);
	return 0;
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
	// clear all capabilities
	cap_t cap_p = cap_get_proc();
	if (cap_p == NULL) {
		errorExit(CAERR);
	}
	// showAllCapabilities();
	if (cap_clear(cap_p) == -1) {
		errorExit(CAERR);
	}
	if (cap_set_proc(cap_p) == -1) {
		errorExit(CAERR);
	}
	// showAllCapabilities();
	cap_free(cap_p);
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

void showAllCapabilities() {
	// for debug
	cap_t cap_p = cap_get_proc();
	if (cap_p == NULL) {
		errorExit(CAERR);
	}
	char *caps = cap_to_text(cap_p, NULL);
	log("Capabilities now: %s\n", caps);
	cap_free(caps); cap_free(cap_p);
}
