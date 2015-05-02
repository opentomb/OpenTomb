-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, CITY.TR2

print("Level script loaded (CITY.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[10] = {coll = COLL_BBOX};            -- Floor lamp
static_tbl[11] = {coll = COLL_BBOX};            -- Lamp upper part
static_tbl[12] = {coll = COLL_BBOX};            -- Floodlight
static_tbl[13] = {coll = COLL_NONE};            -- Rays
static_tbl[14] = {coll = COLL_MESH};            -- Tinnos wall-mounted urn
static_tbl[15] = {coll = COLL_BBOX};            -- Statue
static_tbl[16] = {coll = COLL_BBOX};            -- Stone structure
static_tbl[18] = {coll = COLL_BBOX};            -- Tiki
static_tbl[19] = {coll = COLL_MESH};            -- Wooden slant
static_tbl[20] = {coll = COLL_NONE};            -- Crystal formation
static_tbl[30] = {coll = COLL_BBOX};            -- Metallic structure
static_tbl[31] = {coll = COLL_NONE};            -- Puzzle tile set 1
static_tbl[32] = {coll = COLL_NONE};            -- Puzzle tile set 2
static_tbl[33] = {coll = COLL_NONE};            -- Puzzle tile set 3
static_tbl[34] = {coll = COLL_NONE};            -- Puzzle tile set 4
static_tbl[35] = {coll = COLL_NONE};            -- Puzzle tile set 5
static_tbl[36] = {coll = COLL_BBOX};            -- Tinnos statue
static_tbl[37] = {coll = COLL_BBOX};            -- Tinnos idol
static_tbl[38] = {coll = COLL_BBOX};            -- Fire pedestal
static_tbl[39] = {coll = COLL_BBOX};            -- Thin pillar