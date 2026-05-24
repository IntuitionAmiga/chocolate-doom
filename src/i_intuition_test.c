#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "doomkeys.h"
#include "i_intuition.h"

typedef struct
{
    uint32_t addr;
    uint32_t value;
} write_t;

static write_t writes[512];
static int num_writes;
static uint32_t status_reads[8];
static int num_status_reads;
static uint32_t rtc_hi_reads[8];
static int num_rtc_hi_reads;
static uint32_t rtc_lo_value;
static uint32_t file_status_value;
static uint32_t file_result_len_value;

void IE_TestSetMMIO(uint32_t (*read32)(uint32_t addr),
                    void (*write32)(uint32_t addr, uint32_t value));

static void reset_io(void)
{
    memset(writes, 0, sizeof(writes));
    num_writes = 0;
    memset(status_reads, 0, sizeof(status_reads));
    num_status_reads = 0;
    memset(rtc_hi_reads, 0, sizeof(rtc_hi_reads));
    num_rtc_hi_reads = 0;
    rtc_lo_value = 0;
    file_status_value = 0;
    file_result_len_value = 0;
}

static void write32(uint32_t addr, uint32_t value)
{
    assert(num_writes < (int) (sizeof(writes) / sizeof(writes[0])));
    writes[num_writes].addr = addr;
    writes[num_writes].value = value;
    ++num_writes;
}

static uint32_t read32(uint32_t addr)
{
    if (addr == IE_MIDI_PLAY_STATUS)
    {
        assert(num_status_reads < (int) (sizeof(status_reads) / sizeof(status_reads[0])));
        return status_reads[num_status_reads++];
    }
    if (addr == IE_RTC_MONO_USEC_HI)
    {
        assert(num_rtc_hi_reads < (int) (sizeof(rtc_hi_reads) / sizeof(rtc_hi_reads[0])));
        return rtc_hi_reads[num_rtc_hi_reads++];
    }
    if (addr == IE_RTC_MONO_USEC_LO)
    {
        return rtc_lo_value;
    }
    if (addr == IE_FILE_STATUS)
    {
        return file_status_value;
    }
    if (addr == IE_FILE_RESULT_LEN)
    {
        return file_result_len_value;
    }
    return 0;
}

static void expect_write(int i, uint32_t addr, uint32_t value)
{
    assert(i < num_writes);
    assert(writes[i].addr == addr);
    assert(writes[i].value == value);
}

static void test_timer_high_low_high_retry(void)
{
    uint64_t got;

    reset_io();
    rtc_hi_reads[0] = 1;
    rtc_hi_reads[1] = 2;
    rtc_hi_reads[2] = 2;
    rtc_hi_reads[3] = 2;
    rtc_lo_value = 0xABCDEF01u;

    got = IE_ReadMonotonicUsec();
    assert(got == 0x00000002ABCDEF01ull);
    assert(IE_TimeUsecToTics(1000000) == 35);
}

static void test_video_startup_and_palette(void)
{
    uint8_t palette[256 * 3];

    reset_io();
    IE_VideoInit(0x00123000u);
    expect_write(0, IE_VIDEO_MODE, IE_MODE_320X200);
    expect_write(1, IE_VIDEO_COLOR_MODE, IE_VIDEO_CLUT8);
    expect_write(2, IE_VIDEO_FB_BASE, 0x00123000u);
    expect_write(3, IE_VIDEO_CTRL, 1);

    memset(palette, 0, sizeof(palette));
    palette[10 * 3 + 0] = 0x11;
    palette[10 * 3 + 1] = 0x22;
    palette[10 * 3 + 2] = 0x33;

    reset_io();
    IE_VideoSetPalette(palette);
    expect_write(10, IE_VIDEO_PAL_TABLE + 10 * 4, 0x00112233u);
}

static void test_input_constants_and_scancode_translation(void)
{
    int pressed;

    assert(IE_PROGRAM_START == 0x1000u);
    assert(IE_STACK_TOP == 0xFF0000u);
    assert(IE_TERM_KEY_IN == 0xF0728u);
    assert(IE_TERM_KEY_STATUS == 0xF072Cu);
    assert(IE_SCAN_CODE == 0xF0740u);
    assert(IE_SCAN_STATUS == 0xF0744u);
    assert(IE_SCAN_MODIFIERS == 0xF0748u);
    assert(IE_MOUSE_CTRL == 0xF074Cu);
    assert(IE_MOUSE_DX == 0xF0754u);
    assert(IE_MOUSE_DY == 0xF0758u);
    assert(IE_FILE_NAME_PTR == 0xF2200u);
    assert(IE_FILE_DATA_PTR == 0xF2204u);
    assert(IE_FILE_DATA_LEN == 0xF2208u);
    assert(IE_FILE_CTRL == 0xF220Cu);
    assert(IE_FILE_STATUS == 0xF2210u);
    assert(IE_FILE_RESULT_LEN == 0xF2214u);
    assert(IE_MEDIA_TYPE_MIDI == 8u);

    reset_io();
    IE_InputInit();
    expect_write(0, IE_MOUSE_CTRL, 1);

    assert(IE_TranslateScancode(0x1e, &pressed) == 'a');
    assert(pressed);
    assert(IE_TranslateScancode(0x9e, &pressed) == 'a');
    assert(!pressed);
    assert(IE_TranslateScancode(0x48, &pressed) == KEY_UPARROW);
    assert(IE_TranslateScancode(0x4b, &pressed) == KEY_LEFTARROW);
    assert(IE_TranslateScancode(0x4d, &pressed) == KEY_RIGHTARROW);
    assert(IE_TranslateScancode(0x50, &pressed) == KEY_DOWNARROW);
}

