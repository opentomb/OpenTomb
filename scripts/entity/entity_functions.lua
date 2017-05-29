-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2015

--------------------------------------------------------------------------------
-- This script contains routines for all entity functions. Note that it is not
-- just entity functions, but "function creation" routines, which usually
-- includes some events executed on level start-up. Function itself is placed
-- inside entity function array (entity_funcs).
--------------------------------------------------------------------------------

entity_funcs = {};  -- Initialize entity function array.

-- Erase single entity function.

function efuncs_EraseEntity(index)
    if(entity_funcs[index] ~= nil) then
        if(entity_funcs[index].onDelete ~= nil) then    -- Entity-specific clean-up.
            entity_funcs[index].onDelete(index);
            entity_funcs[index].onDelete = nil;
        end;
        entity_funcs[index].onActivate       = nil;
        entity_funcs[index].onDeactivate     = nil;
        entity_funcs[index].onCollide        = nil;
        entity_funcs[index].onStand          = nil;
        entity_funcs[index].onHit            = nil;
        entity_funcs[index].onLoop           = nil;
        entity_funcs[index].onSave           = nil;
        entity_funcs[index].onLoad           = nil;
        entity_funcs[index]                  = nil;
    end;
end;

-- Clear whole entity functions array. Must be called on each level loading.

function entfuncs_Clear()
    for k,v in pairs(entity_funcs) do
        efuncs_EraseEntity(k);
    end;
    print("Entity function table cleaned");
end;

--------------------------------------------------------------------------------
-- ENTITY FUNCTIONS IMPLEMENTATION
--------------------------------------------------------------------------------

dofile(base_path .. "scripts/entity/entity_functions_common.lua");
dofile(base_path .. "scripts/entity/entity_functions_switch.lua");
dofile(base_path .. "scripts/entity/entity_functions_traps.lua");
dofile(base_path .. "scripts/entity/entity_functions_unique.lua");
dofile(base_path .. "scripts/entity/entity_functions_enemies.lua");
dofile(base_path .. "scripts/entity/entity_functions_platforms.lua");


function getEntitySaveData(id)
    if((entity_funcs ~= nil) and (entity_funcs[id] ~= nil) and (entity_funcs[id].onSave ~= nil)) then
        return entity_funcs[id].onSave();
    end;
    return "";
end;

function gen_soundsource_init(id)    -- Generic sound source (continous)
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
    
    entity_funcs[id].sound_id = 0;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        stopSound(entity_funcs[object_id].sound_id, object_id); -- Needed for deactivation cases.
        return swapEntityActivity(object_id);
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id, tick_state)
        playSound(entity_funcs[object_id].sound_id, object_id);
        if(tick_state == TICK_STOPPED) then
            setEntityActivity(object_id, false)
            stopSound(entity_funcs[object_id].sound_id, object_id);
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].sound_id = nil;
    end
end


function randomized_soundsource_init(id)    -- Randomized sound source

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
    
    entity_funcs[id].chance   = 0;  -- Max. chance is 1000, if large, always plays.
    entity_funcs[id].sound_id = 0;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        return swapEntityActivity(object_id);
    end
    
    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(tick_state == TICK_STOPPED) then setEntityActivity(object_id, 0) end;
        if((math.random(1000) > (1000 - entity_funcs[object_id].chance)) and (getEntityDistance(player, object_id) < 8192.0)) then
            playSound(entity_funcs[object_id].sound_id, object_id);
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].chance   = nil;
        entity_funcs[object_id].sound_id = nil;
    end
    
    prepareEntity(id);
end


function propeller_init(id)      -- Generic propeller (TR1-TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, true);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        local a = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(a ~= 0) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 0, 0);
        end;
    end;
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityAnimState(object_id, ANIM_TYPE_BASE, 1);
    end;
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityAnimState(object_id, ANIM_TYPE_BASE) == 0) then 
            changeCharacterParam(activator_id, PARAM_HEALTH, -6000.0 * frame_time) 
        end;
    end;
    
    prepareEntity(id);
end


function fallblock_init(id)  -- Falling block (TR1-3)

    setEntityCallbackFlag(id, ENTITY_CALLBACK_STAND, 1);
    setEntitySpeed(id, 0.0, 0.0, 0.0);
    setEntityActivity(id, false);

    entity_funcs[id].onStand = function(object_id, activator_id)
        local anim = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(anim == 0) then
            setEntityActivity(object_id, true);
            setEntityAnim(object_id, ANIM_TYPE_BASE, 1, 0);
        end;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(a == 2) then
            setEntityCollision(object_id, false);
            if(dropEntity(object_id, frame_time, true)) then
                setEntityAnim(object_id, ANIM_TYPE_BASE, 3, 0);
            end;
        elseif(a >= 3) then
            if(f + 1 >= c) then
                setEntityActivity(object_id, false);
            end;
        end;
    end;
end


function fallceiling_init(id)  -- Falling ceiling (TR1-3)

    setEntitySpeed(id, 0.0, 0.0, 0.0);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, false);

    local level_version = getLevelVersion();
    if((level_version < TR_II) or (level_version >= TR_III)) then 
        setEntityVisibility(id, false);
    end;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        local anim = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(anim == 0) then
            enableEntity(object_id);
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 1);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
    
    entity_funcs[id].onLoop = function(object_id, tick_state)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(a == 1) then
            if(dropEntity(object_id, frame_time, true)) then
                setEntityAnimState(object_id, ANIM_TYPE_BASE, 2);
            end;
        elseif(a >= 2) then
            if(f + 1 >= c) then
                setEntityCollision(object_id, false);
                setEntityActivity(object_id, false);
            end;
        end;
    end;

    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((getEntityAnim(object_id, ANIM_TYPE_BASE) == 1) and (getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            setCharacterParam(activator_id, PARAM_HEALTH, 0);
        end;
    end;
end
