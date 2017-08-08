-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2016

function setHumanoidBodyParts(id)
    setEntityBodyPartFlag(id,  0, BODY_PART_BODY_LOW);
    setEntityBodyPartFlag(id,  7, BODY_PART_BODY_UPPER);
    setEntityBodyPartFlag(id, 14, BODY_PART_HEAD);

    setEntityBodyPartFlag(id, 11, BODY_PART_LEFT_HAND_1);
    setEntityBodyPartFlag(id, 12, BODY_PART_LEFT_HAND_2);
    setEntityBodyPartFlag(id, 13, BODY_PART_LEFT_HAND_3);
    setEntityBodyPartFlag(id,  8, BODY_PART_RIGHT_HAND_1);
    setEntityBodyPartFlag(id,  9, BODY_PART_RIGHT_HAND_2);
    setEntityBodyPartFlag(id, 10, BODY_PART_RIGHT_HAND_3);

    setEntityBodyPartFlag(id,  1, BODY_PART_LEFT_LEG_1);
    setEntityBodyPartFlag(id,  2, BODY_PART_LEFT_LEG_2);
    setEntityBodyPartFlag(id,  3, BODY_PART_LEFT_LEG_3);
    setEntityBodyPartFlag(id,  4, BODY_PART_RIGHT_LEG_1);
    setEntityBodyPartFlag(id,  5, BODY_PART_RIGHT_LEG_2);
    setEntityBodyPartFlag(id,  6, BODY_PART_RIGHT_LEG_3);
end


function Lara_init(id)
    print("LARA INIT");
    if(getLevelVersion() < TR_II) then
        if(getLevel() == 0) then
            setEntityMeshes(id, 5, 0, 13);
        end;
    elseif(getLevelVersion() < TR_III) then
        print("TR 2");
    elseif(getLevelVersion() < TR_IV) then
        setEntityMeshes(id, 315, 0, 14);
    else
        setEntityMeshes(id, 8, 0, 14);
        setEntitySkinMeshes(id, 9, 1, 14);
    end;

    resetRigidBodies(id);
    setEntityTypeFlag(id, ENTITY_TYPE_ACTOR, 1);
    setEntityTypeFlag(id, ENTITY_TYPE_TRIGGER_ACTIVATOR, 1);
    characterCreate(id);
    setCharacterParam(id, PARAM_HEALTH, 1000.0, 1000.0);
    setCharacterBones(id, 14, 7, 11, 13, 8, 10);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 768.0, 128.0, 288.0, 1920.0, 320.0); -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    setEntityMoveType(id, MOVE_ON_FLOOR);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_LARA);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);

    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, nil, 60.0, nil, nil);
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, -48.0, -54.0, 8.0, 48.0, 32.0, 166.0);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, -32.0, -26.1, -176.0, 32.0, 29.1, -2.7);
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, -32.0, -27.1, -175.3, 32.0, 28.1, 1.7);
    setEntityGhostCollisionShape(id, 10, COLLISION_SHAPE_SPHERE, -32.0, nil, -52.0, 16.0, nil, 0);
    setEntityGhostCollisionShape(id, 13, COLLISION_SHAPE_SPHERE, -16.0, nil, -52.0, 32.0, nil, 0);
    setEntityGhostCollisionShape(id, 14, COLLISION_SHAPE_SPHERE, -56.0, 0, 0, 56.0, 16.0, 64.0);

    setEntityGhostCollisionShape(id, 3,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 6,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 2,  COLLISION_SHAPE_BOX, -30.0, -40.0, -200.0, 24.0, 16.0, 0);
    setEntityGhostCollisionShape(id, 5,  COLLISION_SHAPE_BOX, -24.0, -40.0, -200.0, 30.0, 16.0, 0);
    setEntityGhostCollisionShape(id, 12, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 9,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 11, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    setHumanoidBodyParts(id);
    setCharacterRagdollSetup(id, getRagdollSetup(RD_TYPE_LARA));

    setPlayer(id);

    entity_funcs[id].onShoot = function(object_id, activator_id)
        local weapon = getCharacterCurrentWeapon(object_id);
        if(weapon == 1 or weapon == 2) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 30, 30);
        else
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 130, 130);
        end;

        if(entity_funcs[activator_id].onHit ~= nil) then
            entity_funcs[activator_id].onHit(activator_id, object_id);
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
        end;
    end;
end


