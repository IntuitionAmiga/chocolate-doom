#include <stdint.h>
#include <stddef.h>

#include "i_intuition.h"

void exit(int status);
void *malloc(size_t size);

const char *DEH_String(const char *s)
{
    return s;
}

void I_Error(const char *error, ...)
{
    (void) error;
    exit(1);
}

void *Z_Malloc(int size, int tag, void *user)
{
    (void) tag;
    (void) user;
    return malloc((size_t) size);
}

int W_CheckNumForName(const char *name)
{
    (void) name;
    return -1;
}

int W_GetNumForName(const char *name)
{
    (void) name;
    return -1;
}

void *W_CacheLumpNum(int lump, int tag)
{
    (void) lump;
    (void) tag;
    return NULL;
}

void W_ReleaseLumpNum(int lump)
{
    (void) lump;
}

int W_LumpLength(int lump)
{
    (void) lump;
    return 0;
}
