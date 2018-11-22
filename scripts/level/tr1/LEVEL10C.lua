-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL10C
print("level/tr1/level10c.the_great_pyramid->level_loaded !");

level_PostLoad = function()
    playStream(60);
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


function ScionHolder_init(id)
    setEntityTypeFlag(id, ENTITY_TYPE_ACTOR, 1);  -- make it targetable
    entity_funcs[id].onHit = function(object_id, activator_id)
        setCharacterTarget(activator_id, nil);
        setEntityActivity(object_id, false);
        setEntityTypeFlag(0x6b, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR, 1);
        disableEntity(0x69);
    end;
end;