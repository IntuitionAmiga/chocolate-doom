#ifndef IEDOOM_SDL_ENDIAN_H
#define IEDOOM_SDL_ENDIAN_H

#include <stdint.h>

#define SDL_LIL_ENDIAN 1234
#define SDL_BIG_ENDIAN 4321
#if defined(IEDOOM_M68K)
#define SDL_BYTEORDER SDL_BIG_ENDIAN
#else
#define SDL_BYTEORDER SDL_LIL_ENDIAN
#endif

static inline uint16_t SDL_SwapLE16(uint16_t x)
{
#if defined(IEDOOM_M68K)
    return (uint16_t) ((x << 8) | (x >> 8));
#else
    return x;
#endif
}

static inline uint32_t SDL_SwapLE32(uint32_t x)
{
#if defined(IEDOOM_M68K)
    return ((x & 0x000000ffu) << 24)
         | ((x & 0x0000ff00u) << 8)
         | ((x & 0x00ff0000u) >> 8)
         | ((x & 0xff000000u) >> 24);
#else
    return x;
#endif
}

#endif
