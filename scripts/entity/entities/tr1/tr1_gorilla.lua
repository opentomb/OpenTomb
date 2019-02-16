function tr1_gorilla_init(id)
    basic_init(id);
	
    setCharacterParam(id, PARAM_HEALTH, TR1_GORILLA, TR1_GORILLA);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_GORILLA);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_2);
    setCharacterMoveSizes(id, 512.0, 256.0, 256.0, 768, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    setCharacterBones(id, 14, 7, 0, 0, 0, 0);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 512.0, nil, nil, nil, nil); -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height
	
	setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);   -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);   -- hand
    setEntityGhostCollisionShape(id, 11,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- hand
    setEntityGhostCollisionShape(id, 14,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- head
	
    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_gorilla_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 14 or bone == 15 or bone == 12 or bone == 13 or bone == 9 or bone == 10) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, TR1_GORILLA_DMG * frame_time, TR1_GORILLA_DMG * frame_time);
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