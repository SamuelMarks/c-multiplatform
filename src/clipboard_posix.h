#ifndef CLIPBOARD_POSIX_H
#define CLIPBOARD_POSIX_H

/* clang-format off */
#include <stdio.h>
/* clang-format on */

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
extern FILE *popen(const char *command, const char *type);
extern int pclose(FILE *stream);
#endif

#endif
