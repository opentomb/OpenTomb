-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, KARNAK1

print("Level script loaded (KARNAK1.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------
-- ColType values reference: 
COLL_NONE = 0x00;  -- Object has no collisions
COLL_MESH = 0x01;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x02;  -- Object uses bounding box for collision.
--------------------------------------------------------------------------------

static_tbl = {};

-- PLANT statics (as listed in OBJECTS.H from TRLE)

static_tbl[0]  = {coll = COLL_BBOX};            -- Tree
static_tbl[11] = {coll = COLL_NONE};            -- Gate
static_tbl[12] = {coll = COLL_BBOX, hide = 1};  -- Gate: dummy static 1!
static_tbl[13] = {coll = COLL_BBOX, hide = 1};  -- Gate: dummy static 2!
static_tbl[14] = {coll = COLL_MESH};            -- Plate
static_tbl[15] = {coll = COLL_MESH};            -- Vase
static_tbl[16] = {coll = COLL_NONE, hide = 1};  -- Cube: dummy static 3!
static_tbl[17] = {coll = COLL_BBOX};            -- Pedestal

-- ROCK statics

static_tbl[20] = {coll = COLL_BBOX};            -- Pillar
static_tbl[21] = {coll = COLL_MESH};            -- Pillar (half)
static_tbl[22] = {coll = COLL_MESH};            -- Border stones
static_tbl[23] = {coll = COLL_MESH};            -- Horn
static_tbl[24] = {coll = COLL_BBOX};            -- Hanging statue 1
static_tbl[25] = {coll = COLL_BBOX};            -- Hanging statue 2
static_tbl[26] = {coll = COLL_BBOX};            -- Pillar 2
static_tbl[29] = {coll = COLL_BBOX};            -- Wall pillar

-- ARCHITECTURE statics

static_tbl[30] = {coll = COLL_BBOX};            -- Obelisk, lower part
static_tbl[31] = {coll = COLL_BBOX};            -- Obelisk, higher part
static_tbl[37] = {coll = COLL_BBOX};            -- Pillar 3
static_tbl[38] = {coll = COLL_MESH};            -- Arch
static_tbl[39] = {coll = COLL_BBOX};            -- Grated gate (closed)

-- SHATTER statics
-- Note: all SHATTER statics can be destroyed by SHATTER event (either on hit or something else)

static_tbl[50] = {coll = COLL_MESH};            -- Vase 2

--------------------------------------------------------------------------------

function trGetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;

