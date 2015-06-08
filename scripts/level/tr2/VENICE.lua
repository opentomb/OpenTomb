-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, VENICE.TR2

print("Level script loaded (VENICE.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLL_MESH};       -- Pillar 1
static_tbl[01] = {coll = COLL_MESH};       -- Pillar 2
static_tbl[02] = {coll = COLL_MESH};       -- Pillar 3
static_tbl[03] = {coll = COLL_MESH};       -- Pillar 4
static_tbl[04] = {coll = COLL_MESH};       -- Pillar 5
static_tbl[05] = {coll = COLL_MESH};       -- Vault
static_tbl[06] = {coll = COLL_BBOX};       -- Tree
static_tbl[10] = {coll = COLL_NONE};       -- Chain
static_tbl[13] = {coll = COLL_NONE};       -- Square hole
static_tbl[14] = {coll = COLL_MESH};       -- Window tent
static_tbl[15] = {coll = COLL_BBOX};       -- Smoke pipe
static_tbl[16] = {coll = COLL_BBOX};       -- Fence
static_tbl[20] = {coll = COLL_BBOX};       -- Water pole top
static_tbl[21] = {coll = COLL_BBOX};       -- Water pole bottom
static_tbl[22] = {coll = COLL_BBOX};       -- Water pole middle
static_tbl[30] = {coll = COLL_MESH};       -- Chandelier
static_tbl[31] = {coll = COLL_BBOX};       -- Square pillar
static_tbl[32] = {coll = COLL_BBOX};       -- Square pillar 2
static_tbl[33] = {coll = COLL_BBOX};       -- Horizontal block
static_tbl[34] = {coll = COLL_NONE};       -- Wall light
static_tbl[35] = {coll = COLL_BBOX};       -- Stone fence
static_tbl[36] = {coll = COLL_BBOX};       -- Wooden fence
static_tbl[37] = {coll = COLL_NONE};       -- Broken window
static_tbl[38] = {coll = COLL_MESH};       -- Normal window
static_tbl[39] = {coll = COLL_MESH};       -- Grated fence
static_tbl[40] = {coll = COLL_BBOX};       -- Pipes
static_tbl[41] = {coll = COLL_BBOX};       -- Furnace
static_tbl[43] = {coll = COLL_BBOX};       -- Wooden barrier
static_tbl[44] = {coll = COLL_BBOX};       -- Barbed wire
static_tbl[45] = {coll = COLL_NONE};       -- Books on the floor
static_tbl[46] = {coll = COLL_NONE};       -- Curtain
static_tbl[47] = {coll = COLL_MESH};       -- Chair
static_tbl[48] = {coll = COLL_BBOX};       -- Fireplace
static_tbl[49] = {coll = COLL_NONE};       -- Flags 