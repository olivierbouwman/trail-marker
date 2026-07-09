# Keepsake Ideas — a physical memento of a playthrough

> **Status:** brainstorm, nothing built. A print (or object) that embodies one player's RDR2 journey, in a style/feel true to the game. Driven by the data Trail Marker records (route, honor, deaths, timing, cash — see [LOG_IDEAS.md](LOG_IDEAS.md) for candidate additions like kills, mission stretches, camps).

## Guiding principle
The differentiator is the **actual trail** — an unfakeable record of *this* playthrough. Abstraction keeps it legible at any scale (an 8-hour run and an 800-hour 100% run should both read well). Don't invent a modern abstract style — borrow a period one: let **material and mark-making** (aged paper, ink, wear, muted ochre/oxblood/sepia/gunmetal palette) carry the RDR2 feel, and let the **data** live in the abstraction.

## Formats
1. **Journey route map** *(CHOSEN — see plan below)* — the player's route inked over the RDR2 map; thicker/darker where they lingered, faded where they passed once. Symbols: graves (deaths), tents (camps), stars (story beats). Hand-lettering, compass rose, scale bar, legend, cartouche. Most "unfakeably yours."
2. **Trail-as-art print** — just the route line on aged paper; no base map. Western "Strava-art." Sidesteps map-copyright concerns; scales gracefully.

## Route-map plan (the chosen direction)
Reference mockup style: AI-generated "Trail of J.B. Harcourt" USGS-style aged territorial map. That's the *look* target — the *geometry* comes from real logged data. Never trace AI-invented terrain.

