-- OPENTOMB SYSTEM SCRIPT
-- By TeslaRus, 2014

--------------------------------------------------------------------------------
-- This script defines some system-level functions, like task management.
--------------------------------------------------------------------------------

-- Key query functions

keys_pressed = {};

function addKey(code, pressed)
    if(pressed) then
        keys_pressed[code] = not keys_pressed[code];
    else
        keys_pressed[code] = nil;
    end;
end

function checkKey(code, once)
    if(keys_pressed[code]) then
        if(once) then
            return keys_pressed[code];
        else
            return true;
        end;
    else
        return false;
    end;
end

function clearKeys()
    for k,v in pairs(keys_pressed) do
        keys_pressed[k] = false;
    end;
end


-- Task manager functions

engine_tasks = {};  -- task struct: {functions array}

function addTask(f)
    local i = 0;
    while(engine_tasks[i] ~= nil) do
        i = i + 1;  -- find first empty task index.
    end
    engine_tasks[i] = f;    -- put the task into free element.
end

function doTasks()
    local i = 0;
    while(engine_tasks[i] ~= nil) do
        local t = engine_tasks[i]();
        if(t == false or t == nil) then     -- remove task, if it returns nil or false.
            local j = i;
            while(engine_tasks[j] ~= nil) do
                engine_tasks[j] = engine_tasks[j + 1];
                j = j + 1;
            end
        end
        i = i + 1;
    end
end

function clearTasks()
    local i = 0;
    while(engine_tasks[i] ~= nil) do
        engine_tasks[i] = nil;
        i = i + 1;
    end
end


-- System entity functions

function execEntity(callback_id, object_id, activator_id)
    
    if((object_id == nil) or (callback_id == nil)) then -- Activator may be nil, in case of flyby camera heavy triggering!
        return;
    end

    if(entity_funcs[object_id] ~= nil) then
        if((bit32.band(callback_id, ENTITY_CALLBACK_ACTIVATE) ~= 0) and (entity_funcs[object_id].onActivate ~= nil)) then
            entity_funcs[object_id].onActivate(object_id, activator_id);
        end;
        
        if((bit32.band(callback_id, ENTITY_CALLBACK_DEACTIVATE) ~= 0) and (entity_funcs[object_id].onDeactivate ~= nil)) then
            entity_funcs[object_id].onDeactivate(object_id, activator_id);
        end;

        if((bit32.band(callback_id, ENTITY_CALLBACK_COLLISION) ~= 0) and (entity_funcs[object_id].onCollide ~= nil)) then
            entity_funcs[object_id].onCollide(object_id, activator_id);
        end;

        if((bit32.band(callback_id, ENTITY_CALLBACK_STAND) ~= 0) and (entity_funcs[object_id].onStand ~= nil)) then
            entity_funcs[object_id].onStand(object_id, activator_id);
        end;

        if((bit32.band(callback_id, ENTITY_CALLBACK_HIT) ~= 0) and (entity_funcs[object_id].onHit ~= nil)) then
            entity_funcs[object_id].onHit(object_id, activator_id);
        end;
    end;
end

function loopEntity(object_id)
    if((object_id == nil) or (entity_funcs[object_id] == nil) or (entity_funcs[object_id].onLoop == nil)) then return end;
    entity_funcs[object_id].onLoop(object_id);
end

function tickEntity(object_id)
    local timer = getEntityTimer(object_id);
    if(timer > 0.0) then
        timer = timer - frame_time;
        if(timer < 0.0) then timer = 0.0 end;
        setEntityTimer(object_id, timer);
        if(timer == 0.0) then
            return TICK_STOPPED;
        end;
    else
        return TICK_IDLE;
    end;
    return TICK_ACTIVE;
end

print("System_scripts.lua loaded");
