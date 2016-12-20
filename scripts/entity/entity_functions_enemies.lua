-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2016

--Script_ExecEntity(engine_lua, ENTITY_CALLBACK_HIT, e->id, ent->id);
function baddie_init(id)    -- INVALID!

    setEntityTypeFlag(id, ENTITY_TYPE_ACTOR);
    characterCreate(id, 100.0);
    setEntityGhostCollisionShape(id,  0,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  1,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id,  2,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id,  3,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id,  4,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id,  5,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id,  6,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id,  7,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id,  8,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id,  9,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 10,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 11,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 12,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 13,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 14,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 15,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 16,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 17,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 18,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityGhostCollisionShape(id, 19,  COLLISION_SHAPE_SPHERE, 1, -1, 0, 0, 0, 0);
    setEntityMoveType(id, MOVE_FLY);
    disableEntity(id);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) > 0) and (not getEntityActivity(object_id))) then 
            --print("enemy activated: " .. object_id);
            enableEntity(object_id) 
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;


    entity_funcs[id].onHit = function(object_id, activator_id)
        if(getCharacterParam(object_id, PARAM_HEALTH) == 0) then
            setCharacterTarget(activator_id, nil);
            setEntityActivity(object_id, false);
            setEntityCollision(object_id, false);
            -- DO KILL ANIM
        end;
    end;

    entity_funcs[id].onLoop = function(object_id)
        if((getCharacterParam(object_id, PARAM_HEALTH) == 0) and (getEntityMoveType(object_id) == MOVE_FLY)) then
            if(dropEntity(object_id, frame_time)) then
                setEntityActivity(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;

end