**Base:** the actual RDR2 map (reuse the viewer's canonical map transform), composited with a worn overlay — sepia/desaturate wash, paper grain, edge foxing/darkening, subtle torn or singed border.

**Two independent visual channels (so they don't fight over one line):**
- **Line weight / opacity = how often traveled** — record density per segment (~1 pt/sec on movement). Well-traveled = darker/thicker, passed-once = faint.
- **Hue = the player-selected variable** (see below).

**Player-selectable trail variable:**
- **"Just the journey"** *(default)* — path, frequency-shaded only. Data we already have.
- **Honor** — hue gradient along the line (oxblood ↔ pale). Honor field already logged. The emotional pick — a redemption arc in ink.
- **Kills** — tint/thicken once kill-logging ships (see [LOG_IDEAS.md](LOG_IDEAS.md)); may just feed the cartouche.
- **Dropped:** transport type, money — not poster-worthy.

**Static furniture:** compass rose, scale bar, legend/"Explanation" (lists only glyphs present in that run), cartouche.

**Cartouche — IMPLEMENTED in the viewer** (`viewer/index.html`, `buildCartouche`/`cartoucheText`; art at `viewer/assets/cartouche.png`). Rendered as a Leaflet SVG overlay anchored in the NW margin (`CART_BOUNDS`), pane-level `mix-blend-mode: multiply` drops the art's white ground onto the map paper, text auto-compresses to fit the frame. Composed dynamically from the loaded file as below.

**Cartouche — dynamically composed from the loaded `.bin`.** Style: a period map-credit block (like the mockup's "Trail of J.B. Harcourt" cartouche), NOT a stat dump. **Mostly-narrative** tone chosen; place-name lookup tables (regions/gazetteer) skipped for now. Hard cap ~10 short lines; **omit any line whose data is zero/unknown** (no "0 camps") so it shrinks gracefully.

Template (`[dynamic]` = filled from the bin) — **lean title** (no "MAP / OF A PORTION OF…" masthead):
```
   THE TRAIL OF  [ARTHUR MORGAN]
             — · —
     [Two hundred fourteen miles]
        over [sixty-three days],
          [in the year 1899],
     [who kept to the honest path]
```
Full playthrough (both protagonists + honor arc):
```
   THE TRAIL OF  [ARTHUR MORGAN &
           JOHN MARSTON]
             — · —
    [Two thousand one hundred miles]
        [in the years 1899–1907],
   [who rose from outlaw to honest man]
```

Style rules (decided):
- **Lean title** — start at "THE TRAIL OF"; no survey masthead.
- **Numbers: words in prose, figures for years** — "Two thousand one hundred miles" but "1899–1907".
- **Title = protagonists present**, joined with `&`, chronological (Arthur → John), **excluding character "other" (2)** (mission/cutscene peds).
- **Transport line CUT by default** — "afoot and on horseback" is true of every playthrough. Only surface transport if notably lopsided (e.g. "chiefly by rail").
- **Duration adapts** — short runs "over N days"; multi-year runs lean on the "in the years YYYY–YYYY" span (days get silly across years).
- **Omit any line whose data is zero/unknown** (no "0 camps") — cartouche shrinks gracefully. Hard cap ~8 lines.

Derivable NOW from the current record format:
- **Character** → title (Arthur / John / both).
- **Date span** → first→last `ingameTime` as in-game calendar.
- **In-game days elapsed** → `(last − first)/86400`.
- **Distance** → sum of position deltas (minus the fast-travel jumps / death breaks the viewer already filters); on-foot vs horseback splittable. Flavor, not survey-accurate — scale bar is decorative.
- **Sessions** → count of `segment_start` flags.
- **Deaths** → viewer detects them, but **kept OUT of the cartouche** — a death tally reads as a scoreboard, not a keepsake (they're respawns, not real graves). May still appear as optional map markers; decide later.
- **Honor closing line — the emotional payload.** Use the **arc when significant** (start vs end honor differ meaningfully → "rose from outlaw to honest man" / "fell from grace"); otherwise fall back to a final-state phrase mapped to RDR2 honor tiers: very high "lived honestly to the end" · high "kept to the honest path" · neutral "walked the line between" · low "lived by the gun" · very low "answered to no law". Never a number.

Needs new logging (see [LOG_IDEAS.md](LOG_IDEAS.md)): **kills**, **camps made**, **mission time**.

NOT in the file — do not fake it: **real-world hours played** (only in-game time is logged; session gaps aren't recorded). Use in-game days.

Deferred (needs static lookup tables, skipped for now): region wording ("Across New Hanover & Lemoyne"), named-place endpoints ("From the Grizzlies to Beecher's Hope").

**Print spec:**
- Primary deliverable: **300 DPI PDF, 24×36″ landscape (3:2), with 0.125″ bleed**. For pro/wide-format shops export **PDF/X-1a or PDF/X-4, CMYK**. Fit the map's aspect to the nearest standard size, pad with decorative border to hit it exactly.
- Also emit a **high-res PNG (sRGB)** for home printing / on-screen preview.
- PDF is the universal print-shop format; 300 DPI PNG/TIFF in sRGB also accepted by consumer POD everywhere. ISO A2/A1 cover non-US shops.

## Period graphic languages that read as abstract
Cartography (contours, hatching, cartouches) · steel-engraving / banknote guilloche (scales for free) · cattle-brand / maker's-mark glyphs · ledger/almanac grids · almanac star-charts · frontier quilt/textile geometry.

## Where the new log data could surface
- **Kill tally** → engraved in the map cartouche, or a small tally-mark motif (bonus: split animal/human/law).
- **Mission vs free-roam** → mission stretches of the trail inked differently from wandering.
- **Camps** → tent glyphs where the player pitched camp.

## Open decisions
- **Audience:** one-off (hand-tuned per run) vs self-serve (viewer auto-generates a print-ready file for any mod user) — undecided.
- **Fabrication:** print-at-home PDF vs print-on-demand vs digital-only — undecided.
- **Map licensing:** using the actual RDR2 map for a printed poster — fine for personal one-offs; revisit if this ever ships publicly or for sale.
- **Style pass:** AI mockups drafted (cartographic, engraving, cattle-brand prompts) to lock the *look* before rendering real data programmatically. Keep the two separate — don't fall for an AI-invented route shape.
