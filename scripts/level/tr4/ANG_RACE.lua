-- OPENTOMB PER-LEVEL STATIC MESH COLLISION SCRIPT
-- FOR TOMB RAIDER 4, ANG_RACE

-- ColType values reference: 

NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.

print("Level script loaded (ANG_RACE.lua)");

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

static_tbl[20] = {coll = MESH};            -- Pedestal
static_tbl[21] = {coll = NONE};			   -- Stones
static_tbl[22] = {coll = NONE};			   -- Wall light

-- ARCHITECTURE statics

static_tbl[30] = {coll = BBOX};			   -- Ornate arch
static_tbl[31] = {coll = BBOX};            -- Pillar
static_tbl[32] = {coll = BBOX};			   -- Scripture plate
static_tbl[33] = {coll = BBOX};            -- Dummy panel

-- DEBRIS statics

static_tbl[40] = {coll = NONE};			   -- Ornate arch 2
static_tbl[41] = {coll = MESH};            -- Stone border
static_tbl[42] = {coll = BBOX};            -- Pillar 2
static_tbl[43] = {coll = BBOX};  		   -- Stone lion
static_tbl[46] = {coll = BBOX};			   -- Ornate arch with pillars
static_tbl[47] = {coll = BBOX, hide = 1};  -- Block panel: dummy static!

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;