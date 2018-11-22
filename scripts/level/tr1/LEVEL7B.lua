-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL7B
print("level/tr1/level7b.tomb_of_tihocan->level_loaded !");

level_PostLoad = function()
    -- to do something
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
    static_tbl[37] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Door frame
    static_tbl[38] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};           -- Vase
    static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wall bricks
    static_tbl[43] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Icicle
end;

function centaur_statue_init(id)
    setEntityActivity(id, false);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEnability(object_id)) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 0, 0);
            setEntityActivity(object_id, true);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(f + 1 >= c) then
            disableEntity(object_id);
            local spawned_id = spawnEntity(23, getEntityRoom(object_id), getEntityPos(object_id));
            centaur_init(spawned_id);
            setCharacterTarget(spawned_id, player);
            enableEntity(spawned_id);

            entity_funcs[spawned_id].onSave = function()
                return "centaur_init(" .. spawned_id .. ");";
            end;

            entity_funcs[id].onLoop = nil;
        end;
    end;
end;