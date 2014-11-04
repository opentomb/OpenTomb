
-- gameplay globals

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

ENTITY_IS_ACTIVE    = 0x01;
ENTITY_CAN_TRIGGER  = 0x02;   
ENTITY_TRIGGER      = 0x04;
ENTITY_PICKABLE     = 0x08;

-- global frame time var, in seconds
frame_time = 1.0 / 60.0;

-- task manager
-- task struct: {functions array}
engine_tasks = {};

function addTask(f)
    local i = 0;
    while(engine_tasks[i] ~= nil) do
        i = i + 1;          -- hallow borland pascal =) It was a good time
    end
    engine_tasks[i] = f;
end

function doTasks()
    local i = 0;
    while(engine_tasks[i] ~= nil) do
        local t = engine_tasks[i]();
        if(t == false or t == nil) then
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

dofile("scripts/entity/door_script.lua");
dofile("scripts/entity/switch_script.lua");
--
entity_funcs = {};

-- doors - door id's to open; func - additional things we want to do after activation
function create_keyhole_func(id, doors, func, mask)
    setEntityFlag(id, ENTITY_TRIGGER);
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end
    
    setEntityActivity(id, 0);
    for k, v in ipairs(doors) do
        setEntityActivity(v, 0);
    end
    
    entity_funcs[id].activate = function(object_id, activator_id)
        -- canTriggerEntity(activator_id, object_id, max_dist, offset_x, offset_y, offset_z), and see magick 256.0 OY offset
        if(object_id == nil or getEntityActivity(object_id) >= 1 or canTriggerEntity(activator_id, object_id, 256.0, 0.0, 256.0, 0.0) ~= 1) then
            return;
        end

        if(getEntityActivity(object_id) == 0) then
            setEntityPos(activator_id, getEntityPos(object_id));
            moveEntityLocal(activator_id, 0.0, 256.0, 0.0);
            --
            trigger_activate(object_id, activator_id, 
            function(state)
                for k, v in ipairs(doors) do
                    door_activate(v, mask);
                    setEntityActivity(v, 1);
                end
                if(func ~= nil) then
                    func();
                end
            end);
        end
    end;
end

-- standard switch function generator
-- switch: 0 - disabled
-- switch: 1 - enabled
-- switch: 2 - enable
-- switch: 3 - disable
function create_switch_func(id, doors, func, mask)
    setEntityFlag(id, ENTITY_TRIGGER);
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end

    entity_funcs[id].activate = function (object_id, activator_id)
        -- canTriggerEntity(activator_id, object_id, max_dist, offset_x, offset_y, offset_z)
        if(object_id == nil or canTriggerEntity(activator_id, object_id, 256.0, 0.0, 256.0, 0.0) ~= 1) then
            return;
        end
        setEntityPos(activator_id, getEntityPos(object_id));
        moveEntityLocal(activator_id, 0.0, 256.0, 0.0);
        trigger_activate(object_id, activator_id, 
            function(state)
                for k, v in ipairs(doors) do
                    door_activate(v, mask);
                    setEntityActivity(v, 1);
                end
                if(func ~= nil) then
                    func();
                end
            end);
    end;
end

function create_pickup_func(id, item_id)
    setEntityFlag(id, ENTITY_PICKABLE);
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end

    entity_funcs[id].activate = function (object_id, activator_id)
        if((item_id == nil) or (object_id == nil)) then
            return;
        end
        
        local need_set_pos = true;
        local curr_anim = getEntityAnim(activator_id);
        
        if(curr_anim == 103) then               -- Stay idle
            local dx, dy, dz = getEntityVector(object_id, activator_id);
            if(dz < -256.0) then
                need_set_pos = false;
                setEntityAnim(activator_id, 425);   -- Stay pickup, test version
            else
                setEntityAnim(activator_id, 135);   -- Stay pickup  
            end;
        elseif(curr_anim == 222) then           -- Crouch idle
            setEntityAnim(activator_id, 291);   -- Crouch pickup
        elseif(curr_anim == 263) then           -- Crawl idle
            setEntityAnim(activator_id, 292);   -- Crawl pickup
        elseif(curr_anim == 108) then           -- Swim idle
            setEntityAnim(activator_id, 130);   -- Swim pickup
        else
            return;     -- Disable picking up, if Lara isn't idle.
        end;
        
        print("you try to pick up object ".. object_id);
        
        local px, py, pz = getEntityPos(object_id);
        if(curr_anim == 108) then 
            pz = pz + 128.0                     -- Shift offset for swim pickup.
        end;  
        
        if(need_set_pos) then
            setEntityPos(activator_id, px, py, pz);
        end;

        addTask(
        function()
            local a, f, c = getEntityAnim(activator_id);
            local ver = getLevelVersion();
            
            -- Standing pickup anim makes action on frame 40 in TR1-3, in TR4-5
            -- it was generalized with all rest animations by frame 16.
            
            if((a == 135) and (ver < TR_IV)) then
                if(f < 40) then
                    return true;
                end;
            else
                if(f < 16) then
                    return true;
                end;
            end;
            
            addItem(activator_id, item_id);
            setEntityFlag(object_id, 0x00);                 -- disable entity
            setEntityVisibility(object_id, 0x00);
            getEntityActivity(object_id, 0x00);
            
        end);
    end;
end

function activateEntity(object_id, activator_id)
    --print("try to activate "..object_id.." by "..activator_id)
    if((activator_id == nil) or (object_id == nil)) then
        return;
    end

    if( (entity_funcs[object_id] ~= nil) and
        (entity_funcs[object_id].activate ~= nil)) then
            entity_funcs[object_id].activate(object_id, activator_id);
    end
end

print("system_scripts.lua loaded");
