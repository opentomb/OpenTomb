-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL8B
print("level/tr1/level8b.obelisk_of_khamoon->level_loaded !");

level_PostLoad = function()
    playStream(59);
    rewriteName(player, ITEM_KEY_1, ITEM_NAME_TR1_GOLD_KEY);
    rewriteName(player, ITEM_PUZZLE_1, ITEM_NAME_TRI_PUZZLE_EYE_OF_HORUS);
    rewriteName(player, ITEM_PUZZLE_2, ITEM_NAME_TR1_PUZZLE_SCARAB);
    rewriteName(player, ITEM_PUZZLE_3, ITEM_NAME_TR1_PUZZLE_SEAL_OF_ANUBIS);
    rewriteName(player, ITEM_PUZZLE_4, ITEM_NAME_TR1_PUZZLE_ANKH);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    static_tbl[06] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[08] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[10] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wood barrier
    static_tbl[33] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 1
    static_tbl[34] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 2
    static_tbl[38] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Door frame
    static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wall bricks
    static_tbl[43] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Icicle
end;