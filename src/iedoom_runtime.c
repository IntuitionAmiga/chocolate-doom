// Minimal freestanding runtime entry for Intuition Engine link smoke tests.

#include <stddef.h>
#include <stdarg.h>

typedef struct FILE FILE;

struct FILE
{
    int unused;
};

volatile unsigned int iedoom_bss_probe;
int errno;
FILE *stdin;
FILE *stdout;
FILE *stderr;

int iedoom_main(void);

static unsigned char iedoom_heap[32u * 1024u * 1024u];
static size_t iedoom_heap_used;

static int iedoom_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + ('a' - 'A');
    }

    return c;
}

int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r'
        || c == '\f' || c == '\v';
}

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

int isalpha(int c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}

int toupper(int c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - ('a' - 'A');
    }

    return c;
}

int tolower(int c)
{
    return iedoom_tolower(c);
}

void exit(int status)
{
    (void) status;

    __asm__ volatile (
        "cli\n"
        "1:\n"
        "hlt\n"
        "jmp 1b\n"
    );
    __builtin_unreachable();
}

void *malloc(size_t size)
{
    void *result;

    size = (size + 7u) & ~7u;
    if (size == 0)
    {
        size = 8;
    }

    if (size > sizeof(iedoom_heap) - iedoom_heap_used)
    {
        return NULL;
    }

    result = iedoom_heap + iedoom_heap_used;
    iedoom_heap_used += size;
    return result;
}

void free(void *ptr)
{
    (void) ptr;
}

int printf(const char *fmt, ...)
{
    (void) fmt;
    return 0;
}

int fprintf(FILE *stream, const char *fmt, ...)
{
    (void) stream;
    (void) fmt;
    return 0;
}

int vprintf(const char *fmt, va_list args)
{
    (void) fmt;
    (void) args;
    return 0;
}

int vfprintf(FILE *stream, const char *fmt, va_list args)
{
    (void) stream;
    (void) fmt;
    (void) args;
    return 0;
}

int vsnprintf(char *s, size_t n, const char *fmt, va_list args)
{
    (void) fmt;
    (void) args;

    if (n > 0)
    {
        s[0] = '\0';
    }

    return 0;
}

int snprintf(char *s, size_t n, const char *fmt, ...)
{
    va_list args;
    int result;

    va_start(args, fmt);
    result = vsnprintf(s, n, fmt, args);
    va_end(args);
    return result;
}

int sscanf(const char *s, const char *fmt, ...)
{
    (void) s;
    (void) fmt;
    return 0;
}

int puts(const char *s)
{
    (void) s;
    return 0;
}

int putchar(int c)
{
    return c;
}

FILE *fopen(const char *path, const char *mode)
{
    (void) path;
    (void) mode;
    return NULL;
}

int fclose(FILE *stream)
{
    (void) stream;
    return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    (void) ptr;
    (void) size;
    (void) nmemb;
    (void) stream;
    return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    (void) ptr;
    (void) stream;
    return nmemb == 0 ? 0 : size * nmemb;
}

int fseek(FILE *stream, long offset, int whence)
{
    (void) stream;
    (void) offset;
    (void) whence;
    return 0;
}

long ftell(FILE *stream)
{
    (void) stream;
    return 0;
}

int remove(const char *path)
{
    (void) path;
    return -1;
}

int rename(const char *oldname, const char *newname)
{
    (void) oldname;
    (void) newname;
    return -1;
}

char *getenv(const char *name)
{
    (void) name;
    return NULL;
}

int stat(const char *path, void *buf)
{
    (void) path;
    (void) buf;
    return -1;
}

int mkdir(const char *path, unsigned int mode)
{
    (void) path;
    (void) mode;
    return -1;
}

void SDL_qsort(void *base, size_t nmemb, size_t size,
               int (*compar)(const void *, const void *))
{
    (void) base;
    (void) nmemb;
    (void) size;
    (void) compar;
}

