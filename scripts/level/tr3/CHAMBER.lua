-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, CHAMBER.TR2

print("Level script loaded (CHAMBER.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Stone face
static_tbl[10] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Floodlight
static_tbl[11] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Blue lamp
static_tbl[12] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Floodlight 2
static_tbl[13] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Rays
static_tbl[14] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Red lamp
static_tbl[15] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Metallic structure
static_tbl[16] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Metallic structure 2
static_tbl[17] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};        -- Grated panel
