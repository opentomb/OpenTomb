function tr1_wolf_init(id)
    basic_init(id);
    
    setEntityAnim(id, ANIM_TYPE_BASE, 0, -1);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_WOLF);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterBones(id, 3, 1, 11, 14, 7, 10);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 512.0, 256.0, 256.0, 256, 256); -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    setCharacterParam(id, PARAM_HEALTH, TR1_WOLF, TR1_WOLF);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  3,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    setWolfModelFlag(id);
    --setCharacterDefaultRagdoll(id, 8, 0);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_wolf_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 3 or bone == 6 or bone == 9 or bone == 10 or bone == 13 or bone == 14) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, TR1_WOLF_DMG * frame_time, TR1_WOLF_DMG * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setEntityCollision(object_id, false);
            setCharacterTarget(activator_id, nil);
        end;
    end;

    entity_funcs[id].onLoop = nil;
end;

function setWolfModelFlag(id)
    local m_id = getEntityModelID(id);
    setModelBodyPartFlag(m_id, 3, BODY_PART_HEAD);
    setModelBodyPartFlag(m_id, 0, BODY_PART_BODY_LOW);
    setModelBodyPartFlag(m_id, 1, BODY_PART_BODY_UPPER);
    setModelBodyPartFlag(m_id, 23, BODY_PART_TAIL);
    setModelBodyPartFlag(m_id, 7, BODY_PART_LEFT_LEG_1);
    setModelBodyPartFlag(m_id, 15, BODY_PART_LEFT_LEG_1);
    setModelBodyPartFlag(m_id, 8, BODY_PART_LEFT_LEG_2);
    setModelBodyPartFlag(m_id, 16, BODY_PART_LEFT_LEG_2);
    setModelBodyPartFlag(m_id, 9, BODY_PART_LEFT_LEG_3);
    setModelBodyPartFlag(m_id, 17, BODY_PART_LEFT_LEG_3);
    setModelBodyPartFlag(m_id, 10, BODY_PART_LEFT_LEG_3);
    setModelBodyPartFlag(m_id, 18, BODY_PART_LEFT_LEG_3);
    setModelBodyPartFlag(m_id, 11, BODY_PART_RIGHT_LEG_1);
    setModelBodyPartFlag(m_id, 19, BODY_PART_RIGHT_LEG_1);
    setModelBodyPartFlag(m_id, 12, BODY_PART_RIGHT_LEG_2);
    setModelBodyPartFlag(m_id, 20, BODY_PART_RIGHT_LEG_2);
    setModelBodyPartFlag(m_id, 13, BODY_PART_RIGHT_LEG_3);
    setModelBodyPartFlag(m_id, 21, BODY_PART_RIGHT_LEG_3);
    setModelBodyPartFlag(m_id, 14, BODY_PART_RIGHT_LEG_3);
    setModelBodyPartFlag(m_id, 22, BODY_PART_RIGHT_LEG_3);
end;