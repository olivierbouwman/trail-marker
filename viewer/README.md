# Trail Marker Viewer

A static, no-server web viewer for `TrailMarker.bin` logs. Renders your travelled path
on the RDR2 map (Jean Ropke tiles). Hostable on GitHub Pages as-is.

## Use

Open `index.html` in a browser (double-click works — no server needed), then **drag a
`TrailMarker.bin` onto the page** or pick it with the file chooser. It draws one line per
session (it breaks the path at each game launch / savegame load so reload-teleports don't
draw across the map) and fits the view to your travels.

## Status

Working: the **track line** (split per session), **switchable coloring** (transport / honor /
money / bounty / combat) with an adaptive legend, **death markers**, **hover details** (time,
transport, honor, cash, bounty, flags), and a click-to-read calibration helper.

Filters: **hide cutscenes** (non-gameplay points) and **hide post-reload branches** (records
abandoned by a later savegame load). Point count shows kept/total.

Next up: a time-spent heatmap, and a timeline scrubber.

## Calibration

The game→map transform (`CAL` at the top of `index.html`) is a best estimate. If the
track doesn't line up with map features, that's the knob to tune — three numbers
(`scale`, `offX`, `offY`).
