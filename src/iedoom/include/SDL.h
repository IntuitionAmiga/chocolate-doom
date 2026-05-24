#ifndef IEDOOM_SDL_H
#define IEDOOM_SDL_H

#include "SDL_stdinc.h"

typedef struct SDL_Event
{
    int type;
} SDL_Event;

#define SDL_HINT_NO_SIGNAL_HANDLERS "SDL_NO_SIGNAL_HANDLERS"

static inline int SDL_SetHint(const char *name, const char *value)
{
    (void) name;
    (void) value;
    return 1;
}

#endif
