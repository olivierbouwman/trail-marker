#!/usr/bin/env python3
"""Reference decoder for Trail Marker log files (TrailMarker.bin).

Usage:
    python3 read_trailmarker.py TrailMarker.bin            # summary + first/last rows
    python3 read_trailmarker.py TrailMarker.bin --all      # dump every record as CSV

The file is little-endian, with an 8-byte header followed by fixed 18-byte records:

    Header : char[4] magic "R2GP" | uint16 version | uint16 recordSize
    Record : uint32 ingame_time   (seconds since in-game 1800-01-01)
             int16  x, y, z        (world coord * 2; divide by 2 for units)
             uint8  transport      0 foot,1 horse,2 boat,3 train,4 balloon,5 swim,6 other_vehicle
             uint8  flags          bit0 non-gameplay, 1 keepalive, 2 combat, 3 wanted,
                                   4 dead, 5 segment_start, 6 orphaned (set by tools),
                                   7 mission (inside a scripted story mission)
             int8   honor          -100..+100 (0 neutral)
             uint8  character      0 Arthur, 1 John, 2 other
             uint32 cash           whole dollars
"""
import struct, sys

HEADER = struct.Struct("<4sHH")     # 8 bytes
RECORD = struct.Struct("<IhhhBBbBI")  # 18 bytes
TRANSPORT = {0: "foot", 1: "horse", 2: "boat", 3: "train", 4: "balloon", 5: "swim", 6: "other"}
FLAGS = ["nongameplay", "keepalive", "combat", "wanted", "dead", "segment_start", "orphaned", "mission"]


def flag_names(f):
    return "|".join(n for i, n in enumerate(FLAGS) if f & (1 << i)) or "-"


def records(path):
    with open(path, "rb") as fh:
        magic, version, rsize = HEADER.unpack(fh.read(HEADER.size))
        assert magic == b"R2GP", f"bad magic {magic!r}"
        assert rsize == RECORD.size, f"unexpected record size {rsize}"
        yield ("header", version, rsize)
        while True:
            buf = fh.read(RECORD.size)
            if len(buf) < RECORD.size:
                break  # ignore a torn trailing record
            t, x, y, z, transport, flags, honor, character, cash = RECORD.unpack(buf)
            yield dict(t=t, x=x / 2, y=y / 2, z=z / 2, transport=transport,
                       flags=flags, honor=honor, character=character, cash=cash)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return
    path = sys.argv[1]
    dump_all = "--all" in sys.argv[2:]
    it = records(path)
    _, version, rsize = next(it)
    rows = list(it)
    print(f"# {path}: format v{version}, {len(rows)} records")
    if dump_all:
        print("ingame_time,x,y,z,transport,flags,honor,character,cash")
        for r in rows:
            print(f"{r['t']},{r['x']:.1f},{r['y']:.1f},{r['z']:.1f},"
                  f"{TRANSPORT.get(r['transport'], r['transport'])},{flag_names(r['flags'])},"
                  f"{r['honor']},{r['character']},{r['cash']}")
        return
    if not rows:
        return
    from collections import Counter
    tc = Counter(TRANSPORT.get(r["transport"], r["transport"]) for r in rows)
    print("transport:", dict(tc))
    print("honor range:", min(r["honor"] for r in rows), "..", max(r["honor"] for r in rows))
    print("segment_starts:", sum(1 for r in rows if r["flags"] & 0x20))

    def show(label, r):
        print(f"  {label}: t={r['t']} pos=({r['x']:.1f},{r['y']:.1f},{r['z']:.1f}) "
              f"{TRANSPORT.get(r['transport'], r['transport'])} [{flag_names(r['flags'])}] "
              f"honor={r['honor']} char={r['character']} ${r['cash']}")
    for r in rows[:3]:
        show("first", r)
    for r in rows[-3:]:
        show("last", r)


if __name__ == "__main__":
    main()
