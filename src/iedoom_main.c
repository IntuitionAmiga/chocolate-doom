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
static char iedoom_arg_skill[] = "-skill";
static char iedoom_arg_skill_level[] = "1";
static char iedoom_arg_warp[] = "-warp";
static char iedoom_arg_episode[] = "1";
static char iedoom_arg_map[] = "1";
static char *iedoom_argv[] =
{
    iedoom_arg0,
    iedoom_arg_iwad,
    iedoom_arg_iwad_path,
    iedoom_arg_skill,
    iedoom_arg_skill_level,
    iedoom_arg_warp,
    iedoom_arg_episode,
    iedoom_arg_map,
    NULL,
};

int iedoom_main(void)
{
    myargc = 8;
    myargv = iedoom_argv;

    M_FindResponseFile();
    M_SetExeDir();
    D_DoomMain();
    return 0;
}
