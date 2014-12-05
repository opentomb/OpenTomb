-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, ALEXHUB

print("Level script loaded (ALEXHUB.lua)");

--------------------------------------------------------------------------------
-- ColType values reference: 

COLL_NONE = 0x00;  -- Object has no collisions
COLL_MESH = 0x01;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x02;  -- Object uses bounding box for collision.

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[22] = {coll = COLL_MESH};                 -- Stone arc
static_tbl[23] = {coll = COLL_BBOX};                 -- Dummy mummy
static_tbl[24] = {coll = COLL_MESH};                 -- Stairs
static_tbl[25] = {coll = COLL_BBOX};                 -- Dummy pharaoh
static_tbl[26] = {coll = COLL_NONE};                 -- Wall light
static_tbl[27] = {coll = COLL_BBOX};                 -- Tree
static_tbl[30] = {coll = COLL_MESH};                 -- Stone arc 2
static_tbl[31] = {coll = COLL_MESH};                 -- Stone arc 3
static_tbl[32] = {coll = COLL_BBOX};                 -- Stone arc with pillar 1 (collision only for pillar)
static_tbl[33] = {coll = COLL_BBOX};                 -- Stone arc with pillar 2 (collision only for pillar)
static_tbl[34] = {coll = COLL_BBOX};                 -- Pillar
static_tbl[35] = {coll = COLL_MESH};                 -- Stone arc 2
static_tbl[36] = {coll = COLL_BBOX, hide = 1};       -- Dummy platform: dummy static 1!
static_tbl[40] = {coll = COLL_NONE};                 -- Rays
static_tbl[41] = {coll = COLL_NONE};                 -- Mechanism
static_tbl[42] = {coll = COLL_NONE};                 -- Mechanism 2
static_tbl[43] = {coll = COLL_BBOX};                 -- Fire basket
static_tbl[44] = {coll = COLL_BBOX, hide = 1};       -- Panel: dummy static 2!
static_tbl[46] = {coll = COLL_BBOX};                 -- Hook
static_tbl[47] = {coll = COLL_MESH};                 -- Sweeper
static_tbl[48] = {coll = COLL_MESH};                 -- Stairs 2

-- SHATTER statics
-- Note: all SHATTER statics can be destroyed by SHATTER event (either on hit or something else)

static_tbl[50] = {coll = COLL_BBOX};                 -- Egyptian adventure barricade
static_tbl[51] = {coll = COLL_BBOX};                 -- Lock
static_tbl[52] = {coll = COLL_MESH};                 -- Target

--------------------------------------------------------------------------------

function trGetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;