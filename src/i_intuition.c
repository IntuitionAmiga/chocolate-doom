// Intuition Engine MMIO backend helpers.

#include "i_intuition.h"

#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#ifndef INTUITION_ENGINE_TEST
#include <stdio.h>

#include "deh_str.h"
#include "i_sound.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"
#endif

#define DOOMTICRATE 35

#ifdef INTUITION_ENGINE_TEST
static uint32_t (*test_read32)(uint32_t addr);
static void (*test_write32)(uint32_t addr, uint32_t value);

void IE_TestSetMMIO(uint32_t (*read32)(uint32_t addr),
                    void (*write32)(uint32_t addr, uint32_t value))
{
    test_read32 = read32;
    test_write32 = write32;
}
#endif

int ie_music_mode = IE_MUSIC_MODE_ORIGINAL_MUS;

uint32_t IE_MMIO_Read32(uint32_t addr)
{
#ifdef INTUITION_ENGINE_TEST
    if (test_read32 != NULL)
    {
        return test_read32(addr);
    }
#endif
    return *(volatile uint32_t *) (uintptr_t) addr;
}

void IE_MMIO_Write32(uint32_t addr, uint32_t value)
{
#ifdef INTUITION_ENGINE_TEST
    if (test_write32 != NULL)
    {
        test_write32(addr, value);
        return;
    }
#endif
    *(volatile uint32_t *) (uintptr_t) addr = value;
}

uint64_t IE_ReadMonotonicUsec(void)
{
    uint32_t hi1, lo, hi2;

    do
    {
        hi1 = IE_MMIO_Read32(IE_RTC_MONO_USEC_HI);
        lo = IE_MMIO_Read32(IE_RTC_MONO_USEC_LO);
        hi2 = IE_MMIO_Read32(IE_RTC_MONO_USEC_HI);
    } while (hi1 != hi2);

    return ((uint64_t) hi2 << 32) | lo;
}

int IE_TimeUsecToTics(uint64_t usec)
{
    return (int) ((usec * DOOMTICRATE) / 1000000u);
}

void IE_VideoInit(uint32_t framebuffer_addr)
{
    IE_MMIO_Write32(IE_VIDEO_MODE, IE_MODE_320X200);
    IE_MMIO_Write32(IE_VIDEO_COLOR_MODE, IE_VIDEO_CLUT8);
    IE_MMIO_Write32(IE_VIDEO_FB_BASE, framebuffer_addr);
    IE_MMIO_Write32(IE_VIDEO_CTRL, 1);
}

uint32_t IE_PackPaletteEntry(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t) r << 16) | ((uint32_t) g << 8) | b;
}

void IE_VideoSetPalette(const uint8_t *doompalette)
{
    int i;

    for (i = 0; i < 256; ++i)
    {
        const uint8_t *entry = doompalette + i * 3;
        IE_MMIO_Write32(IE_VIDEO_PAL_TABLE + (uint32_t) i * 4,
                        IE_PackPaletteEntry(entry[0], entry[1], entry[2]));
    }
}

void IE_InputInit(void)
{
    IE_MMIO_Write32(IE_MOUSE_CTRL, 1);
}

int IE_TranslateScancode(uint8_t scancode, int *pressed)
{
    static const int pc_at_scancode_to_key[128] =
    {
        0, KEY_ESCAPE, '1', '2', '3', '4', '5', '6',
        '7', '8', '9', '0', KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE, KEY_TAB,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
        'o', 'p', '[', ']', KEY_ENTER, KEY_RCTRL, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
        '\'', '`', KEY_RSHIFT, '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', KEY_RSHIFT, KEYP_MULTIPLY,
        KEY_RALT, ' ', KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
        KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUMLOCK, KEY_SCRLCK,
        KEY_HOME,
        KEY_UPARROW, KEY_PGUP, KEYP_MINUS, KEY_LEFTARROW, KEYP_5,
        KEY_RIGHTARROW, KEYP_PLUS, KEY_END,
        KEY_DOWNARROW, KEY_PGDN, KEY_INS, KEY_DEL, 0, 0, KEY_NONUSBACKSLASH,
        KEY_F11, KEY_F12,
    };
    uint8_t make_code = scancode & 0x7f;

    if (pressed != NULL)
    {
        *pressed = (scancode & 0x80) == 0;
    }

    if (make_code >= sizeof(pc_at_scancode_to_key)
     / sizeof(pc_at_scancode_to_key[0]))
    {
        return 0;
    }

    return pc_at_scancode_to_key[make_code];
}

