#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sendfile.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <pwd.h>

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
#define USERR "(User) ojs's gid or uid error"
#define CGERR "cgroup error"
#define SCERR "seccomp error"

extern uid_t ojsUID;
extern gid_t ojsGID;

bool isRootUser(void);
void errorExit(char str[]);
void copyFile(char *from, char *to);
void initUser(void);
void setNonPrivilegeUser(void);
char *pathCat(char *path, char *fileName);
bool isPathLink(char *path);
bool isPathDir(char *path);
void bindMountHelper(char *from, char *to);
long long readFileLL(char *path);
int writeFileInt(char *path, int value, bool isOverWrite);
bool clearFile(char *path);
int timevalms(const struct timeval *timev);
void setrlimStruct(rlim_t num, struct rlimit * st);
bool writeFileStr(char *path, char *value, bool isOverWrite);

#endif