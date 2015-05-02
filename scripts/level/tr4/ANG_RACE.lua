-- OPENTOMB PER-LEVEL STATIC MESH COLLISION SCRIPT
-- FOR TOMB RAIDER 4, ANG_RACE

print("Level script loaded (ANG_RACE.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

-- PLANT statics (as listed in OBJECTS.H from TRLE)

static_tbl[00] = {coll = COLL_NONE};            -- Grass
static_tbl[01] = {coll = COLL_NONE};            -- Tree part
static_tbl[02] = {coll = COLL_NONE};            -- Tree part 2
static_tbl[03] = {coll = COLL_NONE};            -- Tree part 3
static_tbl[04] = {coll = COLL_NONE};            -- Tree part 4
static_tbl[05] = {coll = COLL_NONE};            -- Bush 1
static_tbl[06] = {coll = COLL_NONE};            -- Bush 2
static_tbl[07] = {coll = COLL_NONE};            -- Bush 3

-- FURNITURE statics

static_tbl[10] = {coll = COLL_BBOX, hide = 1};  -- Dummy cube

-- ROCK statics

static_tbl[20] = {coll = COLL_MESH};            -- Pedestal
static_tbl[21] = {coll = COLL_NONE};            -- Stones
static_tbl[22] = {coll = COLL_NONE};            -- Wall light

-- ARCHITECTURE statics

static_tbl[30] = {coll = COLL_BBOX};            -- Ornate arch
static_tbl[31] = {coll = COLL_BBOX};            -- Pillar
static_tbl[32] = {coll = COLL_BBOX};            -- Scripture plate
static_tbl[33] = {coll = COLL_BBOX};            -- Dummy panel

-- DEBRIS statics

static_tbl[40] = {coll = COLL_NONE};            -- Ornate arch 2
static_tbl[41] = {coll = COLL_MESH};            -- Stone border
static_tbl[42] = {coll = COLL_BBOX};            -- Pillar 2
static_tbl[43] = {coll = COLL_BBOX};            -- Stone lion
static_tbl[46] = {coll = COLL_BBOX};            -- Ornate arch with pillars
static_tbl[47] = {coll = COLL_BBOX, hide = 1};  -- Block panel: dummy static!