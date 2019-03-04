-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL3A (CAVES)
print("level/tr1/level3a.lost_valley->level_loaded !");

--use it in "postload" when you test hair for TR1 !!
--addCharacterHair(player, getHairSetup(HAIR_TR1));
print("test hair not enabled (LEVEL3A.lua)");

level_PostLoad = function()
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    static_tbl[06] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[07] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[08] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[10] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wood barrier
    static_tbl[33] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 1
    static_tbl[34] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 2
    static_tbl[38] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Door frame
    static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wall bricks
    static_tbl[43] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Icicle
    static_tbl[45] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Skeleton
    static_tbl[48] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Skeleton
end;
