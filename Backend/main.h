#ifndef MAIN_H
#define MAIN_H

#define _GNU_SOURCE // for basename()
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define log(...) fprintf(stderr, __VA_ARGS__)

#endif