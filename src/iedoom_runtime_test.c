#include <assert.h>
#include <stddef.h>
#include <stdio.h>

void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

static void test_memory_primitives(void)
{
    unsigned char buf[8];
    unsigned char src[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    assert(memset(buf, 0xa5, sizeof(buf)) == buf);
    assert(buf[0] == 0xa5);
    assert(buf[7] == 0xa5);

    assert(memcpy(buf, src, sizeof(buf)) == buf);
    assert(memcmp(buf, src, sizeof(buf)) == 0);
    buf[3] = 9;
    assert(memcmp(buf, src, sizeof(buf)) > 0);
    buf[3] = 1;
    assert(memcmp(buf, src, sizeof(buf)) < 0);

    memcpy(buf, src, sizeof(buf));
    assert(memmove(buf + 1, buf, 7) == buf + 1);
    assert(buf[1] == 0);
    assert(buf[7] == 6);

    memcpy(buf, src, sizeof(buf));
    assert(memmove(buf, buf + 1, 7) == buf);
    assert(buf[0] == 1);
    assert(buf[6] == 7);
}

static void test_string_primitives(void)
{
    char buf[8];

    assert(strlen("") == 0);
    assert(strlen("doom") == 4);
    assert(strcmp("doom", "doom") == 0);
    assert(strcmp("e1m1", "e1m2") < 0);
    assert(strcmp("e1m2", "e1m1") > 0);

    assert(strcpy(buf, "mus") == buf);
    assert(strcmp(buf, "mus") == 0);

    memset(buf, 'x', sizeof(buf));
    assert(strncpy(buf, "wad", sizeof(buf)) == buf);
    assert(buf[0] == 'w');
    assert(buf[3] == '\0');
    assert(buf[7] == '\0');

    memset(buf, 'x', sizeof(buf));
    strncpy(buf, "toolong", 3);
    assert(buf[0] == 't');
    assert(buf[2] == 'o');
    assert(buf[3] == 'x');
}

int main(void)
{
    test_memory_primitives();
    test_string_primitives();
    puts("iedoom_runtime tests passed");
    return 0;
}
