function tr1_natla_init(id)
    basic_init(id);
	
    setCharacterParam(id, PARAM_HEALTH, TR1_NATLA, TR1_NATLA);
    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  18,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_NATLA);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterBones(id, 2, 1, 0, 0, 0, 0);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 768.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_natla_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setEntityCollision(object_id, false);
			setCharacterTarget(activator_id, nil); -- need it ? (onLoop add full life after dead)
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("entity_functions_enemies->tr1_natla->onShoot unimplemented ! (lign: 30)");
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if((getCharacterParam(object_id, PARAM_HEALTH) == 0) and (5 == getEntityAnimState(object_id, ANIM_TYPE_BASE))) then
            setEntityCollision(object_id, true);
            setCharacterState(object_id, CHARACTER_STATE_DEAD, 0);
            setCharacterParam(object_id, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        end;
    end;
end;