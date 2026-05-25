#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m_argv.h"

int myargc;
char **myargv;

static int doom_main_called;
static int find_response_file_called;
static int set_exe_dir_called;

static void require(int condition)
{
    if (!condition)
    {
        exit(1);
    }
}

void D_DoomMain(void)
{
    ++doom_main_called;
}

void M_FindResponseFile(void)
{
    ++find_response_file_called;
}

void M_SetExeDir(void)
{
    ++set_exe_dir_called;
}

int iedoom_main(void);

int main(void)
{
    myargc = 99;
    myargv = NULL;

    require(iedoom_main() == 0);
    require(find_response_file_called == 1);
    require(set_exe_dir_called == 1);
    require(doom_main_called == 1);
    require(myargc == 8);
    require(myargv != NULL);
    require(strcmp(myargv[0], "iedoom") == 0);
    require(strcmp(myargv[1], "-iwad") == 0);
    require(strcmp(myargv[2], "doom1.wad") == 0);
    require(strcmp(myargv[3], "-skill") == 0);
    require(strcmp(myargv[4], "1") == 0);
    require(strcmp(myargv[5], "-warp") == 0);
    require(strcmp(myargv[6], "1") == 0);
    require(strcmp(myargv[7], "1") == 0);
    require(myargv[8] == NULL);

    puts("iedoom_main tests passed");
    return 0;
}
