-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, NEVADA.TR2

print("Level script loaded (NEVADA.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};            -- Bush
static_tbl[10] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Desk
static_tbl[11] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Warning sign
static_tbl[12] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Door border
static_tbl[13] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Door border 2
static_tbl[14] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Opened door
static_tbl[15] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Locker
static_tbl[20] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};            -- Cactuss part 1
static_tbl[21] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Cactuss part 2
static_tbl[22] = {coll = COLLISION_NONE,        shape = COLLISION_SHAPE_BOX};            -- Cactuss top
static_tbl[30] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Barbed wire 1
static_tbl[40] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Barbed wire 2
static_tbl[41] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Ceiling tile
static_tbl[48] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX, hide = 1};  -- Horizontal collision panel: dummy static!
static_tbl[49] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Chain