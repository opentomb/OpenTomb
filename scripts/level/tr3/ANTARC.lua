-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, ANTARC.TR2

print("Level script loaded (ANTARC.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};        -- Rounded desk
static_tbl[01] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};        -- Chair
static_tbl[02] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Kitchen trash
static_tbl[10] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Fence
static_tbl[11] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Barrel
static_tbl[12] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Crate
static_tbl[13] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Computer desk
static_tbl[14] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Door border 1
static_tbl[15] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Door border 2
static_tbl[16] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Vertical pipe
static_tbl[17] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Horizontal pipe
static_tbl[18] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};        -- Bent horizontal pipe
static_tbl[19] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Bent vertical pipe
static_tbl[21] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Fence 2
static_tbl[22] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Lamp
static_tbl[23] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Lamp light
static_tbl[24] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};        -- Diagonal panel
static_tbl[25] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Door border 3
static_tbl[26] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Door border 4
static_tbl[30] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Diagonal fence
static_tbl[31] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Ice piece (floating)
static_tbl[32] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Metallic structure
static_tbl[33] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Kitchen trash 2
static_tbl[34] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};        -- Stairs
static_tbl[35] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Door border 5
static_tbl[36] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Door border 6
static_tbl[37] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Wharf buoy
static_tbl[38] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Wall mount
static_tbl[39] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};        -- Cardboxes (2)
