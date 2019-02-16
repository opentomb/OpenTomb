function tr1_skateboardist_init(id)
    basic_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 7, 0);
    entity_funcs[id].skate_id = nil;

    setCharacterParam(id, PARAM_HEALTH, TR1_SKATEBOARDIST, TR1_SKATEBOARDIST);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_SKATEBOARDIST);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterBones(id, 8, 0, 0, 0, 0, 0);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 768.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    entity_funcs[id].onSave = function()
        local sp_save = "";
        if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
            sp_save = sp_save .. "tr1_skateboardist_init(" .. id .. ");\n";
        end;
        if(entity_funcs[id].skate_id ~= nil) then
            sp_save = sp_save .. "entity_funcs[" .. id .. "].skate_id = " .. entity_funcs[id].skate_id .. ";\n";
        end;
        return sp_save;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then
            enableEntity(object_id);
            setCharacterTarget(object_id, player);
            entity_funcs[object_id].skate_id = spawnEntity(29, getEntityRoom(object_id), getEntityPos(object_id));
            setEntityCollision(entity_funcs[object_id].skate_id, false); -- no collision ?
            setEntityActivity(entity_funcs[object_id].skate_id, true);
            --print("skate = " .. entity_funcs[object_id].skate_id);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("entity_functions_enemies->tr1_skateboardist->onShoot unimplemented ! (lign: 42)");
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setEntityCollision(object_id, false);
			setCharacterTarget(activator_id, nil);
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        setEntityAnim(entity_funcs[object_id].skate_id, ANIM_TYPE_BASE, a, f);
        entitySSAnimCopy(entity_funcs[object_id].skate_id, object_id);
        setEntityPos(entity_funcs[object_id].skate_id, getEntityPos(object_id));
        if ((getCharacterState(object_id, CHARACTER_STATE_DEAD) > 1) and (f + 1 >= c)) then
            setEntityActivity(object_id, false);
        end;
    end;
end;