// Trail Marker — Red Dead Redemption 2 player-position logger.
//
// A native ScriptHook ASI that passively records the player's world position and a
// little context (~once a second) to a compact binary file, for later analysis and
// visualization (movement tracks, heatmaps, stats). Designed to be unnoticeable in
// normal play and to handle save files spanning thousands of hours.
//
// Requires ScriptHookRDR2 V2. ScriptHook's entry points are resolved at runtime by
// their exported names, so building needs no SDK headers or import library — just a
// Windows-targeting C++ compiler (see build.sh for the mingw-w64 cross-compile).
//
// Output file format is documented in README.md and decoded by read_trailmarker.py.

#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

typedef uint64_t U64;
typedef void (*ScriptMainFn)();

// ---- ScriptHook entry points (resolved from ScriptHookRDR2.dll at load) ----
static void (*p_nativeInit)(U64);
static void (*p_nativePush64)(U64);
static U64 *(*p_nativeCall)();
static void (*p_scriptRegister)(HMODULE, ScriptMainFn);
static void (*p_scriptWait)(DWORD);
static U64 *(*p_getGlobalPtr)(int);

// A native's Vector3 return value: each float is followed by 4 bytes of padding.
struct Vector3 { float x; int _a; float y; int _b; float z; int _c; };

// ---- RAGE native hashes (alloc8or rdr3 nativedb) ----
static const U64 H_PLAYER_PED_ID            = 0x096275889B8E0EE0ULL;
static const U64 H_PLAYER_ID                = 0x217E9DC48139933DULL;
static const U64 H_GET_ENTITY_COORDS        = 0xA86D5F069399F44DULL;
static const U64 H_GET_ENTITY_MODEL         = 0xDA76A9F39210D365ULL;
static const U64 H_IS_PED_ON_MOUNT          = 0x460BC76A0E10655EULL;
static const U64 H_IS_PED_SWIMMING          = 0x9DE327631295B4C2ULL;
static const U64 H_IS_PED_IN_ANY_TRAIN      = 0x6F972C1AB75A1ED0ULL; // passengers don't count as "in vehicle"
static const U64 H_IS_PED_IN_ANY_VEHICLE    = 0x997ABD671D25CA0BULL;
static const U64 H_GET_VEHICLE_PED_IS_IN    = 0x9A9112A0FE9A4713ULL;
static const U64 H_IS_GAMEPLAY_CAM_RENDER   = 0x8660EA714834E412ULL;
static const U64 H_IS_PED_IN_COMBAT         = 0x4859F1FC66A6278EULL;
static const U64 H_GET_PLAYER_WANTED_LEVEL  = 0xABC532F9098BFD9DULL;
static const U64 H_GET_BOUNTY               = 0x54310AAB97B92816ULL;
static const U64 H_IS_PLAYER_DEAD           = 0x2E9C3FCB6798F397ULL;
static const U64 H_IS_PED_DEAD_OR_DYING     = 0x3317DEDB88C95038ULL;
static const U64 H_IS_ENTITY_DEAD           = 0x7D5B1F88E7504BBAULL;
static const U64 H_MONEY_GET_CASH_BALANCE   = 0x0C02DABFA3B98176ULL; // returns cents
static const U64 H_GET_CLOCK_YEAR           = 0xE136DCA28C4A48BAULL;
static const U64 H_GET_CLOCK_MONTH          = 0x2D44E8FC79EAB1ACULL;
static const U64 H_GET_CLOCK_DAY            = 0xDF2FD796C54480A5ULL;
static const U64 H_GET_CLOCK_HOURS          = 0xC82CF208C2B19199ULL;
static const U64 H_GET_CLOCK_MINUTES        = 0x4E162231B823DBBFULL;
static const U64 H_GET_CLOCK_SECONDS        = 0xB6101ABE62B5F080ULL;

// Player model hashes (joaat) for character identification.
static const unsigned MODEL_ARTHUR = 0x0D7114C9u; // player_zero
static const unsigned MODEL_JOHN   = 0x00B69710u; // player_three

// Honor lives in this script global (read-only here). Internal range ~ -320..+320.
static const int HONOR_GLOBAL = 0x2BA2;

// ---- Native call helpers ----
static int  nv0_int(U64 h)             { p_nativeInit(h); return (int)(*p_nativeCall()); }
static bool nv1_bool(U64 h, U64 a)     { p_nativeInit(h); p_nativePush64(a); return ((*p_nativeCall()) & 0xFF) != 0; }
static bool nv2_bool(U64 h, U64 a, U64 b) { p_nativeInit(h); p_nativePush64(a); p_nativePush64(b); return ((*p_nativeCall()) & 0xFF) != 0; }

