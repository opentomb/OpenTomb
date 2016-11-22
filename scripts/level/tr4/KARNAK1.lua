-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, KARNAK1

print("Level script loaded (KARNAK1.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

-- PLANT statics (as listed in OBJECTS.H from TRLE)

static_tbl[0]  = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Tree
static_tbl[11] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};            -- Gate
static_tbl[12] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX, hide = 1};  -- Gate: dummy static 1!
static_tbl[13] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX, hide = 1};  -- Gate: dummy static 2!
static_tbl[14] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Plate
static_tbl[15] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Vase
static_tbl[16] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX, hide = 1};  -- Cube: dummy static 3!
static_tbl[17] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Pedestal

-- ROCK statics

static_tbl[20] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Pillar
static_tbl[21] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Pillar (half)
static_tbl[22] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Border stones
static_tbl[23] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Horn
static_tbl[24] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Hanging statue 1
static_tbl[25] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Hanging statue 2
static_tbl[26] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Pillar 2
static_tbl[29] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Wall pillar

-- ARCHITECTURE statics

static_tbl[30] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Obelisk, lower part
static_tbl[31] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Obelisk, higher part
static_tbl[37] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Pillar 3
static_tbl[38] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Arch
static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Grated gate (closed)

-- SHATTER statics
-- Note: all SHATTER statics can be destroyed by SHATTER event (either on hit or something else)

static_tbl[50] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Vase 2