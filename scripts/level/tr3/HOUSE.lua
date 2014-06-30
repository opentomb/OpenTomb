-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, HOUSE.TR2

print("Level script loaded (HOUSE.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------
-- ColType values reference: 
NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[00] = {coll = MESH};       -- Flower pot 01
static_tbl[01] = {coll = MESH};       -- Flower pot 02
static_tbl[02] = {coll = MESH};       -- Flower pot 03
static_tbl[03] = {coll = BBOX};       -- Speaker
static_tbl[04] = {coll = BBOX};       -- WC
static_tbl[05] = {coll = BBOX};       -- Bush
static_tbl[06] = {coll = MESH};       -- Hi-fi system
static_tbl[07] = {coll = BBOX};       -- Tree
static_tbl[08] = {coll = MESH};       -- Scion showcase
static_tbl[09] = {coll = MESH};       -- Sofa
static_tbl[10] = {coll = MESH};       -- Chair
static_tbl[11] = {coll = MESH};       -- Idol showcase
static_tbl[12] = {coll = MESH};       -- T-Rex trophy head
static_tbl[13] = {coll = MESH};       -- Piano
static_tbl[14] = {coll = BBOX};       -- Book shelf 1
static_tbl[15] = {coll = BBOX};       -- Crystal cat (from TR1)
static_tbl[16] = {coll = MESH};       -- Column pedestal
static_tbl[17] = {coll = BBOX};       -- Grill
static_tbl[18] = {coll = MESH};       -- Kitchen desk 1
static_tbl[19] = {coll = MESH};       -- Kitchen desk 2
static_tbl[20] = {coll = MESH};       -- Fridge hanging meat
static_tbl[21] = {coll = MESH};       -- Sink
static_tbl[22] = {coll = MESH};       -- Kitchen desk 3
static_tbl[23] = {coll = MESH};       -- Kitchen chair
static_tbl[24] = {coll = MESH};       -- Lara's bed
static_tbl[25] = {coll = MESH};       -- Lara's bed (front)
static_tbl[26] = {coll = MESH};       -- WC (bidet)
static_tbl[27] = {coll = BBOX};       -- Grandpa's clock
static_tbl[28] = {coll = NONE};       -- Furniture part
static_tbl[29] = {coll = NONE};       -- Curtain
static_tbl[30] = {coll = MESH};       -- Fence flat
static_tbl[31] = {coll = MESH};       -- Fence tilt 1
static_tbl[32] = {coll = MESH};       -- Fence tilt 2
static_tbl[33] = {coll = BBOX};       -- Fence column
static_tbl[34] = {coll = NONE};       -- Chandelier
static_tbl[35] = {coll = BBOX};       -- Statue
static_tbl[36] = {coll = MESH};       -- TV
static_tbl[37] = {coll = MESH};       -- Dining room table part 1
static_tbl[38] = {coll = MESH};       -- Dining room table part 2
static_tbl[39] = {coll = NONE};       -- Pole
static_tbl[40] = {coll = MESH};       -- Kitchen shelf 1
static_tbl[41] = {coll = MESH};       -- Kitchen shelf 2
static_tbl[42] = {coll = MESH};       -- Kitchen shelf 3
static_tbl[43] = {coll = MESH};       -- Kitchen sink
static_tbl[44] = {coll = MESH};       -- Kitchen shelf 4 (corner)
static_tbl[45] = {coll = MESH};       -- Horse statue
static_tbl[46] = {coll = NONE};       -- Fireplace
static_tbl[47] = {coll = MESH};       -- Kitchen shelf 5
static_tbl[48] = {coll = BBOX};       -- Shower
static_tbl[49] = {coll = MESH};       -- Lamp 

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;