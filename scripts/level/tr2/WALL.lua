-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, WALL.TR2

print("Level script loaded (WALL.lua)");
 
-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------
-- ColType values reference:
NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[00] = {coll = MESH};            -- Bank rock
static_tbl[01] = {coll = NONE};            -- Death slide rope
static_tbl[02] = {coll = NONE};            -- Skeleton 1
static_tbl[03] = {coll = NONE};            -- Skeleton 2
static_tbl[04] = {coll = NONE};            -- Cobweb
static_tbl[10] = {coll = NONE};            -- Fireplace
static_tbl[11] = {coll = MESH};            -- Crate with book
static_tbl[12] = {coll = MESH};            -- Briefcase
static_tbl[13] = {coll = MESH};            -- Crates

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;