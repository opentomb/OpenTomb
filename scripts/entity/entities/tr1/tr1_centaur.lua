function tr1_centaur_init(id)
    basic_init(id);

    setCharacterParam(id, PARAM_HEALTH, TR1_CENTAUR, TR1_CENTAUR);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 10, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 11, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 17, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 18, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_CENTAUR);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterBones(id, 18, 11, 0, 0, 0, 0);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 1024.0, 256.0, 256.0, 256, 256);                  -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_centaur_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        local spawned_id = spawnEntity(173, getEntityRoom(object_id), getEntityPos(object_id));
        moveEntityLocal(spawned_id, 128, 1024 + 128, 1024 + 256);
        entityRotateToTrigger(spawned_id, getCharacterTarget(object_id), 0);
        projectile_init(spawned_id, 4096, TR1_CENTAUR_MUTANT_DMG_PROJ);
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