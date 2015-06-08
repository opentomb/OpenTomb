-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, AREA51.TR2

print("Level script loaded (AREA51.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[00] = {coll = COLL_MESH};            -- Rocket holder top
static_tbl[01] = {coll = COLL_BBOX};            -- Rocket holder bottom
static_tbl[02] = {coll = COLL_NONE};            -- Alien
static_tbl[03] = {coll = COLL_NONE};            -- Alien (vertical)
static_tbl[10] = {coll = COLL_MESH};            -- Small rocket (vertical)
static_tbl[14] = {coll = COLL_BBOX};            -- Grating
static_tbl[17] = {coll = COLL_BBOX};            -- Big rocket bottom
static_tbl[18] = {coll = COLL_BBOX};            -- Vertical grated holder
static_tbl[19] = {coll = COLL_BBOX};            -- Big rocket top
static_tbl[20] = {coll = COLL_BBOX};            -- Big rocket very top
static_tbl[24] = {coll = COLL_BBOX};            -- Transmitter 1
static_tbl[25] = {coll = COLL_BBOX};            -- Transmitter 2
static_tbl[26] = {coll = COLL_NONE};            -- Prism
static_tbl[27] = {coll = COLL_BBOX};            -- Fixed rocket part 1
static_tbl[28] = {coll = COLL_BBOX};            -- Fixed rocket part 2
static_tbl[29] = {coll = COLL_BBOX};            -- Cell desk holder
static_tbl[30] = {coll = COLL_MESH};            -- Cell furniture
static_tbl[31] = {coll = COLL_BBOX};            -- Locker
static_tbl[32] = {coll = COLL_MESH};            -- Misaligned crate
static_tbl[33] = {coll = COLL_BBOX, hide = 1};  -- Collision panel
static_tbl[34] = {coll = COLL_NONE};            -- Horizontal top hole
static_tbl[35] = {coll = COLL_BBOX};            -- Horizontal panel
static_tbl[37] = {coll = COLL_NONE};            -- Camera
static_tbl[38] = {coll = COLL_MESH};            -- WC
static_tbl[39] = {coll = COLL_MESH};            -- Sink
static_tbl[40] = {coll = COLL_BBOX};            -- Ring
static_tbl[41] = {coll = COLL_BBOX};            -- Armoury locker
static_tbl[42] = {coll = COLL_NONE};            -- MP5 sign
static_tbl[43] = {coll = COLL_NONE};            -- Keypad
static_tbl[48] = {coll = COLL_MESH};            -- Pilot chair
static_tbl[49] = {coll = COLL_NONE};            -- Metallic panel