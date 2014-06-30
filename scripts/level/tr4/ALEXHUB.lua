-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, ALEXHUB

--------------------------------------------------------------------------------

-- ColType values reference: 

NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.

print("Level script loaded (ALEXHUB.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[10] = {coll = BBOX};       			-- Pillar
static_tbl[11] = {coll = NONE};       			-- Wall lamp
static_tbl[12] = {coll = BBOX};       			-- Book shelf
static_tbl[13] = {coll = BBOX, hide = 1};       -- Book shelf 1, only upper part
static_tbl[14] = {coll = BBOX, hide = 1};       -- Book shelf 2, only upper part
static_tbl[18] = {coll = BBOX};                 -- Locker
static_tbl[19] = {coll = NONE};                 -- Curtains
static_tbl[20] = {coll = BBOX};                 -- Book shelf 3
static_tbl[23] = {coll = BBOX};                 -- Doorway
static_tbl[25] = {coll = MESH};                 -- Stairs
static_tbl[28] = {coll = BBOX, hide = 1};       -- Doorway blocker: dummy static 1!
static_tbl[29] = {coll = BBOX, hide = 1};       -- Doorway blocker: dummy static 2!
static_tbl[31] = {coll = MESH};                 -- Stone arc
static_tbl[32] = {coll = BBOX};                 -- Stone arc with pillar 1 (collision only for pillar)
static_tbl[33] = {coll = BBOX};                 -- Stone arc with pillar 2 (collision only for pillar)
static_tbl[34] = {coll = BBOX};                 -- Pillar
static_tbl[35] = {coll = MESH};                 -- Stone arc 2
static_tbl[36] = {coll = MESH};                 -- White ornate arc 1
static_tbl[37] = {coll = MESH};                 -- White ornate arc 2
static_tbl[38] = {coll = MESH};                 -- White ornate arc 3
static_tbl[39] = {coll = MESH};                 -- White ornate arc 4
static_tbl[40] = {coll = BBOX};                 -- Train connector
static_tbl[41] = {coll = BBOX};                 -- Train connector 2
static_tbl[42] = {coll = NONE};                 -- Train wheels
static_tbl[43] = {coll = BBOX};                 -- Pedestal
static_tbl[44] = {coll = NONE};                 -- Train carcass
static_tbl[45] = {coll = NONE};                 -- Train part
static_tbl[46] = {coll = MESH};                 -- Pillars

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;