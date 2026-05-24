#ifndef IEDOOM_ASSERT_H
#define IEDOOM_ASSERT_H

#ifndef NDEBUG
#define assert(expr) ((void) ((expr) ? 0 : 0))
#else
#define assert(expr) ((void) 0)
#endif

#endif
