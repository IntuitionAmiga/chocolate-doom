// Minimal freestanding runtime entry for Intuition Engine link smoke tests.

#include <stddef.h>

volatile unsigned int iedoom_bss_probe;

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

__attribute__((naked, noreturn, section(".text.entry"), used))
void iedoom_entry(void)
{
    __asm__ volatile (
        "cli\n"
        "1:\n"
        "hlt\n"
        "jmp 1b\n"
    );
}
