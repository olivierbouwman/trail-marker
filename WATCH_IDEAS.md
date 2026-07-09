# Pocket Apple Watch — idea & feasibility notes

> **Status:** brainstorm / feasibility notes. Nothing built yet. Side-project idea spun out of Trail Marker.

A side-project concept: replace Arthur/John's **pocket watch** with an anachronistic **Apple Watch**. Open the watch → instead of the time, you see activity rings, heart rate, workouts, and health metrics. The joke is the anachronism; the point is layering Apple-Watch-style **gamification** onto RDR2 free roam.

## The core idea

The watch's value isn't the readout — it's the **behavioral loop** Apple Watch nails: daily goals + streaks + surprise nudges + a collection of medals you didn't know you wanted. It imposes a soft goal structure on open-world wandering — *"be a healthy, active outlaw"* — and rewards it with its own currency (rings, streaks, medals, celebrations).

It can't grant in-game cash/items (would need far more invasive natives) — so rewards are **meta**: dopamine, streaks, a persistent profile. That's the same deal as a real Apple Watch.

### Two-part product ("RDR2 Fitness+")

1. **In-game watch HUD** — open pocket watch → Apple Watch face: live rings, heart rate, nudges, workout summaries, celebrations.
2. **Companion web "Fitness app"** — extends the existing Trail Marker web viewer: activity history, ring streaks, medals earned, and a **workout log where each entry replays the actual route on the map** (reuses the trail data + replay we already have).

## The three rings — reskinned to be *fun* to fill

Apple's "Stand" ring is the annoying one; swap it for exploration (the best part of RDR2).

| Ring | Cowboy version | Fills when… | Why it's fun |
|---|---|---|---|
| Move (red) | **Miles** | distance traveled (foot weighted higher than horse) | rewards the scenic route |
| Exercise (green) | **Grit** | high-exertion minutes — sprint, swim, climb, gunfights, low stamina core | rewards action & chaos |
| Stand (blue) | **Frontier** | distance covered in *unvisited* terrain | rewards exploring, not grinding |

Travel / action / discovery — the three core RDR2 loops, all readable from position + transport + combat + cores. Close all three → cowboy celebration (hat toss). **Streaks** of perfect days are the retention hook.

*Open question:* third ring could instead be **Deeds/honor** or a hunting ring (hunting is hard to observe — see constraints).

## Other Apple Watch features → cowboy mappings

- **Auto-detected workouts + summary cards.** Mount up → "Horseback Ride" starts. End a gunfight → *"Gunfight complete — 4 min, peak HR 168, 230 cal."* Each logs to the web viewer with its route map.
- **Medals / trophies** (collection meta-game): *Iron Horse* (100 mi ridden), *Marathon Man* (longest hike), *Night Owl* (active 12–4am), *Survivor* (7 days no death), *Saint/Sinner* (honor extremes), *Globetrotter* (all 5 states), *High Roller* (cash). Plus **personal records** and **gag real-world-date medals** (a Thanksgiving turkey medal in 1899).
- **Heart rate as a tension meter, not a stat.** Resting ~60, spikes to 160+ when wanted/chased/in combat. The **Breathe/mindfulness gag**: after a bloodbath the watch buzzes *"Take a breath, partner"* and rewards sitting to bring HR down. **ECG flatlines on death.**
- **Nudges** (the behavioral tap): *"2 miles from closing Miles."* · *"You've been at camp an hour, partner."* (anti-AFK) · *"New personal best — longest ride!"* · bounty-as-notifications gag.
- **Sleep tracking.** Sleeping at camp/hotel skips the clock → detectable as a time jump → *"You slept 8 hours."*
- **Trends & competitions.** This week vs. last. No multiplayer, so: **compete against your own ghost** — once you're John, race Arthur's recorded stats. Or a fictional rival (*"Beat Dutch's weekly miles"*).
- **Fall / Crash detection** → *"Looks like you took a hard fall, partner."* on a bucked-off horse or cliff fall (sudden Z-drop + health hit). Stagecoach/wagon wrecks too.
- **Find My** → ping your horse's location, Apple-style. Useful *and* funny.
- **Noise app** → *"Loud sounds may damage your hearing"* during gunfights/dynamite.
- **Time to Walk** → guided audio stroll, reskinned as a narrated trail ride.

## How it changes play

You take the long way on foot (Miles), pick fights and hunt (Grit), ride into blank map (Frontier), bathe/eat/sleep because the watch nags, avoid dying to protect a streak, and log in at 3am for the Night Owl medal. Real gamification on top of a sandbox.

## Feasibility (verified against the rdr3 nativedb)

