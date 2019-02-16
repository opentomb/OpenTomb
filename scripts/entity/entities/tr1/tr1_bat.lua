function tr1_bat_init(id)
	--print("entity_functions_enemies->entity->tr1_bat.lua loaded !");
    basic_init(id);

    setCharacterParam(id, PARAM_HEALTH, TR1_BAT, TR1_BAT);
    --setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_SPHERE, -64, -64, -64, 64, 64, 64);
    -- can fix collision ?
    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityMoveType(id, MOVE_FLY);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_BAT);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_FLY);
    -- added bones for hand
    setCharacterBones(id, 4, 0, 1, 3, 5, 7);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 64.0, 32.0, 0.0, 0.0, 64.0); -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_bat_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then
            enableEntity(object_id);
            setCharacterTarget(object_id, player);
            checkRoomCollisionFixer(object_id);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
		if(getEntityTypeFlag(activator_id, ENTITY_TYPE_ACTOR)) then
			changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
		end;
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setEntityCollision(object_id, false);
			setCharacterTarget(activator_id, nil);
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if (bone == 0 or bone == 4) then
            if (getEntityTypeFlag(activator_id, ENTITY_TYPE_ACTOR)) then
				setCharacterParam(object_id, PARAM_HIT_DAMAGE, TR1_BAT_DMG * frame_time, TR1_BAT_DMG * frame_time);
				if(entity_funcs[activator_id].onHit ~= nil) then
					entity_funcs[activator_id].onHit(activator_id, object_id);
				end;
			end;
        end;
    end;
	
	entity_funcs[id].onLoop = nil;
end;

function checkRoomCollisionFixer(object_id)
	local hit, frac, hx, hy, hz = getEntityRayTest(object_id, COLLISION_GROUP_STATIC_ROOM, 0, 0, 1024, 0, 0, -512);  
    if (hit) then
        local x, y, z = getEntityPos(object_id);
        z = hz - 320;
        print("tr1_bat.lua->fixed_position now");
        setEntityPos(object_id, x, y, z);
    end;
end;