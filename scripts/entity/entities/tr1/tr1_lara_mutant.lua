function tr1_lara_mutant_init(id)
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end;

    setEntityTypeFlag(id, ENTITY_TYPE_ACTOR);
    characterCreate(id);

    setCharacterParam(id, PARAM_HEALTH, 1000.0, 1000.0);
    setCharacterBones(id, 14, 7, 11, 13, 8, 10);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 768.0, 128.0, 288.0, 1920.0, 320.0); -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    setEntityMoveType(id, MOVE_ON_FLOOR);
    setCharacterAIParams(id, -1, -1);

    local x0 = 36864;
    local y0 = 61440;

    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, -46.0, -54.0, 10.0, 46.0, 32.0, 160.0);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, -32.0, -26.1, -176.0, 32.0, 29.1, -2.7);
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, -32.0, -27.1, -175.3, 32.0, 28.1, 1.7);
    setEntityGhostCollisionShape(id, 10, COLLISION_SHAPE_SPHERE, -32.0, nil, -52.0, 16.0, nil, 0);
    setEntityGhostCollisionShape(id, 13, COLLISION_SHAPE_SPHERE, -16.0, nil, -52.0, 32.0, nil, 0);
    setEntityGhostCollisionShape(id, 14, COLLISION_SHAPE_SPHERE, -56.0, 0, 0, 56.0, 16.0, 64.0);

    setEntityGhostCollisionShape(id, 3,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 6,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 5,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 12, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 9,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 11, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    setHumanoidBodyParts(id);
    setCharacterRagdollSetup(id, getRagdollSetup(RD_TYPE_LARA));

    entity_funcs[id].onHit = function(object_id, activator_id)
        hp = getCharacterParam(player, PARAM_HEALTH) - getCharacterParam(player, PARAM_HIT_DAMAGE);
        setCharacterParam(player, PARAM_HEALTH, hp);
        setCharacterParam(object_id, PARAM_HEALTH, hp);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterRagdollActivity(object_id, true);
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local room = getEntityRoom(player);

        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterState(object_id, CHARACTER_STATE_DEAD, 2);
            setEntityTypeFlag(object_id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR, 1);
            setCharacterRagdollActivity(object_id, true);
            setEntityActivity(object_id, false);
            return;
        end;

        if(room == 10 or room == 57 or room == 59) then
            local x, y, z, az, ax, ay = getEntityPos(player);
            local dp_x, dp_y, dp_z = getEntityPos(object_id);
            local trap_x, trap_y = getEntityPos(32);
            trap_x = trap_x - dp_x;
            trap_y = trap_y - dp_y;

            if(trap_x * trap_x + trap_y * trap_y < 600 * 600) then
                z = dp_z;
                if(getEntityMoveType(object_id) == MOVE_FREE_FALLING) then
                    return;
                end;
            end;

            setEntityPos(object_id, 2 * x0 - x, 2 * y0 - y, z, 180.0 + az, ax, ay);
            local vx, vy, vz = getEntitySpeed(player);
            setEntitySpeed(object_id, 0, 0, vz);
            entitySSAnimCopy(object_id, player);
        end;
    end;
end;