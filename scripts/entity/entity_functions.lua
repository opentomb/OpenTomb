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
dofile(base_path .. "scripts/entity/entity_functions_traps.lua");
dofile(base_path .. "scripts/entity/entity_functions_unique.lua");

function gen_soundsource_init(id)    -- Generic sound source (continous)
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
    
    entity_funcs[id].sound_id = 0;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        stopSound(entity_funcs[object_id].sound_id, object_id); -- Needed for deactivation cases.
        return swapEntityActivity(object_id);
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        playSound(entity_funcs[object_id].sound_id, object_id);
        if(tickEntity(object_id) == TICK_STOPPED) then
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
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityActivity(object_id, 0) end;
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
        return swapEntityState(object_id, 0, 1);
    end;
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then 
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 0) 
        end;
    end;
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityAnimState(object_id, ANIM_TYPE_BASE) == 0) then 
            changeCharacterParam(activator_id, PARAM_HEALTH, -100 * 60.0 * frame_time) 
        end;
    end;
    
    prepareEntity(id);
end


function plough_init(id)     -- Plough (TR4)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, false);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, false);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityActivity(object_id, 0) end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityActivity(object_id)) then
            changeCharacterParam(activator_id, PARAM_HEALTH, -50);
        end;
    end
    
    prepareEntity(id);
end


function crystal_TR3_init(id)   -- "Savegame" crystal (TR3 version)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, true);
    
    entity_funcs[id].onLoop = function(object_id)
        if(getEntityDistance(player, object_id) < 512.0) then
            playSound(SOUND_MEDIPACK);
            changeCharacterParam(player, PARAM_HEALTH, 200);
            disableEntity(object_id);
        end;
    end
end


function fallblock_init(id)  -- Falling block (TR1-3)

    setEntityCallbackFlag(id, ENTITY_CALLBACK_STAND, 1);
    setEntitySpeed(id, 0.0, 0.0, 0.0);

    entity_funcs[id].onStand = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return;
        end

        local anim = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(anim == 0) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 1, 0);
            -- print("you trapped to id = "..object_id);
            local once = true;
            addTask(
            function()
                local anim = getEntityAnim(object_id, ANIM_TYPE_BASE);
                if(anim == 1) then
                    return true;
                end;
                if(once) then
                    setEntityCollision(object_id, 0);
                    once = false;
                end;
                if(dropEntity(object_id, frame_time, nil)) then
                    setEntityAnim(object_id, ANIM_TYPE_BASE, 3, 0);
                    return false;
                end;
                return true;
            end);
        end;
    end;
end


