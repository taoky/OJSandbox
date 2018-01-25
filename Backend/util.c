#include "util.h"

uid_t nobodyUID;
gid_t nobodyGID;

bool isRootUser(void) {
	return !(geteuid() || getegid());
}

void errorExit(char str[]) {
	perror(str);
	exit(-1);
}

void copyFile(char *from, char *to) {
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

void initNobody(void) {
    struct passwd *nobody = getpwnam("nobody");
	nobodyUID = nobody->pw_uid;
	nobodyGID = nobody->pw_gid;
	if (!nobodyUID || !nobodyGID) {
		errorExit(NBERR);
	}
}

void setNonPrivilegeUser(void) {
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

char *pathCat(char *path, char *fileName) {
	/*
	 * dynamic alloc memory
     * free when necessary
	 */
	int plen = strlen(path), flen = strlen(fileName);
	char *res = malloc(plen + flen);
	strcpy(res, path);
	res[plen] = '/';
	strcpy(res + plen + 1, fileName);
	return res;
}

bool isPathLink(char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        printf("%s\n", path);
        errorExit(FSERR);
    }
    if ((st.st_mode & S_IFMT) == S_IFLNK) return true;
    else return false;
}

bool isPathDir(char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        printf("%s\n", path);
        errorExit(FSERR);
    }
    if ((st.st_mode & S_IFMT) == S_IFDIR) return true;
    else return false;
}

void bindMountHelper(char *from, char *to) {
    int res;
    if (isPathLink(from)) {
        res = symlink(from, to);
        printf("%s %s symlink %d\n", from, to, res);
    }
    else if (isPathDir(from)) {
        mount(from, to, "", MS_BIND | MS_NOSUID, NULL);
        mount(from, to, "", MS_BIND | MS_NOSUID | MS_REMOUNT | MS_RDONLY, NULL);
        printf("%s %s mount %d\n", from, to, res);
    }
    else {
        printf("%s %s WTF?\n", from, to);
    }
}