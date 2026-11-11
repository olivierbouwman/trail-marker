# Trail Marker Viewer

A static, no-server web viewer for `TrailMarker.bin` logs. Renders your travelled path
on the RDR2 map (Rockstar Games tiles). Hostable on GitHub Pages as-is.

## Use

Open `index.html` in a browser (double-click works — no server needed), then **drag a
`TrailMarker.bin` onto the page** or pick it with the file chooser. It draws one line per
session (it breaks the path at each game launch / savegame load so reload-teleports don't
draw across the map) and fits the view to your travels.

A small sample log, `TrailMarker.bin`, ships next to this README — drag it onto the viewer
to see it work without playing first. The file is read entirely in your browser; nothing is
uploaded anywhere.

> Hosted copy: **https://olivierbouwman.github.io/trail-marker/** (served from this repo via
> GitHub Pages — same single-file viewer, nothing to install).

## Status

Working: the **track line** (split per session), **switchable coloring** (transport / honor /
money / combat) with an adaptive legend, **death markers** (exact where the game flagged a
death, plus ones inferred from respawns), and **hover details** (time, transport, honor, cash,
flags).

**Map styles:** Classic (Rockstar's own tiles) or Detailed (the same map with every road,
trail and railway drawn bold, from RDR2Map). Detailed is hosted in `tiles/detailed/` so the
poster export keeps working with it. The choice is remembered per browser.

**Show** checkboxes. Every point belongs to exactly one category, so each box means "show this
kind of point" and checked boxes add up: **cutscenes**, **mission travel**, **free roam**,
**post-reload branches** (play undone by loading a save) and **benchmark runs** (sessions
recorded while RDR2's built-in benchmark played). **Fast-travel jumps** and **death markers**
draw over the visible trail, or over all real play when no trail box is checked.

**Guarma** is drawn as a framed inset in the empty sea at the bottom right of the map (and of
the poster), using the in-game map of the walkable part of the island. It, and its checkbox
under Show, only appear when the log has been there; unchecking it hides the island and its
trail. Calibration is from known landmark coordinates (see the `GUARMA` block in
`index.html`) and hasn't yet been checked against a real Chapter 5 log.

**Playthroughs:** a new game (Colter, in-game 1899-01-05 08:00, $0) starts a new playthrough.
When a log holds more than one, a multi-select appears, defaulting to the latest; the map
title, colors, replay and exports cover only the selection.

## Performance & large logs (known limitation)

The logger is designed to produce very large files — roughly **3.6M records (~65 MB) per
1000 in-game hours**. The current viewer is **not** built for that scale and will become
sluggish or crash the browser tab on a full multi-thousand-hour playthrough. The bottlenecks,
in rough order of impact:

- **Hover hit-testing is O(n) per mouse move** — `showHover()` linearly scans every point on
  each `mousemove`. At millions of points this stalls the UI. Fix: build a **spatial index**
  (uniform grid or quadtree) once per load and query that instead.
- **One Leaflet polyline per colour-run, and runs split on every colour change** — in the
  honor/cash modes the quantised colour changes constantly, producing huge numbers of tiny
  2-point polylines (plus a `circleMarker` per death). Fix: render the track to a **single
  canvas/WebGL layer** (`L.canvas` / a GL overlay) rather than thousands of SVG/vector layers.
- **No level-of-detail** — every point is materialised regardless of zoom. Fix: **decimate by
  zoom** (e.g. Douglas–Peucker / Ramer line simplification per zoom level, or grid-snapping),
  so far-out views draw a simplified track and only close-in views draw full detail.
- **Whole-file parse into JS objects up front** — fine at a few hundred MB but adds memory
  pressure. Optional later: parse on a Web Worker / keep records in typed arrays instead of an
  array of objects.

Until those land, the viewer is comfortable with logs up to roughly the low hundreds of
thousands of points (tens of in-game hours). Larger logs work best after pre-decimation.

## Roadmap / ideas

These all work on the **existing** log — no new data needs to be captured:

- **Stats dashboard** — total distance travelled (excluding fast-travel jumps, which we
  already detect), broken down by transport (foot vs. horse vs. train …), number of deaths,
  session count, date span, honor high/low, richest/poorest moment.
- **Journey replay** — a play button + timeline scrubber that animates a marker along the
  track by in-game time; watch a whole playthrough in seconds.
- **Time-spent heatmap** — where you actually lingered. Weight each point by the time gap to
  the next record (dwell time), *not* by point count, so dense fast-travel routes don't fake
  a hotspot and a campsite doesn't read as cold.
- **Honor & cash over time** — small sparkline charts: your moral arc and your wallet across
  the playthrough.
- **Speed coloring** — a colour mode derived from position/time deltas (skipping teleports).
