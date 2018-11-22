-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 1, GYM
print("level/tr1/gym->level_loaded !");

level_PostLoad = function()
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    static_tbl[00] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Flower pot 1
    static_tbl[01] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Flower pot 2
    static_tbl[02] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Flower pot 3
    static_tbl[03] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Flower pot 4
    static_tbl[04] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};                 -- Plant
    static_tbl[10] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Chair
    static_tbl[11] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Fireplace
    static_tbl[12] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Harp
    static_tbl[13] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Piano
    static_tbl[14] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Desk
    static_tbl[15] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Statue
    static_tbl[16] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};                 -- Column
    static_tbl[18] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Indiana Jones Lost Ark
    static_tbl[19] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Treasures
    static_tbl[30] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Fence flat
    static_tbl[31] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Fence tilt 1
    static_tbl[32] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Fence tilt 2
    static_tbl[33] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};                 -- Stick
    static_tbl[34] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};             -- Chandelier
    static_tbl[35] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};                 -- Armor
end;