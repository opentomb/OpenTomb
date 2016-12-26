-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, CITNEW.TR4

print("Level script loaded (CITNEW.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};       -- Horizontal rope
static_tbl[01] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Pillar 1
static_tbl[02] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Pillar 2
static_tbl[03] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};       -- Horizontal rope 2
static_tbl[04] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};       -- Floor debris
static_tbl[05] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Wall light
static_tbl[08] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Wall pillar
static_tbl[09] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stone arch
static_tbl[10] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};       -- Rays
static_tbl[11] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Tower part
static_tbl[12] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stone arch 2
static_tbl[13] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Ceiling with hole
static_tbl[15] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stairs
static_tbl[16] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stairs 2
static_tbl[17] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Wall light 2
static_tbl[18] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Pillar 3
static_tbl[19] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stone arch 3
static_tbl[20] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Building block 1
static_tbl[21] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stone arch part 1
static_tbl[22] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Thin pillar 1
static_tbl[23] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stone arch part 2
static_tbl[24] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Thin pillar 2
static_tbl[25] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Building block 2
static_tbl[26] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Rounded wall stone
static_tbl[27] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Ornate arch
static_tbl[28] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Pillar 4
static_tbl[29] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Thin pillar 3
static_tbl[32] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stone arch 4
static_tbl[33] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Fence 1
static_tbl[34] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Fence 2
static_tbl[35] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Cubical structure
static_tbl[36] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stone arch 5
static_tbl[37] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Horizontal rod
static_tbl[38] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};       -- Rectangular hole
static_tbl[39] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};       -- Grated pole
static_tbl[40] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stone block
static_tbl[41] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Pillar 5
static_tbl[42] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Pillar 6
static_tbl[43] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};       -- Pole
static_tbl[44] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Wall stone block
static_tbl[45] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Rounded wall block
static_tbl[46] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Grated arch part
static_tbl[47] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Grated arch part 2
static_tbl[48] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Wall pedestal
static_tbl[49] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};   -- Stone arch 6

-- SHATTER statics

static_tbl[59] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};       -- Wooden structure