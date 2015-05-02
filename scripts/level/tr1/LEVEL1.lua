-- OPENTOMB STATIC MESH COLLISION SCRIPT
-- FOR TOMB RAIDER, LEVEL1 (CAVES)

print("Level script loaded (LEVEL1.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[06] = {coll = COLL_NONE};             -- Hanging plant
static_tbl[08] = {coll = COLL_NONE};             -- Hanging plant
static_tbl[10] = {coll = COLL_MESH};             -- Wood barrier
static_tbl[33] = {coll = COLL_MESH};             -- Bridge part 1
static_tbl[34] = {coll = COLL_MESH};             -- Bridge part 2
static_tbl[38] = {coll = COLL_NONE};             -- Door frame
static_tbl[39] = {coll = COLL_MESH};             -- Wall bricks
static_tbl[43] = {coll = COLL_NONE};             -- Icicle