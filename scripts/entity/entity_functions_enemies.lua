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
        setEntityGhostCollisionShape(id,  m,  COLLISION_SHAPE_BOX, 0, 0, 0, 0, 0, 0);
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

function bat_init(id, is_spawned)
    baddie_init(id);
    
    setCharacterParam(id, PARAM_HEALTH, 100, 100);
    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_SPHERE, -64, -64, -64, 64, 64, 64);
    setEntityMoveType(id, MOVE_FLY);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_BAT);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "bat_init(" .. id .. ", true);\n";
        end;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then 
            enableEntity(object_id);
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
            setEntityAnim(object_id, ANIM_TYPE_BASE, 3, 0);
        end;
    end;

    entity_funcs[id].onLoop = nil;

end;


function wolf_init(id, is_spawned)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, -1);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_WOLF);

    setCharacterParam(id, PARAM_HEALTH, 200, 200);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  3,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "wolf_init(" .. id .. ", true);\n";
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


function bear_init(id, is_spawned)
    baddie_init(id);
    
    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id,  14,  COLLISION_SHAPE_SPHERE, -256, -128, -256, 256, 256, 128);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_BEAR);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "bear_init(" .. id .. ", true);\n";
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


function raptor_init(id, is_spawned)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_RAPTOR);

    setCharacterParam(id, PARAM_HEALTH, 200, 200);
    setEntityGhostCollisionShape(id,  21,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  22,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  24,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "raptor_init(" .. id .. ", true);\n";
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


function lion_init(id, is_spawned)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_LION);

    setCharacterParam(id, PARAM_HEALTH, 200, 200);
    setEntityGhostCollisionShape(id,  7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  19,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  20,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "lion_init(" .. id .. ", true);\n";
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


function puma_init(id, is_spawned)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_PUMA);

    setCharacterParam(id, PARAM_HEALTH, 200, 200);
    setEntityGhostCollisionShape(id,  7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  19,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  20,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "puma_init(" .. id .. ", true);\n";
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


function winged_mutant_init(id, is_spawned)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_WINGED_MUTANT);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  3,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  15,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- wing
    setEntityGhostCollisionShape(id,  18,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- wing

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "winged_mutant_init(" .. id .. ", true);\n";
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


function trex_init(id, is_spawned)
    baddie_init(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityAnimState(id, ANIM_TYPE_BASE, 1);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_TREX);

    setCharacterParam(id, PARAM_HEALTH, 2000, 2000);
    setEntityGhostCollisionShape(id,  9,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  10,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  11,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  12,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  13,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  20,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  21,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    
    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "trex_init(" .. id .. ", true);\n";
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


function gorilla_init(id, is_spawned)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);   -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);   -- hand
    setEntityGhostCollisionShape(id, 11,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- hand
    setEntityGhostCollisionShape(id, 14,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- head
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_GORILLA);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "gorilla_init(" .. id .. ", true);\n";
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


function crocodile_init(id, is_spawned)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 400, 400);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 10,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 11,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_CROCODILE);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "crocodile_init(" .. id .. ", true);\n";
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


function rat_init(id, is_spawned)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 150, 150);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 9,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_RAT);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "rat_init(" .. id .. ", true);\n";
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


function centaur_init(id, is_spawned)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 550, 550);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 10, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 11, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 17, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 18, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_CENTAUR);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "centaur_init(" .. id .. ", true);\n";
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


function Larson_init(id, is_spawned)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_LARSON);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "Larson_init(" .. id .. ", true);\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
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


function Pierre_init(id, is_spawned)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_PIERRE);
    entity_funcs[id].is_flee = (getLevel() ~= 9) or (getEntityRoom(id) ~= 110);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "Pierre_init(" .. id .. ", true);\n";
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

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local hp = getCharacterParam(object_id, PARAM_HEALTH);
        if((hp == 0) and (getLevel() == 9)) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if((a == 12) and (f + 1 >= c)) then
                local spawned_id = spawnEntity(133, getEntityRoom(object_id), getEntityPos(object_id));
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        elseif((hp == 1) and entity_funcs[object_id].is_flee) then

        end;
    end;
end;


function cowboy_init(id, is_spawned)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_COWBOY);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "cowboy_init(" .. id .. ", true);\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end


