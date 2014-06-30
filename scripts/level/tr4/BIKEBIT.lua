-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 4, BIKEBIT.TR4

print("Level script loaded (BIKEBIT)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------
-- ColType values reference: 
NONE = 0x0000;  -- Object has no collisions
MESH = 0x0001;  -- Object uses real mesh data for collision.
BBOX = 0x0002;  -- Object uses bounding box for collision.
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[01] = {coll = NONE};              -- Tower
static_tbl[04] = {coll = MESH};              -- Wall light
static_tbl[10] = {coll = BBOX};              -- Stone wall block
static_tbl[11] = {coll = BBOX};              -- Stone wall block 2
static_tbl[12] = {coll = BBOX};              -- Stone wall block 3
static_tbl[13] = {coll = BBOX};              -- Stone wall block 4
static_tbl[14] = {coll = BBOX};              -- Ladder	
static_tbl[15] = {coll = BBOX};              -- Green top
static_tbl[16] = {coll = BBOX};              -- Pole
static_tbl[17] = {coll = BBOX};              -- Stone arch
static_tbl[18] = {coll = BBOX};              -- Thick pillar
static_tbl[19] = {coll = MESH};              -- Ceiling part
static_tbl[20] = {coll = MESH};              -- Stairs
static_tbl[21] = {coll = MESH};              -- Stairs 2
static_tbl[22] = {coll = BBOX};              -- Pillar
static_tbl[23] = {coll = BBOX};              -- Pillar arches
static_tbl[24] = {coll = NONE};              -- Street around the corner
static_tbl[25] = {coll = MESH};              -- Stairs 3
static_tbl[26] = {coll = MESH};              -- Stairs 4
static_tbl[27] = {coll = BBOX};              -- Pedestal
static_tbl[28] = {coll = BBOX};              -- Pedestal 2
static_tbl[31] = {coll = MESH};              -- Stone arch part
static_tbl[32] = {coll = BBOX};              -- Ceiling part
static_tbl[34] = {coll = MESH};              -- Rounded building
static_tbl[36] = {coll = NONE};              -- Tower 2
static_tbl[37] = {coll = NONE};              -- Tower 3
static_tbl[38] = {coll = MESH};              -- Balcony
static_tbl[39] = {coll = MESH};              -- Balcony 2
static_tbl[40] = {coll = BBOX};              -- Doorway
static_tbl[41] = {coll = BBOX, hide = 1};    -- Doorway dummy mesh 1
static_tbl[42] = {coll = BBOX, hide = 1};    -- Doorway dummy mesh 2
static_tbl[43] = {coll = MESH};              -- Stairs 5
static_tbl[44] = {coll = BBOX};              -- Fence
static_tbl[45] = {coll = MESH};              -- Grated fence 1
static_tbl[46] = {coll = MESH};              -- Grated fence 2
static_tbl[47] = {coll = NONE};              -- Stone decoration
static_tbl[48] = {coll = MESH};              -- Stone arch part 2
static_tbl[49] = {coll = MESH};              -- Stone arch part 3

-- SHATTER STATICS

static_tbl[51] = {coll = BBOX};              -- Ice spirit capsule
static_tbl[52] = {coll = BBOX};              -- Warning fence
static_tbl[53] = {coll = MESH};              -- Fuel barrel

--------------------------------------------------------------------------------

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;