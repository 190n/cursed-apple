#ifndef __DEBUG_H__
#define __DEBUG_H__
// #define DO_DEBUG

// adopted from code shared by Miles Glapa-Grossklag

/**
 * void DEBUG(int level, const char *fmt, ...)
 * Prints debugging information to stderr, in the format:
 * [thread <ID> file.c:line_number] message
 *
 * level:               only print if DEBUG_LEVEL is defined and >= level
 * fmt:                 printf(3)-style format string
 * remaining arguments: zero or more values to include in the output
 */
#define DEBUG(...) __debug(__FILE__, __LINE__, __VA_ARGS__)

void __debug(const char *file, unsigned long long line, int level, const char *fmt, ...);

#endif