function MrT_init(id, is_spawned)
    baddie_init(id);

    setCharacterParam(id, PARAM_HEALTH, 300, 300);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, 0, 60.0, nil, 16.0);   -- base
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- torso
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_SPHERE, nil, nil, nil, nil, nil, nil);     -- head
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);        -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_MRT);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "MrT_init(" .. id .. ", true);\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;
end


function skateboardist_init(id, is_spawned)
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

    entity_funcs[id].onSave = function()
        if(entity_funcs[id].skate_id ~= nil) then
            local sp_save = "";
            if(is_spawned ~= nil) then
                sp_save = "skateboardist_init(" .. id .. ", true);\n";
            end;
            return sp_save .. "entity_funcs[" .. id .. "].skate_id = " .. entity_funcs[id].skate_id .. ";\n";
        end;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then 
            enableEntity(object_id);
            entity_funcs[object_id].skate_id = spawnEntity(29, getEntityRoom(object_id), getEntityPos(object_id));
            setEntityCollision(entity_funcs[object_id].skate_id, false);
            setEntityActivity(entity_funcs[object_id].skate_id, true);
            --print("skate = " .. entity_funcs[object_id].skate_id);
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

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local a, f = getEntityAnim(object_id, ANIM_TYPE_BASE);
        setEntityAnim(entity_funcs[object_id].skate_id, ANIM_TYPE_BASE, a, f);
        entitySSAnimCopy(entity_funcs[object_id].skate_id, object_id);
        setEntityPos(entity_funcs[object_id].skate_id, getEntityPos(object_id));
    end;
end


function TorsoBoss_init(id, is_spawned)
    baddie_init(id);
    setCharacterParam(id, PARAM_HEALTH, 1500, 1500);
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, nil, nil, 128, nil, nil, 768);     -- base
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_TORSO_BOSS);

    enableEntity(id);
    setEntityAnim(id, ANIM_TYPE_BASE, 1, 0);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "TorsoBoss_init(" .. id .. ", true);\n";
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


function Natla_init(id, is_spawned)
    baddie_init(id);
    setCharacterParam(id, PARAM_HEALTH, 600, 600);
    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  18,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);  -- leg
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_NATLA);

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "Natla_init(" .. id .. ", true);\n";
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if((getCharacterParam(object_id, PARAM_HEALTH) == 0) and (5 == getEntityAnimState(object_id, ANIM_TYPE_BASE))) then
            setEntityCollision(object_id, true);
            setCharacterState(object_id, CHARACTER_STATE_DEAD, 0);
            setCharacterParam(object_id, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        end;
    end;
end


function mummy_init(id, is_spawned)
    winged_mutant_init(id);
    setEntityGhostCollisionShape(id,  15,  COLLISION_SHAPE_BOX, 0, 0, 0, 0, 0, 0);  -- wing
    setEntityGhostCollisionShape(id,  18,  COLLISION_SHAPE_BOX, 0, 0, 0, 0, 0, 0);  -- wing
    --TODO: delete wings!!!

    if(is_spawned ~= nil) then
        entity_funcs[id].onSave = function()
            return "mummy_init(" .. id .. ", true);\n";
        end;
    end;
end


function mummy_spawner_init(id)
    entity_funcs[id].spawned_id = nil;
    
    entity_funcs[id].onSave = function()
        if(entity_funcs[id].spawned_id ~= nil) then
            return "entity_funcs[" .. id .. "].spawned_id = " .. entity_funcs[id].spawned_id .. ";\n";
        end;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(entity_funcs[object_id].spawned_id == nil) then
            entity_funcs[object_id].spawned_id = spawnEntity(20, getEntityRoom(object_id), getEntityPos(object_id));
            mummy_init(entity_funcs[object_id].spawned_id, true);
            enableEntity(entity_funcs[object_id].spawned_id);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
end


function mutant_spawner_init(id)
    entity_funcs[id].spawned_id = nil;
    
    entity_funcs[id].onSave = function()
        if(entity_funcs[id].spawned_id ~= nil) then
            return "entity_funcs[" .. id .. "].spawned_id = " .. entity_funcs[id].spawned_id .. ";\n";
        end;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(entity_funcs[object_id].spawned_id == nil) then
            entity_funcs[object_id].spawned_id = spawnEntity(20, getEntityRoom(object_id), getEntityPos(object_id));
            winged_mutant_init(entity_funcs[object_id].spawned_id, true);
            enableEntity(entity_funcs[object_id].spawned_id);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
end


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
                TorsoBoss_init(spawned_id, true);

                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end;
