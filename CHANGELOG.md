# Changelog — Trail Marker (the mod)

Covers `trail_marker.cpp` and the on-disk `.bin` format it writes. The viewer
(`viewer/`) changes independently and isn't tracked here.

## 1.1.0 — 2026-07-08
- Added a per-record mission flag (`flags` bit `0x80`, `F_MISSION`): set while
  `GET_MISSION_FLAG` reports the game's "can't save now" lock, i.e. inside a
  scripted story/stranger mission. Verified in-game against a real play session
  (a couple of story missions plus several stranger missions). Does not cover
  ambient random encounters, which don't invoke the same lock.
- **Non-breaking:** uses a previously-reserved bit in the existing `flags` byte.
  No new field, no record-size change, file-format version stays `1`. Old
  `.bin` files keep reading exactly as before (the bit is simply never set on
  records written by older builds); old `TrailMarker.asi` builds keep writing
  files new readers can parse.

## 1.0.0 — 2026-06-18
- First public release. Position/time/transport/honor/character/cash per
  record, non-gameplay/keepalive/combat/wanted/dead/segment_start flags,
  savegame-reload detection via `segment_start`, keepalive heartbeat while
  stationary.
