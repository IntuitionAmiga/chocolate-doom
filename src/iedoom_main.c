// Intuition Engine freestanding entry point for the Doom target.

#include <stddef.h>

void D_DoomMain(void);
void M_FindResponseFile(void);
void M_SetExeDir(void);
extern int myargc;
extern char **myargv;

static char iedoom_arg0[] = "iedoom";
static char iedoom_arg_iwad[] = "-iwad";
static char iedoom_arg_iwad_path[] = "doom1.wad";
static char *iedoom_argv[] =
{
    iedoom_arg0,
    iedoom_arg_iwad,
    iedoom_arg_iwad_path,
    NULL,
};

int iedoom_main(void)
{
    myargc = 3;
    myargv = iedoom_argv;

    M_FindResponseFile();
    M_SetExeDir();
    D_DoomMain();
    return 0;
}
