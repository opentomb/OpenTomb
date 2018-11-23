-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL3B (CAVES)
print("level/tr1/level3b.tomb_of_qualopec->level_loaded !");

level_PostLoad = function()
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


function tallblock_init(id)    -- Tall moving block (TR1)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);

    entity_funcs[id].distance_passed = 0;

    entity_funcs[id].onSave = function()
        local addr = "\nentity_funcs[" .. id .. "].";
        return addr .. "distance_passed = " .. entity_funcs[id].distance_passed .. ";";
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 0) then
            setEntityActivity(object_id, true);
            playSound(64, object_id);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;

	entity_funcs[id].onDeactivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) ~= 0) then
            setEntityActivity(object_id, true);
            playSound(64, object_id);
            return ENTITY_TRIGGERING_DEACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local move_dist = 32.0 * 60.0 * frame_time;
        if(getEntityEvent(object_id) == 0) then
            move_dist = 0.0 - move_dist;
        end;

        entity_funcs[object_id].distance_passed = entity_funcs[object_id].distance_passed + move_dist;
        moveEntityLocal(object_id, 0.0, move_dist, 0.0);
        if(math.abs(entity_funcs[object_id].distance_passed) >= 2048.0) then
            stopSound(64, object_id);
            setEntityActivity(object_id, false);
            entity_funcs[object_id].distance_passed = 0;
        end;
    end;

    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].distance_passed = nil;
    end;

    prepareEntity(id);
end;