function baddie_init(id)    -- INVALID!
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end;

    setEntityTypeFlag(id, ENTITY_TYPE_ACTOR);
    characterCreate(id);
    setCharacterParam(id, PARAM_HEALTH, 100.0, 100.0);

    local meshes_count = getEntityMeshCount(id);
    local m = 1;

    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    while(m < meshes_count) do
        setEntityGhostCollisionShape(id,  m,  COLLISION_SHAPE_BOX, 0, 0, 0, 0, 0, 0);
        m = m + 1;
    end;

    setEntityMoveType(id, MOVE_ON_FLOOR);
    disableEntity(id);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then
            enableEntity(object_id);
            setCharacterTarget(object_id, player);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;


    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityActivity(object_id, false);
            setEntityCollision(object_id, false);
            -- DO KILL ANIM
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(getCharacterState(object_id, CHARACTER_STATE_DEAD) > 1) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(f + 1 >= c) then
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end


function Doppelgagner_init(id)
    baddie_init(id);
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
            setCharacterTarget(activator_id, nil);
            setCharacterRagdollActivity(object_id, true);
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local room = getEntityRoom(player);

        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
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

function bat_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 100, 100);
    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_SPHERE, -64, -64, -64, 64, 64, 64);
    setEntityMoveType(id, MOVE_FLY);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_BAT);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_FLY);
    setCharacterMoveSizes(id, 64.0, 64.0, 0.0, 0.0, 64.0); -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "bat_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then
            enableEntity(object_id);
            setCharacterTarget(object_id, player);
            local hit, frac, hx, hy, hz = getEntityRayTest(object_id, COLLISION_GROUP_STATIC_ROOM, 0, 0, 1024, 0, 0, -512);
            if(hit) then
                local x, y, z = getEntityPos(object_id);
                z = hz - 320;
                print("bat fix");
                setEntityPos(object_id, x, y, z);
            end;
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        setCharacterParam(object_id, PARAM_HIT_DAMAGE, 150 * frame_time, 150 * frame_time);
        if(entity_funcs[activator_id].onHit ~= nil) then
            entity_funcs[activator_id].onHit(activator_id, object_id);
        end;
    end;

end;


function wolf_init(id)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, -1);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_WOLF);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterBones(id, 3, 1, nil, nil, nil, nil);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 512.0, 256.0, 256.0, 256, 256); -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    setCharacterParam(id, PARAM_HEALTH, 200, 200);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  3,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "wolf_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 3 or bone == 6 or bone == 9 or bone == 10 or bone == 13 or bone == 14) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 150 * frame_time, 150 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function bear_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id,  14,  COLLISION_SHAPE_SPHERE, -256, -128, -256, 256, 256, 128);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_BEAR);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 512.0, 256.0, 256.0, 256, 256);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "bear_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 14 or bone == 17 or bone == 2 or bone == 3 or bone == 5 or bone == 6) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 250 * frame_time, 250 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function raptor_init(id)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_RAPTOR);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 512.0, 256.0, 256.0, 256, 256);

    setCharacterParam(id, PARAM_HEALTH, 200, 200);
    setEntityGhostCollisionShape(id,  21,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  22,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  24,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "raptor_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone >= 9 and bone <= 23) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 200 * frame_time, 200 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function lion_init(id)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_LION);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 512.0, 256.0, 256.0, 256, 256);

    setCharacterParam(id, PARAM_HEALTH, 200, 200);
    setEntityGhostCollisionShape(id,  7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  19,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  20,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "lion_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 20 or bone == 21 or bone == 2 or bone == 3 or bone == 5 or bone == 6) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 200 * frame_time, 200 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function puma_init(id)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_PUMA);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 512.0, 256.0, 256.0, 256, 256);

    setCharacterParam(id, PARAM_HEALTH, 200, 200);
    setEntityGhostCollisionShape(id,  7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  19,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  20,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "puma_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 20 or bone == 21 or bone == 2 or bone == 3 or bone == 5 or bone == 6) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 200 * frame_time, 200 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function winged_mutant_init(id)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_WINGED_MUTANT);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
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
            return "winged_mutant_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        local spawned_id = spawnEntity(173, getEntityRoom(object_id), getEntityPos(object_id));
        moveEntityLocal(spawned_id, 128, 1024 - 380, 1024 - 64);
        local target = getCharacterTarget(object_id);
        if(target >= 0) then
            entityRotateToTrigger(spawned_id, target, 0);
        end;
        projectile_init(spawned_id, 4096, 150);
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone >= 3 and bone <= 10) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 200 * frame_time, 200 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function trex_init(id)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_TREX);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 2048.0, 256.0, 256.0, 256, 256);

    setCharacterParam(id, PARAM_HEALTH, 2000, 2000);
    setEntityGhostCollisionShape(id,  9,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  10,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  11,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  12,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  13,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  20,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  21,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "trex_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 12 or bone == 13) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 450 * frame_time, 450 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        else
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 120 * frame_time, 120 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function gorilla_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);   -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);   -- hand
    setEntityGhostCollisionShape(id, 11,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- hand
    setEntityGhostCollisionShape(id, 14,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- head
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_GORILLA);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_2);
    setCharacterMoveSizes(id, 512.0, 256.0, 256.0, 768, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    setCharacterMoveSizes(id, 512.0, nil, nil, nil, nil); -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "gorilla_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 14 or bone == 15 or bone == 12 or bone == 13 or bone == 9 or bone == 10) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 200 * frame_time, 200 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function crocodile_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 400, 400);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 10,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 11,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_CROCODILE);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 256.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "crocodile_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 8 or bone == 9) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 200 * frame_time, 200 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function rat_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 150, 150);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 9,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setCharacterBones(id, 2, 0, 0, 0, 0, 0);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_RAT);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 256.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "rat_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone == 2 or bone == 3) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 150 * frame_time, 150 * frame_time);
            if(entity_funcs[activator_id].onHit ~= nil) then
                entity_funcs[activator_id].onHit(activator_id, object_id);
            end;
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function centaur_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 550, 550);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 10, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 11, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 17, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 18, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_CENTAUR);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 1024.0, 256.0, 256.0, 256, 256);                  -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "centaur_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        local spawned_id = spawnEntity(173, getEntityRoom(object_id), getEntityPos(object_id));
        moveEntityLocal(spawned_id, 128, 1024 + 128, 1024 + 256);
        entityRotateToTrigger(spawned_id, getCharacterTarget(object_id), 0);
        projectile_init(spawned_id, 4096, 150);
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end;


