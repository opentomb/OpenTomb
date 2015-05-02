-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, CRASH.TR2

print("Level script loaded (CRASH.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLL_BBOX};            -- Tree 1
static_tbl[01] = {coll = COLL_NONE};            -- Tree 2
static_tbl[02] = {coll = COLL_NONE};            -- Hanging plant
static_tbl[03] = {coll = COLL_NONE};            -- Hanging plant (tilt)
static_tbl[05] = {coll = COLL_BBOX};            -- Tree 3
static_tbl[06] = {coll = COLL_NONE};            -- Tree 4
static_tbl[07] = {coll = COLL_NONE};            -- Grass
static_tbl[08] = {coll = COLL_BBOX};            -- Tree 5
static_tbl[11] = {coll = COLL_MESH};            -- Cockpit
static_tbl[12] = {coll = COLL_MESH};            -- Turret
static_tbl[16] = {coll = COLL_NONE};            -- Ray
static_tbl[17] = {coll = COLL_NONE};            -- Rays
static_tbl[18] = {coll = COLL_MESH};            -- Control panel 1
static_tbl[19] = {coll = COLL_MESH};            -- Control panel 2
static_tbl[33] = {coll = COLL_BBOX};            -- Fence
static_tbl[34] = {coll = COLL_NONE};            -- Wall light
static_tbl[35] = {coll = COLL_NONE};            -- Wheel and ropes
static_tbl[36] = {coll = COLL_NONE};            -- Ropes
static_tbl[37] = {coll = COLL_NONE};            -- Ropes (horizontal)
static_tbl[38] = {coll = COLL_BBOX, hide = 1};  -- Dummy cube
static_tbl[39] = {coll = COLL_NONE};            -- Ring
static_tbl[40] = {coll = COLL_MESH};            -- Cockpit 2
static_tbl[41] = {coll = COLL_MESH};            -- Control panel 3
static_tbl[42] = {coll = COLL_MESH};            -- Control panel 4
static_tbl[48] = {coll = COLL_MESH};            -- Bed
static_tbl[49] = {coll = COLL_NONE};            -- Rope