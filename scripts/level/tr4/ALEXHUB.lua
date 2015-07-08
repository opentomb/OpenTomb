-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, ALEXHUB

print("Level script loaded (ALEXHUB.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[10] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Pillar
static_tbl[11] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};                 -- Wall lamp
static_tbl[12] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Book shelf
static_tbl[13] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = true};    -- Book shelf 1, only upper part
static_tbl[14] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = true};    -- Book shelf 2, only upper part
static_tbl[18] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Locker
static_tbl[19] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};                 -- Curtains
static_tbl[20] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Book shelf 3
static_tbl[23] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Doorway
static_tbl[25] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Stairs
static_tbl[28] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = true};    -- Doorway blocker: dummy static 1!
static_tbl[29] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = true};    -- Doorway blocker: dummy static 2!
static_tbl[31] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Stone arc
static_tbl[32] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Stone arc with pillar 1 (collision only for pillar)
static_tbl[33] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Stone arc with pillar 2 (collision only for pillar)
static_tbl[34] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Pillar
static_tbl[35] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Stone arc 2
static_tbl[36] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- White ornate arc 1
static_tbl[37] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- White ornate arc 2
static_tbl[38] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- White ornate arc 3
static_tbl[39] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- White ornate arc 4
static_tbl[40] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Train connector
static_tbl[41] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Train connector 2
static_tbl[42] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};                 -- Train wheels
static_tbl[43] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};                 -- Pedestal
static_tbl[44] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};                 -- Train carcass
static_tbl[45] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};                 -- Train part
static_tbl[46] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};             -- Pillars
