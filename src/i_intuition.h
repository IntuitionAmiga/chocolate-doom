// Intuition Engine MMIO backend helpers.

#ifndef __I_INTUITION__
#define __I_INTUITION__

#include <stdint.h>

#define IE_PROGRAM_START        0x00001000u
#define IE_STACK_TOP            0x00FF0000u

#define IE_TERM_KEY_IN          0x000F0728u
#define IE_TERM_KEY_STATUS      0x000F072Cu
#define IE_MOUSE_X              0x000F0730u
#define IE_MOUSE_Y              0x000F0734u
#define IE_MOUSE_BUTTONS        0x000F0738u
#define IE_MOUSE_STATUS         0x000F073Cu
#define IE_SCAN_CODE            0x000F0740u
#define IE_SCAN_STATUS          0x000F0744u
#define IE_SCAN_MODIFIERS       0x000F0748u
#define IE_MOUSE_CTRL           0x000F074Cu
#define IE_MOUSE_DX             0x000F0754u
#define IE_MOUSE_DY             0x000F0758u
#define IE_RTC_MONO_USEC_LO      0x000F075Cu
#define IE_RTC_MONO_USEC_HI      0x000F0760u

#define IE_VIDEO_CTRL            0x000F0000u
#define IE_VIDEO_MODE            0x000F0004u
#define IE_VIDEO_COLOR_MODE      0x000F0080u
#define IE_VIDEO_FB_BASE         0x000F0084u
#define IE_VIDEO_PAL_TABLE       0x000F0088u
#define IE_MODE_320X200          0x00000004u
#define IE_VIDEO_CLUT8           0x00000001u

#define IE_MIDI_PLAY_PTR         0x000F0BA0u
#define IE_MIDI_PLAY_LEN         0x000F0BA4u
#define IE_MIDI_PLAY_CTRL        0x000F0BA8u
#define IE_MIDI_PLAY_STATUS      0x000F0BACu
#define IE_MIDI_VOLUME           0x000F0BB4u
#define IE_MIDI_CTRL_START       0x00000001u
#define IE_MIDI_CTRL_STOP        0x00000002u
#define IE_MIDI_CTRL_LOOP        0x00000004u
#define IE_MIDI_CTRL_PAUSE       0x00000008u
#define IE_MIDI_STATUS_ERROR     0x00000002u
#define IE_MIDI_STATUS_LOADING   0x00000008u
#define IE_MEDIA_TYPE_MIDI       8u

#define IE_FILE_NAME_PTR         0x000F2200u
#define IE_FILE_DATA_PTR         0x000F2204u
#define IE_FILE_DATA_LEN         0x000F2208u
#define IE_FILE_CTRL             0x000F220Cu
#define IE_FILE_STATUS           0x000F2210u
#define IE_FILE_RESULT_LEN       0x000F2214u
#define IE_FILE_OP_READ          1u
#define IE_FILE_OP_WRITE         2u
#define IE_FILE_OP_LIST          3u

#define IE_SFX_CH_BASE           0x000F0E80u
#define IE_SFX_CH_STRIDE         0x00000020u
#define IE_SFX_CHANNELS          4u
#define IE_SFX_PTR               0x00000000u
#define IE_SFX_LEN               0x00000004u
#define IE_SFX_FREQ              0x00000010u
#define IE_SFX_VOL               0x00000014u
#define IE_SFX_FORMAT            0x00000018u
#define IE_SFX_CTRL              0x0000001Cu
#define IE_SFX_FORMAT_SIGNED8    0u
#define IE_SFX_FORMAT_UNSIGNED8  1u
#define IE_SFX_CTRL_TRIGGER      1u

#define IE_MUSIC_MODE_ORIGINAL_MUS 0
#define IE_MUSIC_MODE_NONE         1

typedef struct
{
    const uint8_t *samples;
    uint32_t sample_addr;
    uint32_t sample_len;
    uint32_t sample_rate;
} ie_dmx_sound_t;

extern int ie_music_mode;

uint32_t IE_MMIO_Read32(uint32_t addr);
void IE_MMIO_Write32(uint32_t addr, uint32_t value);
uint64_t IE_ReadMonotonicUsec(void);
int IE_TimeUsecToTics(uint64_t usec);

void IE_VideoInit(uint32_t framebuffer_addr);
void IE_VideoSetPalette(const uint8_t *doompalette);
uint32_t IE_PackPaletteEntry(uint8_t r, uint8_t g, uint8_t b);

void IE_InputInit(void);
int IE_TranslateScancode(uint8_t scancode, int *pressed);

int IE_FileReadAll(const char *name, void *buffer, uint32_t buffer_len,
                   uint32_t *result_len);

void IE_MusicSetVolume(int volume);
void IE_MusicStart(uint32_t data_addr, uint32_t len, int looping);
void IE_MusicStop(void);
void IE_MusicPause(void);
void IE_MusicResume(void);
int IE_MusicLoadFailed(void);

int IE_ParseDMXSound(const uint8_t *data, uint32_t len,
                     uint32_t guest_addr, ie_dmx_sound_t *out);
uint32_t IE_ScaleSfxVolume(int doom_volume, int max_doom_volume);
uint32_t IE_SfxChannelAddr(unsigned int channel, uint32_t offset);
void IE_SfxTrigger(unsigned int channel, const ie_dmx_sound_t *sound,
                   int doom_volume, int max_doom_volume);

#endif