function fallceiling_init(id)  -- Falling ceiling (TR1-3)

    setEntitySpeed(id, 0.0, 0.0, 0.0);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    
    local level_version = getLevelVersion();
    if((level_version < TR_II) or (level_version >= TR_III)) then setEntityVisibility(id, 0) end;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end
        
        local anim = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(anim == 0) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 1, 0);
            setEntityVisibility(object_id, 1);
            addTask(
            function()
                if(dropEntity(object_id, frame_time, nil)) then
                    setEntityAnim(object_id, ANIM_TYPE_BASE, 2, 0);
                    setEntityCollision(object_id, 0);
                    return false;
                end;
                return true;
            end);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((getEntityAnim(object_id, ANIM_TYPE_BASE) == 1) and (getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            setCharacterParam(activator_id, PARAM_HEALTH, 0);
        end;
    end
end


-- TODO: delete PUSH - Lara are moved up without it
function twobp_init(id)        -- Two-block platform

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_STAND, 1);
    setEntityActivity(id, false);
    
    entity_funcs[id].push_height    = 128.0;
    entity_funcs[id].push_speed     = 2.0;
    entity_funcs[id].push           = false;   -- Push flag, set when Lara is on platform.
    entity_funcs[id].current_height = 0.0;     -- Used for all modes.
    entity_funcs[id].waiting        = true;    -- Initial state is stopped.
    
    local curr_OCB = getEntityOCB(id);
    
    entity_funcs[id].raise_height = bit32.rshift(bit32.band(curr_OCB, 0xFFF0), 4) * 256.0;
    entity_funcs[id].raise_speed  = bit32.band(curr_OCB, 0x000F) / 2.0; -- Double FPS.
    
    -- Mode legend: 0 - normal ascent, 1 - normal descent, 2 - push.
    -- Only two classic modes are parsed from OCB, extra modes can be implemented through script.
    
    if(curr_OCB == 0) then 
        entity_funcs[id].mode = 2 
    else 
        entity_funcs[id].mode = 0 
    end;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 1) then
            return ENTITY_TRIGGERING_NOT_READY;
        end;

        if(getEntityActivity(object_id)) then
            if(entity_funcs[object_id].mode < 2) then
                if(entity_funcs[object_id].mode == 0) then 
                    entity_funcs[object_id].mode = 1 
                else 
                    entity_funcs[object_id].mode = 0 
                end;
                entity_funcs[object_id].current_height = entity_funcs[object_id].raise_height - entity_funcs[object_id].current_height;
            else
                setEntityActivity(object_id, false);
            end;
        else
            setEntityActivity(object_id, true);
        end;
        
        entity_funcs[object_id].waiting = false;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) ~= 1) then
            return ENTITY_TRIGGERING_DEACTIVATED;
        end;

        return entity_funcs[id].onActivate(object_id, activator_id);
    end;
    
    entity_funcs[id].onLoop = function(object_id, activator_id)
        if(entity_funcs[object_id].waiting == false) then
            if(entity_funcs[object_id].mode < 2) then
                if(entity_funcs[object_id].current_height < entity_funcs[object_id].raise_height) then
                    local dz = entity_funcs[object_id].raise_speed * 60.0 * frame_time;
                    if(entity_funcs[object_id].mode == 0) then
                        moveEntityLocal(object_id, 0.0, 0.0, dz);
                    else
                        moveEntityLocal(object_id, 0.0, 0.0, -dz);
                    end;
                    
                    entity_funcs[object_id].current_height = entity_funcs[object_id].current_height + dz;
                else
                    if(entity_funcs[object_id].mode == 0) then 
                        entity_funcs[object_id].mode = 1 
                    else 
                        entity_funcs[object_id].mode = 0 
                    end;
                    entity_funcs[object_id].waiting = true;
                    entity_funcs[object_id].current_height = 0.0; -- Reset height counter.
                    setEntityActivity(object_id, false);
                end;
            elseif(entity_funcs[object_id].mode == 2) then
                if(entity_funcs[object_id].push == true) then
                    if(entity_funcs[object_id].current_height < entity_funcs[object_id].push_height) then
                        local dz = entity_funcs[object_id].push_speed * 60.0 * frame_time;
                        moveEntityLocal(object_id, 0.0, 0.0, -dz);
                        entity_funcs[object_id].current_height = entity_funcs[object_id].current_height + dz;
                    else
                        entity_funcs[object_id].current_height = entity_funcs[object_id].push_height;
                    end;
                else
                    if(entity_funcs[object_id].current_height > 0.0) then
                        local dz = entity_funcs[object_id].push_speed * 60.0 * frame_time;
                        moveEntityLocal(object_id, 0.0, 0.0, dz);
                        entity_funcs[object_id].current_height = entity_funcs[object_id].current_height - dz;
                    else
                        entity_funcs[object_id].current_height = 0.0;
                    end;
                end;
                
                entity_funcs[object_id].push = false;
            end;
        end;
    end;
    
    entity_funcs[id].onStand = function(object_id, activator_id)
        if(getEntityActivity(object_id) and (getEntityModel(activator_id) == 0)) then  -- Lara
            entity_funcs[object_id].push = true;
        end;
    end;
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].push_height     = nil;
        entity_funcs[object_id].push_speed      = nil;
        entity_funcs[object_id].push            = nil;
        entity_funcs[object_id].raise_height    = nil;
        entity_funcs[object_id].raise_speed     = nil;
        entity_funcs[object_id].waiting         = nil;
        entity_funcs[object_id].mode            = nil;
        entity_funcs[object_id].current_height  = nil;
    end
    
    prepareEntity(id);
end


