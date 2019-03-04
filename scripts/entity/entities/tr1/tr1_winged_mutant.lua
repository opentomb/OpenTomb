function tr1_winged_mutant_init(id)
    basic_init(id);
    
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_WINGED_MUTANT);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterBones(id, 3, 1, 0, 0, 0, 0);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 512.0, 256.0, 256.0, 256, 256);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  3,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    setEntityBoneCollision(id, 15, false);      -- wing
    setEntityBoneCollision(id, 16, false);
    setEntityBoneCollision(id, 17, false);
    setEntityBoneCollision(id, 18, false);      -- wing
    setEntityBoneCollision(id, 19, false);
    setEntityBoneCollision(id, 20, false);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_winged_mutant_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        local spawned_id = spawnEntity(173, getEntityRoom(object_id), getEntityPos(object_id));
        moveEntityLocal(spawned_id, 128, 1024 - 380, 1024 - 64);
        local target = getCharacterTarget(object_id);
        if(target >= 0) then
            entityRotateToTrigger(spawned_id, target, 0);
        end;
        projectile_init(spawned_id, 4096, TR1_WINGED_MUTANT_DMG_PROJ);
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone >= 3 and bone <= 10) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, TR1_WINGED_MUTANT_DMG * frame_time, TR1_WINGED_MUTANT_DMG * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            -- this entity explode when die, but function unimplemented.
            setEntityCollision(object_id, false);
            setCharacterTarget(activator_id, nil);
        end;
    end;
    
    entity_funcs[id].onLoop = nil;
end;