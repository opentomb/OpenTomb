-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2015

--------------------------------------------------------------------------------
-- This script contains routines for all entity functions. Note that it is not
-- just entity functions, but "function creation" routines, which usually
-- includes some events executed on level start-up. Function itself is placed
-- inside entity function array (entity_funcs).
--------------------------------------------------------------------------------

entity_funcs = {};  -- Initialize entity function array.

-- Load up some extra entity scripts.

dofile("scripts/entity/script_door.lua");       -- Additional door scripts.
dofile("scripts/entity/script_switch.lua");     -- Additional switch scripts.

-- Erase single entity function.

function efuncs_EraseEntity(index)
    if(entity_funcs[index] ~= nil) then
        entity_funcs[index].onActivate      = nil;
        entity_funcs[index].onDeactivate    = nil;
        entity_funcs[index].onCollide       = nil;
        entity_funcs[index].onStand         = nil;
        entity_funcs[index].onHit           = nil;
        entity_funcs[index].onLoop          = nil;
        entity_funcs[index]                 = nil;
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

function door_func(id)   -- NORMAL doors only!

    setEntityTypeFlag(id, ENTITY_TYPE_DECORATION);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};  -- Create function.
    end

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(object_id == nil) then return end;
        door_activate(object_id);
    end;
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityState(object_id, 0) end;
    end
    
    door_activate(id);
end

function keyhole_func(id)    -- Key and puzzle holes.

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};  -- Create function.
    end

    -- onActivate function
    entity_funcs[id].onActivate = function(object_id, activator_id)
        -- canTriggerEntity(activator_id, object_id, max_dist, offset_x, offset_y, offset_z)
        -- MEANING: only continue if object can be triggered by activator on a certain distance and offset.
        
        if(object_id == nil or getEntityActivity(object_id) >= 1 or canTriggerEntity(activator_id, object_id, 256.0, 0.0, 256.0, 0.0) ~= 1) then
            return;
        end
        
        if(getEntityActivity(object_id) == 0) then
            setEntityPos(activator_id, getEntityPos(object_id));
            moveEntityLocal(activator_id, 0.0, 384.0, 0.0);
            switch_activate(object_id, activator_id);
        end
    end;
end

function switch_func(id)     -- Ordinary switches.
    
    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};  -- Create function.
    end

    -- onActivate function
    entity_funcs[id].onActivate = function(object_id, activator_id)
        -- canTriggerEntity(activator_id, object_id, max_dist, offset_x, offset_y, offset_z)
        -- MEANING: only continue if object can be triggered by activator on a certain distance and offset.
        if(object_id == nil or canTriggerEntity(activator_id, object_id, 256.0, 0.0, 256.0, 0.0) ~= 1) then
            return;
        end
        
        setEntityPos(activator_id, getEntityPos(object_id));    -- Move activator right next to object.
        moveEntityLocal(activator_id, 0.0, 384.0, 0.0);         -- Shift activator back to proper distance.
        switch_activate(object_id, activator_id);               -- Make switching routines.
    end;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityState(object_id, 1);
            setEntitySectorStatus(object_id, 1);
        end;
    end;
end

function anim_func(id)      -- Ordinary animatings.

    setEntityTypeFlag(id, ENTITY_TYPE_DECORATION);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};  -- Create function.
    end
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        setEntityState(object_id, 1);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
        setEntityState(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityState(object_id, 0) end;
    end
end

function venicebird_func(id)    -- Venice singing birds (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_DECORATION);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};              -- Create function.
    end
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
      setEntityActivity(object_id, 1);
    end
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityActivity(object_id, 0) end;
        if(getEntityActivity(object_id) == 1) then
            if(math.random(100000) > 99500) then playSound(316, object_id) end;
        end;
    end
    
    activateEntity(id);
end

function swingblade_func(id)        -- Swinging blades (TR1)

    setEntityTypeFlag(id, ENTITY_TYPE_DECORATION);
    setEntityActivity(id, 1);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};  -- Create function.
    end
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityState(object_id, 2);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityState(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityState(object_id, 0) end;
    end
    
    activateEntity(id);
end

function slamdoor_func(id)      -- Slamming doors (TR1-TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_DECORATION);
    setEntityActivity(id, 1);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};  -- Create function.
    end
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityState(object_id, 1);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityState(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityState(object_id, 0) end;
    end
    
    activateEntity(id);
end

function wallblade_func(id)     -- Wall blade (TR1-TR3)

    setEntityTypeFlag(id, ENTITY_TYPE_DECORATION);
    setEntityActivity(id, 0);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};  -- Create function.
    end
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityActivity(object_id, 0) end;
        if(getEntityActivity(object_id) == 1) then
            local anim_number = getEntityAnim(object_id)
            if(anim_number == 2) then
                setEntityAnim(object_id, 3)
            elseif(anim_number == 1) then
                setEntityAnim(object_id, 0)
            end;
        end;
    end
    
    activateEntity(id);
