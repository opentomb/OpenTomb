-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2016

--Script_ExecEntity(engine_lua, ENTITY_CALLBACK_HIT, e->id, ent->id);
function baddie_init(id)    -- INVALID!

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

    entity_funcs[id].onLoop = function(object_id)
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(f + 1 >= c) then
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end


function bat_init(id)
    baddie_init(id);
    
    setCharacterParam(id, PARAM_HEALTH, 100, 100);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityMoveType(id, MOVE_FLY);
    noFixEntityCollision(id);

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityCollision(object_id, false);
            setEntityAnim(object_id, ANIM_TYPE_BASE, 3, 0);
        end;
    end;

    entity_funcs[id].onLoop = function(object_id)
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