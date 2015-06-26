-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, ALEXHUB

print("Level script loaded (ALEXHUB.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[22] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Stone arc
static_tbl[23] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Dummy mummy
static_tbl[24] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Stairs
static_tbl[25] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Dummy pharaoh
static_tbl[26] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};                 -- Wall light
static_tbl[27] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Tree
static_tbl[30] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Stone arc 2
static_tbl[31] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Stone arc 3
static_tbl[32] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Stone arc with pillar 1 (collision only for pillar)
static_tbl[33] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Stone arc with pillar 2 (collision only for pillar)
static_tbl[34] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Pillar
static_tbl[35] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Stone arc 2
static_tbl[36] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = 1};       -- Dummy platform: dummy static 1!
static_tbl[40] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};                 -- Rays
static_tbl[41] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};                 -- Mechanism
static_tbl[42] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};                 -- Mechanism 2
static_tbl[43] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Fire basket
static_tbl[44] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = 1};       -- Panel: dummy static 2!
static_tbl[46] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Hook
static_tbl[47] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Sweeper
static_tbl[48] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Stairs 2

-- SHATTER statics
-- Note: all SHATTER statics can be destroyed by SHATTER event (either on hit or something else)

static_tbl[50] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Egyptian adventure barricade
static_tbl[51] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Lock
static_tbl[52] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Target
