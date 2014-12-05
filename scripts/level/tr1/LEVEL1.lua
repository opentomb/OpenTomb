-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL1 (CAVES)

-- ColType values reference: 
COLL_NONE = 0x00;  -- Object has no collisions
COLL_MESH = 0x01;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x02;  -- Object uses bounding box for collision.

print("Level script loaded (LEVEL1.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

static_tbl[06] = {coll = COLL_NONE};             -- Hanging plant
static_tbl[08] = {coll = COLL_NONE};             -- Hanging plant
static_tbl[10] = {coll = COLL_MESH};             -- Wood barrier
static_tbl[33] = {coll = COLL_MESH};             -- Bridge part 1
static_tbl[34] = {coll = COLL_MESH};             -- Bridge part 2
static_tbl[38] = {coll = COLL_NONE};             -- Door frame
static_tbl[39] = {coll = COLL_MESH};             -- Wall bricks
static_tbl[43] = {coll = COLL_NONE};             -- Icicle

function trGetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;