static int      PLAYER_PED_ID()              { return nv0_int(H_PLAYER_PED_ID); }
static int      PLAYER_ID()                  { return nv0_int(H_PLAYER_ID); }
static bool     IS_PED_ON_MOUNT(int p)       { return nv1_bool(H_IS_PED_ON_MOUNT, (U64)p); }
static bool     IS_PED_SWIMMING(int p)       { return nv1_bool(H_IS_PED_SWIMMING, (U64)p); }
static bool     IS_PED_IN_ANY_TRAIN(int p)   { return nv1_bool(H_IS_PED_IN_ANY_TRAIN, (U64)p); }
static bool     IS_PED_IN_ANY_VEHICLE(int p) { return nv2_bool(H_IS_PED_IN_ANY_VEHICLE, (U64)p, 0); }
static bool     IS_ENTITY_DEAD(int e)        { return nv1_bool(H_IS_ENTITY_DEAD, (U64)e); }
static bool     IS_PED_DEAD_OR_DYING(int p)  { return nv2_bool(H_IS_PED_DEAD_OR_DYING, (U64)p, 1); }
static bool     IS_PLAYER_DEAD(int player)   { return nv1_bool(H_IS_PLAYER_DEAD, (U64)player); }
static bool     IS_GAMEPLAY_CAM_RENDERING()  { p_nativeInit(H_IS_GAMEPLAY_CAM_RENDER); return ((*p_nativeCall()) & 0xFF) != 0; }
static bool     IS_PED_IN_COMBAT(int ped, int target) { p_nativeInit(H_IS_PED_IN_COMBAT); p_nativePush64((U64)ped); p_nativePush64((U64)target); return ((*p_nativeCall()) & 0xFF) != 0; }
static int      GET_PLAYER_WANTED_LEVEL(int player) { p_nativeInit(H_GET_PLAYER_WANTED_LEVEL); p_nativePush64((U64)player); return (int)(*p_nativeCall()); }
static int      GET_BOUNTY(int player) { p_nativeInit(H_GET_BOUNTY); p_nativePush64((U64)player); return (int)(*p_nativeCall()); }
static int      MONEY_GET_CASH_BALANCE()     { return nv0_int(H_MONEY_GET_CASH_BALANCE); }
static unsigned GET_ENTITY_MODEL(int e)      { p_nativeInit(H_GET_ENTITY_MODEL); p_nativePush64((U64)e); return (unsigned)(*p_nativeCall()); }
static int      GET_VEHICLE_PED_IS_IN(int p) { p_nativeInit(H_GET_VEHICLE_PED_IS_IN); p_nativePush64((U64)p); p_nativePush64(0); return (int)(*p_nativeCall()); }
static Vector3  GET_ENTITY_COORDS(int e) {
    p_nativeInit(H_GET_ENTITY_COORDS);
    p_nativePush64((U64)e); p_nativePush64(1); p_nativePush64(0); // entity, alive, realCoords
    return *(Vector3 *)p_nativeCall();
}

static int readHonorRaw() {
    if (!p_getGlobalPtr) return 0;
    U64 *g = p_getGlobalPtr(HONOR_GLOBAL);
    return g ? *(int *)g : 0;
}

// ---- In-game date/time -> seconds since in-game 1800-01-01 ----
static long long days_from_civil(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    long long era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = (unsigned)(y - era * 400);
    unsigned doy = (153u * (m + (m > 2 ? -3u : 9u)) + 2u) / 5u + d - 1u;
    unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
    return era * 146097LL + (long long)doe - 719468LL;
}
static uint32_t ingameSeconds() {
    int Y = nv0_int(H_GET_CLOCK_YEAR), Mo = nv0_int(H_GET_CLOCK_MONTH), D = nv0_int(H_GET_CLOCK_DAY);
    int h = nv0_int(H_GET_CLOCK_HOURS), mi = nv0_int(H_GET_CLOCK_MINUTES), s = nv0_int(H_GET_CLOCK_SECONDS);
    if (Mo < 1) Mo = 1;
    if (D < 1) D = 1;
    long long days = days_from_civil(Y, (unsigned)Mo, (unsigned)D) - days_from_civil(1800, 1, 1);
    long long secs = days * 86400LL + h * 3600LL + mi * 60LL + s;
    return secs < 0 ? 0u : (uint32_t)secs;
}

