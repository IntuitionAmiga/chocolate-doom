#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "i_intuition.h"
#include "i_sound.h"
#include "doom/deh_misc.h"
#include "sha1.h"

#define IE_TERM_OUT 0x000F0700u

void exit(int status);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

typedef void (*iedoom_atexit_func_t)(void);

typedef struct iedoom_atexit_entry_s iedoom_atexit_entry_t;

struct iedoom_atexit_entry_s
{
    iedoom_atexit_func_t func;
    int run_on_error;
    iedoom_atexit_entry_t *next;
};

static iedoom_atexit_entry_t *iedoom_atexit_list;

static void IE_TermWriteString(const char *s)
{
    while (s != NULL && *s != '\0')
    {
        *(volatile uint32_t *) (uintptr_t) IE_TERM_OUT = (uint8_t) *s;
        ++s;
    }
}

int use_analog;
int joystick_move_sensitivity = 10;
int joystick_turn_sensitivity = 10;
int vanilla_keyboard_mapping = 1;
int mouse_threshold = 10;
float mouse_acceleration = 2.0f;
char *snd_dmxoption = "";
int opl_io_port = 0x388;
char *music_pack_path = "";
char *timidity_cfg_path = "";
int use_libsamplerate;
float libsamplerate_scale = 0.65f;
int deh_initial_health = DEH_DEFAULT_INITIAL_HEALTH;
int deh_initial_bullets = DEH_DEFAULT_INITIAL_BULLETS;
int deh_max_health = DEH_DEFAULT_MAX_HEALTH;
int deh_max_armor = DEH_DEFAULT_MAX_ARMOR;
int deh_green_armor_class = DEH_DEFAULT_GREEN_ARMOR_CLASS;
int deh_blue_armor_class = DEH_DEFAULT_BLUE_ARMOR_CLASS;
int deh_max_soulsphere = DEH_DEFAULT_MAX_SOULSPHERE;
int deh_soulsphere_health = DEH_DEFAULT_SOULSPHERE_HEALTH;
int deh_megasphere_health = DEH_DEFAULT_MEGASPHERE_HEALTH;
int deh_god_mode_health = DEH_DEFAULT_GOD_MODE_HEALTH;
int deh_idfa_armor = DEH_DEFAULT_IDFA_ARMOR;
int deh_idfa_armor_class = DEH_DEFAULT_IDFA_ARMOR_CLASS;
int deh_idkfa_armor = DEH_DEFAULT_IDKFA_ARMOR;
int deh_idkfa_armor_class = DEH_DEFAULT_IDKFA_ARMOR_CLASS;
int deh_bfg_cells_per_shot = DEH_DEFAULT_BFG_CELLS_PER_SHOT;
int deh_species_infighting = DEH_DEFAULT_SPECIES_INFIGHTING;

static boolean IEDoom_NullMusicInit(void)
{
    return false;
}

const music_module_t music_pack_module =
{
    NULL,
    0,
    IEDoom_NullMusicInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

__attribute__((weak)) const char *DEH_String(const char *s)
{
    return s;
}

void DEH_printf(const char *fmt, ...)
{
    (void) fmt;
}

void DEH_fprintf(FILE *fstream, const char *fmt, ...)
{
    (void) fstream;
    (void) fmt;
}

void DEH_snprintf(char *buffer, size_t len, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, len, fmt, args);
    va_end(args);
}

void DEH_AddStringReplacement(const char *from_text, const char *to_text)
{
    (void) from_text;
    (void) to_text;
}

void DEH_ParseCommandLine(void)
{
}

int DEH_LoadFile(const char *filename)
{
    (void) filename;
    return 0;
}

void DEH_AutoLoadPatches(const char *path)
{
    (void) path;
}

int DEH_LoadLump(int lumpnum, int allow_long, int allow_error)
{
    (void) lumpnum;
    (void) allow_long;
    (void) allow_error;
    return 0;
}

int DEH_LoadLumpByName(const char *name, int allow_long, int allow_error)
{
    (void) name;
    (void) allow_long;
    (void) allow_error;
    return 0;
}

void DEH_Checksum(sha1_digest_t digest)
{
    memset(digest, 0, sizeof(sha1_digest_t));
}

__attribute__((weak)) void I_Error(const char *error, ...)
{
    char buf[512];
    va_list args;

    va_start(args, error);
    vsnprintf(buf, sizeof(buf), error, args);
    va_end(args);

    IE_TermWriteString("I_Error: ");
    IE_TermWriteString(buf);
    IE_TermWriteString("\n");
    exit(1);
}

