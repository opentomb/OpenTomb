function tr1_larson_init(id)
    basic_init(id);
	
	setCharacterParam(id, PARAM_HEALTH, TR1_LARSON_1LIFE, TR1_LARSON_1LIFE);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_LARSON);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterBones(id, 8, 7, 0, 0, 0, 0);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 768.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height
	
	setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
	
    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_larson_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
			-- need collision because entity is not dead (animation),
			-- but lara can't target it after defeat it !
			setCharacterTarget(activator_id, nil);
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("entity_functions_enemies->tr1_larson->onShoot unimplemented ! (lign: 35)");
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            if(getLevel() == 4) then
                local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
                if((a == 15) and (f + 1 >= c)) then
                    local dist = getEntityDistance(object_id, player);
                    if(dist < 2048) then
                        gameflowSend(GF_OP_LEVELCOMPLETE, getLevel() + 1);
                    end;
                end;
            else
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end;