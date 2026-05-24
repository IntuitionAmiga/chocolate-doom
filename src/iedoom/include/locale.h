#ifndef IEDOOM_LOCALE_H
#define IEDOOM_LOCALE_H

#define LC_ALL 0

struct lconv
{
    char *decimal_point;
};

char *setlocale(int category, const char *locale);
struct lconv *localeconv(void);

#endif