// ---- Transport / character classification ----
enum { T_FOOT = 0, T_HORSE = 1, T_BOAT = 2, T_TRAIN = 3, T_BALLOON = 4, T_SWIM = 5, T_OTHER_VEH = 6 };

static int classifyCharacter(unsigned model) {
    if (model == MODEL_ARTHUR) return 0;
    if (model == MODEL_JOHN)   return 1;
    return 2;
}

// Vehicle model-hash -> transport category, generated from the femga/rdr3_discoveries
// RDR2 vehicle list. Only boat(2)/train(3)/balloon(4) are listed; any other vehicle
// model is other_vehicle(6).
static const struct { unsigned hash; unsigned char cat; } VEH_TABLE[] = {
    {0x055BF98Fu, 3}, {0x09D4E809u, 3}, {0x0F228F06u, 3}, {0x0FD337B7u, 3}, {0x18296CDEu, 3},
    {0x1C10E9C9u, 3}, {0x1C8D173Au, 3}, {0x20925D76u, 3}, {0x22250EF5u, 3}, {0x29EA09A9u, 3},
    {0x384E6422u, 3}, {0x39584F5Au, 3}, {0x427A2D4Cu, 2}, {0x4717D8D8u, 3}, {0x4D5B5089u, 3},
    {0x4E018632u, 3}, {0x578D6513u, 2}, {0x5E56769Cu, 2}, {0x5EB0BAE0u, 4}, {0x61EC29C0u, 3},
    {0x6422679Du, 2}, {0x6A80D253u, 3}, {0x6F8F7EE4u, 3}, {0x75BDDBD6u, 2}, {0x7724C788u, 3},
    {0x7DD49B09u, 2}, {0x7F4258C9u, 3}, {0x876E6EB7u, 2}, {0x8C0224C6u, 3}, {0x930442ECu, 3},
    {0x9A0A187Au, 3}, {0x9FD6BA58u, 2}, {0xA385E1C7u, 2}, {0xAD516118u, 3}, {0xAE057F07u, 2},
    {0xC27964BEu, 3}, {0xC40B0265u, 3}, {0xC50FC5D0u, 3}, {0xC6FA5BFFu, 2}, {0xD84D4530u, 2},
    {0xDA152CA6u, 3}, {0xDADC0B67u, 2}, {0xDF86C25Au, 3}, {0xE1FE4FD4u, 2}, {0xE84E6B74u, 2},
    {0xECD7E90Eu, 3}, {0xEE645446u, 3}, {0xEF91537Fu, 2}, {0xF097BC6Cu, 2}, {0xF1FE5FB8u, 3},
    {0xF539E5A0u, 2}, {0xF632A662u, 3},
};
static int vehicleCategory(unsigned model) {
    if (model == 0) return T_OTHER_VEH;
    for (unsigned i = 0; i < sizeof(VEH_TABLE) / sizeof(VEH_TABLE[0]); ++i)
        if (VEH_TABLE[i].hash == model) return VEH_TABLE[i].cat;
    return T_OTHER_VEH;
}

// ---- Binary file format (little-endian; x86-64 target is little-endian) ----
#pragma pack(push, 1)
struct Header {
    char     magic[4];   // "R2GP"
    uint16_t version;    // 1
    uint16_t recordSize; // 18
};
struct Record {
    uint32_t ingameTime; // seconds since in-game 1800-01-01
    int16_t  x, y, z;    // round(world coord * 2)  -> ~0.5-unit resolution
    uint8_t  transport;  // see T_* enum
    uint8_t  flags;      // bitfield (see below)
    int8_t   honor;      // -100..+100 (0 = neutral)
    uint8_t  character;  // 0 Arthur, 1 John, 2 other
    uint32_t cash;       // whole dollars
    uint16_t bounty;     // total bounty in dollars (0..65535)
};
#pragma pack(pop)
static_assert(sizeof(Header) == 8, "header must be 8 bytes");
static_assert(sizeof(Record) == 20, "record must be 20 bytes");

// flags bits
enum {
    F_NONGAMEPLAY   = 0x01, // cutscene / non-gameplay camera
    F_KEEPALIVE     = 0x02, // written by the keepalive timer, not by movement
    F_COMBAT        = 0x04,
    F_WANTED        = 0x08,
    F_DEAD          = 0x10, // dead/dying (best-effort; see README)
    F_SEGMENT_START = 0x20, // first point of a session (game launch / savegame load)
    // 0x40 reserved (orphaned; set by post-processing tools)
};

