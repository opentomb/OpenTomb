-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, BIKEBIT.TR4

print("Level script loaded (BIKEBIT)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[01] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};              -- Tower
static_tbl[04] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Wall light
static_tbl[10] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Stone wall block
static_tbl[11] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Stone wall block 2
static_tbl[12] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Stone wall block 3
static_tbl[13] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Stone wall block 4
static_tbl[14] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Ladder
static_tbl[15] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Green top
static_tbl[16] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Pole
static_tbl[17] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Stone arch
static_tbl[18] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Thick pillar
static_tbl[19] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Ceiling part
static_tbl[20] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Stairs
static_tbl[21] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Stairs 2
static_tbl[22] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Pillar
static_tbl[23] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Pillar arches
static_tbl[24] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};              -- Street around the corner
static_tbl[25] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Stairs 3
static_tbl[26] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Stairs 4
static_tbl[27] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Pedestal
static_tbl[28] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Pedestal 2
static_tbl[31] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Stone arch part
static_tbl[32] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Ceiling part
static_tbl[34] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Rounded building
static_tbl[36] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};              -- Tower 2
static_tbl[37] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};              -- Tower 3
static_tbl[38] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Balcony
static_tbl[39] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Balcony 2
static_tbl[40] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Doorway
static_tbl[41] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = 1};    -- Doorway dummy mesh 1
static_tbl[42] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = 1};    -- Doorway dummy mesh 2
static_tbl[43] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Stairs 5
static_tbl[44] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Fence
static_tbl[45] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Grated fence 1
static_tbl[46] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Grated fence 2
static_tbl[47] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};              -- Stone decoration
static_tbl[48] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Stone arch part 2
static_tbl[49] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Stone arch part 3

-- SHATTER STATICS

static_tbl[51] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Ice spirit capsule
static_tbl[52] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};              -- Warning fence
static_tbl[53] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};          -- Fuel barrel
