-- OPENTOMB STATIC MESH COLLISION SCRIPT
-- FOR TOMB RAIDER, LEVEL1 (CAVES)

print("Level script loaded (LEVEL7B.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[06] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};           -- Hanging plant
static_tbl[08] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};           -- Hanging plant
static_tbl[10] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wood barrier
static_tbl[33] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 1
static_tbl[34] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 2
static_tbl[38] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};           -- Door frame
static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wall bricks
static_tbl[43] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};           -- Icicle
