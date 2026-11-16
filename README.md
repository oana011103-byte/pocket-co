**English** · [简体中文](README.zh_CN.md)

# Pocket co

An offline answer book for FoloToy AI Passport. Think of a question, press OK,
and let the little Co companion reveal one of 110 Chinese answers.

![Pocket co screens](assets/images/pocket-co-preview.png)

## Install the firmware

Download `pocket-co-v1.0.3-share.zip` from the
[version release](https://github.com/oana011103-byte/pocket-co/releases/tag/pocket-co-v1.0.3).
Extract it, open the [official flashing tool](https://ai-passport.folotoy.cn/tools/web-flasher/)
in desktop Chrome or Edge, choose local firmware installation, and select
`pocket-co-v1.0.3-full.bin`. Connect the powered-on Passport using a data-capable
USB cable and keep it connected until installation and restart finish.

Only FoloToy AI Passport with ESP32-C3 and 8 MB Flash is supported. The merged
image is written at `0x0`; it replaces the existing application and may reset
saved settings. A whole-chip erase is not required.

## Controls

- Short press OK: draw an answer; press again to draw another.
- Hold OK for about half a second: return to the cover, including while thinking.
- After 60 seconds idle the backlight dims; the first short press wakes it.
- The upper-right percentage is battery charge; `--` means unavailable.

The companion uses the original Cola idle, thinking and completion animations.
All 110 answers are included locally. There is no Wi-Fi, Bluetooth, audio,
account, question collection or stored answer history.

## Build and customize

Use ESP-IDF 5.5.3 with the ESP32-C3 toolchain, Python and a host C compiler.
Activate ESP-IDF, clone this repository's default `codex/pocket-co` branch, then run:

```bash
./tools/validate.sh
```

The gate runs repository checks, host tests and an isolated firmware build.
Flashable output: `build/FoloToy-AI-Passport-full.bin`. Matching ELF/MAP and
component images are archived under `build/firmware/<sha256>/`.

The checked-in fonts and frame tables are sufficient for a normal build.
To change answers, update `main/pocket-co-content.c` and the canonical list in
`assets/text/pocket-co-answers.txt`, then regenerate fonts and images:

```bash
python3 -m venv .local-tools/assets-venv
.local-tools/assets-venv/bin/python -m pip install -r tools/requirements-pocket-co.txt
npm install --prefix .local-tools/font-converter lv_font_conv@1.5.3
.local-tools/assets-venv/bin/python tools/generate-pocket-co-assets.py \
  --converter .local-tools/font-converter/node_modules/lv_font_conv/lv_font_conv.js
./tools/validate.sh
```

Adjust the answer-count tests if changing the number of entries. Verify all new
text on the display; a successful build alone does not establish glyph layout.
See the [application guide](docs/apps/pocket-co.md),
[asset sources](assets/README.md#pocket-co-application-assets), and
[ESP-IDF environment guide](docs/development/engineering/environment-setup.md).

## Validation and release identity

Version 1.0.3 passed the build and host gates. The November 2026 batch had 19
successful flash verifications: 18 startup-log checks passed; the remaining unit
was confirmed working by its user after USB was disconnected during observation.
Physical display, button, battery and idle checks remain the recipient's checks.
No device identities or raw logs are published.

The release retains the exact previously installed firmware, with SHA-256:

```text
db717af8f0c53f4416607ad82853c8d3a5d582ebcd8a9a271f339901c6c6d8df
```

Release preparation adds documentation, licensing and portable generator metadata;
it does not change application behavior. A fresh build is not promised to be
byte-identical because the toolchain embeds build metadata.

## Licenses and credits

Based on [FoloToy/ai-passport](https://github.com/folotoy/ai-passport).
Project-authored application code and answer text are MIT licensed; retain the
[license](LICENSE) and existing third-party notices. Third-party components retain
their own licenses. Noto Sans SC fonts and generated subsets use the
[SIL Open Font License](assets/fonts/noto-ofl.txt).

**Cola brand assets are excluded from the MIT code license.** Their original
character, animations, generated frames and previews may be redistributed with
Pocket co under the [project redistribution permission](assets/images/composer-co/LICENSE.txt).
Brand ownership and trademark rights remain with their respective holders;
unrelated asset reuse needs separate permission. The repository therefore
contains open-source code alongside separately licensed brand artwork.
