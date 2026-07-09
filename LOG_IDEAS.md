# Log Ideas — new data points to record

> **Status:** feasibility notes, nothing built. Candidates for extending what `trail_marker.cpp` records per position sample. Verdicts verified against alloc8or rdr3-nativedb + femga/rdr3_discoveries (2026-07-05).

## Player kill counter — ✅ ship-able
Detect "the player killed something" and keep a running count on the record.
- Enumerate nearby peds: `GET_PED_NEARBY_PEDS` `0x23F8F5FC7E8C4A6B` (fills a caller `Any*` array).
- On a ped's alive→dead transition (`IS_ENTITY_DEAD` `0x7D5B1F88E7504BBA`), attribute via `GET_PED_SOURCE_OF_DEATH` `0x93C8B64DEB84728C` — if killer == player ped, increment.
- Split animal/human free: `IS_PED_HUMAN` `0xB980061DA992779D` / `GET_PED_TYPE` `0xFF059E1E4C01E63C`. Lawman via `GET_PED_RELATIONSHIP_GROUP_HASH` `0x7DBDD04862D95F04` vs `REL_COP` etc.
- **Caveats:** poll faster than the current ~1 Hz (corpses despawn); indirect kills (fire/explosion) may misattribute — undercounts, never over. Kill-count STAT shortcut NOT viable (RDR2 StatId is a struct, no name→id lookup).
- **Cost:** new record field → file-format v2.

## Mission flag per record — ✅ BUILT (2026-07-08), verified in-game (2026-07-08)
Tag each record as story-mission vs free-roam. Implemented in `trail_marker.cpp` as the
`0x80` bit (`F_MISSION`); decoders (`read_trailmarker.py`, viewer, README) updated.
- `GET_MISSION_FLAG` `0xB15CD1CF58771DE1` (RDR3 hash, *not* the GTA5 one; no params, BOOL). It's the "can't save" lock — reliable during scripted missions.
- Mission **start points** fall out of the flag flipping 0→1.
- **Confirmed working** on a real play session (a couple of story missions + several stranger/NPC
  missions): 7 distinct mission-flagged spans, matching the save-lock semantics — flag rises at
  mission accept, holds through the mission (including its internal clock-sets/cutscenes), drops
  at completion/failure.
- **Scope limitation found:** it does *not* cover every "mission-shaped" thing the player does.
  Rescuing and escorting the kidnapped woman to Emerald Ranch (a random/ambient encounter, not a
  formal mission) did **not** get flagged for its travel leg — consistent with random encounters
  not invoking the same save-lock as a real mission. This is expected given what the native
  actually gates, not a bug; worth calling out so nobody mistakes ambient-encounter travel for a
  gap later. Whether it ever spuriously trips on non-mission cutscenes/tutorials remains untested
  either way.

## Camp — ⚠️ feasible, needs a one-time hunt
No single-player camp native (the `GANG` namespace is all Red Dead Online).
- **Mobile/portable camp (priority):** `GET_CLOSEST_OBJECT_OF_TYPE` `0xE143FA2249364369` vs the campfire/tent prop model hash — must identify that hash empirically first (pitch a camp, enumerate nearby object models).
- **Gang story camps (secondary):** fixed per-chapter coordinate table + proximity; gather coords from own logs (wikis don't publish internal XYZ).

## Out of reach
- **Which** mission (no name getter) and **chapter number** (no global found). Blip-enumeration natives are stripped from RDR2.

## Before shipping — in-game verification passes (not code risk)
1. Campfire/tent prop model hash.
2. ~~`GET_MISSION_FLAG` coverage on a couple of known missions.~~ Done — see above; covers formal
   missions, not ambient random encounters.

Same empirical approach that pinned down the honor (`0x2BA2`) and cash globals.