void IE_MusicSetVolume(int volume)
{
    if (ie_music_mode == IE_MUSIC_MODE_NONE)
    {
        return;
    }

    if (volume < 0)
    {
        volume = 0;
    }
    else if (volume > 127)
    {
        volume = 127;
    }

    IE_MMIO_Write32(IE_MIDI_VOLUME, (uint32_t) ((volume * 255) / 127));
}

void IE_MusicStart(uint32_t data_addr, uint32_t len, int looping)
{
    if (ie_music_mode == IE_MUSIC_MODE_NONE)
    {
        return;
    }

    IE_MMIO_Write32(IE_MIDI_PLAY_PTR, data_addr);
    IE_MMIO_Write32(IE_MIDI_PLAY_LEN, len);
    IE_MMIO_Write32(IE_MIDI_PLAY_CTRL,
                    IE_MIDI_CTRL_START | (looping ? IE_MIDI_CTRL_LOOP : 0));
}

void IE_MusicStop(void)
{
    if (ie_music_mode == IE_MUSIC_MODE_NONE)
    {
        return;
    }

    IE_MMIO_Write32(IE_MIDI_PLAY_CTRL, IE_MIDI_CTRL_STOP);
}

void IE_MusicPause(void)
{
    if (ie_music_mode == IE_MUSIC_MODE_NONE)
    {
        return;
    }

    IE_MMIO_Write32(IE_MIDI_PLAY_CTRL, IE_MIDI_CTRL_PAUSE);
}

void IE_MusicResume(void)
{
    if (ie_music_mode == IE_MUSIC_MODE_NONE)
    {
        return;
    }

    IE_MMIO_Write32(IE_MIDI_PLAY_CTRL, 0);
}

int IE_MusicLoadFailed(void)
{
    uint32_t status;

    if (ie_music_mode == IE_MUSIC_MODE_NONE)
    {
        return 0;
    }

    do
    {
        status = IE_MMIO_Read32(IE_MIDI_PLAY_STATUS);
    } while ((status & IE_MIDI_STATUS_LOADING) != 0);

    return (status & IE_MIDI_STATUS_ERROR) != 0;
}

int IE_ParseDMXSound(const uint8_t *data, uint32_t len,
                     uint32_t guest_addr, ie_dmx_sound_t *out)
{
    uint32_t payload_len;

    if (data == NULL || out == NULL || len < 8 || data[0] != 0x03
     || data[1] != 0x00)
    {
        return 0;
    }

    payload_len = ((uint32_t) data[4])
                | ((uint32_t) data[5] << 8)
                | ((uint32_t) data[6] << 16)
                | ((uint32_t) data[7] << 24);

    if (payload_len > len - 8)
    {
        return 0;
    }

    out->samples = data + 8;
    out->sample_addr = guest_addr + 8;
    out->sample_len = payload_len;
    out->sample_rate = ((uint32_t) data[2]) | ((uint32_t) data[3] << 8);
    return 1;
}

uint32_t IE_ScaleSfxVolume(int doom_volume, int max_doom_volume)
{
    if (doom_volume <= 0 || max_doom_volume <= 0)
    {
        return 0;
    }

    if (doom_volume >= max_doom_volume)
    {
        return 65535;
    }

    return (uint32_t) (((uint64_t) doom_volume * 65535u)
                    / (uint32_t) max_doom_volume);
}

uint32_t IE_SfxChannelAddr(unsigned int channel, uint32_t offset)
{
    return IE_SFX_CH_BASE + channel * IE_SFX_CH_STRIDE + offset;
}

