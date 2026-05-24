// Minimal freestanding runtime entry for Intuition Engine link smoke tests.

#include <stddef.h>
#include <stdarg.h>

typedef struct FILE FILE;

struct FILE
{
    int unused;
};

#define IE_TERM_OUT 0x000F0700u

volatile unsigned int iedoom_bss_probe;
int errno;
FILE *stdin;
FILE *stdout;
FILE *stderr;

int iedoom_main(void);
void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int vprintf(const char *fmt, va_list args);
int vfprintf(FILE *stream, const char *fmt, va_list args);
int vsnprintf(char *s, size_t n, const char *fmt, va_list args);

static size_t iedoom_heap_used;
static unsigned int iedoom_rand_state = 1;

#ifdef IEDOOM_GUEST
#define IEDOOM_HEAP_BASE ((unsigned char *) 0x02000000u)
#define IEDOOM_HEAP_SIZE (96u * 1024u * 1024u)
#else
static unsigned char iedoom_heap[32u * 1024u * 1024u];
#define IEDOOM_HEAP_BASE iedoom_heap
#define IEDOOM_HEAP_SIZE sizeof(iedoom_heap)
#endif

typedef union
{
    size_t size;
    long double align;
} iedoom_heap_header_t;

static void iedoom_term_write_char(char c)
{
#ifdef IEDOOM_GUEST
    *(volatile unsigned int *) IE_TERM_OUT = (unsigned char) c;
#else
    (void) c;
#endif
}

static void iedoom_term_write_string(const char *s)
{
    while (s != NULL && *s != '\0')
    {
        iedoom_term_write_char(*s++);
    }
}

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

