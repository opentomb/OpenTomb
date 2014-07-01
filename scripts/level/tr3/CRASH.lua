-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, CRASH.TR2

-- ColType values reference: 

COLL_NONE = 0x0000;  -- Object has no collisions
COLL_MESH = 0x0001;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x0002;  -- Object uses bounding box for collision.

print("Level script loaded (CRASH.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

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
static_tbl[16] = {coll = COLL_NONE};			-- Ray
static_tbl[17] = {coll = COLL_NONE};            -- Rays
static_tbl[18] = {coll = COLL_MESH};            -- Control panel 1
static_tbl[19] = {coll = COLL_MESH};            -- Control panel 2
static_tbl[33] = {coll = COLL_BBOX};            -- Fence
static_tbl[34] = {coll = COLL_NONE};            -- Wall light
static_tbl[35] = {coll = COLL_NONE};            -- Wheel and ropes
static_tbl[36] = {coll = COLL_NONE};			-- Ropes
static_tbl[37] = {coll = COLL_NONE};            -- Ropes (horizontal)
static_tbl[38] = {coll = COLL_BBOX, hide = 1};	-- Dummy cube
static_tbl[39] = {coll = COLL_NONE};			-- Ring
static_tbl[40] = {coll = COLL_MESH};			-- Cockpit 2
static_tbl[41] = {coll = COLL_MESH};            -- Control panel 3
static_tbl[42] = {coll = COLL_MESH};            -- Control panel 4
static_tbl[48] = {coll = COLL_MESH};			-- Bed
static_tbl[49] = {coll = COLL_NONE};			-- Rope

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;