-- OPENTOMB SYSTEM SCRIPT
-- By TeslaRus, 2014

--------------------------------------------------------------------------------
-- This script defines some system-level functions, like task management.
--------------------------------------------------------------------------------

-- Gameplay globals

TR_I                = 0;
TR_I_DEMO           = 1;
TR_I_UB             = 2;
TR_II               = 3;
TR_II_DEMO          = 4;
TR_III              = 5;
TR_IV               = 6;
TR_IV_DEMO          = 7;
TR_V                = 8;
TR_UNKNOWN          = 127;

ENTITY_STATE_ENABLED                      = 0x0001;     -- Entity is enabled
ENTITY_STATE_ACTIVE                       = 0x0002;     -- Entity is animated - RENAME IT TO ENTITY_STATE_ANIMATING
ENTITY_STATE_VISIBLE                      = 0x0004;     -- Entity is visible

ENTITY_TYPE_GENERIC                       = 0x0000;
ENTITY_TYPE_INTERACTIVE                   = 0x0001;
ENTITY_TYPE_TRIGGER_ACTIVATOR             = 0x0002;
ENTITY_TYPE_PICKABLE                      = 0x0004;
ENTITY_TYPE_TRAVERSE                      = 0x0008;
ENTITY_TYPE_TRAVERSE_FLOOR                = 0x0010;
ENTITY_TYPE_ACTOR                         = 0x0020;

ENTITY_CALLBACK_NONE                      = 0x00000000;
ENTITY_CALLBACK_ACTIVATE                  = 0x00000001;
ENTITY_CALLBACK_DEACTIVATE                = 0x00000002;
ENTITY_CALLBACK_COLLISION                 = 0x00000004;
ENTITY_CALLBACK_STAND                     = 0x00000008;
ENTITY_CALLBACK_HIT                       = 0x00000010;

MOVE_STATIC_POS    = 0;
MOVE_KINEMATIC     = 1;
MOVE_ON_FLOOR      = 2;
MOVE_WADE          = 3;
MOVE_QUICKSAND     = 4;
MOVE_ON_WATER      = 5;
MOVE_UNDERWATER    = 6;
MOVE_FREE_FALLING  = 7;
MOVE_CLIMBING      = 8;
MOVE_MONKEYSWING   = 9;
MOVE_WALLS_CLIMB   = 10;
MOVE_DOZY          = 11;

TICK_IDLE    = 0;
TICK_STOPPED = 1;
TICK_ACTIVE  = 2;


-- Global frame time variable, in seconds

frame_time = 1.0 / 60.0;

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

-- timer test function
function tt()
    local t = 0.0;          -- we can store time only here
    addTask(
    function()
        if(t < 8.0) then
            t = t + frame_time;
            print(t);
            return true;
        end
        print("8 seconds complete!");
    end);
end

function execEntity(object_id, activator_id, callback_id)
    
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

print("system_scripts.lua loaded");
