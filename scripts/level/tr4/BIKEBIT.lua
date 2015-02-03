-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, BIKEBIT.TR4

print("Level script loaded (BIKEBIT)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------
-- ColType values reference: 
COLL_NONE = 0x00;  -- Object has no collisions
COLL_MESH = 0x01;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x02;  -- Object uses bounding box for collision.
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[01] = {coll = COLL_NONE};              -- Tower
static_tbl[04] = {coll = COLL_MESH};              -- Wall light
static_tbl[10] = {coll = COLL_BBOX};              -- Stone wall block
static_tbl[11] = {coll = COLL_BBOX};              -- Stone wall block 2
static_tbl[12] = {coll = COLL_BBOX};              -- Stone wall block 3
static_tbl[13] = {coll = COLL_BBOX};              -- Stone wall block 4
static_tbl[14] = {coll = COLL_BBOX};              -- Ladder
static_tbl[15] = {coll = COLL_BBOX};              -- Green top
static_tbl[16] = {coll = COLL_BBOX};              -- Pole
static_tbl[17] = {coll = COLL_BBOX};              -- Stone arch
static_tbl[18] = {coll = COLL_BBOX};              -- Thick pillar
static_tbl[19] = {coll = COLL_MESH};              -- Ceiling part
static_tbl[20] = {coll = COLL_MESH};              -- Stairs
static_tbl[21] = {coll = COLL_MESH};              -- Stairs 2
static_tbl[22] = {coll = COLL_BBOX};              -- Pillar
static_tbl[23] = {coll = COLL_BBOX};              -- Pillar arches
static_tbl[24] = {coll = COLL_NONE};              -- Street around the corner
static_tbl[25] = {coll = COLL_MESH};              -- Stairs 3
static_tbl[26] = {coll = COLL_MESH};              -- Stairs 4
static_tbl[27] = {coll = COLL_BBOX};              -- Pedestal
static_tbl[28] = {coll = COLL_BBOX};              -- Pedestal 2
static_tbl[31] = {coll = COLL_MESH};              -- Stone arch part
static_tbl[32] = {coll = COLL_BBOX};              -- Ceiling part
static_tbl[34] = {coll = COLL_MESH};              -- Rounded building
static_tbl[36] = {coll = COLL_NONE};              -- Tower 2
static_tbl[37] = {coll = COLL_NONE};              -- Tower 3
static_tbl[38] = {coll = COLL_MESH};              -- Balcony
static_tbl[39] = {coll = COLL_MESH};              -- Balcony 2
static_tbl[40] = {coll = COLL_BBOX};              -- Doorway
static_tbl[41] = {coll = COLL_BBOX, hide = 1};    -- Doorway dummy mesh 1
static_tbl[42] = {coll = COLL_BBOX, hide = 1};    -- Doorway dummy mesh 2
static_tbl[43] = {coll = COLL_MESH};              -- Stairs 5
static_tbl[44] = {coll = COLL_BBOX};              -- Fence
static_tbl[45] = {coll = COLL_MESH};              -- Grated fence 1
static_tbl[46] = {coll = COLL_MESH};              -- Grated fence 2
static_tbl[47] = {coll = COLL_NONE};              -- Stone decoration
static_tbl[48] = {coll = COLL_MESH};              -- Stone arch part 2
static_tbl[49] = {coll = COLL_MESH};              -- Stone arch part 3

-- SHATTER STATICS

static_tbl[51] = {coll = COLL_BBOX};              -- Ice spirit capsule
static_tbl[52] = {coll = COLL_BBOX};              -- Warning fence
static_tbl[53] = {coll = COLL_MESH};              -- Fuel barrel

--------------------------------------------------------------------------------

function trGetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;