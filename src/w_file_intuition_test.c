#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "w_file.h"

void abort(void);

#undef assert
#define assert(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #expr); \
        abort(); \
    } \
} while (0)

static unsigned int large_zone_allocs;
static unsigned int zone_allocs;
static uint32_t last_file_read_cap;
static unsigned char *last_file_read_buffer;

void *Z_Malloc(int size, int tag, void *user)
{
    (void) tag;
    (void) user;

    ++zone_allocs;
    if (size > 1024)
    {
        ++large_zone_allocs;
        return NULL;
    }

    return malloc((size_t) size);
}

void Z_Free(void *ptr)
{
    free(ptr);
}

char *M_StringDuplicate(const char *s)
{
    size_t len = strlen(s) + 1;
    char *result = malloc(len);

    assert(result != NULL);
    memcpy(result, s, len);
    return result;
}

int IE_FileReadAll(const char *name, void *buffer, uint32_t buffer_len,
                   uint32_t *result_len)
{
    static const unsigned char wad[] = { 'I', 'W', 'A', 'D' };

    assert(strcmp(name, "doom1.wad") == 0);
    assert(buffer_len >= sizeof(wad));
    last_file_read_cap = buffer_len;
    last_file_read_buffer = buffer;

    memcpy(buffer, wad, sizeof(wad));
    *result_len = sizeof(wad);
    return 1;
}

int main(void)
{
    wad_file_t *wad = intuition_wad_file.OpenFile("doom1.wad");
    unsigned char buf[4];

    assert(wad != NULL);
    assert(zone_allocs == 0);
    assert(large_zone_allocs == 0);
    assert(last_file_read_cap == 64u * 1024u * 1024u);
    assert(wad->length == 4);
    assert(last_file_read_buffer != NULL);
    assert(wad->mapped != last_file_read_buffer);
    assert(W_Read(wad, 0, buf, sizeof(buf)) == sizeof(buf));
    assert(memcmp(buf, "IWAD", 4) == 0);
    W_CloseFile(wad);
    return 0;
}
