-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, HOUSE.TR2

print("Level script loaded (HOUSE.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------
-- ColType values reference: 
COLL_NONE = 0x00;  -- Object has no collisions
COLL_MESH = 0x01;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x02;  -- Object uses bounding box for collision.
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[00] = {coll = COLL_MESH};       -- Flower pot 01
static_tbl[01] = {coll = COLL_MESH};       -- Flower pot 02
static_tbl[02] = {coll = COLL_MESH};       -- Flower pot 03
static_tbl[03] = {coll = COLL_BBOX};       -- Speaker
static_tbl[04] = {coll = COLL_BBOX};       -- WC
static_tbl[05] = {coll = COLL_BBOX};       -- Bush
static_tbl[06] = {coll = COLL_MESH};       -- Hi-fi system
static_tbl[07] = {coll = COLL_BBOX};       -- Tree
static_tbl[08] = {coll = COLL_MESH};       -- Scion showcase
static_tbl[09] = {coll = COLL_MESH};       -- Sofa
static_tbl[10] = {coll = COLL_MESH};       -- Chair
static_tbl[11] = {coll = COLL_MESH};       -- Idol showcase
static_tbl[12] = {coll = COLL_MESH};       -- T-Rex trophy head
static_tbl[13] = {coll = COLL_MESH};       -- Piano
static_tbl[14] = {coll = COLL_BBOX};       -- Book shelf 1
static_tbl[15] = {coll = COLL_BBOX};       -- Crystal cat (from TR1)
static_tbl[16] = {coll = COLL_MESH};       -- Column pedestal
static_tbl[17] = {coll = COLL_BBOX};       -- Grill
static_tbl[18] = {coll = COLL_MESH};       -- Kitchen desk 1
static_tbl[19] = {coll = COLL_MESH};       -- Kitchen desk 2
static_tbl[20] = {coll = COLL_MESH};       -- Fridge hanging meat
static_tbl[21] = {coll = COLL_MESH};       -- Sink
static_tbl[22] = {coll = COLL_MESH};       -- Kitchen desk 3
static_tbl[23] = {coll = COLL_MESH};       -- Kitchen chair
static_tbl[24] = {coll = COLL_MESH};       -- Lara's bed
static_tbl[25] = {coll = COLL_MESH};       -- Lara's bed (front)
static_tbl[26] = {coll = COLL_MESH};       -- WC (bidet)
static_tbl[27] = {coll = COLL_BBOX};       -- Grandpa's clock
static_tbl[28] = {coll = COLL_NONE};       -- Furniture part
static_tbl[29] = {coll = COLL_NONE};       -- Curtain
static_tbl[30] = {coll = COLL_MESH};       -- Fence flat
static_tbl[31] = {coll = COLL_MESH};       -- Fence tilt 1
static_tbl[32] = {coll = COLL_MESH};       -- Fence tilt 2
static_tbl[33] = {coll = COLL_BBOX};       -- Fence column
static_tbl[34] = {coll = COLL_NONE};       -- Chandelier
static_tbl[35] = {coll = COLL_BBOX};       -- Statue
static_tbl[36] = {coll = COLL_MESH};       -- TV
static_tbl[37] = {coll = COLL_MESH};       -- Dining room table part 1
static_tbl[38] = {coll = COLL_MESH};       -- Dining room table part 2
static_tbl[39] = {coll = COLL_NONE};       -- Pole
static_tbl[40] = {coll = COLL_MESH};       -- Kitchen shelf 1
static_tbl[41] = {coll = COLL_MESH};       -- Kitchen shelf 2
static_tbl[42] = {coll = COLL_MESH};       -- Kitchen shelf 3
static_tbl[43] = {coll = COLL_MESH};       -- Kitchen sink
static_tbl[44] = {coll = COLL_MESH};       -- Kitchen shelf 4 (corner)
static_tbl[45] = {coll = COLL_MESH};       -- Horse statue
static_tbl[46] = {coll = COLL_NONE};       -- Fireplace
static_tbl[47] = {coll = COLL_MESH};       -- Kitchen shelf 5
static_tbl[48] = {coll = COLL_BBOX};       -- Shower
static_tbl[49] = {coll = COLL_MESH};       -- Lamp 

--------------------------------------------------------------------------------

function trGetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;