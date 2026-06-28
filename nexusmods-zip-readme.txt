TRAIL MARKER  v1.0.0
A passive travel logger for Red Dead Redemption 2 (Story Mode)
================================================================

Trail Marker quietly records where you go in RDR2 while you play, then lets
you see your whole journey drawn on the map. It runs in the background and
logs your position and a little context (about once a second) to a tiny file.
No menus, no hotkeys, nothing to configure -- install it once and forget it's
there.

Later, drop your log onto the web viewer and watch your playthrough unfold:
how you traveled, where you fought and died, your honor rising and falling,
and your wallet over time.

Web viewer (no install, nothing uploaded -- your data is read entirely in
your browser):
    https://olivierbouwman.github.io/trail-marker/


REQUIREMENTS
------------
- Red Dead Redemption 2 (PC, Story Mode only -- do NOT use mods in RDR Online).
- ScriptHookRDR2 V2, matching your game version. Tested on RDR2 build 1491.50.
    https://www.nexusmods.com/reddeadredemption2/mods/1472


INSTALL
-------
1. Install ScriptHookRDR2 V2 (download from its page above -- it is not
   bundled here): its loader (e.g. dinput8.dll) and ScriptHookRDR2.dll go in
   your RDR2 folder (the one with RDR2.exe).
2. Copy TrailMarker.asi (from this archive) into that same RDR2 folder,
   alongside ScriptHook.
3. Launch the game and play. Your log is written to TrailMarker.bin in that
   same folder.

To view it, open the web viewer above and drag TrailMarker.bin onto the page.
To stop logging, just remove TrailMarker.asi.

Heads-up: some antivirus flags .asi/ScriptHook plugins as a false positive.
Trail Marker is open source -- build it yourself if you'd rather. Use at your
own risk.


WHAT IT RECORDS
---------------
- Set-and-forget logging -- runs in the background, no menus or hotkeys.
- Smart sampling -- a point when you've actually moved, plus an occasional
  point while you're still, so travel is dense and idle time stays tiny.
- Transport mode for every point: foot, horse, boat, train, balloon,
  swimming, or other vehicles.
- Context -- in combat, wanted by the law, deaths, cutscenes, plus your
  honor, character (Arthur / John), and cash.
- Save-aware -- marks session boundaries so the viewer can separate save
  branches and pinpoint where you died.
- Tiny -- a compact binary log (~65 MB even after 1000 hours), flushed
  continuously so a crash costs almost nothing.


CREDITS
-------
- kepmehz -- ScriptHookRDR2 V2, the runtime this plugs into.
- Alexander Blade -- the original ScriptHookRDR2 and its API, which V2 builds on.
- alloc8or -- the RDR3 native database.
- femga / rdr3_discoveries -- the vehicle list used to classify how you travel.
- Rockstar Games -- the in-game map tiles the viewer draws on.
- Jean Ropke -- the coordinate transform that maps game positions onto tiles.
- Leaflet -- the open-source mapping library the web viewer is built on.
- Built with Claude (Anthropic's AI) as a coding partner.
- Everyone in the RDR2 modding community who shares knowledge and tools.


LICENSE / SOURCE
----------------
Open source, public domain (The Unlicense).
    https://github.com/olivierbouwman/trail-marker
