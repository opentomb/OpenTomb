-- OPENTOMB SYSTEM SCRIPT
-- By TeslaRus, 2014

--------------------------------------------------------------------------------
-- This script defines some system-level functions, like task management.
--------------------------------------------------------------------------------

-- Global frame time variable, in seconds

frame_time = 1.0 / 60.0;


-- Key query functions

keys_pressed = {};

function addKey(code, event)
    if(event >= 1) then
        keys_pressed[code] = (keys_pressed[code] == nil);
    else
        keys_pressed[code] = nil;
    end;
end

function checkKey(code, once)
    if(keys_pressed[code] ~= nil) then
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
        keys_pressed[k] = nil;
    end;
end

function printTable(tbl)
    for k, v in pairs(tbl) do
        if(v ~= nil) then
            print("tbl[" .. k .. "] = " .. v);
        else
            print("tbl[" .. k .. "] = nil");
        end;
    end;
end;

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
        return -1;
    end

    if(entity_funcs[object_id] ~= nil) then
        if((bit32.band(callback_id, ENTITY_CALLBACK_ACTIVATE) ~= 0) and (entity_funcs[object_id].onActivate ~= nil)) then
            return entity_funcs[object_id].onActivate(object_id, activator_id);
        end;
        
        if((bit32.band(callback_id, ENTITY_CALLBACK_DEACTIVATE) ~= 0) and (entity_funcs[object_id].onDeactivate ~= nil)) then
            return entity_funcs[object_id].onDeactivate(object_id, activator_id);
        end;

        if((bit32.band(callback_id, ENTITY_CALLBACK_COLLISION) ~= 0) and (entity_funcs[object_id].onCollide ~= nil)) then
            return entity_funcs[object_id].onCollide(object_id, activator_id);
        end;

        if((bit32.band(callback_id, ENTITY_CALLBACK_STAND) ~= 0) and (entity_funcs[object_id].onStand ~= nil)) then
            return entity_funcs[object_id].onStand(object_id, activator_id);
        end;

        if((bit32.band(callback_id, ENTITY_CALLBACK_HIT) ~= 0) and (entity_funcs[object_id].onHit ~= nil)) then
            return entity_funcs[object_id].onHit(object_id, activator_id);
        end;
    end;
    return -1;
end

function loopEntity(object_id, tick_state)
    if((object_id ~= nil) and (entity_funcs[object_id] ~= nil) and (entity_funcs[object_id].onLoop ~= nil)) then
        entity_funcs[object_id].onLoop(object_id, tick_state);
    end;
end

print("System_scripts.lua loaded");
