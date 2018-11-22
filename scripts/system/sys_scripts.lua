-- OPENTOMB SYSTEM SCRIPT
-- By TeslaRus, 2014
print("sys_scripts->loaded !");

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

print("System_scripts.lua loaded");
