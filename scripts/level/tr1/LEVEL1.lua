-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL1 (CAVES)

-- ColType values reference: 
--   0x0000  - Object has no collisions
--   0x0001  - Object uses real mesh data for collision.
--   0x0002  - Object uses bounding box for collision.

print("Level script loaded (LEVEL1.lua)");

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

tr_static_tbl = {};

tr_static_tbl[06] = {coll = 0x0000};             -- Hanging plant
tr_static_tbl[08] = {coll = 0x0000};             -- Hanging plant
tr_static_tbl[10] = {coll = 0x0001};             -- Wood barrier
tr_static_tbl[33] = {coll = 0x0001};             -- Bridge part 1
tr_static_tbl[34] = {coll = 0x0001};             -- Bridge part 2
tr_static_tbl[38] = {coll = 0x0000};             -- Door frame
tr_static_tbl[39] = {coll = 0x0001};             -- Wall bricks
tr_static_tbl[43] = {coll = 0x0000};             -- Icicle

function GetStaticMeshFlags(ver, id)
	if(static_tbl[id] == nil) then
		return nil, nil;
	else
		return static_tbl[id].coll, static_tbl[id].hide;
	end;
end;