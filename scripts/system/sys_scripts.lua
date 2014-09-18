
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
            function(state)                                                     -- вырезать жёсткий активатор, заменить коллбэком!!!!!
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
                    func(state);
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
        
        local dx, dy, dz = getEntityVector(object_id, activator_id);
        local pickup_state = 39;
        if(dz < -256.0) then
            pickup_state = 137;                             -- FIXME: warious states for difference items relative Z positions
        end;

        if(getEntityState(activator_id) == pickup_state) then
            return;
        end;
        print("you try to pick up object ".. object_id);

        setEntityState(activator_id, pickup_state);         -- set the pick up animation.
        addTask(
        function()
            local a, f, c = getEntityAnim(actor_id);
            if(f < c - 1) then
                return true;
            end;
            addItem(activator_id, item_id, 1);
        end);
    end;
end

function activateEntity(object_id, activator_id, trigger_mask)
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
