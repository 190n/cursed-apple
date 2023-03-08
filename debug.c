#include "debug.h"

// for gettid
#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

// pretty colors
unsigned int counter = 0;
int colors[] = { 3, 7, 5, 0 };

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

void __debug(const char *file, unsigned long long line, int level, const char *fmt, ...) {
	if (DEBUG_LEVEL >= level) {
		va_list arg;
		va_start(arg, fmt);
		fprintf(stderr, "\033[9%dm[thread %d %s:%llu]\033[0m ", colors[(counter++) % 4], gettid(),
		    file, line);
		vfprintf(stderr, fmt, arg);
		fprintf(stderr, "\n");
		va_end(arg);
	}
}
