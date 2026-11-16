<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Assets

This directory stores reusable fonts, images, music, sound effects, and battery profiles, organized by asset type.

Keep each asset in the matching subdirectory and document its destination, naming, integration method, and source/license. Do not mix binary assets with Markdown documentation.

## Fonts

Store reusable font files and generated font sources in `fonts/`.

- Use descriptive names that include the family, weight, size, and format when relevant.
- Document the source, license, character range, conversion command, and expected destination.
- Check Flash and internal-RAM impact before adding a font; the ESP32-C3 has no PSRAM.
- Do not commit fonts whose license does not permit redistribution.

## Images

Store reusable source images and generated display assets in `images/`.

| File | Dimensions and format | Use and source |
| --- | --- | --- |
| [`images/home.jpg`](images/home.jpg) | 3840 × 2160, JPEG | Product hero image embedded in both project README files to foreground AI Passport and its open, maker-oriented identity. |
| [`images/readme-hardware-specs.png`](images/readme-hardware-specs.png) | 2172 × 724, PNG RGBA | Optional technical infographic retained as a reference asset; it is no longer used as the homepage hero. Generated for this repository with the built-in image generation tool on 2026-09-17; the six labels and values were checked against the documented hardware contract. |
| [`images/logo-wordmark.png`](images/logo-wordmark.png) | 1648 × 336, PNG RGBA | Transparent black wordmark extracted from the repository's original `images/logo.png`; embedded in both project README files for light backgrounds. |
| [`images/logo-wordmark-dark.png`](images/logo-wordmark-dark.png) | 1648 × 336, PNG RGBA | White version of the extracted wordmark, used by the README `<picture>` element when GitHub is in dark mode. |

- Use descriptive names and document dimensions, pixel format, conversion steps, and destination.
- Prefer formats suitable for the 240 × 320 RGB565 display and account for Flash and internal RAM.
- Preserve editable sources where licensing permits, and record the source and license.
- Never commit device QR secrets, credentials, or personal data in images.

## Music and sound effects

Store reusable music and sound-effect sources in `music/`.

- Document the source, license, sample rate, bit depth, channels, conversion command, and destination.
- Prefer 16 kHz, 16-bit mono PCM when it matches the current BSP audio path.
- Check Flash and internal-RAM cost before embedding audio; stream or chunk long recordings.
- Do not commit media without redistribution permission.

## Battery profiles

[`battery/wanli-500mah-profile2-xil40x-20261020.txt`](battery/wanli-500mah-profile2-xil40x-20261020.txt)
contains the supplied CW2017 profile for the Wanli New Energy 500 mAh cell
(profile2, XIL40X, 2026-10-20). It is an unchanged copy of the project-supplied
file, retained at the maintainer's request for this firmware update. Its
original filename is recorded in the paired Chinese document. No separate
license was included in the supplied file.

The format is 80 lines of two hexadecimal digits, one byte per line in register
order. The BSP embeds those bytes in `components/bsp/src/bsp_battery.c`; the
host gate checks the source file and initializer for an exact match. Text-file
SHA-256: `7ea96eba4634f4495a37c25ffacd54c1deaa0693b162c892d671a2ba68460919`.
Decoded 80-byte SHA-256:
`fa7365671093ef449c5bdcfb4adb279e3d275f888109f50b6b2f21e9b96220d5`.
This is a fuel-gauge profile, not a charging configuration or a generic profile
for every 500 mAh cell; see the [hardware guide](../docs/hardware-design/AI_HARDWARE_DEVELOPMENT_GUIDE.md#9-cw2017-fuel-gauge).

## Pocket co application assets

- `images/composer-co/perching-co.riv` is the unchanged Rive file used by
  Cola's input-box companion, copied from the user's local Cola project
  (`apps/desktop/dev/composer-co/perching-rive/build/perching-co.riv`). It is
  a Cola brand asset. Public redistribution with Pocket co source and firmware
  is authorized under `images/composer-co/LICENSE.txt`; brand ownership and
  trademark rights are retained. This permission is separate from the MIT code
  license. The `Idle`, `Thinking`, and `Complete` animations are
  exported without changing the original character artwork or keyframes.
- `images/composer-co/frames/` retains 128 x 110 transparent PNG samples at
  80 ms intervals, plus a manifest binding them to source/runtime hashes.
  `tools/capture-pocket-co-frames.mjs` exports them using the original Rive
  JS/WASM runtime. Pass `--runtime-dir`, `--playwright` and optionally
  `--browser` to select local tools without changing system configuration.
- `images/pocket-co-images.c` and `pocket-co-animation.h` contain deduplicated,
  flash-resident RGB565 frame tables. They preserve Rive colors and composite
  over the application's background. Thirty unique frames use 844,800 bytes
  of Flash; no full-frame animation buffer or Rive runtime is needed on-device.
- `fonts/noto-sans-sc.ttf` is Noto Sans SC from Google Fonts
  (`ofl/notosanssc/NotoSansSC[wght].ttf`), licensed under the SIL Open Font
  License in `fonts/noto-ofl.txt`. `fonts/noto-sans-sc-medium.ttf` is its weight
  500 instance. The generated `pocket-co-font-16.c` and `pocket-co-font-22.c`
  cover ASCII plus the application's fixed Chinese strings at 16/22 px, 4 bpp,
  without compression. `fonts/pocket-co-symbols.txt` retains the inventory.
- Regenerate with Python Pillow 11.3.0 / fonttools 4.60.2 and
  `python3 tools/generate-pocket-co-assets.py --converter <lv_font_conv.js>`
  using LVGL's `lv_font_conv` 1.5.3. The script checks source glyph coverage;
  the application and host renderer check the actual linked LVGL font mappings.

## Text

`text/pocket-co-answers.txt` preserves the user's 110 approved Chinese answers
with numbering. Firmware strings match that text exactly after removing display
line breaks. The final ten add warmer, imaginative prompts. No answer has a
trailing full stop.
