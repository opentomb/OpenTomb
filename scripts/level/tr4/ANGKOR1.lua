-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, ANGKOR1

UVRotate = 4;

print("Level script loaded (ANGKOR1.lua)");

--------------------------------------------------------------------------------
-- ColType values reference: 

COLL_NONE = 0x00;  -- Object has no collisions
COLL_MESH = 0x01;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x02;  -- Object uses bounding box for collision.

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

-- PLANT statics (as listed in OBJECTS.H from TRLE)

static_tbl[00] = {coll = COLL_NONE};            -- Grass
static_tbl[01] = {coll = COLL_NONE};            -- Tree part
static_tbl[02] = {coll = COLL_NONE};            -- Tree part 2
static_tbl[03] = {coll = COLL_NONE};            -- Tree part 3
static_tbl[04] = {coll = COLL_NONE};            -- Tree part 4
static_tbl[05] = {coll = COLL_NONE};            -- Bush 1
static_tbl[06] = {coll = COLL_NONE};            -- Bush 2
static_tbl[07] = {coll = COLL_NONE};            -- Bush 3

-- FURNITURE statics

static_tbl[10] = {coll = COLL_BBOX, hide = 1};  -- Dummy cube

-- ROCK statics

static_tbl[20] = {coll = COLL_NONE};            -- Wall light

-- ARCHITECTURE statics

static_tbl[30] = {coll = COLL_BBOX};            -- Ornate arch
static_tbl[31] = {coll = COLL_BBOX};            -- Pillar
static_tbl[32] = {coll = COLL_BBOX, hide = 1};  -- Dummy block

-- DEBRIS statics

static_tbl[40] = {coll = COLL_NONE};            -- Ornate arch 2
static_tbl[41] = {coll = COLL_MESH};            -- Stone border
static_tbl[42] = {coll = COLL_BBOX};            -- Pillar 2
static_tbl[43] = {coll = COLL_BBOX};            -- Stone lion
static_tbl[44] = {coll = COLL_NONE};            -- Rays
static_tbl[45] = {coll = COLL_MESH};            -- Stone face

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;
