#ifndef IEDOOM_SDL_ENDIAN_H
#define IEDOOM_SDL_ENDIAN_H

#include <stdint.h>

#define SDL_LIL_ENDIAN 1234
#define SDL_BIG_ENDIAN 4321
#define SDL_BYTEORDER SDL_LIL_ENDIAN

static inline uint16_t SDL_SwapLE16(uint16_t x)
{
    return x;
}

static inline uint32_t SDL_SwapLE32(uint32_t x)
{
    return x;
}

#endif
