#ifndef STRTOK_POSIX_H
#define STRTOK_POSIX_H

#if !defined(_MSC_VER)
extern char *strtok_r(char *str, const char *delim, char **saveptr);
#endif

#endif
