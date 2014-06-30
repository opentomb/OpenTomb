-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, VENICE.TR2

print("Level script loaded (VENICE.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------
-- ColType values reference: 
NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[00] = {coll = MESH};       -- Pillar 1
static_tbl[01] = {coll = MESH};       -- Pillar 2
static_tbl[02] = {coll = MESH};       -- Pillar 3
static_tbl[03] = {coll = MESH};       -- Pillar 4
static_tbl[04] = {coll = MESH};       -- Pillar 5
static_tbl[05] = {coll = MESH};       -- Vault
static_tbl[06] = {coll = BBOX};       -- Tree
static_tbl[10] = {coll = NONE};       -- Chain
static_tbl[13] = {coll = NONE};       -- Square hole
static_tbl[14] = {coll = MESH};       -- Window tent
static_tbl[15] = {coll = BBOX};       -- Smoke pipe
static_tbl[16] = {coll = BBOX};       -- Fence
static_tbl[20] = {coll = BBOX};       -- Water pole top
static_tbl[21] = {coll = BBOX};       -- Water pole bottom
static_tbl[22] = {coll = BBOX};       -- Water pole middle
static_tbl[30] = {coll = MESH};       -- Chandelier
static_tbl[31] = {coll = BBOX};       -- Square pillar
static_tbl[32] = {coll = BBOX};       -- Square pillar 2
static_tbl[33] = {coll = BBOX};       -- Horizontal block
static_tbl[34] = {coll = NONE};       -- Wall light
static_tbl[35] = {coll = BBOX};       -- Stone fence
static_tbl[36] = {coll = BBOX};       -- Wooden fence
static_tbl[37] = {coll = NONE};       -- Broken window
static_tbl[38] = {coll = MESH};       -- Normal window
static_tbl[39] = {coll = MESH};       -- Grated fence
static_tbl[40] = {coll = BBOX};       -- Pipes
static_tbl[41] = {coll = BBOX};       -- Furnace
static_tbl[43] = {coll = BBOX};       -- Wooden barrier
static_tbl[44] = {coll = BBOX};       -- Barbed wire
static_tbl[45] = {coll = NONE};       -- Books on the floor
static_tbl[46] = {coll = NONE};       -- Curtain
static_tbl[47] = {coll = MESH};       -- Chair
static_tbl[48] = {coll = BBOX};       -- Fireplace
static_tbl[49] = {coll = NONE};       -- Flags 

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;