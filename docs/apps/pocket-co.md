[简体中文](pocket-co.zh_CN.md) · **English**

# Pocket co 1.0.3

A Cola-themed, fully offline answer book for FoloToy AI Passport (ESP32-C3,
240 x 320, 8 MB Flash). The application starts on its own home screen, uses
Cola's existing mascot and orange brand color, and never enters the BSP demo menu.

## Controls

Think of a question and click OK. After 1.1 seconds, one of 110 original Chinese
answers appears. Click OK again for another answer. Consecutive answers do not
repeat. Short clicks during the thinking phase are ignored. Hold OK for about
0.5 seconds to return to the cover from any phase, including while thinking;
this cancels a pending draw and restores brightness. Releasing the long press
does not draw an answer. UP and DOWN have no application action. A quick double click counts as one request.

After 60 seconds without activity, brightness falls from 75% to 12%. The first
OK click restores brightness while preserving the current screen; the next draws
an answer. This is backlight dimming, not deep sleep. Power cycling returns to
the home screen; no question, answer history or settings are stored. Battery
percentage appears when the gauge is available; `--` means unavailable.

No Wi-Fi, Bluetooth, microphone, audio, account or network service is used.
Answers are playful prompts for reflection, not predictions or decision advice.

## Build and install

Use ESP-IDF 5.5.3 and run `./tools/validate.sh`. The verified merged image is
`build/FoloToy-AI-Passport-full.bin`, written from `0x0`. Matching debug artifacts
are retained under `build/firmware/<sha256>/`; verify them with
`python3 tools/archive_firmware.py verify <archive-directory>`.

Open <https://ai-passport.folotoy.cn/tools/web-flasher/> in desktop Chrome or
Edge, choose the local merged image and connect the powered-on Passport with a
data-capable USB-C cable. Wait for completion without disconnecting. Installing
replaces the current play and can reset saved data. No whole-chip erase is
required. Device flashing requires the user's confirmation.

## Validation

The gate runs the upstream BSP tests and the application's state tests (20,000
draws, all 110 answers reachable, no consecutive repeats, busy input, dim/wake,
empty/single-item pools, return/cancel behavior and 32-bit timer wrap). The host LVGL renderer checks
linked fonts and every answer's text bounds. A successful build or host capture
does not establish real screen, button or battery behavior.

On hardware, check Chinese glyphs, all screens, short/double/long presses,
repeated presses while thinking, idle dimming/waking, battery indication and
operation after disconnecting USB. Record the exact merged-image hash tested.

## Assets and scope

See [asset provenance](../../assets/README.md#pocket-co-application-assets).
The interface uses a dark warm background, cream text, the original white input-box companion and Cola orange
`#F1752D` for the physical-button hint. The 16/22 px fonts are subsets of Noto
Sans SC; the font-generation script must be rerun when fixed text changes.

## Composer companion update

The home heading asks users to consult little Co when undecided. All three
phases keep the companion at 128 x 110, at the same screen position. Its
original Rive idle blink, thinking bubble and short completion reaction are
sampled at 12.5 fps. After completion it returns to idle; dimming suspends
frame updates. The answer text moves below the companion and all 110 answers
fit the 82 px text region. Short-press draw behavior is unchanged. Long-press OK now returns to the cover;
the answer footer explains both controls.

## Approved answer copy

The 110 answers match the user's approved text in
[`assets/text/pocket-co-answers.txt`](../../assets/text/pocket-co-answers.txt).
The first 100 use concise answer-book wording; the final ten add warmth and
imagination. None ends with a full stop. Only display line breaks are inserted
in `main/pocket-co-content.c`; removing those breaks reproduces the approved
wording exactly. Regenerate the font subsets whenever the answer copy changes.
The offline device does not read the user's question or classify its topic;
answers remain randomly drawn reflection prompts rather than personalized advice.
