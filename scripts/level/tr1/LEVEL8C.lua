-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL8C
print("level/tr1/level8c.sanctuary_of_the_scion->level_loaded !");

level_PostLoad = function()
    playStream(59);
    sectorAddTrigger(62, 9, 3, 0, TR_FD_TRIGTYPE_PICKUP, 0x1F, 0 , 0);          -- (room_id, index_x, index_y, function, sub_function, mask, once, timer)" 
    sectorAddTriggerCommand(62, 9, 3, TR_FD_TRIGFUNC_OBJECT, 0x49, 0);
    sectorAddTriggerCommand(62, 9, 3, TR_FD_TRIGFUNC_ENDLEVEL, 0, 0);           -- WORKAROUND: play cutscene first!
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
    static_tbl[38] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- FACE
    static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wall bricks
    static_tbl[43] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Icicle
end;