function Larson_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_LARSON);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 768.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "Larson_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("peu");
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            if(getLevel() == 4) then
                local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
                if((a == 15) and (f + 1 >= c)) then
                    local dist = getEntityDistance(object_id, player);
                    if(dist < 2048) then
                        setLevel(5);  -- really play cutscene first
                    end;
                end;
            else
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end;


function Pierre_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_PIERRE);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 768.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    entity_funcs[id].is_flee = (getLevel() ~= 9) or (getEntityRoom(id) ~= 110);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "Pierre_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            if(not entity_funcs[object_id].is_flee) then
                setCharacterTarget(activator_id, nil);
                setEntityCollision(object_id, false);
            else
                setCharacterParam(object_id, PARAM_HEALTH, 1);
            end;
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("peu");
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local hp = getCharacterParam(object_id, PARAM_HEALTH);
        if((hp == 0) and (getLevel() == 9)) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if((a == 12) and (f + 1 >= c)) then
                local spawned_id = spawnEntity(133, getEntityRoom(object_id), getEntityPos(object_id));
                pickable_init(spawned_id);
                spawned_id = spawnEntity(150, getEntityRoom(object_id), getEntityPos(object_id));
                pickable_init(spawned_id);
                spawned_id = spawnEntity(101, getEntityRoom(object_id), getEntityPos(object_id));
                pickable_init(spawned_id);
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        elseif((hp == 1) and entity_funcs[object_id].is_flee) then

        end;
    end;
end;


function cowboy_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_COWBOY);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 768.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "cowboy_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("peu");
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end


function MrT_init(id)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_MRT);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 800.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "MrT_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("peu");
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end


function skateboardist_init(id)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 7, 0);
    entity_funcs[id].skate_id = nil;

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_SKATEBOARDIST);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 768.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    entity_funcs[id].onSave = function()
        local sp_save = "";
        if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
            sp_save = sp_save .. "skateboardist_init(" .. id .. ");\n";
        end;
        if(entity_funcs[id].skate_id ~= nil) then
            sp_save = sp_save .. "entity_funcs[" .. id .. "].skate_id = " .. entity_funcs[id].skate_id .. ";\n";
        end;
        return sp_save;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then
            enableEntity(object_id);
            setCharacterTarget(object_id, player);
            entity_funcs[object_id].skate_id = spawnEntity(29, getEntityRoom(object_id), getEntityPos(object_id));
            setEntityCollision(entity_funcs[object_id].skate_id, false);
            setEntityActivity(entity_funcs[object_id].skate_id, true);
            --print("skate = " .. entity_funcs[object_id].skate_id);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("peu");
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        setEntityAnim(entity_funcs[object_id].skate_id, ANIM_TYPE_BASE, a, f);
        entitySSAnimCopy(entity_funcs[object_id].skate_id, object_id);
        setEntityPos(entity_funcs[object_id].skate_id, getEntityPos(object_id));
        if ((getCharacterState(object_id, CHARACTER_STATE_DEAD) > 1) and (f + 1 >= c)) then
            setEntityActivity(object_id, false);
        end;
    end;
