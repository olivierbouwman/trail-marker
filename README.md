# Trail Marker

A lightweight Red Dead Redemption 2 mod that **passively logs where you go**. It runs in
the background and records your world position — plus a little context — about once a second
to a compact binary file, so you (or a companion tool) can later draw movement tracks,
heatmaps, and retrospective stats across an entire playthrough.

It's a native ScriptHook plugin (`.asi`) that loads automatically at game launch. No menus,
no hotkeys, no per-session steps. It's built to be unnoticeable in normal play and to handle
logs spanning thousands of hours (~65 MB for 1000 h, worst case).

## Features

- **Automatic background logging** of player position (X, Y, Z) and in-game time.
- **Smart sampling** — records a point when you've actually moved, plus a periodic
  "keepalive" point while stationary, so travel is densely captured and idle time stays tiny.
- **Transport mode** per point: foot, horse, boat, train, balloon, swimming, or other vehicle.
- **Context flags**: cutscene/non-gameplay, in combat, wanted, dead, keepalive, session-start.
  (The `wanted` flag captures active law pursuit; a dollar bounty figure was dropped — the
  game exposes no clean way to read your total across regions. See the format notes.)
- **Honor** (−100…+100), **character** (Arthur / John / other), and **cash** (whole dollars).
- **Save-aware**: session boundaries are marked so a viewer can separate playthrough branches
  (e.g. after loading an earlier save) and reconstruct death locations.
- **Compact & robust**: fixed-size little-endian records, append-only, periodic flush; a crash
  costs at most one record. Negligible performance impact.

## Requirements

- Red Dead Redemption 2 (PC, Story Mode).
- **ScriptHookRDR2 V2** installed (this mod depends on it).

## Install

1. Install ScriptHookRDR2 V2 (its loader, e.g. `dinput8.dll`, and `ScriptHookRDR2.dll`).
2. Copy **`TrailMarker.asi`** and **`TrailMarker.ini`** into your RDR2 folder (the one with
   `RDR2.exe`), alongside ScriptHook.
3. Launch the game and play. The log is written to **`TrailMarker.bin`** in the same folder.

To stop logging, remove `TrailMarker.asi`. Story Mode only — ScriptHook does not run in RDR Online.

## Configuration — `TrailMarker.ini`

| Key | Default | Meaning |
|---|---|---|
| `sampleIntervalMs` | `1000` | how often to check position (real time, framerate-independent) |
| `movementThreshold` | `3.0` | world units of movement that trigger a recorded point |
| `keepaliveSeconds` | `45` | record at least this often while stationary (flagged) |
| `flushSeconds` | `30` | how often buffered records are flushed to disk |
| `outputPath` | *(next to the .asi)* | override the output file location |

## Output format — `TrailMarker.bin`

Little-endian. An 8-byte header, then fixed **18-byte records** — so tools can index by
offset (`recordCount = (fileSize − 8) / 18`) with no parsing.

**Header**

| Offset | Type | Field |
|---|---|---|
| 0 | `char[4]` | magic `"R2GP"` |
| 4 | `uint16` | version (`1`) |
| 6 | `uint16` | record size (`18`) |

**Record**

| Offset | Type | Field | Notes |
|---|---|---|---|
| 0 | `uint32` | `ingame_time` | seconds since in-game `1800-01-01` (in-game clock, not real time) |
| 4 | `int16` | `x` | world coord × 2 (divide by 2 for units) |
| 6 | `int16` | `y` | × 2 |
| 8 | `int16` | `z` | × 2 |
| 10 | `uint8` | `transport` | 0 foot · 1 horse · 2 boat · 3 train · 4 balloon · 5 swim · 6 other_vehicle |
| 11 | `uint8` | `flags` | bitfield (below) |
| 12 | `int8` | `honor` | −100…+100 (0 = neutral) |
| 13 | `uint8` | `character` | 0 Arthur · 1 John · 2 other |
| 14 | `uint32` | `cash` | whole dollars |

**Flag bits:** `1` non-gameplay (cutscene) · `2` keepalive · `4` combat · `8` wanted ·
`16` dead/dying · `32` segment_start (first point of a launch or savegame load) ·
`64` orphaned (reserved for tools, not written by the mod).

**Notes for tool authors**

- The file is append-only and record order is true real-world chronology; `ingame_time` is a
  label and may jump (sleep, fast-travel, death, or a savegame load).
- `segment_start` marks each game launch / savegame load. To hide an abandoned branch (e.g.
  you reached an area, then loaded an earlier save), drop records whose `ingame_time` is later
  than the time at a subsequent `segment_start`.
- `dead` is best-effort (the death window is brief). The reliable way to find **death locations**
  is the respawn that always follows a real death — a backward `ingame_time` jump + position
  teleport with **no** `segment_start`; the death spot is the last record before it.

## Viewing & reading the data

- **`viewer/`** — a static, no-server web viewer that draws your travels on the RDR2 map
  (colored by transport / honor / money / combat, with death markers, hover
  details, and filters). Open `viewer/index.html` and drag a `TrailMarker.bin` onto it.
  See `viewer/README.md`.
- **`read_trailmarker.py`** — a tiny dependency-free reference decoder for the binary format
  (quick CLI inspection, and a worked example for anyone writing their own tool):

```sh
python3 read_trailmarker.py TrailMarker.bin          # summary + first/last records
python3 read_trailmarker.py TrailMarker.bin --all    # dump every record as CSV
```

## Building from source

Cross-compiles to a Windows `.asi` from any platform with **mingw-w64** — no Windows, MSVC,
or ScriptHook SDK required (ScriptHook entry points are resolved at runtime).

```sh
brew install mingw-w64                 # macOS (or your platform's mingw-w64 package)
./build.sh                             # -> TrailMarker.asi
# set GAME=/path/to/RDR2 to auto-deploy for testing
```

## Credits

- **ScriptHookRDR2 V2** — the scripting runtime this mod plugs into.
- **alloc8or** — the RDR3 native database (function hashes).
- **femga/rdr3_discoveries** — the vehicle model list used to classify transport.

## License

MIT — see `LICENSE`.
