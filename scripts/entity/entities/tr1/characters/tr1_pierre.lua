function tr1_pierre_init(id)
    basic_init(id);
    
    setCharacterParam(id, PARAM_HEALTH, TR1_PIERRE_1LIFE, TR1_PIERRE_1LIFE);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_PIERRE);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterBones(id, 8, 7, 0, 0, 0, 0);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 768.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height
    
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
                                                  -- not "and" here ?
    entity_funcs[id].is_flee = (getLevel() ~= 10) or (getEntityRoom(id) ~= 110);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_pierre_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            if(not entity_funcs[object_id].is_flee) then
                setEntityCollision(object_id, false);
                setCharacterTarget(activator_id, nil);
            else
                setCharacterParam(object_id, PARAM_HEALTH, 1);
            end;
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("entity_functions_enemies->tr1_pierre->onShoot unimplemented ! (lign: 36)");
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local hp = getCharacterParam(object_id, PARAM_HEALTH);
        if((hp == 0) and (getLevel() == 10)) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            local spawned_id;
            if((a == 12) and (f + 1 >= c)) then
                spawned_id = spawnEntity(133, getEntityRoom(object_id), getEntityPos(object_id));
                pickable_init(spawned_id);
                spawned_id = spawnEntity(150, getEntityRoom(object_id), getEntityPos(object_id));
                pickable_init(spawned_id);
                spawned_id = spawnEntity(101, getEntityRoom(object_id), getEntityPos(object_id));
                pickable_init(spawned_id);
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        elseif((hp == 1) and entity_funcs[object_id].is_flee) then
            print("entity_functions_enemies->tr1_pierre->isFlee unimplemented ! (lign: 56)");
        end;
    end;
end;