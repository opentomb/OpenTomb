-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, CITNEW.TR4

print("Level script loaded (CITNEW.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLL_NONE};       -- Horizontal rope
static_tbl[01] = {coll = COLL_BBOX};       -- Pillar 1
static_tbl[02] = {coll = COLL_BBOX};       -- Pillar 2
static_tbl[03] = {coll = COLL_NONE};       -- Horizontal rope 2
static_tbl[04] = {coll = COLL_NONE};       -- Floor debris
static_tbl[05] = {coll = COLL_MESH};       -- Wall light
static_tbl[08] = {coll = COLL_BBOX};       -- Wall pillar
static_tbl[09] = {coll = COLL_MESH};       -- Stone arch
static_tbl[10] = {coll = COLL_NONE};       -- Rays
static_tbl[11] = {coll = COLL_MESH};       -- Tower part
static_tbl[12] = {coll = COLL_MESH};       -- Stone arch 2
static_tbl[13] = {coll = COLL_MESH};       -- Ceiling with hole
static_tbl[15] = {coll = COLL_MESH};       -- Stairs
static_tbl[16] = {coll = COLL_MESH};       -- Stairs 2
static_tbl[17] = {coll = COLL_MESH};       -- Wall light 2
static_tbl[18] = {coll = COLL_BBOX};       -- Pillar 3
static_tbl[19] = {coll = COLL_MESH};       -- Stone arch 3
static_tbl[20] = {coll = COLL_MESH};       -- Building block 1
static_tbl[21] = {coll = COLL_MESH};       -- Stone arch part 1
static_tbl[22] = {coll = COLL_MESH};       -- Thin pillar 1
static_tbl[23] = {coll = COLL_MESH};       -- Stone arch part 2
static_tbl[24] = {coll = COLL_MESH};       -- Thin pillar 2
static_tbl[25] = {coll = COLL_MESH};       -- Building block 2
static_tbl[26] = {coll = COLL_MESH};       -- Rounded wall stone
static_tbl[27] = {coll = COLL_MESH};       -- Ornate arch
static_tbl[28] = {coll = COLL_BBOX};       -- Pillar 4
static_tbl[29] = {coll = COLL_MESH};       -- Thin pillar 3
static_tbl[32] = {coll = COLL_MESH};       -- Stone arch 4
static_tbl[33] = {coll = COLL_BBOX};       -- Fence 1
static_tbl[34] = {coll = COLL_BBOX};       -- Fence 2
static_tbl[35] = {coll = COLL_BBOX};       -- Cubical structure
static_tbl[36] = {coll = COLL_MESH};       -- Stone arch 5
static_tbl[37] = {coll = COLL_BBOX};       -- Horizontal rod
static_tbl[38] = {coll = COLL_NONE};       -- Rectangular hole
static_tbl[39] = {coll = COLL_NONE};       -- Grated pole
static_tbl[40] = {coll = COLL_MESH};       -- Stone block
static_tbl[41] = {coll = COLL_BBOX};       -- Pillar 5
static_tbl[42] = {coll = COLL_MESH};       -- Pillar 6
static_tbl[43] = {coll = COLL_NONE};       -- Pole
static_tbl[44] = {coll = COLL_MESH};       -- Wall stone block
static_tbl[45] = {coll = COLL_MESH};       -- Rounded wall block
static_tbl[46] = {coll = COLL_MESH};       -- Grated arch part
static_tbl[47] = {coll = COLL_MESH};       -- Grated arch part 2
static_tbl[48] = {coll = COLL_BBOX};       -- Wall pedestal
static_tbl[49] = {coll = COLL_MESH};       -- Stone arch 6

-- SHATTER statics

static_tbl[59] = {coll = COLL_BBOX};       -- Wooden structure