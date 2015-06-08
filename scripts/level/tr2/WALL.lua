-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, WALL.TR2

print("Level script loaded (WALL.lua)");
 
-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLL_MESH};            -- Bank rock
static_tbl[01] = {coll = COLL_NONE};            -- Death slide rope
static_tbl[02] = {coll = COLL_NONE};            -- Skeleton 1
static_tbl[03] = {coll = COLL_NONE};            -- Skeleton 2
static_tbl[04] = {coll = COLL_NONE};            -- Cobweb
static_tbl[10] = {coll = COLL_NONE};            -- Fireplace
static_tbl[11] = {coll = COLL_MESH};            -- Crate with book
static_tbl[12] = {coll = COLL_MESH};            -- Briefcase
static_tbl[13] = {coll = COLL_MESH};            -- Crates