int isprint(int c)
{
    return c >= 32 && c < 127;
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

char *setlocale(int category, const char *locale)
{
    (void) category;
    (void) locale;
    return NULL;
}

struct iedoom_lconv
{
    char *decimal_point;
};

struct iedoom_lconv *localeconv(void)
{
    static struct iedoom_lconv lc = { "." };

    return &lc;
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
    iedoom_heap_header_t *header;
    size_t total;

    size = (size + 7u) & ~7u;
    if (size == 0)
    {
        size = 8;
    }

    total = size + sizeof(*header);
    if (total < size || total > IEDOOM_HEAP_SIZE - iedoom_heap_used)
    {
        return NULL;
    }

    header = (iedoom_heap_header_t *) (IEDOOM_HEAP_BASE + iedoom_heap_used);
    header->size = size;
    iedoom_heap_used += total;
    return header + 1;
}

void *calloc(size_t nmemb, size_t size)
{
    size_t total = nmemb * size;
    void *result = malloc(total);

    if (result != NULL)
    {
        memset(result, 0, total);
    }

    return result;
}

void free(void *ptr)
{
    (void) ptr;
}

void *realloc(void *ptr, size_t size)
{
    iedoom_heap_header_t *header;
    void *result;
    size_t copy_size;

    if (ptr == NULL)
    {
        return malloc(size);
    }

    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    header = ((iedoom_heap_header_t *) ptr) - 1;
    copy_size = header->size < size ? header->size : size;
    result = malloc(size);

    if (result != NULL)
    {
        memcpy(result, ptr, copy_size);
    }

    return result;
}

char *SDL_GetPrefPath(const char *org, const char *app)
{
    (void) org;
    (void) app;
    return NULL;
}

void SDL_free(void *mem)
{
    free(mem);
}

int printf(const char *fmt, ...)
{
    va_list args;
    int result;

    va_start(args, fmt);
    result = vprintf(fmt, args);
    va_end(args);
    return result;
}

int fprintf(FILE *stream, const char *fmt, ...)
{
    va_list args;
    int result;

    va_start(args, fmt);
    result = vfprintf(stream, fmt, args);
    va_end(args);
    return result;
}

int vprintf(const char *fmt, va_list args)
{
    char buf[512];
    int result = vsnprintf(buf, sizeof(buf), fmt, args);

    iedoom_term_write_string(buf);
    return result;
}

int vfprintf(FILE *stream, const char *fmt, va_list args)
{
    (void) stream;
    return vprintf(fmt, args);
}

static void iedoom_format_putc(char *s, size_t n, size_t *pos,
                               size_t *written, char c)
{
    if (*pos + 1 < n)
    {
        s[*pos] = c;
        ++*pos;
    }
    ++*written;
}

static void iedoom_format_write(char *s, size_t n, size_t *pos,
                                size_t *written, const char *text)
{
    if (text == NULL)
    {
        text = "(null)";
    }

    while (*text != '\0')
    {
        iedoom_format_putc(s, n, pos, written, *text);
        ++text;
    }
}

static void iedoom_format_uint(char *s, size_t n, size_t *pos,
                               size_t *written, unsigned long long value,
                               unsigned int base, int width, char pad,
                               int uppercase)
{
    char tmp[32];
    int len = 0;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    do
    {
        tmp[len++] = digits[value % base];
        value /= base;
    } while (value != 0 && len < (int) sizeof(tmp));

    while (width > len)
    {
        iedoom_format_putc(s, n, pos, written, pad);
        --width;
    }

    while (len > 0)
    {
        iedoom_format_putc(s, n, pos, written, tmp[--len]);
    }
}

int vsnprintf(char *s, size_t n, const char *fmt, va_list args)
{
    size_t pos = 0;
    size_t written = 0;

    if (fmt == NULL)
    {
        fmt = "";
    }

    while (*fmt != '\0')
    {
        int width = 0;
        char pad = ' ';
        int long_count = 0;

        if (*fmt != '%')
        {
            iedoom_format_putc(s, n, &pos, &written, *fmt++);
            continue;
        }

        ++fmt;
        if (*fmt == '%')
        {
            iedoom_format_putc(s, n, &pos, &written, *fmt++);
            continue;
        }

        if (*fmt == '0')
        {
            pad = '0';
            ++fmt;
        }
        while (*fmt >= '0' && *fmt <= '9')
        {
            width = width * 10 + (*fmt - '0');
            ++fmt;
        }
        while (*fmt == 'l')
        {
            ++long_count;
            ++fmt;
        }

        switch (*fmt)
        {
            case 's':
                iedoom_format_write(s, n, &pos, &written,
                                    va_arg(args, const char *));
                break;
            case 'c':
                iedoom_format_putc(s, n, &pos, &written,
                                   (char) va_arg(args, int));
                break;
            case 'd':
            case 'i':
            {
                long long value;

                if (long_count >= 2)
                {
                    value = va_arg(args, long long);
                }
                else if (long_count == 1)
                {
                    value = va_arg(args, long);
                }
                else
                {
                    value = va_arg(args, int);
                }

                if (value < 0)
                {
                    iedoom_format_putc(s, n, &pos, &written, '-');
                    value = -value;
                }
                iedoom_format_uint(s, n, &pos, &written,
                                   (unsigned long long) value, 10, width,
                                   pad, 0);
                break;
            }
            case 'u':
            case 'x':
            case 'X':
            {
                unsigned long long value;

                if (long_count >= 2)
                {
                    value = va_arg(args, unsigned long long);
                }
                else if (long_count == 1)
                {
                    value = va_arg(args, unsigned long);
                }
                else
                {
                    value = va_arg(args, unsigned int);
                }

                iedoom_format_uint(s, n, &pos, &written, value,
                                   *fmt == 'u' ? 10u : 16u, width, pad,
                                   *fmt == 'X');
                break;
            }
            case 'p':
                iedoom_format_write(s, n, &pos, &written, "0x");
                iedoom_format_uint(s, n, &pos, &written,
                                   (unsigned long long) (size_t)
                                   va_arg(args, void *),
                                   16, width, '0', 0);
                break;
            default:
                iedoom_format_putc(s, n, &pos, &written, '%');
                if (*fmt != '\0')
                {
                    iedoom_format_putc(s, n, &pos, &written, *fmt);
                }
                break;
        }

        if (*fmt != '\0')
        {
            ++fmt;
        }
    }

    if (n > 0)
    {
        s[pos < n ? pos : n - 1] = '\0';
    }

    return (int) written;
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

int fscanf(FILE *stream, const char *fmt, ...)
{
    (void) stream;
    (void) fmt;
    return -1;
}

int puts(const char *s)
{
    iedoom_term_write_string(s);
    iedoom_term_write_char('\n');
    return 0;
}

int putchar(int c)
{
    iedoom_term_write_char((char) c);
    return c;
}

int isatty(int fd)
{
    (void) fd;
    return 0;
}

int fileno(FILE *stream)
{
    (void) stream;
    return -1;
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

int feof(FILE *stream)
{
    (void) stream;
    return 1;
}

int fgetc(FILE *stream)
{
    (void) stream;
    return -1;
}

int ungetc(int c, FILE *stream)
{
    (void) c;
    (void) stream;
    return -1;
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

unsigned long long __umoddi3(unsigned long long n, unsigned long long d)
{
    if (d == 0)
    {
        return 0;
    }

    return n - __udivdi3(n, d) * d;
}

long long __divdi3(long long n, long long d)
{
    int negative = 0;
    unsigned long long un;
    unsigned long long ud;
    unsigned long long result;

    if (n < 0)
    {
        negative = !negative;
        un = (unsigned long long) -n;
    }
    else
    {
        un = (unsigned long long) n;
    }

    if (d < 0)
    {
        negative = !negative;
        ud = (unsigned long long) -d;
    }
    else
    {
        ud = (unsigned long long) d;
    }

    result = __udivdi3(un, ud);
    return negative ? -(long long) result : (long long) result;
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

double atof(const char *s)
{
    return (double) atoi(s);
}

double fabs(double x)
{
    return x < 0.0 ? -x : x;
}

int abs(int x)
{
    return x < 0 ? -x : x;
}

void srand(unsigned int seed)
{
    iedoom_rand_state = seed == 0 ? 1 : seed;
}

int rand(void)
{
    iedoom_rand_state = iedoom_rand_state * 1103515245u + 12345u;
    return (int) ((iedoom_rand_state >> 16) & 0x7fff);
}

long time(long *tloc)
{
    if (tloc != NULL)
    {
        *tloc = 0;
    }

    return 0;
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
