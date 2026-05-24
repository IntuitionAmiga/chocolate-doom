#ifndef IEDOOM_STDLIB_H
#define IEDOOM_STDLIB_H

#include <stddef.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void exit(int status) __attribute__((noreturn));
int atoi(const char *s);
double atof(const char *s);
int abs(int x);
int rand(void);
void srand(unsigned int seed);
char *getenv(const char *name);

#endif
