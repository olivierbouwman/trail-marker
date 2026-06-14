#!/usr/bin/env bash
# Build Trail Marker (a native ScriptHook RDR2 .asi) by cross-compiling with mingw-w64,
# then optionally deploy it into a local RDR2 install for testing.
#
#   brew install mingw-w64      # one-time
#   ./build.sh                  # build (+ deploy if GAME points at an RDR2 folder)
#
# Set GAME to your RDR2 folder to auto-deploy; otherwise the build artifact is left here.
set -euo pipefail

GAME="${GAME:-/Users/olivierbouwman/Library/Application Support/CrossOver/Bottles/Steam/drive_c/Program Files (x86)/Steam/steamapps/common/Red Dead Redemption 2}"
CXX="${CXX:-x86_64-w64-mingw32-g++}"
OUT=TrailMarker.asi

"$CXX" trail_marker.cpp -o "$OUT" \
    -shared -O2 -s -std=c++17 \
    -static -static-libgcc -static-libstdc++ \
    -Wl,--kill-at

echo "Built $OUT ($(wc -c < "$OUT") bytes)"

if [[ -d "$GAME" ]]; then
    cp "$OUT" "$GAME/$OUT"
    [[ -f "$GAME/TrailMarker.ini" ]] || cp TrailMarker.ini "$GAME/TrailMarker.ini"
    echo "Deployed -> $GAME/$OUT"
else
    echo "GAME folder not found; skipped deploy. Copy $OUT + TrailMarker.ini into your RDR2 folder."
fi
