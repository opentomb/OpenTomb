-- OPENTOMB PER-LEVEL STATIC MESH COLLISION SCRIPT
-- FOR TOMB RAIDER 4, ANG_RACE

print("Level script loaded (ANG_RACE.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

-- PLANT statics (as listed in OBJECTS.H from TRLE)

static_tbl[00] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Grass
static_tbl[01] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Tree part
static_tbl[02] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Tree part 2
static_tbl[03] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Tree part 3
static_tbl[04] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Tree part 4
static_tbl[05] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Bush 1
static_tbl[06] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Bush 2
static_tbl[07] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Bush 3

-- FURNITURE statics

static_tbl[10] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = 1};  -- Dummy cube

-- ROCK statics

static_tbl[20] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};        -- Pedestal
static_tbl[21] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Stones
static_tbl[22] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Wall light

-- ARCHITECTURE statics

static_tbl[30] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Ornate arch
static_tbl[31] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Pillar
static_tbl[32] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Scripture plate
static_tbl[33] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Dummy panel

-- DEBRIS statics

static_tbl[40] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};            -- Ornate arch 2
static_tbl[41] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};        -- Stone border
static_tbl[42] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Pillar 2
static_tbl[43] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Stone lion
static_tbl[46] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};            -- Ornate arch with pillars
static_tbl[47] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = 1};  -- Block panel: dummy static!
