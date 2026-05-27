#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "d_event.h"
#include "doomtype.h"
#include "i_intuition.h"
#include "i_video.h"

typedef struct
{
    uint32_t addr;
    uint32_t value;
} write_t;

static write_t writes[16];
static int num_writes;

void IE_TestSetMMIO(uint32_t (*read32)(uint32_t addr),
                    void (*write32)(uint32_t addr, uint32_t value));

void D_PostEvent(event_t *ev)
{
    (void) ev;
}

void M_BindStringVariable(const char *name, char **location)
{
    (void) name;
    (void) location;
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
    (void) addr;
    return 0;
}

static void expect_write(int i, uint32_t addr, uint32_t value)
{
    assert(i < num_writes);
    assert(writes[i].addr == addr);
    assert(writes[i].value == value);
}

static void reset_writes(void)
{
    memset(writes, 0, sizeof(writes));
    num_writes = 0;
}

static void test_framebuffer_separate_scanout_after_present(void)
{
    pixel_t copy[SCREENWIDTH * SCREENHEIGHT];
    uint32_t initial_scanout;
    uint32_t draw_buffer_addr;
    uint32_t presented_scanout;
    int i;

    IE_TestSetMMIO(read32, write32);
    I_InitGraphics();
    draw_buffer_addr = (uint32_t) (uintptr_t) I_VideoBuffer;

    expect_write(0, IE_MOUSE_CTRL, 1);
    expect_write(1, IE_VIDEO_MODE, IE_MODE_320X200);
    expect_write(2, IE_VIDEO_COLOR_MODE, IE_VIDEO_CLUT8);
    expect_write(3, IE_VIDEO_FB_BASE, writes[3].value);
    expect_write(4, IE_VIDEO_CTRL, 1);
    initial_scanout = writes[3].value;
    assert(initial_scanout != draw_buffer_addr);

    for (i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
    {
        I_VideoBuffer[i] = (pixel_t) (i & 0xff);
    }

    reset_writes();
    I_FinishUpdate();
    expect_write(0, IE_VIDEO_FB_BASE, writes[0].value);
    presented_scanout = writes[0].value;
    assert(presented_scanout != draw_buffer_addr);
    assert(presented_scanout != initial_scanout);

    memset(copy, 0, sizeof(copy));
    I_ReadScreen(copy);

    assert(memcmp(copy, I_VideoBuffer, sizeof(copy)) == 0);
}

int main(void)
{
    test_framebuffer_separate_scanout_after_present();
    puts("i_video_intuition tests passed");
    return 0;
}
