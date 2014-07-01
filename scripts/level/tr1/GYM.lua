-- OPENTOMB PER-LEVEL STATIC MESH COLLISION SCRIPT
-- FOR TOMB RAIDER 1, GYM


print("Level script loaded (GYM.lua)");

-- ColType values reference: 
--   0x0000  - Object has no collisions
--   0x0001  - Object uses real mesh data for collision.
--   0x0002  - Object uses bounding box for collision.

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[00] = {coll = 0x0001};             -- Flower pot 1
static_tbl[01] = {coll = 0x0001};             -- Flower pot 2
static_tbl[02] = {coll = 0x0001};             -- Flower pot 3
static_tbl[03] = {coll = 0x0001};             -- Flower pot 4
static_tbl[04] = {coll = 0x0000};             -- Plant
static_tbl[10] = {coll = 0x0001};             -- Chair
static_tbl[11] = {coll = 0x0001};             -- Fireplace
static_tbl[12] = {coll = 0x0001};             -- Harp
static_tbl[13] = {coll = 0x0001};             -- Piano
static_tbl[14] = {coll = 0x0001};             -- Desk
static_tbl[15] = {coll = 0x0001};             -- Statue
static_tbl[16] = {coll = 0x0002};             -- Column
static_tbl[18] = {coll = 0x0001};             -- Indiana Jones Lost Ark
static_tbl[19] = {coll = 0x0001};             -- Treasures
static_tbl[30] = {coll = 0x0001};             -- Fence flat
static_tbl[31] = {coll = 0x0001};             -- Fence tilt 1
static_tbl[32] = {coll = 0x0001};             -- Fence tilt 2
static_tbl[33] = {coll = 0x0002};             -- Stick
static_tbl[34] = {coll = 0x0001};             -- Chandelier
static_tbl[35] = {coll = 0x0002};             -- Armor

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;