unsigned long long __udivdi3(unsigned long long n, unsigned long long d)
{
    unsigned long long q = 0;
    unsigned long long bit = 1;

    if (d == 0)
    {
        return 0;
    }

    while ((d & (1ull << 63)) == 0 && d < n)
    {
        d <<= 1;
        bit <<= 1;
    }

    while (bit != 0)
    {
        if (n >= d)
        {
            n -= d;
            q |= bit;
        }
        d >>= 1;
        bit >>= 1;
    }

    return q;
}

int atoi(const char *s)
{
    int sign = 1;
    int result = 0;

    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r'
        || *s == '\f' || *s == '\v')
    {
        ++s;
    }

    if (*s == '-')
    {
        sign = -1;
        ++s;
    }
    else if (*s == '+')
    {
        ++s;
    }

    while (*s >= '0' && *s <= '9')
    {
        result = result * 10 + (*s - '0');
        ++s;
    }

    return sign * result;
}

void *memset(void *dest, int c, size_t n)
{
    unsigned char *d = dest;

    while (n-- > 0)
    {
        *d++ = (unsigned char) c;
    }

    return dest;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    while (n-- > 0)
    {
        *d++ = *s++;
    }

    return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d == s || n == 0)
    {
        return dest;
    }

    if (d < s)
    {
        return memcpy(dest, src, n);
    }

    d += n;
    s += n;
    while (n-- > 0)
    {
        *--d = *--s;
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *a = s1;
    const unsigned char *b = s2;

    while (n-- > 0)
    {
        if (*a != *b)
        {
            return (int) *a - (int) *b;
        }
        ++a;
        ++b;
    }

    return 0;
}

size_t strlen(const char *s)
{
    const char *p = s;

    while (*p != '\0')
    {
        ++p;
    }

    return (size_t) (p - s);
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 != '\0' && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }

    return (unsigned char) *s1 - (unsigned char) *s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
    {
        return 0;
    }

    while (n-- > 0)
    {
        if (*s1 != *s2 || *s1 == '\0')
        {
            return (unsigned char) *s1 - (unsigned char) *s2;
        }
        ++s1;
        ++s2;
    }

    return 0;
}

int strcasecmp(const char *s1, const char *s2)
{
    int c1;
    int c2;

    do
    {
        c1 = iedoom_tolower((unsigned char) *s1++);
        c2 = iedoom_tolower((unsigned char) *s2++);
    } while (c1 != '\0' && c1 == c2);

    return c1 - c2;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    int c1;
    int c2;

    if (n == 0)
    {
        return 0;
    }

    do
    {
        c1 = iedoom_tolower((unsigned char) *s1++);
        c2 = iedoom_tolower((unsigned char) *s2++);
    } while (--n > 0 && c1 != '\0' && c1 == c2);

    return c1 - c2;
}

char *strcpy(char *dest, const char *src)
{
    char *d = dest;

    do
    {
        *d++ = *src;
    } while (*src++ != '\0');

    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *d = dest;

    while (n > 0 && *src != '\0')
    {
        *d++ = *src++;
        --n;
    }

    while (n-- > 0)
    {
        *d++ = '\0';
    }

    return dest;
}

char *strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *result = malloc(len);

    if (result == NULL)
    {
        return NULL;
    }

    memcpy(result, s, len);
    return result;
}

char *strchr(const char *s, int c)
{
    while (*s != '\0')
    {
        if (*s == (char) c)
        {
            return (char *) s;
        }
        ++s;
    }

    return c == '\0' ? (char *) s : NULL;
}

char *strrchr(const char *s, int c)
{
    const char *result = NULL;

    do
    {
        if (*s == (char) c)
        {
            result = s;
        }
    } while (*s++ != '\0');

    return (char *) result;
}

char *strstr(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);

    if (needle_len == 0)
    {
        return (char *) haystack;
    }

    while (*haystack != '\0')
    {
        if (strncmp(haystack, needle, needle_len) == 0)
        {
            return (char *) haystack;
        }
        ++haystack;
    }

    return NULL;
}

__attribute__((naked, noreturn, section(".text.entry"), used))
void iedoom_entry(void)
{
    __asm__ volatile (
        "call iedoom_main\n"
        "cli\n"
        "1:\n"
        "hlt\n"
        "jmp 1b\n"
    );
}