end

function pickup_func(id, item_id)    -- Pick-ups

    setEntityTypeFlag(id, ENTITY_TYPE_PICKABLE);
    setEntityActivationOffset(id, 0.0, 0.0, 0.0, 480.0);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end

    entity_funcs[id].onActivate = function(object_id, activator_id)
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

        print("You're trying to pick up object " .. object_id);


        addTask(
        function()
        
            -- Position corrector
        
            if(getEntityMoveType(activator_id) == 6) then
                if(getEntityDistance(object_id, activator_id) > 128.0) then
                    moveEntityToEntity(activator_id, object_id, 25.0);
                end;
            else
                if(getEntityDistance(object_id, activator_id) > 32.0) then
                    moveEntityToEntity(activator_id, object_id, 50.0);
                end;;
            end;
            
            local a, f, c = getEntityAnim(activator_id);
            
            -- Standing pickup anim makes action on frame 40 in TR1-3, in TR4-5
            -- it was generalized with all rest animations by frame 16.

            if((a == 135) and (getLevelVersion() < TR_IV)) then
                if(f < 40) then
                    return true;
                end;
            else
                if(f < 16) then
                    return true;
                end;
            end;

            addItem(activator_id, item_id);
            disableEntity(object_id);
            return false;   -- Item successfully picked up, kill the task.
        end);
    end;
end


function fallblock_func(id)  -- Falling block (TR1-3)

    local f1, f2, f3 = getEntityFlags(id);  -- f1 - state flags, f2 - type flags, f3 - callback flags
    setEntityFlags(id, nil, nil, bit32.bor(f3, ENTITY_CALLBACK_STAND));
    setEntitySpeed(id, 0.0, 0.0, 0.0);
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end

    entity_funcs[id].onStand = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return;
        end

        local anim = getEntityAnim(object_id);
        if(anim == 0) then
            setEntityAnim(object_id, 1);
            -- print("you trapped to id = "..object_id);
            local once = true;
            addTask(
            function()
                local anim = getEntityAnim(object_id);
                if(anim == 1) then
                    return true;
                end;
                if(once) then
                    setEntityCollision(object_id, 0);
                    once = false;
                end;
                if(dropEntity(object_id, frame_time)) then
                    setEntityAnim(object_id, 3);
                    return false;
                end;
                return true;
            end);
        end;
    end;
end


function fallceiling_func(id)  -- Falling ceiling (TR1-3)

    setEntitySpeed(id, 0.0, 0.0, 0.0);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return;
        end

        local anim = getEntityAnim(object_id);
        if(anim == 0) then
            setEntityAnim(object_id, 1);
            local once = true;
            addTask(
            function()
                local anim = getEntityAnim(object_id);
                if(anim == 1) then
                    return true;
                end;
                if(once) then
                    setEntityCollision(object_id, 0);
                    once = false;
                end;
                if(dropEntity(object_id, frame_time)) then
                    setEntityAnim(object_id, 3);
                    return false;
                end;
                return true;
            end);
        end;
    end;
end

function pushdoor_func(id)   -- Pushdoors (TR4)

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivity(id, 0);
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return;
        end;

        if((getEntityActivity(object_id) == 0) and (getEntityDirDot(object_id, activator_id) < -0.9)) then
            setEntityActivity(object_id, 1);
            local x, y, z, az, ax, ay = getEntityPos(object_id);
            setEntityPos(activator_id, x, y, z, az + 180.0, ax, ay);
            moveEntityLocal(activator_id, 0.0, 256.0, 0.0);
            -- floor door 317 anim
            -- vertical door 412 anim
            setEntityAnim(activator_id, 412);
        end;
    end;
end


function oldspike_func(id)  -- Teeth spikes (INVALID)

    setEntityActivity(id, 1);
    local f1, f2, f3 = getEntityFlags(id);
    setEntityFlags(id, nil, bit32.bor(f2, ENTITY_TYPE_DECORATION), bit32.bor(f3, ENTITY_CALLBACK_COLLISION));
    
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end

    entity_funcs[id].onCollision = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return;
        end;
        
        if(getModelID(activator_id) == 0) then  -- Lara
        
            local s1, s2, s3 = getEntitySpeed(activator_id);
            -- check if Lara and remove her HP!!!!
            print("speed: " .. s1 .. " " .. s2 .. " " .. s3);
        
        end;
    end;
end