-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, NEVADA.TR2

-- ColType values reference: 

COLL_NONE = 0x0000;  -- Object has no collisions
COLL_MESH = 0x0001;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x0002;  -- Object uses bounding box for collision.

print("Level script loaded (NEVADA.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[00] = {coll = COLL_NONE};            -- Bush
static_tbl[10] = {coll = COLL_BBOX};            -- Desk
static_tbl[11] = {coll = COLL_BBOX};            -- Warning sign
static_tbl[12] = {coll = COLL_BBOX};            -- Door border
static_tbl[13] = {coll = COLL_BBOX};            -- Door border 2
static_tbl[14] = {coll = COLL_MESH};            -- Opened door
static_tbl[15] = {coll = COLL_BBOX};            -- Locker
static_tbl[20] = {coll = COLL_NONE};            -- Cactuss part 1
static_tbl[21] = {coll = COLL_BBOX};            -- Cactuss part 2
static_tbl[22] = {coll = COLL_NONE};            -- Cactuss top
static_tbl[30] = {coll = COLL_BBOX};            -- Barbed wire 1
static_tbl[40] = {coll = COLL_BBOX};            -- Barbed wire 2
static_tbl[41] = {coll = COLL_BBOX};            -- Ceiling tile
static_tbl[48] = {coll = COLL_BBOX, hide = 1};  -- Horizontal collision panel: dummy static!
static_tbl[49] = {coll = COLL_BBOX};            -- Chain

--------------------------------------------------------------------------------

function trGetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;