-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, HOUSE.TR2

print("Level script loaded (HOUSE.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Flower pot 01
static_tbl[01] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Flower pot 02
static_tbl[02] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Flower pot 03
static_tbl[03] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Speaker
static_tbl[04] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- WC
static_tbl[05] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Bush
static_tbl[06] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Hi-fi system
static_tbl[07] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Tree
static_tbl[08] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Scion showcase
static_tbl[09] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Sofa
static_tbl[10] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Chair
static_tbl[11] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Idol showcase
static_tbl[12] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- T-Rex trophy head
static_tbl[13] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Piano
static_tbl[14] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Book shelf 1
static_tbl[15] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Crystal cat (from TR1)
static_tbl[16] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Column pedestal
static_tbl[17] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Grill
static_tbl[18] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen desk 1
static_tbl[19] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen desk 2
static_tbl[20] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Fridge hanging meat
static_tbl[21] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Sink
static_tbl[22] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen desk 3
static_tbl[23] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen chair
static_tbl[24] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Lara's bed
static_tbl[25] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Lara's bed (front)
static_tbl[26] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- WC (bidet)
static_tbl[27] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Grandpa's clock
static_tbl[28] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Furniture part
static_tbl[29] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Curtain
static_tbl[30] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Fence flat
static_tbl[31] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Fence tilt 1
static_tbl[32] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Fence tilt 2
static_tbl[33] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Fence column
static_tbl[34] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Chandelier
static_tbl[35] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Statue
static_tbl[36] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- TV
static_tbl[37] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Dining room table part 1
static_tbl[38] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Dining room table part 2
static_tbl[39] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Pole
static_tbl[40] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen shelf 1
static_tbl[41] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen shelf 2
static_tbl[42] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen shelf 3
static_tbl[43] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen sink
static_tbl[44] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen shelf 4 (corner)
static_tbl[45] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Horse statue
static_tbl[46] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Fireplace
static_tbl[47] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Kitchen shelf 5
static_tbl[48] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Shower
static_tbl[49] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Lamp 