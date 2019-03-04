print("entity_functions_enemies->basic_function loaded !");

-- default function !
function basic_init(id) -- INVALID!
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
            setEntityActivity(object_id, false);
            -- DO KILL ANIM
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(getCharacterState(object_id, CHARACTER_STATE_DEAD) > 1) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(f + 1 >= c) then
                setEntityActivity(object_id, false);
                setEntityCollision(object_id, false);
                entity_funcs[object_id].onLoop = nil;
            end;
        end;
    end;
end;