function rblock_init(id)        -- Raising block (generic)
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
    
    entity_funcs[id].max_height  = 1024.0;
    entity_funcs[id].move_speed  = 8.0;
    entity_funcs[id].dummy       = (getEntityOCB(id) == -2);    -- TR5 functionality
    
    if(entity_funcs[id].dummy == true) then
        setEntityScaling(id, 1.0, 1.0, 1.0);
        setEntityVisibility(id, 0);
        entity_funcs[id].curr_height = entity_funcs[id].max_height;
        entity_funcs[id].direction   = 2;
    else
        setEntityScaling(id, 1.0, 1.0, 0.0);
        entity_funcs[id].curr_height = 0.0;
        entity_funcs[id].direction   = 1;
    end;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id, activator_id)
        if(entity_funcs[object_id].direction == 1) then
            if((entity_funcs[object_id].dummy == false) and (entity_funcs[object_id].curr_height < entity_funcs[object_id].max_height)) then
                local dz = 60.0 * frame_time * entity_funcs[object_id].move_speed;
                entity_funcs[object_id].curr_height = entity_funcs[object_id].curr_height + dz;
                camShake(125.0, 0.2, object_id);
            else
                entity_funcs[object_id].curr_height = entity_funcs[object_id].max_height;
                entity_funcs[object_id].direction = 2;
                setEntityActivity(object_id, false);
            end;
        else
            if((entity_funcs[object_id].dummy == false) and (entity_funcs[object_id].curr_height > 0.0)) then
                local dz = 60.0 * frame_time * entity_funcs[object_id].move_speed;
                entity_funcs[object_id].curr_height = entity_funcs[object_id].curr_height - dz;
                camShake(125.0, 0.2, object_id);
            else
                entity_funcs[object_id].curr_height = 0.0;
                entity_funcs[object_id].direction = 1;
                setEntityActivity(object_id, false);
            end;
        end;
        
        setEntityScaling(object_id, 1.0, 1.0, (entity_funcs[object_id].curr_height / entity_funcs[object_id].max_height));
    end;
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].curr_height = nil;
        entity_funcs[object_id].max_height  = nil;
        entity_funcs[object_id].move_speed  = nil;
        entity_funcs[object_id].direction   = nil;
        entity_funcs[object_id].dummy       = nil;
    end
    
    prepareEntity(id);
    
end


function rblock2_init(id)   -- Raising block x2 - same as RB1, only max height / speed is changed.
    rblock_init(id);
    entity_funcs[id].max_height = 2048.0;
    entity_funcs[id].move_speed = 16.0;
end


function expplatform_init(id)        -- Expanding platform
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
    
    entity_funcs[id].max_width  = 1024.0;
    entity_funcs[id].move_speed = 8.0;
    entity_funcs[id].curr_width = 0.0;
    entity_funcs[id].direction  = 1;
    
    setEntityScaling(id, 1.0, 0.0, 1.0);
    moveEntityLocal(id, 0.0, entity_funcs[id].max_width / 2, 0.0);  -- Fix position
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id, activator_id)
        if(entity_funcs[object_id].direction == 1) then
            if(entity_funcs[object_id].curr_width < entity_funcs[object_id].max_width) then
                local dz = 60.0 * frame_time * entity_funcs[object_id].move_speed;
                entity_funcs[object_id].curr_width = entity_funcs[object_id].curr_width + dz;
            else
                entity_funcs[object_id].curr_width = entity_funcs[object_id].max_width;
                entity_funcs[object_id].direction = 2;
                setEntityActivity(object_id, false);
            end;
        else
            if(entity_funcs[object_id].curr_width > 0.0) then
                local dz = 60.0 * frame_time * entity_funcs[object_id].move_speed;
                entity_funcs[object_id].curr_width = entity_funcs[object_id].curr_width - dz;
            else
                entity_funcs[object_id].curr_width = 0.0;
                entity_funcs[object_id].direction = 1;
                setEntityActivity(object_id, false);
            end;
        end;
        
        setEntityScaling(object_id, 1.0, (entity_funcs[object_id].curr_width / entity_funcs[object_id].max_width), 1.0);
        
    end;
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].curr_width = nil;
        entity_funcs[object_id].max_width  = nil;
        entity_funcs[object_id].move_speed  = nil;
        entity_funcs[object_id].direction   = nil;
    end
    
    prepareEntity(id);
end


function baddie_init(id)    -- INVALID!

    setEntityTypeFlag(id, ENTITY_TYPE_ACTOR);
    disableEntity(id);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then 
            enableEntity(object_id) 
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
    
end