static void test_music_controls_and_loading_status(void)
{
    reset_io();
    ie_music_mode = IE_MUSIC_MODE_ORIGINAL_MUS;
    IE_MusicStart(0x2000, 1234, true);
    expect_write(0, IE_MIDI_PLAY_PTR, 0x2000);
    expect_write(1, IE_MIDI_PLAY_LEN, 1234);
    expect_write(2, IE_MIDI_PLAY_CTRL, IE_MIDI_CTRL_START | IE_MIDI_CTRL_LOOP);

    reset_io();
    IE_MusicStop();
    IE_MusicPause();
    IE_MusicResume();
    IE_MusicSetVolume(127);
    expect_write(0, IE_MIDI_PLAY_CTRL, IE_MIDI_CTRL_STOP);
    expect_write(1, IE_MIDI_PLAY_CTRL, IE_MIDI_CTRL_PAUSE);
    expect_write(2, IE_MIDI_PLAY_CTRL, 0);
    expect_write(3, IE_MIDI_VOLUME, 255);

    reset_io();
    status_reads[0] = IE_MIDI_STATUS_LOADING;
    status_reads[1] = IE_MIDI_STATUS_ERROR;
    assert(IE_MusicLoadFailed());
    assert(num_status_reads == 2);

    reset_io();
    ie_music_mode = IE_MUSIC_MODE_NONE;
    IE_MusicStart(0x2000, 1234, true);
    IE_MusicStop();
    IE_MusicPause();
    IE_MusicResume();
    IE_MusicSetVolume(127);
    assert(num_writes == 0);
    ie_music_mode = IE_MUSIC_MODE_ORIGINAL_MUS;
}

static void test_dmx_sfx_parse_and_trigger(void)
{
    uint8_t lump[16] = {
        0x03, 0x00,
        0x11, 0x2B,
        0x08, 0x00, 0x00, 0x00,
        0x80, 0x81, 0x82, 0x83,
        0x84, 0x85, 0x86, 0x87,
    };
    ie_dmx_sound_t sound;
    uint32_t base;

    assert(IE_ParseDMXSound(lump, sizeof(lump), 0x3000, &sound));
    assert(sound.samples == lump + 8);
    assert(sound.sample_addr == 0x3008);
    assert(sound.sample_len == 8);
    assert(sound.sample_rate == 11025);
    assert(IE_ScaleSfxVolume(127, 127) == 65535);

    reset_io();
    IE_SfxTrigger(2, &sound, 64, 127);
    base = IE_SFX_CH_BASE + 2 * IE_SFX_CH_STRIDE;
    expect_write(0, base + IE_SFX_PTR, 0x3008);
    expect_write(1, base + IE_SFX_LEN, 8);
    expect_write(2, base + IE_SFX_FREQ, 11025);
    expect_write(3, base + IE_SFX_VOL, (uint32_t) ((64ull * 65535u) / 127u));
    expect_write(4, base + IE_SFX_FORMAT, IE_SFX_FORMAT_UNSIGNED8);
    expect_write(5, base + IE_SFX_CTRL, IE_SFX_CTRL_TRIGGER);
}

static void test_file_read_all_register_sequence(void)
{
    const char name[] = "doom.wad";
    uint8_t buffer[16];
    uint32_t result_len = 0;

    reset_io();
    file_status_value = 0;
    file_result_len_value = 12;

    assert(IE_FileReadAll(name, buffer, sizeof(buffer), &result_len));
    expect_write(0, IE_FILE_NAME_PTR, (uint32_t) (uintptr_t) name);
    expect_write(1, IE_FILE_DATA_PTR, (uint32_t) (uintptr_t) buffer);
    expect_write(2, IE_FILE_DATA_LEN, sizeof(buffer));
    expect_write(3, IE_FILE_CTRL, IE_FILE_OP_READ);
    assert(result_len == 12);

    reset_io();
    file_status_value = 1;
    file_result_len_value = 7;
    result_len = 99;

    assert(!IE_FileReadAll(name, buffer, sizeof(buffer), &result_len));
    assert(result_len == 0);
}

int main(void)
{
    IE_TestSetMMIO(read32, write32);
    test_timer_high_low_high_retry();
    test_video_startup_and_palette();
    test_input_constants_and_scancode_translation();
    test_music_controls_and_loading_status();
    test_dmx_sfx_parse_and_trigger();
    test_file_read_all_register_sequence();
    puts("i_intuition tests passed");
    return 0;
}
