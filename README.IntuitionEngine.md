# Intuition Engine Port Notes

This tree contains an experimental Intuition Engine backend for the Doom target.
It is selected with the `INTUITION_ENGINE` compile definition and is intended for
a flat x86 `.ie86` guest image.

Current backend coverage:

- Monotonic timing reads `RTC_MONO_USEC_HI`, `RTC_MONO_USEC_LO`,
  `RTC_MONO_USEC_HI` and retries if the high halves differ.
- Video uses IE VideoChip `MODE_320x200` with CLUT8 framebuffer presentation.
- Doom palette triples are written to `VIDEO_PAL_TABLE` as `0x00RRGGBB`.
- Input drains IE PC/AT make/break scancodes from `SCAN_STATUS`/`SCAN_CODE`
  into Doom key events and enables relative mouse mode through `MOUSE_CTRL`.
  Mouse deltas are read from `MOUSE_DX`/`MOUSE_DY` and posted as Doom mouse
  movement events.
- Music sends original WAD MUS/MIDI lump bytes directly to the IE MIDI/MUS MMIO
  player. Loop, stop, pause, resume, volume, and async loading/error status are
  handled through `MIDI_PLAY_*` registers.
- `ie_music_mode` controls music playback: `0` = Original MUS, `1` = None.
  Original MUS is the default. The setup sound menu exposes these options when
  built with `INTUITION_ENGINE`.
- Sound effects parse Doom DMX sound lump headers and trigger IE SFX channels
  with unsigned 8-bit PCM payload pointers, payload lengths, sample rates, and
  scaled 0..65535 volume. Parsed sound metadata is cached on Chocolate Doom's
  `sfxinfo_t::driver_data` so MMIO playback points at stable lump memory.
- WAD files are loaded through IE File I/O MMIO when `INTUITION_ENGINE` is
  defined. The backend reads the whole WAD into guest memory once, exposes it as
  a mapped WAD buffer, and serves Chocolate Doom's offset reads from that cache.
  The current maximum single WAD size is 64 MiB.

The Intuition Engine x86 loader contract is load-at-0/start-at-0. A freestanding
`.ie86` image that links main Doom code at a higher address must include a reset
trampoline at address `0` to set up the stack/C runtime and jump to the port
entry point.

The host-side unit test for the backend helpers can be run without SDL. It
covers the IE MMIO ABI constants, timer retry reads, video/palette writes,
input setup, music controls and load status, DMX SFX parsing/triggering, and
File I/O read-all register handling:

```sh
cc -DINTUITION_ENGINE_TEST -Isrc -I. \
  src/i_intuition.c src/i_intuition_test.c \
  -o /tmp/i_intuition_test && /tmp/i_intuition_test
```