void IE_SfxTrigger(unsigned int channel, const ie_dmx_sound_t *sound,
                   int doom_volume, int max_doom_volume)
{
    uint32_t base;

    if (sound == NULL || channel >= IE_SFX_CHANNELS)
    {
        return;
    }

    base = IE_SfxChannelAddr(channel, 0);
    IE_MMIO_Write32(base + IE_SFX_PTR, sound->sample_addr);
    IE_MMIO_Write32(base + IE_SFX_LEN, sound->sample_len);
    IE_MMIO_Write32(base + IE_SFX_FREQ, sound->sample_rate);
    IE_MMIO_Write32(base + IE_SFX_VOL,
                    IE_ScaleSfxVolume(doom_volume, max_doom_volume));
    IE_MMIO_Write32(base + IE_SFX_FORMAT, IE_SFX_FORMAT_UNSIGNED8);
    IE_MMIO_Write32(base + IE_SFX_CTRL, IE_SFX_CTRL_TRIGGER);
}

#ifndef INTUITION_ENGINE_TEST

typedef struct
{
    uint32_t data_addr;
    uint32_t len;
} ie_music_handle_t;

typedef struct
{
    void *lump_data;
    ie_dmx_sound_t sound;
} ie_cached_sound_t;

static int ie_sfx_prefix;
static unsigned int ie_next_sfx_channel;
static int ie_music_playing;
static int ie_music_paused;

static void IE_CacheSoundsModule(sfxinfo_t *sounds, int num_sounds);

static const snddevice_t ie_sound_devices[] =
{
    SNDDEVICE_SB,
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32,
};

static void IE_GetSfxLumpName(sfxinfo_t *sfx, char *buf, size_t buf_len)
{
    if (sfx->link != NULL)
    {
        sfx = sfx->link;
    }

    if (ie_sfx_prefix)
    {
        M_snprintf(buf, buf_len, "ds%s", DEH_String(sfx->name));
    }
    else
    {
        M_StringCopy(buf, DEH_String(sfx->name), buf_len);
    }
}

static int IE_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
    char namebuf[9];

    IE_GetSfxLumpName(sfxinfo, namebuf, sizeof(namebuf));
    return W_GetNumForName(namebuf);
}

static int IE_CheckSfxLumpNum(sfxinfo_t *sfxinfo)
{
    char namebuf[9];

    IE_GetSfxLumpName(sfxinfo, namebuf, sizeof(namebuf));
    return W_CheckNumForName(namebuf);
}

static int IE_StartSoundModule(sfxinfo_t *sfxinfo, int channel,
                               int vol, int sep, int pitch)
{
    ie_cached_sound_t *cached;
    unsigned int ie_channel;

    (void) channel;
    (void) sep;
    (void) pitch;

    if (sfxinfo->link != NULL)
    {
        vol += sfxinfo->volume;
        sfxinfo = sfxinfo->link;
    }

    if (sfxinfo->driver_data == NULL)
    {
        IE_CacheSoundsModule(sfxinfo, 1);
    }

    cached = (ie_cached_sound_t *) sfxinfo->driver_data;

    if (cached == NULL)
    {
        return 0;
    }

    ie_channel = ie_next_sfx_channel++ % IE_SFX_CHANNELS;
    IE_SfxTrigger(ie_channel, &cached->sound, vol, 127);
    return (int) ie_channel;
}

static int IE_InitSoundModule(GameMission_t mission)
{
    ie_sfx_prefix = mission == doom || mission == strife;
    ie_next_sfx_channel = 0;
    return 1;
}

static void IE_ShutdownSoundModule(void)
{
}

static void IE_UpdateSoundModule(void)
{
}

static void IE_UpdateSoundParamsModule(int channel, int vol, int sep)
{
    (void) channel;
    (void) vol;
    (void) sep;
}

static void IE_StopSoundModule(int channel)
{
    if (channel >= 0 && channel < (int) IE_SFX_CHANNELS)
    {
        IE_MMIO_Write32(IE_SfxChannelAddr((unsigned int) channel, IE_SFX_CTRL),
                        2);
    }
}

static boolean IE_SoundIsPlayingModule(int channel)
{
    (void) channel;
    return false;
}

