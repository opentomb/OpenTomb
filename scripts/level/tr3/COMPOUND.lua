-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, COMPOUND.TR2

-- ColType values reference: 

COLL_NONE = 0x00;  -- Object has no collisions
COLL_MESH = 0x01;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x02;  -- Object uses bounding box for collision.

print("Level script loaded (COMPOUND.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[10] = {coll = COLL_MESH};            -- Misaligned crate
static_tbl[20] = {coll = COLL_MESH};            -- Truck
static_tbl[21] = {coll = COLL_BBOX};            -- Truck wheels
static_tbl[23] = {coll = COLL_MESH};            -- Dining desk 1
static_tbl[24] = {coll = COLL_MESH};            -- Dining desk 2
static_tbl[25] = {coll = COLL_NONE};            -- Twin chain
static_tbl[26] = {coll = COLL_NONE};            -- Ceiling lamp
static_tbl[30] = {coll = COLL_MESH};            -- Sink
static_tbl[31] = {coll = COLL_MESH};            -- WC
static_tbl[32] = {coll = COLL_BBOX};            -- Vertical pole
static_tbl[33] = {coll = COLL_BBOX};            -- Grated panel
static_tbl[34] = {coll = COLL_MESH};            -- Chair
static_tbl[35] = {coll = COLL_BBOX};            -- Mainframe 1
static_tbl[36] = {coll = COLL_BBOX};            -- Mainframe 2
static_tbl[37] = {coll = COLL_MESH};            -- Sink set
static_tbl[38] = {coll = COLL_MESH};            -- WC set
static_tbl[39] = {coll = COLL_NONE};            -- Keypad
static_tbl[40] = {coll = COLL_BBOX};            -- Pipe
static_tbl[41] = {coll = COLL_NONE};            -- Food 1
static_tbl[42] = {coll = COLL_NONE};            -- Food 2
static_tbl[47] = {coll = COLL_NONE};            -- F-117

--------------------------------------------------------------------------------

function trGetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;