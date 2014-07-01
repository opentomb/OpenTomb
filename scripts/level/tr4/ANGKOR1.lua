-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, ANGKOR1

UVRotate	= 4;

print("Level script loaded (ANGKOR1.lua)");

--------------------------------------------------------------------------------
-- ColType values reference: 

NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

-- PLANT statics (as listed in OBJECTS.H from TRLE)

static_tbl[00] = {coll = NONE};            -- Grass
static_tbl[01] = {coll = NONE};            -- Tree part
static_tbl[02] = {coll = NONE};            -- Tree part 2
static_tbl[03] = {coll = NONE};            -- Tree part 3
static_tbl[04] = {coll = NONE};            -- Tree part 4
static_tbl[05] = {coll = NONE};            -- Bush 1
static_tbl[06] = {coll = NONE};            -- Bush 2
static_tbl[07] = {coll = NONE};            -- Bush 3

-- FURNITURE statics

static_tbl[10] = {coll = BBOX, hide = 1};  -- Dummy cube

-- ROCK statics

static_tbl[20] = {coll = NONE};			   -- Wall light

-- ARCHITECTURE statics

static_tbl[30] = {coll = BBOX};			   -- Ornate arch
static_tbl[31] = {coll = BBOX};            -- Pillar
static_tbl[32] = {coll = BBOX, hide = 1};  -- Dummy block

-- DEBRIS statics

static_tbl[40] = {coll = NONE};			   -- Ornate arch 2
static_tbl[41] = {coll = MESH};            -- Stone border
static_tbl[42] = {coll = BBOX};            -- Pillar 2
static_tbl[43] = {coll = BBOX};  		   -- Stone lion
static_tbl[44] = {coll = NONE};  		   -- Rays
static_tbl[45] = {coll = MESH};			   -- Stone face

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;