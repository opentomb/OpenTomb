-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, CHAMBER.TR2

-- ColType values reference: 

COLL_NONE = 0x0000;  -- Object has no collisions
COLL_MESH = 0x0001;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x0002;  -- Object uses bounding box for collision.

print("Level script loaded (CHAMBER.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[00] = {coll = COLL_BBOX};            -- Stone face
static_tbl[10] = {coll = COLL_NONE};            -- Floodlight
static_tbl[11] = {coll = COLL_NONE};            -- Blue lamp
static_tbl[12] = {coll = COLL_BBOX};            -- Floodlight 2
static_tbl[13] = {coll = COLL_NONE};            -- Rays
static_tbl[14] = {coll = COLL_NONE};            -- Red lamp
static_tbl[15] = {coll = COLL_BBOX};            -- Metallic structure
static_tbl[16] = {coll = COLL_BBOX};            -- Metallic structure 2
static_tbl[17] = {coll = COLL_MESH};            -- Grated panel

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;