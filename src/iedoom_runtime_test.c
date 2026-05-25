#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#undef assert
#define assert(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "assertion failed: %s\n", #expr); \
            abort(); \
        } \
    } while (0)

void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strdup(const char *s);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
int atoi(const char *s);
double atof(const char *s);
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
int snprintf(char *s, size_t n, const char *fmt, ...);
unsigned long long __udivdi3(unsigned long long n, unsigned long long d);
unsigned long long __umoddi3(unsigned long long n, unsigned long long d);
long long __divdi3(long long n, long long d);
int isspace(int c);
int isdigit(int c);
int isalpha(int c);
int isalnum(int c);
int isprint(int c);
int toupper(int c);
int tolower(int c);
int abs(int x);
double fabs(double x);
int rand(void);
void srand(unsigned int seed);
void abort(void);
int IE_FileReadAll(const char *name, void *buffer, unsigned int buffer_len,
                   unsigned int *result_len);

static unsigned int file_read_all_calls;

int IE_FileReadAll(const char *name, void *buffer, unsigned int buffer_len,
                   unsigned int *result_len)
{
    (void) name;
    (void) buffer;
    (void) buffer_len;
    ++file_read_all_calls;
    if (result_len != NULL)
    {
        *result_len = 0;
    }
    return 0;
}

int iedoom_main(void)
{
    return 0;
}

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
    assert(strncmp("e1m1", "e1m2", 3) == 0);
    assert(strncmp("e1m1", "e1m2", 4) < 0);
    assert(strcasecmp("Doom", "doom") == 0);
    assert(strcasecmp("Alpha", "beta") < 0);
    assert(strncasecmp("DoomWAD", "doomTXT", 4) == 0);
    assert(strncasecmp("DoomWAD", "doomTXT", 5) > 0);

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

    assert(strchr("doom.wad", '.') != NULL);
    assert(strchr("doom", '.') == NULL);
    assert(strrchr("a/b/c", '/') != NULL);
    assert(*strrchr("a/b/c", '/') == '/');
    assert(strstr("doom2.wad", "2.wad") != NULL);
    assert(strstr("doom2.wad", "heretic") == NULL);
    assert(strcmp(strdup("iwad"), "iwad") == 0);
}

static void test_stdlib_primitives(void)
{
    void *a;
    void *b;
    void *c;

    assert(atoi("0") == 0);
    assert(atoi("35") == 35);
    assert(atoi("-12") == -12);
    assert(atoi("7tic") == 7);
    assert(atoi(" 42") == 42);
    assert(atof("-12") == -12.0);
    assert(abs(-12) == 12);
    assert(abs(7) == 7);
    assert(fabs(-1.5) == 1.5);

    a = malloc(4);
    assert(a != NULL);
    b = realloc(a, 16);
    assert(b != NULL);
    c = malloc(4);
    assert(c == a);
    free(b);
    free(c);

    a = malloc(16);
    b = malloc(16);
    assert(a != NULL);
    assert(b != NULL);
    assert(a != b);
    memset(a, 0x5a, 16);
    free(a);
    free(NULL);

    a = malloc(24);
    assert(a != NULL);
    free(a);
    b = malloc(24);
    assert(b == a);

    a = malloc(12u * 1024u * 1024u);
    assert(a != NULL);
    free(a);
    b = malloc(12u * 1024u * 1024u);
    assert(b == a);

    a = malloc(40u * 1024u * 1024u);
    assert(a != NULL);
    b = malloc(64u * 1024u * 1024u);
    assert(b != NULL);
    free(a);
    free(b);

    a = calloc(4, 4);
    assert(a != NULL);
    assert(((unsigned char *) a)[0] == 0);
    assert(((unsigned char *) a)[15] == 0);

    a = malloc(4);
    assert(a != NULL);
    ((unsigned char *) a)[0] = 0xde;
    ((unsigned char *) a)[1] = 0xad;
    ((unsigned char *) a)[2] = 0xbe;
    ((unsigned char *) a)[3] = 0xef;
    b = realloc(a, 16);
    assert(b != NULL);
    assert(((unsigned char *) b)[0] == 0xde);
    assert(((unsigned char *) b)[1] == 0xad);
    assert(((unsigned char *) b)[2] == 0xbe);
    assert(((unsigned char *) b)[3] == 0xef);

    assert(__udivdi3(100, 5) == 20);
    assert(__udivdi3(10000000000ull, 1000) == 10000000ull);
    assert(__udivdi3(7, 2) == 3);
    assert(__umoddi3(7, 2) == 1);
    assert(__divdi3(-9, 2) == -4);
    srand(1);
    assert(rand() == 16838);
}

static void test_ctype_primitives(void)
{
    assert(isspace(' '));
    assert(isspace('\n'));
    assert(!isspace('x'));
    assert(isdigit('0'));
    assert(isdigit('9'));
    assert(!isdigit('x'));
    assert(isalpha('a'));
    assert(isalpha('Z'));
    assert(!isalpha('7'));
    assert(isalnum('a'));
    assert(isalnum('7'));
    assert(!isalnum('-'));
    assert(isprint(' '));
    assert(isprint('~'));
    assert(!isprint('\n'));
    assert(toupper('a') == 'A');
    assert(toupper('A') == 'A');
    assert(tolower('Z') == 'z');
    assert(tolower('z') == 'z');
}

static void test_format_primitives(void)
{
    char buf[32];

    assert(snprintf(buf, sizeof(buf), "ds%s", "pistol") == 8);
    assert(strcmp(buf, "dspistol") == 0);
    assert(snprintf(buf, sizeof(buf), "AMMNUM%d", 3) == 7);
    assert(strcmp(buf, "AMMNUM3") == 0);
    assert(snprintf(buf, sizeof(buf), "%s.lmp", "demo1") == 9);
    assert(strcmp(buf, "demo1.lmp") == 0);
    assert(snprintf(buf, sizeof(buf), "%08x", 0x2a) == 8);
    assert(strcmp(buf, "0000002a") == 0);
    assert(snprintf(buf, sizeof(buf), "STCFN%.3d", 33) == 8);
    assert(strcmp(buf, "STCFN033") == 0);
    assert(snprintf(buf, 5, "abcdef") == 6);
    assert(strcmp(buf, "abcd") == 0);
}

static void test_file_primitives(void)
{
    file_read_all_calls = 0;
    assert(fopen("default.cfg", "wb") == NULL);
    assert(fopen("savegame.dsg", "w") == NULL);
    assert(fopen("missing.cfg", "rb") == NULL);
    assert(fopen("missing-extra.cfg", "rb") == NULL);
    assert(file_read_all_calls == 2);
}

int main(void)
{
    test_memory_primitives();
    test_string_primitives();
    test_stdlib_primitives();
    test_ctype_primitives();
    test_format_primitives();
    test_file_primitives();
    puts("iedoom_runtime tests passed");
    return 0;
}
