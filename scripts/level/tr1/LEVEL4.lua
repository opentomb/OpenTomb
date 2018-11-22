-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL4
print("level/tr1/level4->level_loaded !");

level_PostLoad = function()

end;

level_PreLoad = function()
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

function damocles_init(id)      -- Sword of Damocles

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityCollisionFlags(id, COLLISION_GROUP_CHARACTERS, nil, COLLISION_GROUP_CHARACTERS);
    setEntityActivity(id, false);
    local rot_speed = 60.0 * (((math.random(20) - 10) / 5) + 1);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, false);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        rotateEntity(object_id, rot_speed * frame_time);

        if(sameRoom(player, object_id)) then
            local dx, dy, dz = getEntityVector(player, object_id);
            local vx, vy, vz = getEntitySpeed(object_id);
            if((vz < 0.0) or (dx * dx + dy * dy < 1048576.0)) then
                moveEntityToEntity(object_id, player, 48.0 * 60.0 * frame_time, true);
                if(dropEntity(object_id, frame_time, true)) then
                    playSound(103, object_id);
                    setEntityActivity(object_id, false);
                    entity_funcs[id].onLoop = nil;
                end;
            end;
        end;
    end;

    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityActivity(object_id) and (activator_id == player) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            setCharacterParam(activator_id, PARAM_HEALTH, 0);
            playSound(SOUND_GEN_DEATH, activator_id);
            playSound(103, object_id);
            setCharacterRagdollActivity(activator_id, true);
        end;
    end;
end;

function Thor_hummer_init(id)      -- map 5

    setEntityActivity(id, false);

    entity_funcs[id].spawned_id = spawnEntity(45, 25, getEntityPos(63));
    --print("thor spawned id = " .. entity_funcs[id].spawned_id);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(a == 0) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 1, 0);
            setEntityAnim(entity_funcs[object_id].spawned_id, ANIM_TYPE_BASE, 1, 0);
            setEntityActivity(object_id, true);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(a == 1) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 0, 0);
            setEntityAnim(entity_funcs[object_id].spawned_id, ANIM_TYPE_BASE, 0, 0);
        end;
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);

        if((tick_state == TICK_STOPPED) and (a <= 1)) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 0, 0);
            setEntityAnim(entity_funcs[object_id].spawned_id, ANIM_TYPE_BASE, 0, 0);
            return;
        end;

        if(a == 1) then
            if((f + 1 >= c) and (getEntityTimer(object_id) >= 0.75)) then
                setEntityAnim(object_id, ANIM_TYPE_BASE, 2, 0);
                setEntityAnim(entity_funcs[object_id].spawned_id, ANIM_TYPE_BASE, 2, 0);
            end;
        elseif((a == 2) and (f + 1 >= c)) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 3, 0);
            setEntityAnim(entity_funcs[object_id].spawned_id, ANIM_TYPE_BASE, 3, 0);

            setEntityCollision(19, true);
            setEntityVisibility(19, true);
            moveEntityLocal(19, 0, 0, -1024);
            setEntityCollision(20, true);
            setEntityVisibility(20, true);
            moveEntityLocal(20, 0, 0, -1024);
        elseif(a == 3) then
            local b1_dropped = dropEntity(19, frame_time, true);
            local b2_dropped = dropEntity(20, frame_time, true);
            if(b1_dropped and b2_dropped) then
                setEntityActivity(object_id, false);
            end;
        end;
    end;
end;