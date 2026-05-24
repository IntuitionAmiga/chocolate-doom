#ifndef IEDOOM_SDL_STDINC_H
#define IEDOOM_SDL_STDINC_H

#include <stddef.h>

void SDL_qsort(void *base, size_t nmemb, size_t size,
               int (*compar)(const void *, const void *));

#endif