__attribute__((weak)) void I_AtExit(void (*func)(void), int run_on_error)
{
    iedoom_atexit_entry_t *entry;

    if (func == NULL)
    {
        return;
    }

    entry = malloc(sizeof(*entry));
    if (entry == NULL)
    {
        I_Error("I_AtExit: failed to allocate exit hook");
    }

    entry->func = func;
    entry->run_on_error = run_on_error;
    entry->next = iedoom_atexit_list;
    iedoom_atexit_list = entry;
}

__attribute__((weak)) void I_Quit(void)
{
    iedoom_atexit_entry_t *entry;

    for (entry = iedoom_atexit_list; entry != NULL; entry = entry->next)
    {
        entry->func();
    }

    exit(0);
}

__attribute__((weak)) void I_PrintBanner(const char *msg)
{
    IE_TermWriteString(msg);
    IE_TermWriteString("\n");
}

__attribute__((weak)) void I_PrintDivider(void)
{
    IE_TermWriteString("===========================================================================\n");
}

__attribute__((weak)) void I_PrintStartupBanner(const char *gamedescription)
{
    I_PrintDivider();
    I_PrintBanner(gamedescription);
    I_PrintDivider();
}

__attribute__((weak)) int I_ConsoleStdout(void)
{
    return 0;
}

__attribute__((weak)) int I_CheckIsScreensaver(void)
{
    return 0;
}

unsigned int I_GetMemoryValue(unsigned int offset, int size)
{
    (void) offset;
    (void) size;
    return 0;
}

__attribute__((weak)) void *Z_Malloc(int size, int tag, void *user)
{
    (void) tag;
    (void) user;
    return malloc((size_t) size);
}

__attribute__((weak)) int W_CheckNumForName(const char *name)
{
    (void) name;
    return -1;
}

__attribute__((weak)) int W_GetNumForName(const char *name)
{
    (void) name;
    return -1;
}

__attribute__((weak)) void *W_CacheLumpNum(int lump, int tag)
{
    (void) lump;
    (void) tag;
    return NULL;
}

__attribute__((weak)) void W_ReleaseLumpNum(int lump)
{
    (void) lump;
}

__attribute__((weak)) int W_LumpLength(int lump)
{
    (void) lump;
    return 0;
}

void *I_Realloc(void *ptr, size_t size)
{
    void *result = realloc(ptr, size);

    if (result == NULL && size != 0)
    {
        I_Error("I_Realloc: failed on reallocation of %zu bytes", size);
    }

    return result;
}

__attribute__((weak)) void V_BeginRead(void)
{
}

void I_Endoom(const unsigned char *endoom_data)
{
    (void) endoom_data;
}

void I_BindInputVariables(void)
{
}

void I_BindJoystickVariables(void)
{
}

void I_InitJoystick(void)
{
}

void NET_WaitForLaunch(void)
{
}

void I_StartTextInput(int x1, int y1, int x2, int y2)
{
    (void) x1;
    (void) y1;
    (void) x2;
    (void) y2;
}

void I_StopTextInput(void)
{
}

void I_InitTimidityConfig(void)
{
}

void I_Tactile(int on, int off, int total)
{
    (void) on;
    (void) off;
    (void) total;
}

void I_OPL_DevMessages(char *result, size_t result_len)
{
    if (result_len > 0)
    {
        result[0] = '\0';
    }
}

void I_SetOPLDriverVer(opl_driver_ver_t version)
{
    (void) version;
}

__attribute__((weak)) void W_MergeFile(const char *filename)
{
    (void) filename;
}

__attribute__((weak)) void W_NWTDashMerge(const char *filename)
{
    (void) filename;
}

__attribute__((weak)) void W_NWTMergeFile(const char *filename, int flags)
{
    (void) filename;
    (void) flags;
}

typedef struct glob_s glob_t;

__attribute__((weak)) glob_t *I_StartMultiGlob(const char *directory, int flags,
                                               const char *glob, ...)
{
    (void) directory;
    (void) flags;
    (void) glob;
    return NULL;
}

__attribute__((weak)) const char *I_NextGlob(glob_t *glob)
{
    (void) glob;
    return NULL;
}

__attribute__((weak)) void I_EndGlob(glob_t *glob)
{
    (void) glob;
}

__attribute__((weak)) const char *D_GameMissionString(GameMission_t mission)
{
    (void) mission;
    return "doom";
}

unsigned char *I_ZoneBase(int *size)
{
    *size = 16 * 1024 * 1024;
    return malloc((size_t) *size);
}
