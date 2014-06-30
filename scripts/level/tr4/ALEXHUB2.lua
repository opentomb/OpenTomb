-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, ALEXHUB

print("Level script loaded (ALEXHUB.lua)");

--------------------------------------------------------------------------------
-- ColType values reference: 

NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[22] = {coll = MESH};       			-- Stone arc
static_tbl[23] = {coll = BBOX};       			-- Dummy mummy
static_tbl[24] = {coll = MESH};       			-- Stairs
static_tbl[25] = {coll = BBOX};                 -- Dummy pharaoh
static_tbl[26] = {coll = NONE};                 -- Wall light
static_tbl[27] = {coll = BBOX};                 -- Tree
static_tbl[30] = {coll = MESH};                 -- Stone arc 2
static_tbl[31] = {coll = MESH};                 -- Stone arc 3
static_tbl[32] = {coll = BBOX};                 -- Stone arc with pillar 1 (collision only for pillar)
static_tbl[33] = {coll = BBOX};                 -- Stone arc with pillar 2 (collision only for pillar)
static_tbl[34] = {coll = BBOX};                 -- Pillar
static_tbl[35] = {coll = MESH};                 -- Stone arc 2
static_tbl[36] = {coll = BBOX, hide = 1};       -- Dummy platform: dummy static 1!
static_tbl[40] = {coll = NONE};                 -- Rays
static_tbl[41] = {coll = NONE};                 -- Mechanism
static_tbl[42] = {coll = NONE};                 -- Mechanism 2
static_tbl[43] = {coll = BBOX};                 -- Fire basket
static_tbl[44] = {coll = BBOX, hide = 1};       -- Panel: dummy static 2!
static_tbl[46] = {coll = BBOX};                 -- Hook
static_tbl[47] = {coll = MESH};                 -- Sweeper
static_tbl[48] = {coll = MESH};                 -- Stairs 2

-- SHATTER statics
-- Note: all SHATTER statics can be destroyed by SHATTER event (either on hit or something else)

static_tbl[50] = {coll = BBOX};                 -- Egyptian adventure barricade
static_tbl[51] = {coll = BBOX};                 -- Lock
static_tbl[52] = {coll = MESH};                 -- Target

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;