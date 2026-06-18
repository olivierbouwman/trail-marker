#!/usr/bin/env bash
# Build Trail Marker (a native ScriptHook RDR2 .asi) by cross-compiling with mingw-w64,
# then optionally deploy it into a local RDR2 install for testing.
#
#   brew install mingw-w64                       # one-time (macOS; use your platform's pkg)
#   ./build.sh                                   # build -> TrailMarker.asi
#   GAME="/path/to/Red Dead Redemption 2" ./build.sh   # build + auto-deploy for testing
#
# The committed TrailMarker.asi is tracked in git, so after changing trail_marker.cpp,
# rebuild and commit the refreshed TrailMarker.asi alongside your source change.
set -euo pipefail

GAME="${GAME:-}"
CXX="${CXX:-x86_64-w64-mingw32-g++}"
OUT=TrailMarker.asi

"$CXX" trail_marker.cpp -o "$OUT" \
    -shared -O2 -s -std=c++17 \
    -static -static-libgcc -static-libstdc++ \
    -Wl,--kill-at

echo "Built $OUT ($(wc -c < "$OUT") bytes)"

if [[ -z "$GAME" ]]; then
    echo "Set GAME=/path/to/RDR2 to auto-deploy; otherwise copy $OUT into your RDR2 folder."
elif [[ -d "$GAME" ]]; then
    cp "$OUT" "$GAME/$OUT"
    echo "Deployed -> $GAME/$OUT"
else
    echo "Error: GAME is set but not a directory: $GAME" >&2
    exit 1
fi
