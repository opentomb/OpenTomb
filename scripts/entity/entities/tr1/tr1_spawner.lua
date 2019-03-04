local level = getLevel();

function tr1_mummy_init(id)
    tr1_winged_mutant_init(id); -- tr1_winged_mutant already have entitybone defined !
    
    setEntityBoneVisibility(id, 15, false);      -- wing
    setEntityBoneVisibility(id, 16, false);
    setEntityBoneVisibility(id, 17, false);
    setEntityBoneVisibility(id, 18, false);      -- wing
    setEntityBoneVisibility(id, 19, false);
    setEntityBoneVisibility(id, 20, false);

    if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
        entity_funcs[id].onSave = function()
            return "tr1_mummy_init(" .. id .. ");\n";
        end;
    end;
end;

function tr1_mummy_spawner_init(id)
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 0) then
            local spawned_id = spawnEntity(20, getEntityRoom(object_id), getEntityPos(object_id));
            tr1_mummy_init(spawned_id);
            enableEntity(spawned_id);
            setCharacterTarget(spawned_id, player);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
end;

function tr1_mutant_spawner_init(id)
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 0) then
            local spawned_id = spawnEntity(20, getEntityRoom(object_id), getEntityPos(object_id));
            tr1_winged_mutant_init(spawned_id);
            enableEntity(spawned_id);
            setCharacterTarget(spawned_id, player);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
end;

function tr1_mutant_egg_init(id)
    setEntityActivity(id, false);

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
                setEntityCollision(object_id, false);
                local spawned_id = spawnEntity(20, getEntityRoom(object_id), getEntityPos(object_id));
                moveEntityLocal(spawned_id, -512.0, 512.0, 0.0);
                tr1_winged_mutant_init(spawned_id);
                setCharacterTarget(spawned_id, player);
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

function tr1_mutant_boss_egg_init(id)
    if(level == 19) then
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
                    tr1_torsoboss_init(spawned_id);
                    setCharacterTarget(spawned_id, player);

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