The current mod (`trail_marker.cpp`) is a **pure background logger** — it does **zero** on-screen drawing today. The watch is a genuinely new capability, but the native-binding plumbing (`nv0_int` / `nv1_bool` / etc. at `trail_marker.cpp:71-94`) and build setup (`build.sh`, mingw-w64 cross-compile) are reusable.

### Readable game data
- **Already read by Trail Marker:** position, transport mode (foot/horse/boat/train/swim), combat flag, wanted level, honor (script global `0x2BA2`), cash, in-game clock, character (Arthur/John), death state.
- **Health / Stamina / Dead-Eye cores** → `_GET_ATTRIBUTE_CORE_VALUE` (`0x36731AC041289BB1`), ped + core index. *(In the nativedb; untested in this repo — verify in-game.)*
- **Attribute ranks** (leveled-up tiers) → `GET_ATTRIBUTE_RANK` (`0xA4C8E23E29040DE0`), `GET_MAX_ATTRIBUTE_RANK` (`0x704674A0535A471D`), `GET_ATTRIBUTE_BASE_RANK` (`0x147149F2E909323C`).
- **Entity health value** → `GET_ENTITY_HEALTH` (ENTITY namespace).

### Must be synthesized (no native)
- **Heart rate** → derive from stamina-core drain + sprint/combat/wanted state.
- **Steps** → integrate on-foot position deltas (loop already polls position at ~1 Hz).

### Rendering (feasible)
- `DRAW_SPRITE` (`0xC9884ECADE94CB34`): texture dict + name, x/y/w/h, heading, RGBA.
- Text: `_DISPLAY_TEXT` (`0xD79334A4BB99BAD1`), `SET_TEXT_SCALE` (`0x4170B650590B3B00`), `_SET_TEXT_COLOR` (`0x50A41AD966910F03`), `SET_TEXT_CENTRE` (`0xBE5261939FBECB8C`).
- Render targets exist (`REGISTER_NAMED_RENDERTARGET` `0x98AF2BB6F62BD588`, etc.) for advanced compositing.
- **The one tricky piece — the circular rings.** Two options:
  - (a) **Custom texture pack** (`.ytd`) with arc/ring sprites — cleaner, but real asset work + packaging.
  - (b) **Procedural** — draw each ring as N small rotated sprite segments around a circle using a built-in white/dot texture. No custom assets; pragmatic MVP.

### Hard / out of scope to observe
- Hunting kills, fishing — no animal/fishing natives in use; would need research. Rings lean on movement/combat/exploration, which read cleanly.
- Bounty total — removed earlier (commit `4a54e22`); only per-region, over-reports. Stick to wanted flag + honor.

## Prior art — does this exist? No.

- **RDR2 (Nexus):** pocket-watch mods are cosmetic only (skins, RDR1 UI, save unlocks). Health/Stamina mods just tweak drain/regen mechanics — no HUD, no tracking, no gamification. *Info HUD Remover* strips the time/temp HUD "so the pocket watch becomes useful again" — signal that players want the watch to do more. Zero hits for rings/steps/heart-rate/fitness/smartwatch.
- **GTA V (adjacent):** *Fitness & Vitality*, *VitalityPlus+* (has a "Rings & Cores" circular HUD), *VHUD* — but all framed as **survival-needs simulation** (hunger/thirst/supplements), not Apple-Watch gamification. No workout log, HR drama, medal collection, or anachronism gag.
- **Gap / differentiator:** nobody has built the Apple Watch **behavioral loop** (rings-as-goals, streaks, collectible medals, auto-workouts, HR-as-tension) + the **anachronism humor** + a **companion fitness web app**. GTA fitness mods being popular shows appetite for the genre.

## Open decisions (for later)

- **First deliverable:** visual mockup (HTML/CSS artifact) first vs. straight to in-game proof-of-concept.
- **Metric fidelity:** real cores + synthesized HR/steps vs. only rock-solid data vs. maximal kitchen-sink.
- **Code home:** separate ASI module (e.g. `PocketWatch.asi`, shares native-binding pattern) vs. inside `trail_marker.cpp`.
- **Ring trio:** Miles/Grit/Frontier vs. swap Frontier for Deeds/honor.
- **Day definition / pacing:** in-game days (close rings several times per session — frequent dopamine) vs. real-session days (slower, more "real").
- **Reward scope:** meta-only (medals/streaks) vs. eventually affecting the game (more invasive).
- **Ring rendering:** procedural segments (no assets, MVP) vs. custom `.ytd` texture pack (nicer).
- **Web companion:** core to the vision vs. stretch goal vs. in-game only.

## Reference links

- RDR3 native DB: https://alloc8or.re/rdr3/nativedb/ · mirror https://www.rdr2mods.com/nativedb/
- GTA V cousins (rendering/UX reference): VitalityPlus+ https://www.gta5-mods.com/scripts/vitalityplus-net
