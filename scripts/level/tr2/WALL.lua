-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, WALL.TR2

print("Level script loaded (WALL.lua)");

level_PostLoad = function()
    addCharacterHair(player, HAIR_TR2);
    playStream(33);
end;

level_PreLoad = function() 
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    static_tbl[00] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Bank rock
    static_tbl[01] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Death slide rope
    static_tbl[02] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Skeleton 1
    static_tbl[03] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Skeleton 2
    static_tbl[04] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Cobweb
    static_tbl[10] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Fireplace
    static_tbl[11] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Crate with book
    static_tbl[12] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Briefcase
    static_tbl[13] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Crates
end;