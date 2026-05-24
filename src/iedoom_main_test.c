#include <assert.h>
#include <stdio.h>

#include "m_argv.h"

int myargc;
char **myargv;

static int doom_main_called;

void D_DoomMain(void)
{
    ++doom_main_called;
}

int iedoom_main(void);

int main(void)
{
    myargc = 99;
    myargv = NULL;

    assert(iedoom_main() == 0);
    assert(doom_main_called == 1);
    assert(myargc == 3);
    assert(myargv != NULL);
    assert(myargv[0] != NULL);
    assert(myargv[1] != NULL);
    assert(myargv[2] != NULL);
    assert(myargv[3] == NULL);

    puts("iedoom_main tests passed");
    return 0;
}