end


function TorsoBoss_init(id)
    baddie_init(id);
    setCharacterParam(id, PARAM_HEALTH, 1500, 1500);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, nil, nil, 128, nil, nil, 768);     -- base
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_TORSO_BOSS);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 1680.0, 256.0, 256.0, 256, 256);                  -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    enableEntity(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 1, 0);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "TorsoBoss_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onAttack = function(object_id, activator_id)
        local bone = entity_funcs[object_id].col_part_from;
        if(bone >= 4 and bone <= 25) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 200 * frame_time, 200 * frame_time);
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
                setCharacterTarget(activator_id, nil);
                setEntityCollision(object_id, false);
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


function Natla_init(id)
    baddie_init(id);
    setCharacterParam(id, PARAM_HEALTH, 600, 600);
    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  18,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_NATLA);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    setCharacterAIParams(id, 0xFF, ZONE_TYPE_1);
    setCharacterMoveSizes(id, 768.0, 256.0, 256.0, 256, 256);                   -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "Natla_init(" .. id .. ");\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;

    entity_funcs[id].onShoot = function(object_id, activator_id)
        print("peu");
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if((getCharacterParam(object_id, PARAM_HEALTH) == 0) and (5 == getEntityAnimState(object_id, ANIM_TYPE_BASE))) then
            setEntityCollision(object_id, true);
            setCharacterState(object_id, CHARACTER_STATE_DEAD, 0);
            setCharacterParam(object_id, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        end;
    end;
end


function mummy_init(id)
    winged_mutant_init(id);
    setEntityBoneVisibility(id, 15, false);      -- wing
    setEntityBoneVisibility(id, 16, false);
    setEntityBoneVisibility(id, 17, false);
    setEntityBoneVisibility(id, 18, false);      -- wing
    setEntityBoneVisibility(id, 19, false);
    setEntityBoneVisibility(id, 20, false);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "mummy_init(" .. id .. ");\n";
        end;
    end;
end


function mummy_spawner_init(id)
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 0) then
            local spawned_id = spawnEntity(20, getEntityRoom(object_id), getEntityPos(object_id));
            mummy_init(spawned_id);
            enableEntity(spawned_id);
            setCharacterTarget(object_id, player);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
end


function mutant_spawner_init(id)
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 0) then
            local spawned_id = spawnEntity(20, getEntityRoom(object_id), getEntityPos(object_id));
            winged_mutant_init(spawned_id);
            enableEntity(spawned_id);
            setCharacterTarget(object_id, player);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
end


function mutant_egg_init(id)
    setEntityActivity(id, false);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 0) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 1, 0);
            setEntityActivity(object_id, true);
            setCharacterTarget(object_id, player);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(getEntityEvent(object_id) ~= 0) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if((a == 1) and (f + 1 >= c)) then
                setEntityCollision(object_id, false);
                local spawned_id = spawnEntity(20, getEntityRoom(object_id), getEntityPos(object_id));
                moveEntityLocal(spawned_id, -512.0, 512.0, 0.0);
                winged_mutant_init(spawned_id);
                enableEntity(spawned_id);

                local i = 1;
                while(i < 25) do
                    setEntityBoneVisibility(object_id, i, false);
                    i = i + 1;
                end;

                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end;


function mutant_boss_egg_init(id)
    if(getLevel() == 15) then
        setEntityActivity(id, false);
        setEntityCollision(id, false);

        entity_funcs[id].onActivate = function(object_id, activator_id)
            if(getEntityEvent(object_id) == 0) then
                setEntityAnim(object_id, ANIM_TYPE_BASE, 1, 0);
                setEntityActivity(object_id, true);
            end;
            return ENTITY_TRIGGERING_ACTIVATED;
        end;

        entity_funcs[id].onLoop = function(object_id, tick_state)
            if(getEntityEvent(object_id) ~= 0) then
                local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
                if((a == 1) and (f + 1 >= c)) then
                    local spawned_id = spawnEntity(34, getEntityRoom(object_id), getEntityPos(object_id));
                    moveEntityLocal(spawned_id, -512.0, 512.0, -4096.0);
                    TorsoBoss_init(spawned_id);

                    local i = 1;
                    while(i < 25) do
                        setEntityBoneVisibility(object_id, i, false);
                        i = i + 1;
                    end;

                    setEntityActivity(object_id, false);
                    entity_funcs[object_id].onLoop = nil;
                end;
            end;
        end;
    end;
end;


--                          TR_II

function MaskedGoon_init(id)
    baddie_init(id);
    setEntityBaseAnimModel(id, 16);
end
