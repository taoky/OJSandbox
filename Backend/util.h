#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sendfile.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <pwd.h>

#define log(...) fprintf(stderr, __VA_ARGS__)

extern const char *const RLERR;
extern const char *const GIERR;
extern const char *const UIERR;
extern const char *const RDERR;
extern const char *const FOERR;
extern const char *const SGERR;
extern const char *const EXERR;
extern const char *const TPERR;
extern const char *const FIERR;
extern const char *const FSERR;
extern const char *const CPERR;
extern const char *const USERR;
extern const char *const CGERR;
extern const char *const SCERR;

extern const char *const RES_OK;
extern const char *const RES_RE;
extern const char *const RES_TLE;
extern const char *const RES_FSE;
extern const char *const RES_MLE;

extern uid_t ojsUID;
extern gid_t ojsGID;

typedef void (*sighandler_t)(int);

bool isRootUser(void);
void errorExit(const char *str);
void copyFile(const char *from, const char *to);
void initUser(void);
void setNonPrivilegeUser(void);
char *pathCat(const char *path, const char *fileName);
int timevalms(const struct timeval *timev);
void setrlimStruct(rlim_t num, struct rlimit * st);
bool writeFileStr(const char *path, const char *value, bool isOverWrite);
// char *unstandard_basename(const char *str);
void default_signal(int signum, sighandler_t handler);

#endif
