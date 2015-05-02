-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, ALEXHUB

print("Level script loaded (ALEXHUB.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl[10] = {coll = COLL_BBOX};                 -- Pillar
static_tbl[11] = {coll = COLL_NONE};                 -- Wall lamp
static_tbl[12] = {coll = COLL_BBOX};                 -- Book shelf
static_tbl[13] = {coll = COLL_BBOX, hide = 1};       -- Book shelf 1, only upper part
static_tbl[14] = {coll = COLL_BBOX, hide = 1};       -- Book shelf 2, only upper part
static_tbl[18] = {coll = COLL_BBOX};                 -- Locker
static_tbl[19] = {coll = COLL_NONE};                 -- Curtains
static_tbl[20] = {coll = COLL_BBOX};                 -- Book shelf 3
static_tbl[23] = {coll = COLL_BBOX};                 -- Doorway
static_tbl[25] = {coll = COLL_MESH};                 -- Stairs
static_tbl[28] = {coll = COLL_BBOX, hide = 1};       -- Doorway blocker: dummy static 1!
static_tbl[29] = {coll = COLL_BBOX, hide = 1};       -- Doorway blocker: dummy static 2!
static_tbl[31] = {coll = COLL_MESH};                 -- Stone arc
static_tbl[32] = {coll = COLL_BBOX};                 -- Stone arc with pillar 1 (collision only for pillar)
static_tbl[33] = {coll = COLL_BBOX};                 -- Stone arc with pillar 2 (collision only for pillar)
static_tbl[34] = {coll = COLL_BBOX};                 -- Pillar
static_tbl[35] = {coll = COLL_MESH};                 -- Stone arc 2
static_tbl[36] = {coll = COLL_MESH};                 -- White ornate arc 1
static_tbl[37] = {coll = COLL_MESH};                 -- White ornate arc 2
static_tbl[38] = {coll = COLL_MESH};                 -- White ornate arc 3
static_tbl[39] = {coll = COLL_MESH};                 -- White ornate arc 4
static_tbl[40] = {coll = COLL_BBOX};                 -- Train connector
static_tbl[41] = {coll = COLL_BBOX};                 -- Train connector 2
static_tbl[42] = {coll = COLL_NONE};                 -- Train wheels
static_tbl[43] = {coll = COLL_BBOX};                 -- Pedestal
static_tbl[44] = {coll = COLL_NONE};                 -- Train carcass
static_tbl[45] = {coll = COLL_NONE};                 -- Train part
static_tbl[46] = {coll = COLL_MESH};                 -- Pillars