static int16_t coord16(float v) {
    long s = lround((double)v * 2.0);
    if (s > 32767) s = 32767;
    else if (s < -32768) s = -32768;
    return (int16_t)s;
}

// ---- Configuration (TrailMarker.ini next to the .asi) ----
struct Config {
    int   sampleIntervalMs  = 1000; // how often to check position
    float movementThreshold = 3.0f; // world units of movement that trigger a recorded point
    int   keepaliveSeconds  = 45;   // record at least this often while stationary
    int   flushSeconds      = 30;   // flush buffered records to disk this often
    char  outputPath[MAX_PATH] = {0}; // empty => TrailMarker.bin next to the .asi
} g_cfg;

static void trim(char *s) {
    char *p = s; while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t n = strlen(s);
    while (n && (s[n-1]=='\r'||s[n-1]=='\n'||s[n-1]==' '||s[n-1]=='\t')) s[--n] = 0;
}
static void loadConfig(const char *iniPath) {
    FILE *f = fopen(iniPath, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == ';') continue;
        char *eq = strchr(line, '='); if (!eq) continue;
        *eq = 0;
        char key[128], val[384];
        strncpy(key, line, sizeof key - 1); key[sizeof key - 1] = 0;
        strncpy(val, eq + 1, sizeof val - 1); val[sizeof val - 1] = 0;
        trim(key); trim(val);
        if      (!strcmp(key, "sampleIntervalMs"))  g_cfg.sampleIntervalMs = atoi(val);
        else if (!strcmp(key, "movementThreshold")) g_cfg.movementThreshold = (float)atof(val);
        else if (!strcmp(key, "keepaliveSeconds"))  g_cfg.keepaliveSeconds = atoi(val);
        else if (!strcmp(key, "flushSeconds"))      g_cfg.flushSeconds = atoi(val);
        else if (!strcmp(key, "outputPath"))        strncpy(g_cfg.outputPath, val, MAX_PATH - 1);
    }
    fclose(f);
    if (g_cfg.sampleIntervalMs < 50) g_cfg.sampleIntervalMs = 50;
}

// ---- Paths & output ----
static char  g_dir[MAX_PATH] = {0}; // directory of this .asi (trailing backslash)
static char  g_outPath[MAX_PATH] = {0};
static FILE *g_out = nullptr;

// Safe to call from DllMain: module path only, no file I/O.
static void resolveDir(HMODULE hModule) {
    GetModuleFileNameA(hModule, g_dir, MAX_PATH);
    char *slash = strrchr(g_dir, '\\');
    if (slash) slash[1] = 0; else g_dir[0] = 0;
}

// Called from the script thread (not DllMain): reads config, opens the output file.
static void initOutput() {
    char ini[MAX_PATH];
    snprintf(ini, sizeof ini, "%sTrailMarker.ini", g_dir);
    loadConfig(ini);
    if (g_cfg.outputPath[0]) strncpy(g_outPath, g_cfg.outputPath, MAX_PATH - 1);
    else snprintf(g_outPath, sizeof g_outPath, "%sTrailMarker.bin", g_dir);

    g_out = fopen(g_outPath, "ab");
    if (!g_out) return;
    fseek(g_out, 0, SEEK_END);
    if (ftell(g_out) == 0) { // new file -> write the header once
        Header h = { {'R', '2', 'G', 'P'}, 1, (uint16_t)sizeof(Record) };
        fwrite(&h, sizeof h, 1, g_out);
        fflush(g_out);
    }
}

