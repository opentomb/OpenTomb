-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2016

--Script_ExecEntity(engine_lua, ENTITY_CALLBACK_HIT, e->id, ent->id);
function baddie_init(id)    -- INVALID!
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end;

    setEntityTypeFlag(id, ENTITY_TYPE_ACTOR);
    characterCreate(id, 100.0);

    local meshes_count = getEntityMeshCount(id);
    local m = 1;

    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    while(m < meshes_count) do
        setEntityGhostCollisionShape(id,  m,  COLLISION_SHAPE_BOX, 1, -1, 0, 0, 0, 0);
        m = m + 1;
    end;

    setEntityMoveType(id, MOVE_ON_FLOOR);
    disableEntity(id);
    setCharacterTarget(id, player);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then 
            enableEntity(object_id);
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
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
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

    entity_funcs[id].onHit = function(object_id, activator_id)
        hp = getCharacterParam(player, PARAM_HEALTH) - getCharacterParam(player, PARAM_HIT_DAMAGE);
        setCharacterParam(player, PARAM_HEALTH, hp);
        setCharacterParam(object_id, PARAM_HEALTH, hp);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            addEntityRagdoll(object_id, RD_TYPE_LARA);
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local room = getEntityRoom(player);

        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setEntityTypeFlag(object_id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR, 1);
            addEntityRagdoll(object_id, RD_TYPE_LARA);
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
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityMoveType(id, MOVE_FLY);
    noFixEntityCollision(id);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then 
            enableEntity(object_id);
            local hit, frac, hx, hy, hz = getEntityRayTest(object_id, COLLISION_GROUP_STATIC_ROOM, 0, 0, 640, 0, 0, -320);
            if (hit) then
                local x, y, z = getEntityPos(object_id);
                z = hz - 256;
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
            setEntityAnim(object_id, ANIM_TYPE_BASE, 3, 0);
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if((getCharacterParam(object_id, PARAM_HEALTH) == 0) and getEntityActivity(object_id)) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if((a == 3) and dropEntity(object_id, frame_time)) then
                setEntityAnim(object_id, ANIM_TYPE_BASE, 4, 0);
            end;

            if((a == 4) and (f + 1 >= c)) then
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;

end;


function wolf_init(id)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, -1);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);

    setCharacterParam(id, PARAM_HEALTH, 200, 200);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  3,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    entity_funcs[id].onHit = function(object_id, activator_id)
        local damage = getCharacterParam(activator_id, PARAM_HIT_DAMAGE);
        changeCharacterParam(object_id, PARAM_HEALTH, -damage);
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
            if(damage > 80) then
                setEntityAnim(object_id, ANIM_TYPE_BASE, 22, 0);
            else
                setEntityAnim(object_id, ANIM_TYPE_BASE, 20, 0);
            end;
        end;
    end;
end;


function bear_init(id)
    baddie_init(id);
    
    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id,  14,  COLLISION_SHAPE_SPHERE, -256, -128, -256, 256, 256, 128);

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            local a = getEntityAnim(object_id, ANIM_TYPE_BASE);
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
            if(a >= 4 and a <= 10 or a == 14) then
                setEntityAnim(object_id, ANIM_TYPE_BASE, 19, 0);
            else
                setEntityAnim(object_id, ANIM_TYPE_BASE, 20, 0);
            end;
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

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
            setEntityAnim(object_id, ANIM_TYPE_BASE, 15, 0);
        end;
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
    entity_funcs[id].is_flee = (getLevel() ~= 9) or (getEntityRoom(id) ~= 110);

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            if(not entity_funcs[object_id].is_flee) then
                setCharacterTarget(activator_id, nil);
                setEntityCollision(object_id, false);
                setEntityAnim(object_id, ANIM_TYPE_BASE, 12, 0);
            else
                --flee
            end;
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            if(getLevel() == 9) then
                local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
                if((a == 12) and (f + 1 >= c)) then
                    local spawned_id = spawnEntity(133, getEntityRoom(object_id), getEntityPos(object_id));
                    setEntityActivity(object_id, false);
                    entity_funcs[object_id].onLoop = nil;
                end;
            else
                -- if not visible then...
                disableEntity(object_id); -- TODO: implement flee
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end;


function TorsoBoss_init(id)
    baddie_init(id);
    setCharacterParam(id, PARAM_HEALTH, 500, 500);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, nil, nil, 128, nil, nil, 768);     -- base

    enableEntity(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 1, 0);

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

    entity_funcs[id].onSave = function()
        return "TorsoBoss_init(" .. id .. ");\n";
    end;
end;


function MutantEgg_init(id)
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
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(f + 1 >= c) then
                local spawned_id = spawnEntity(34, getEntityRoom(object_id), getEntityPos(object_id));
                moveEntityLocal(spawned_id, -512.0, 512.0, -4096.0);
                TorsoBoss_init(spawned_id);
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end;
