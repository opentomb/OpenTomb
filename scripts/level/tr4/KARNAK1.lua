-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, KARNAK1

print("Level script loaded (KARNAK1.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------
-- ColType values reference: 
NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.
--------------------------------------------------------------------------------

static_tbl = {};

-- PLANT statics (as listed in OBJECTS.H from TRLE)

static_tbl[0]  = {coll = BBOX};            -- Tree
static_tbl[11] = {coll = NONE};            -- Gate
static_tbl[12] = {coll = BBOX, hide = 1};  -- Gate: dummy static 1!
static_tbl[13] = {coll = BBOX, hide = 1};  -- Gate: dummy static 2!
static_tbl[14] = {coll = MESH};            -- Plate
static_tbl[15] = {coll = MESH};            -- Vase
static_tbl[16] = {coll = NONE, hide = 1};  -- Cube: dummy static 3!
static_tbl[17] = {coll = BBOX};            -- Pedestal

-- ROCK statics

static_tbl[20] = {coll = BBOX};            -- Pillar
static_tbl[21] = {coll = MESH};            -- Pillar (half)
static_tbl[22] = {coll = MESH};			   -- Border stones
static_tbl[23] = {coll = MESH};			   -- Horn
static_tbl[24] = {coll = BBOX};            -- Hanging statue 1
static_tbl[25] = {coll = BBOX};            -- Hanging statue 2
static_tbl[26] = {coll = BBOX};			   -- Pillar 2
static_tbl[29] = {coll = BBOX};            -- Wall pillar

-- ARCHITECTURE statics

static_tbl[30] = {coll = BBOX};            -- Obelisk, lower part
static_tbl[31] = {coll = BBOX};            -- Obelisk, higher part
static_tbl[37] = {coll = BBOX};            -- Pillar 3
static_tbl[38] = {coll = MESH};            -- Arch
static_tbl[39] = {coll = BBOX};            -- Grated gate (closed)

-- SHATTER statics
-- Note: all SHATTER statics can be destroyed by SHATTER event (either on hit or something else)

static_tbl[50] = {coll = MESH};            -- Vase 2

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;

