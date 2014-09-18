-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, CITNEW.TR4

print("Level script loaded (CITNEW.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------
-- ColType values reference: 
NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[00] = {coll = NONE};       -- Horizontal rope
static_tbl[01] = {coll = BBOX};       -- Pillar 1
static_tbl[02] = {coll = BBOX};       -- Pillar 2
static_tbl[03] = {coll = NONE};       -- Horizontal rope 2
static_tbl[04] = {coll = NONE};       -- Floor debris
static_tbl[05] = {coll = MESH};       -- Wall light
static_tbl[08] = {coll = BBOX};       -- Wall pillar
static_tbl[09] = {coll = MESH};       -- Stone arch
static_tbl[10] = {coll = NONE};       -- Rays
static_tbl[11] = {coll = MESH};       -- Tower part
static_tbl[12] = {coll = MESH};       -- Stone arch 2
static_tbl[13] = {coll = MESH};       -- Ceiling with hole
static_tbl[15] = {coll = MESH};       -- Stairs
static_tbl[16] = {coll = MESH};       -- Stairs 2
static_tbl[17] = {coll = MESH};       -- Wall light 2
static_tbl[18] = {coll = BBOX};       -- Pillar 3
static_tbl[19] = {coll = MESH};       -- Stone arch 3
static_tbl[20] = {coll = MESH};       -- Building block 1
static_tbl[21] = {coll = MESH};       -- Stone arch part 1
static_tbl[22] = {coll = MESH};       -- Thin pillar 1
static_tbl[23] = {coll = MESH};       -- Stone arch part 2
static_tbl[24] = {coll = MESH};       -- Thin pillar 2
static_tbl[25] = {coll = MESH};       -- Building block 2
static_tbl[26] = {coll = MESH};       -- Rounded wall stone
static_tbl[27] = {coll = MESH};       -- Ornate arch
static_tbl[28] = {coll = BBOX};       -- Pillar 4
static_tbl[29] = {coll = MESH};       -- Thin pillar 3
static_tbl[32] = {coll = MESH};       -- Stone arch 4
static_tbl[33] = {coll = BBOX};       -- Fence 1
static_tbl[34] = {coll = BBOX};       -- Fence 2
static_tbl[35] = {coll = BBOX};       -- Cubical structure
static_tbl[36] = {coll = MESH};       -- Stone arch 5
static_tbl[37] = {coll = BBOX};       -- Horizontal rod
static_tbl[38] = {coll = NONE};       -- Rectangular hole
static_tbl[39] = {coll = NONE};       -- Grated pole
static_tbl[40] = {coll = MESH};       -- Stone block
static_tbl[41] = {coll = BBOX};       -- Pillar 5
static_tbl[42] = {coll = MESH};       -- Pillar 6
static_tbl[43] = {coll = NONE};       -- Pole
static_tbl[44] = {coll = MESH};       -- Wall stone block
static_tbl[45] = {coll = MESH};       -- Rounded wall block
static_tbl[46] = {coll = MESH};       -- Grated arch part
static_tbl[47] = {coll = MESH};       -- Grated arch part 2
static_tbl[48] = {coll = BBOX};       -- Wall pedestal
static_tbl[49] = {coll = MESH};       -- Stone arch 6

-- SHATTER statics

static_tbl[59] = {coll = BBOX};       -- Wooden structure

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;