static void IE_CacheSoundsModule(sfxinfo_t *sounds, int num_sounds)
{
    int i;

    for (i = 0; i < num_sounds; ++i)
    {
        int lumpnum;
        int lumplen;
        uint8_t *data;
        ie_cached_sound_t *cached;
        sfxinfo_t *sfx = &sounds[i];

        if (sfx->link != NULL)
        {
            sfx = sfx->link;
        }

        if (sfx->driver_data != NULL)
        {
            continue;
        }

        if (sfx->lumpnum <= 0)
        {
            sfx->lumpnum = IE_CheckSfxLumpNum(sfx);
        }

        if (sfx->lumpnum < 0)
        {
            continue;
        }

        lumpnum = sfx->lumpnum;
        data = W_CacheLumpNum(lumpnum, PU_STATIC);
        lumplen = W_LumpLength(lumpnum);

        cached = malloc(sizeof(*cached));
        if (cached == NULL)
        {
            W_ReleaseLumpNum(lumpnum);
            continue;
        }

        if (!IE_ParseDMXSound(data, (uint32_t) lumplen,
                              (uint32_t) (uintptr_t) data, &cached->sound))
        {
            W_ReleaseLumpNum(lumpnum);
            free(cached);
            continue;
        }

        cached->lump_data = data;
        sfx->driver_data = cached;
    }
}

const sound_module_t sound_ie_module =
{
    ie_sound_devices,
    arrlen(ie_sound_devices),
    IE_InitSoundModule,
    IE_ShutdownSoundModule,
    IE_GetSfxLumpNum,
    IE_UpdateSoundModule,
    IE_UpdateSoundParamsModule,
    IE_StartSoundModule,
    IE_StopSoundModule,
    IE_SoundIsPlayingModule,
    IE_CacheSoundsModule,
};

static int IE_InitMusicModule(void)
{
    return 1;
}

static void IE_ShutdownMusicModule(void)
{
    IE_MusicStop();
}

static void IE_SetMusicVolumeModule(int volume)
{
    IE_MusicSetVolume(volume);
}

static void IE_PauseMusicModule(void)
{
    IE_MusicPause();
    ie_music_paused = 1;
}

static void IE_ResumeMusicModule(void)
{
    IE_MusicResume();
    ie_music_paused = 0;
}

static void *IE_RegisterSongModule(void *data, int len)
{
    ie_music_handle_t *handle;

    if (data == NULL || len <= 0)
    {
        return NULL;
    }

    handle = malloc(sizeof(*handle));
    if (handle == NULL)
    {
        return NULL;
    }

    handle->data_addr = (uint32_t) (uintptr_t) data;
    handle->len = (uint32_t) len;
    return handle;
}

static void IE_UnRegisterSongModule(void *handle)
{
    free(handle);
}

static void IE_PlaySongModule(void *handle, boolean looping)
{
    ie_music_handle_t *music = (ie_music_handle_t *) handle;

    if (music == NULL)
    {
        return;
    }

    if (ie_music_mode == IE_MUSIC_MODE_NONE)
    {
        ie_music_playing = 0;
        ie_music_paused = 0;
        return;
    }

    IE_MusicStart(music->data_addr, music->len, looping);
    ie_music_playing = !IE_MusicLoadFailed();
    ie_music_paused = 0;
}

static void IE_StopSongModule(void)
{
    IE_MusicStop();
    ie_music_playing = 0;
    ie_music_paused = 0;
}

static boolean IE_MusicIsPlayingModule(void)
{
    return ie_music_playing && !ie_music_paused;
}

static const snddevice_t ie_music_devices[] =
{
    SNDDEVICE_SB,
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32,
};

const music_module_t music_ie_module =
{
    ie_music_devices,
    arrlen(ie_music_devices),
    IE_InitMusicModule,
    IE_ShutdownMusicModule,
    IE_SetMusicVolumeModule,
    IE_PauseMusicModule,
    IE_ResumeMusicModule,
    IE_RegisterSongModule,
    IE_UnRegisterSongModule,
    IE_PlaySongModule,
    IE_StopSongModule,
    IE_MusicIsPlayingModule,
    NULL,
};

#endif
