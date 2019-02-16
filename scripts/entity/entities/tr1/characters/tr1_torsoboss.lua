function tr1_torsoboss_init(id)
    basic_init(id);
    
    setCharacterParam(id, PARAM_HEALTH, TR1_TORSOBOSS, TR1_TORSOBOSS);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, nil, nil, 128, nil, nil, 768);     -- base
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_TORSO_BOSS);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterBones(id, 3, 0, 0, 0, 0, 0);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 1680.0, 256.0, 256.0, 256, 256);                  -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    enableEntity(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 1, 0);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_torsoboss_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone >= 4 and bone <= 25) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, TR1_TORSOBOSS_DMG * frame_time, TR1_TORSOBOSS_DMG * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            local a = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(a ~= 13) then
                setEntityCollision(object_id, false);
                setCharacterTarget(activator_id, nil);
                setEntityAnim(object_id, ANIM_TYPE_BASE, 13, 0);
            end;
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(f + 1 >= c) then
                activateEntity(86, object_id, 0x1F, TRIGGER_OP_OR, 0x01, 0.0);
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end;