// ---- Script thread ----
static void ScriptMain() {
    initOutput(); // g_dir was set in DllMain
    if (!g_out) return;

    bool   haveLast = false;
    float  lastX = 0, lastY = 0, lastZ = 0;
    int    msSinceRecord = 0;
    int    msSinceFlush  = 0;
    // segment_start marks the first recorded point of this script session (a game launch,
    // or ScriptHook's script reload on a savegame load). The reader uses it together with
    // the recorded time to split sessions and identify abandoned (orphaned) branches.
    bool   firstRecord = true;
    const int keepaliveMs = g_cfg.keepaliveSeconds * 1000;
    const int flushMs     = g_cfg.flushSeconds * 1000;

    for (;;) {
        p_scriptWait((DWORD)g_cfg.sampleIntervalMs);
        msSinceRecord += g_cfg.sampleIntervalMs;
        msSinceFlush  += g_cfg.sampleIntervalMs;

        int ped = PLAYER_PED_ID();
        if (ped == 0) continue;
        Vector3 c = GET_ENTITY_COORDS(ped);
        // Skip loading screens / pre-spawn: coords read as (0,0,0).
        if (c.x > -0.01f && c.x < 0.01f && c.y > -0.01f && c.y < 0.01f && c.z > -0.01f && c.z < 0.01f) continue;

        // Movement-or-keepalive: record on enough movement, or once per keepalive interval.
        bool keepalive = false;
        if (haveLast) {
            float dx = c.x - lastX, dy = c.y - lastY, dz = c.z - lastZ;
            bool moved = dx*dx + dy*dy + dz*dz > g_cfg.movementThreshold * g_cfg.movementThreshold;
            if (!moved) {
                if (msSinceRecord >= keepaliveMs) keepalive = true;
                else continue;
            }
        }

        int player = PLAYER_ID();

        int transport;
        if (IS_PED_ON_MOUNT(ped))            transport = T_HORSE;
        else if (IS_PED_SWIMMING(ped))       transport = T_SWIM;
        else if (IS_PED_IN_ANY_TRAIN(ped))   transport = T_TRAIN; // passengers don't register as "in vehicle"
        else if (IS_PED_IN_ANY_VEHICLE(ped)) {
            int veh = GET_VEHICLE_PED_IS_IN(ped);
            transport = vehicleCategory(veh ? GET_ENTITY_MODEL(veh) : 0);
        } else transport = T_FOOT;

        uint8_t flags = 0;
        if (!IS_GAMEPLAY_CAM_RENDERING())                                     flags |= F_NONGAMEPLAY;
        if (keepalive)                                                        flags |= F_KEEPALIVE;
        if (IS_PED_IN_COMBAT(ped, 0))                                         flags |= F_COMBAT;
        if (GET_PLAYER_WANTED_LEVEL(player) > 0)                              flags |= F_WANTED;
        if (IS_PLAYER_DEAD(player) || IS_PED_DEAD_OR_DYING(ped) || IS_ENTITY_DEAD(ped)) flags |= F_DEAD;
        if (firstRecord)                                                      flags |= F_SEGMENT_START;

        int honor = readHonorRaw() * 100 / 320; // internal -320..+320 -> -100..+100
        if (honor > 100) honor = 100; else if (honor < -100) honor = -100;

        Record r;
        r.ingameTime = ingameSeconds();
        r.x = coord16(c.x); r.y = coord16(c.y); r.z = coord16(c.z);
        r.transport = (uint8_t)transport;
        r.flags     = flags;
        r.honor     = (int8_t)honor;
        r.character = (uint8_t)classifyCharacter(GET_ENTITY_MODEL(ped));
        r.cash      = (uint32_t)(MONEY_GET_CASH_BALANCE() / 100); // cents -> dollars
        int bty = GET_BOUNTY(player);
        r.bounty    = (uint16_t)(bty < 0 ? 0 : (bty > 65535 ? 65535 : bty));
        fwrite(&r, sizeof r, 1, g_out);

        haveLast = true; lastX = c.x; lastY = c.y; lastZ = c.z;
        msSinceRecord = 0;
        firstRecord = false;

        if (msSinceFlush >= flushMs) { fflush(g_out); msSinceFlush = 0; }
    }
}

// ---- DLL entry ----
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        resolveDir(hModule);

        HMODULE sh = GetModuleHandleA("ScriptHookRDR2.dll");
        if (!sh) return TRUE;
        p_nativeInit     = (void (*)(U64))               GetProcAddress(sh, "?nativeInit@@YAX_K@Z");
        p_nativePush64   = (void (*)(U64))               GetProcAddress(sh, "?nativePush64@@YAX_K@Z");
        p_nativeCall     = (U64 *(*)())                  GetProcAddress(sh, "?nativeCall@@YAPEA_KXZ");
        p_scriptRegister = (void (*)(HMODULE, ScriptMainFn)) GetProcAddress(sh, "?scriptRegister@@YAXPEAUHINSTANCE__@@P6AXXZ@Z");
        p_scriptWait     = (void (*)(DWORD))             GetProcAddress(sh, "?scriptWait@@YAXK@Z");
        p_getGlobalPtr   = (U64 *(*)(int))               GetProcAddress(sh, "?getGlobalPtr@@YAPEA_KH@Z");
        if (p_nativeInit && p_nativePush64 && p_nativeCall && p_scriptRegister && p_scriptWait)
            p_scriptRegister(hModule, ScriptMain);
    } else if (reason == DLL_PROCESS_DETACH) {
        if (g_out) { fflush(g_out); fclose(g_out); g_out = nullptr; }
    }
    return TRUE;
}
