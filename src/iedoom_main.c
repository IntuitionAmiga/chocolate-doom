// Intuition Engine freestanding entry point for the Doom target.

#include <stddef.h>

void D_DoomMain(void);
extern int myargc;
extern char **myargv;

static char iedoom_arg0[] = "iedoom";
static char *iedoom_argv[] =
{
    iedoom_arg0,
    NULL,
};

int iedoom_main(void)
{
    myargc = 1;
    myargv = iedoom_argv;

    D_DoomMain();
    return 0;
}
