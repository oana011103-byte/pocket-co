#!/usr/bin/env python3
"""Generate the application's font subsets and flash-resident RGB565 frames."""
import argparse
import json
from pathlib import Path
import re
import subprocess

from PIL import Image
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont

ROOT = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser()
parser.add_argument('--converter', required=True, help='Pinned lv_font_conv 1.5.3 JS entry point')
args = parser.parse_args()
text = '\n'.join((ROOT / 'main' / f).read_text() for f in ('pocket-co-content.c', 'pocket-co-content.h'))
literals = re.findall(r'"((?:[^"\\]|\\.)*)"', text)
strings = [json.loads('"' + s + '"') for s in literals]
symbols = ''.join(sorted(set(''.join(strings)) - {'\n', '\r', '\t'}))
font_dir = ROOT / 'assets/fonts'
font = TTFont(font_dir / 'noto-sans-sc.ttf')
cmap = font.getBestCmap()
assert all(ord(c) in cmap for c in symbols), 'Missing source glyph'
font = instantiateVariableFont(font, {'wght': 500}, inplace=True)
font.save(font_dir / 'noto-sans-sc-medium.ttf')
(font_dir / 'pocket-co-symbols.txt').write_text(symbols, encoding='utf-8')
for size in (16, 22):
    subprocess.run([
        'node', str(Path(args.converter).resolve()), '--font', str(font_dir / 'noto-sans-sc-medium.ttf'),
        '--symbols', symbols, '--range', '0x20-0x7e', '--size', str(size), '--bpp', '4',
        '--format', 'lvgl', '--no-compress', '--no-kerning', '--lv-include', 'lvgl.h',
        '--lv-font-name', f'pocket_co_font_{size}',
        '--output', str(font_dir / f'pocket-co-font-{size}.c'),
    ], check=True)
    generated = font_dir / f'pocket-co-font-{size}.c'
    # Converter metadata must not publish the build machine's absolute paths.
    generated.write_text(generated.read_text().replace(str(ROOT) + '/', '').rstrip() + '\n')

manifest = json.loads((ROOT / 'assets/images/composer-co/frames/manifest.json').read_text())
width, height = manifest['width'], manifest['height']
background = (0x21, 0x1c, 0x18)
unique = {}
sequences = {}
for pose, spec in manifest['poses'].items():
    sequence = []
    for filename in spec['frames']:
        image = Image.open(ROOT / 'assets/images/composer-co/frames' / filename).convert('RGBA')
        assert image.size == (width, height)
        surface = Image.new('RGBA', image.size, background + (255,))
        surface.alpha_composite(image)
        values = [((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                  for r, g, b in surface.convert('RGB').getdata()]
        key = tuple(values)
        if key not in unique:
            unique[key] = len(unique)
        sequence.append(unique[key])
    sequences[pose] = sequence

out = ['// Generated RGB565 Rive frames. Provenance: assets/README.md.',
       '#include "pocket-co-animation.h"', '']
for values, index in unique.items():
    symbol = f'pocket_co_frame_{index}'
    out.append(f'static const uint16_t {symbol}_data[] = {{')
    for i in range(0, len(values), 24):
        out.append('    ' + ','.join(f'0x{v:04x}' for v in values[i:i+24]) + ',')
    out.extend(['};', f'static const lv_image_dsc_t {symbol} = {{',
                '    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,',
                f'                .w = {width}, .h = {height}, .stride = {width * 2} }},',
                f'    .data_size = sizeof({symbol}_data), .data = (const uint8_t *){symbol}_data,',
                '};', ''])
header = ['// Generated frame tables; constant image data stays in Flash.',
          '#pragma once', '#include "lvgl.h"',
          f"#define POCKET_CO_FRAME_MS {manifest['frame_ms']}u", '']
for pose, sequence in sequences.items():
    symbol = f'pocket_co_{pose}_frames'
    out.append(f'const lv_image_dsc_t *const {symbol}[] = {{')
    out.append('    ' + ','.join(f'&pocket_co_frame_{i}' for i in sequence))
    out.extend(['};', ''])
    header.extend([f'#define POCKET_CO_{pose.upper()}_FRAME_COUNT {len(sequence)}u',
                   f'extern const lv_image_dsc_t *const {symbol}[];', ''])
(ROOT / 'assets/images/pocket-co-animation.h').write_text('\n'.join(header))
(ROOT / 'assets/images/pocket-co-images.c').write_text('\n'.join(out))
print(f'Generated fonts: {len(symbols)} UI characters + ASCII; '
      f'{len(unique)} unique RGB565 frames, {len(unique) * width * height * 2} bytes in Flash.')
