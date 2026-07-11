#!/usr/bin/env python3
"""Build the two cartouche pieces from Arthur's journal sketches (RDR2 game art).

Sources: viewer/assets/journal/ if present, else ~/Downloads/AM-Journal-Entries/story/
(journal-art rip via https://www.aicosu.co/arthurmorgan).

Outputs two independently placed assets, both trimmed to their ink and kept at native
resolution (the viewer scales them in SVG units):
  assets/cartouche_top.png    - the "RIDING OUT" scene from 0.9.png
  assets/cartouche_bottom.png - the pencil rule + steer head exactly as drawn on 27.png

The viewer (buildCartouche in index.html) flows top art / title text / bottom art
vertically with fixed gaps, so the bottom piece rides up or down with the amount of
text. The CART_TOP/CART_BOT aspect ratios printed below are hardcoded there.

Pipeline: luminance -> ink alpha (white point clamps out paper noise, gamma lifts the
soft graphite mid-tones so the art holds up at map scale), then a duotone ink ramp:
light hatching stays the map's line colour #ae987a, dense ink leans toward the map's
label colour #4e473f so focal darks keep their punch.
"""
import os
import numpy as np
from PIL import Image

# Prefer a curated in-repo copy if present; fall back to the raw rip in Downloads.
_CANDIDATES = [
    os.path.join(os.path.dirname(__file__), 'assets', 'journal', ''),
    os.path.expanduser('~/Downloads/AM-Journal-Entries/story/'),
]
SRC = next(p for p in _CANDIDATES if os.path.isdir(p))
DST = os.path.join(os.path.dirname(__file__), 'assets')
LIGHT = np.array([0xAE, 0x98, 0x7A], np.float32)
DARK = np.array([0x4E, 0x47, 0x3F], np.float32)


def ink_alpha(im, wp=250.0, bp=40.0, gamma=0.75):
    g = np.asarray(im.convert('L')).astype(np.float32)
    return np.clip((wp - g) / (wp - bp), 0, 1) ** gamma


def save_duotone(alpha, name):
    alpha = alpha.copy()
    alpha[alpha < 0.03] = 0
    # trim to ink bbox with a small margin
    ys, xs = np.where(alpha > 0.03)
    y0, y1 = max(ys.min() - 8, 0), min(ys.max() + 9, alpha.shape[0])
    x0, x1 = max(xs.min() - 8, 0), min(xs.max() + 9, alpha.shape[1])
    alpha = alpha[y0:y1, x0:x1]
    t = (alpha ** 1.5)[..., None]
    rgb = LIGHT * (1 - t) + DARK * t
    out = np.zeros((*alpha.shape, 4), np.uint8)
    out[..., :3] = rgb.round().astype(np.uint8)
    out[..., 3] = (alpha * 255).round().astype(np.uint8)
    path = os.path.join(DST, name)
    Image.fromarray(out).save(path, optimize=True)
    h, w = alpha.shape
    print(f'{name}: {w}x{h} (aspect {w / h:.4f})')


save_duotone(ink_alpha(Image.open(SRC + '0.9.png')), 'cartouche_top.png')
# 27.png: the rule + steer head live below the handwriting; crop just that block. On the
# page the steer sits left of the rule's centre, so re-centre it: blank its rows and
# re-paste the steer block shifted so its midline matches the rule's (the viewer centres
# the whole asset on the text axis).
bot = ink_alpha(Image.open(SRC + '27.png').crop((20, 425, 680, 700)))  # rule ~y0-75, steer below
steer = bot[85:, :].copy()
ys, xs = np.where(steer > 0.03)
rys, rxs = np.where(bot[:85, :] > 0.03)
shift = int(round((rxs.min() + rxs.max()) / 2 - (xs.min() + xs.max()) / 2))
bot[85:, :] = 0
bot[85:, :] = np.roll(steer, shift, axis=1)
save_duotone(bot, 'cartouche_bottom.png')
