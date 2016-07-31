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

dofile("scripts/entity/script_switch.lua");     -- Additional switch scripts.

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

function door_init(id)   -- NORMAL doors only!

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(object_id, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        local a, f, c = getEntityAnim(object_id);
        if(c == 1) then
            return swapEntityState(object_id, 0, 1);
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;    -- Same function.
    
    entity_funcs[id].onLoop = function(object_id)
        if(getEntityEvent(object_id) == 0) then
            setEntityTimer(object_id, 0.0);
        end;

        if((tickEntity(object_id) == TICK_STOPPED) and (getEntityEvent(object_id) ~= 0)) then
            swapEntityState(object_id, 0, 1);
            setEntityEvent(object_id, 0);
        end;
    end
    
    prepareEntity(id);
end

function keyhole_init(id)    -- Key and puzzle holes

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(object_id == nil or getEntityActivity(object_id) or not canTriggerEntity(activator_id, object_id, 256.0, 0.0, 256.0, 0.0)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end
        
        if((not getEntityActivity(object_id)) and (switch_activate(object_id, activator_id) == 1)) then
            setEntityPos(activator_id, getEntityPos(object_id));
            moveEntityLocal(activator_id, 0.0, 360.0, 0.0);
            return ENTITY_TRIGGERING_ACTIVATED;
        end
        return ENTITY_TRIGGERING_NOT_READY;
    end;
end

function switch_init(id)     -- Ordinary switches
    
    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(object_id == nil or not canTriggerEntity(activator_id, object_id, 256.0, 0.0, 256.0, 0.0)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end
        
        if(switch_activate(object_id, activator_id) == 1) then
            setEntityPos(activator_id, getEntityPos(object_id));    -- Move activator right next to object.
            moveEntityLocal(activator_id, 0.0, 360.0, 0.0);         -- Shift activator back to proper distance.
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 1);
            setEntitySectorStatus(object_id, 1);
        end;
    end;
end

function anim_init(id)      -- Ordinary animatings

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    disableEntity(id);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        swapEntityState(object_id, 1, 0);
        swapEntityActivity(object_id);
        return swapEntityEnability(object_id);
    end    
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 0);
            setEntityActivity(object_id, 0);
        end;
    end
end

function pushable_init(id)
    setEntityTypeFlag(id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
end


function gen_soundsource_init(id)    -- Generic sound source (continous)
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    
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
    setEntityActivity(id, 0);
    
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


function venicebird_init(id)    -- Venice singing birds (TR2)
    randomized_soundsource_init(id);
    entity_funcs[id].chance   = 2;
    entity_funcs[id].sound_id = 316;
end


function drips_init(id)         -- Maria Doria drips (TR2)
    randomized_soundsource_init(id);
    entity_funcs[id].chance   = 5;
    entity_funcs[id].sound_id = 329;
end


function doorbell_init(id)    -- Lara's Home doorbell (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        return swapEntityActivity(object_id);
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        if(getEntityDistance(player, object_id) < 4096.0) then
            playSound(334, object_id);
            setEntityActivity(object_id, 0);
        end;
    end
end


function alarm_TR2_init(id)    -- Offshore Rig alarm (TR2)

    gen_soundsource_init(id);
    entity_funcs[id].sound_id = 332;
end

function alarmbell_init(id)    -- Home Sweet Home alarm (TR2)

    gen_soundsource_init(id);
    entity_funcs[id].sound_id = 335;
end


function heli_TR2_init(id)    -- Helicopter (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    setEntityVisibility(id, false);
    
    entity_funcs[id].distance_passed = 0;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then
            setEntityActivity(object_id, 1);
            setEntityVisibility(id, 1);
            playSound(297, object_id);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        local dy = 40.0 * 60.0 * frame_time;
        entity_funcs[object_id].distance_passed = entity_funcs[object_id].distance_passed + dy;
        moveEntityLocal(object_id, 0.0, dy, 0.0);
        if(entity_funcs[object_id].distance_passed > 30720) then
            stopSound(297, object_id);
            disableEntity(object_id);
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].distance_passed = nil;
    end
    
    prepareEntity(id);
end


function heli_rig_TR2_init(id)    -- Helicopter in Offshore Rig (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    setEntityVisibility(id, false);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        setEntityVisibility(object_id, 1);
        return ENTITY_TRIGGERING_ACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(getEntityLock(object_id)) then
            if(getEntityAnimState(object_id, ANIM_TYPE_BASE) ~= 2) then
                setEntityAnimState(object_id, ANIM_TYPE_BASE, 2);
            else
                local anim, frame, count = getEntityAnim(object_id);
                if(frame == count-1) then disableEntity(object_id) end;
            end
        end;
    end
    
    prepareEntity(id);
end


function swingblade_init(id)        -- Swinging blades (TR1)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityAnimState(object_id, ANIM_TYPE_BASE, 2);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityAnimState(object_id, ANIM_TYPE_BASE, 0);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityAnimState(object_id, ANIM_TYPE_BASE, 0) end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        changeCharacterParam(activator_id, PARAM_HEALTH, -45);
    end
    
    prepareEntity(id);
end


function tallblock_init(id)    -- Tall moving block (TR1)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    
    entity_funcs[id].distance_passed = 0;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then
            setEntityActivity(object_id, 1);
            playSound(64, object_id);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        local move_speed = 32.0 * 60.0 * frame_time;;
        if(not getEntityEvent(object_id)) then 
            move_speed = 0.0 - move_speed;
        end;
        
        entity_funcs[object_id].distance_passed = entity_funcs[object_id].distance_passed + move_speed;
        moveEntityLocal(object_id, 0.0, move_speed, 0.0);
        if(math.abs(entity_funcs[object_id].distance_passed) >= 2048.0) then
            stopSound(64, object_id);
            setEntityActivity(object_id, 0);
            entity_funcs[object_id].distance_passed = 0;
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].distance_passed = nil;
    end
    
    prepareEntity(id);
end


function gen_trap_init(id)      -- Generic traps (TR1-TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        return swapEntityState(object_id, 0, 1);
    end;
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then 
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 0) 
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityAnimState(object_id, ANIM_TYPE_BASE) == 1) then 
            changeCharacterParam(activator_id, PARAM_HEALTH, -35) 
        end;
    end
    
    prepareEntity(id);
end


function sethblade_init(id)      -- Seth blades (TR4)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        return swapEntityState(object_id, 1, 2);
    end;
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 2)
            setEntityActivity(object_id, 0);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityAnimState(object_id, ANIM_TYPE_BASE) == 1) then setCharacterParam(activator_id, PARAM_HEALTH, 0) end;
    end
    
    prepareEntity(id);
end


function slicerdicer_init(id)      -- Slicer-dicer (TR4)

    disableEntity(id);
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    
    entity_funcs[id].current_angle =  0.0;
    entity_funcs[id].speed         = -0.5;  -- Full turn in 14 seconds, clockwise.
    entity_funcs[id].radius        =  math.abs(4096.0 * frame_time * entity_funcs[id].speed);
    
    moveEntityLocal(id, 512.0, 0.0, 0.0);  -- Fix position
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then 
            enableEntity(object_id);
            return ENTITY_TRIGGERING_ACTIVATED;
        else 
            disableEntity(object_id)
            return ENTITY_TRIGGERING_DEACTIVATED;
        end;
    end;
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityActivity(object_id, 0);
        end;
        
        entity_funcs[object_id].current_angle = entity_funcs[object_id].current_angle + entity_funcs[object_id].speed;
        
        if(entity_funcs[object_id].current_angle < 0.0) then
            entity_funcs[object_id].current_angle = 360.0 + entity_funcs[object_id].current_angle;
        elseif(entity_funcs[object_id].current_angle > 360.0) then
            entity_funcs[object_id].current_angle = entity_funcs[object_id].current_angle - 360.0;
        end;
        
        moveEntityLocal(object_id, 0.0, math.cos(math.rad(entity_funcs[object_id].current_angle)) * entity_funcs[object_id].radius, -math.sin(math.rad(entity_funcs[object_id].current_angle)) * entity_funcs[object_id].radius);
        
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityActivity(object_id)) then changeCharacterParam(activator_id, PARAM_HEALTH, -35.0) end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].current_angle = nil;
        entity_funcs[object_id].speed         = nil;
        entity_funcs[object_id].radius        = nil;
    end
    
    prepareEntity(id);
end


function lasersweep_init(id)      -- Laser sweeper (TR3)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    entity_funcs[id].speed         = 1.0;
    entity_funcs[id].phase_length  = math.abs(2048.0 * frame_time * entity_funcs[id].speed);
    entity_funcs[id].current_phase = 0.0;
    
    entity_funcs[id].stopping      = false;  -- Sweeper will be stopped on the next phase end.
    entity_funcs[id].current_wait  = 0.0;    -- Wait timer.
    entity_funcs[id].max_wait      = 1.5;    -- Sweeper will wait for 1.5 seconds at both ends.
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then
            setEntityActivity(id, 1);
            return ENTITY_TRIGGERING_ACTIVATED;
        else
            entity_funcs[object_id].stopping = true;
            return ENTITY_TRIGGERING_DEACTIVATED;
        end;
    end;
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityActivity(object_id, 0);
            return;
        end;
        
        if(entity_funcs[object_id].current_wait == 0.0) then
            local next_phase = entity_funcs[object_id].current_phase + entity_funcs[object_id].speed;
            
            if(((next_phase >= 90.0) and (entity_funcs[object_id].current_phase < 90.0)) or
               ((next_phase >= 270.0) and (entity_funcs[object_id].current_phase < 270.0))) then
                entity_funcs[object_id].current_wait = frame_time;
            end;
            
            entity_funcs[object_id].current_phase = next_phase;
            
            if(entity_funcs[object_id].current_phase > 360.0) then
                entity_funcs[object_id].current_phase = entity_funcs[object_id].current_phase - 360.0;
            end;
            
            moveEntityLocal(object_id, 0.0, math.cos(math.rad(entity_funcs[object_id].current_phase)) * entity_funcs[object_id].phase_length, 0.0);
        else
            if(entity_funcs[object_id].stopping == true) then
                setEntityActivity(object_id, 0);
                entity_funcs[object_id].stopping = false;
            else
                entity_funcs[object_id].current_wait = entity_funcs[object_id].current_wait + frame_time;
                
                if(entity_funcs[object_id].current_wait > entity_funcs[object_id].max_wait) then
                    entity_funcs[object_id].current_wait = 0.0;
                end;
            end;
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityActivity(object_id)) then changeCharacterParam(activator_id, PARAM_HEALTH, -15.0) end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].current_phase = nil;
        entity_funcs[object_id].phase_length  = nil;
        entity_funcs[object_id].speed         = nil;
        entity_funcs[object_id].stopping      = nil;
        entity_funcs[object_id].current_wait  = nil;
        entity_funcs[object_id].max_wait      = nil;
    end
    
    prepareEntity(id);
end


function propeller_init(id)      -- Generic propeller (TR1-TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        return swapEntityState(object_id, 0, 1);
    end;
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityAnimState(object_id, ANIM_TYPE_BASE, 0) end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityAnimState(object_id, ANIM_TYPE_BASE) == 0) then changeCharacterParam(activator_id, PARAM_HEALTH, -100 * 60.0 * frame_time) end;
    end
    
    prepareEntity(id);
end


function wallblade_init(id)     -- Wall blade (TR1-TR3)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityActivity(object_id, 0) end;
        local anim_number = getEntityAnim(object_id);
        if(anim_number == 2) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 3, 0);
        elseif(anim_number == 1) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 0, 0);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        changeCharacterParam(activator_id, PARAM_HEALTH, -50);
    end
    
    prepareEntity(id);
end


function plough_init(id)     -- Plough (TR4)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
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


function boulder_init(id)

    setEntityTypeFlag(id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
    setEntityAnimFlag(id, ANIM_TYPE_BASE, ANIM_FRAME_LOCK);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then
            setEntityBodyMass(object_id, getEntityMeshCount(object_id), 2000.0);
            setEntityActivity(object_id, 1);
            
            if(getLevelVersion() < TR_IV) then
                pushEntityBody(object_id, 0, math.random(150) + 2500.0, 10.0, true);
                lockEntityBodyLinearFactor(object_id, 0);
            end;
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end

end


function pickup_init(id, item_id)    -- Pick-ups

    setEntityTypeFlag(id, ENTITY_TYPE_PICKABLE);
    setEntityActivationOffset(id, 0.0, 0.0, 0.0, 480.0);
    setEntityActivity(id, 0);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((item_id == nil) or (object_id == nil)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end
        
        local need_set_pos = true;
        local curr_anim = getEntityAnim(activator_id);

        if(curr_anim == 103) then                 -- Stay idle
            local dx, dy, dz = getEntityVector(object_id, activator_id);
            if(dz < -256.0) then
                need_set_pos = false;
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 425, 0); -- Standing pickup, test version
                --noFixEntityCollision(activator_id);
            else
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 135, 0); -- Stay pickup
            end;
        elseif(curr_anim == 222) then             -- Crouch idle
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 291, 0);     -- Crouch pickup
        elseif(curr_anim == 263) then             -- Crawl idle
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 292, 0);     -- Crawl pickup
        elseif(curr_anim == 108) then             -- Swim idle
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 130, 0);     -- Swim pickup
        else
            return ENTITY_TRIGGERING_NOT_READY;                      -- Disable picking up, if Lara isn't idle.
        end;

        addTask(
        function()
            -- Position corrector
            if(getEntityMoveType(activator_id) == MOVE_UNDERWATER) then
                if(getEntityDistance(object_id, activator_id) > 128.0) then
                    moveEntityToEntity(activator_id, object_id, 25.0);
                end;
            elseif(need_set_pos) then
                if(getEntityDistance(object_id, activator_id) > 32.0) then
                    moveEntityToEntity(activator_id, object_id, 50.0);
                end;
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
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
end


function crystal_TR3_init(id)   -- "Savegame" crystal (TR3 version)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 1);
    
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

        local anim = getEntityAnim(object_id);
        if(anim == 0) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 1, 0);
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
        
        local anim = getEntityAnim(object_id);
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
        if((getEntityAnim(object_id) == 1) and (getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            setCharacterParam(activator_id, PARAM_HEALTH, 0);
        end;
    end
end


function pushdoor_init(id)   -- Pushdoors (TR4)

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivity(id, 0);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end;

        if((not getEntityActivity(object_id)) and (getEntityDirDot(object_id, activator_id) < -0.9)) then
            setEntityActivity(object_id, 1);
            local x, y, z, az, ax, ay = getEntityPos(object_id);
            setEntityPos(activator_id, x, y, z, az + 180.0, ax, ay);
            moveEntityLocal(activator_id, 0.0, 256.0, 0.0);
            -- floor door 317 anim
            -- vertical door 412 anim
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 412, 0);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
end


function midastouch_init(id)    -- Midas gold touch

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    
    entity_funcs[id].onLoop = function(object_id)
        if(getEntityDistance(player, object_id) < 1024.0) then
            local lara_anim, frame, count = getEntityAnim(player);
            local lara_sector = getEntitySectorIndex(player);
            local hand_sector = getEntitySectorIndex(object_id);
            
            if((lara_sector == hand_sector) and (getEntityMoveType(player) == MOVE_ON_FLOOR) and (getEntityAnim(player) ~= 50)) then
                setCharacterParam(player, PARAM_HEALTH, 0);
                entitySSAnimEnsureExists(player, ANIM_TYPE_MISK_1, 5); --ANIM_TYPE_MISK_1 - add const
                setEntityAnim(player, ANIM_TYPE_MISK_1, 1, 0);
                entitySSAnimSetEnable(player, ANIM_TYPE_MISK_1, 1);
                entitySSAnimSetEnable(player, ANIM_TYPE_BASE, 0);
                disableEntity(object_id);
            end;
        end;
    end
    
    prepareEntity(id);
end


function twobp_init(id)        -- Two-block platform

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_STAND, 1);
    setEntityActivity(id, 0);
    
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
    
    if(curr_OCB == 0) then entity_funcs[id].mode = 2 else entity_funcs[id].mode = 0 end;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityActivity(object_id)) then
            if(entity_funcs[object_id].mode < 2) then
                if(entity_funcs[object_id].mode == 0) then entity_funcs[object_id].mode = 1 else entity_funcs[object_id].mode = 0 end;
                entity_funcs[object_id].current_height = entity_funcs[object_id].raise_height - entity_funcs[object_id].current_height;
            else
                setEntityActivity(object_id, 0);
            end;
        else
            setEntityActivity(object_id, 1);
        end;
        
        entity_funcs[object_id].waiting = false;
        return ENTITY_TRIGGERING_ACTIVATED;
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id, activator_id)
        if(entity_funcs[object_id].waiting == false) then
            if(entity_funcs[object_id].mode < 2) then
                if(entity_funcs[object_id].current_height < entity_funcs[object_id].raise_height) then
                    if(entity_funcs[object_id].mode == 0) then
                        moveEntityLocal(object_id, 0.0, 0.0, entity_funcs[object_id].raise_speed);
                    else
                        moveEntityLocal(object_id, 0.0, 0.0, -entity_funcs[object_id].raise_speed);
                    end;
                    
                    entity_funcs[object_id].current_height = entity_funcs[object_id].current_height + entity_funcs[object_id].raise_speed;
                else
                    if(entity_funcs[object_id].mode == 0) then entity_funcs[object_id].mode = 1 else entity_funcs[object_id].mode = 0 end;
                    entity_funcs[object_id].waiting = true;
                    entity_funcs[object_id].current_height = 0.0; -- Reset height counter.
                    setEntityActivity(object_id, 0);
                end;
            elseif(entity_funcs[object_id].mode == 2) then
                if(entity_funcs[object_id].push == true) then
                    if(entity_funcs[object_id].current_height < entity_funcs[object_id].push_height) then
                        moveEntityLocal(object_id, 0.0, 0.0, -entity_funcs[object_id].push_speed);
                        entity_funcs[object_id].current_height = entity_funcs[object_id].current_height + entity_funcs[object_id].push_speed;
                    else
                        entity_funcs[object_id].current_height = entity_funcs[object_id].push_height;
                    end;
                else
                    if(entity_funcs[object_id].current_height > 0.0) then
                        moveEntityLocal(object_id, 0.0, 0.0, entity_funcs[object_id].push_speed);
                        entity_funcs[object_id].current_height = entity_funcs[object_id].current_height - entity_funcs[object_id].push_speed;
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
    setEntityActivity(id, 0);
    
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
        setEntityActivity(object_id, 1);
        return ENTITY_TRIGGERING_ACTIVATED;
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id, activator_id)
        if(entity_funcs[object_id].direction == 1) then
            if((entity_funcs[object_id].dummy == false) and (entity_funcs[object_id].curr_height < entity_funcs[object_id].max_height)) then
                entity_funcs[object_id].curr_height = entity_funcs[object_id].curr_height + entity_funcs[object_id].move_speed;
                camShake(125.0, 0.2, object_id);
            else
                entity_funcs[object_id].curr_height = entity_funcs[object_id].max_height;
                entity_funcs[object_id].direction = 2;
                setEntityActivity(object_id, 0);
            end;
        else
            if((entity_funcs[object_id].dummy == false) and (entity_funcs[object_id].curr_height > 0.0)) then
                entity_funcs[object_id].curr_height = entity_funcs[object_id].curr_height - entity_funcs[object_id].move_speed;
                camShake(125.0, 0.2, object_id);
            else
                entity_funcs[object_id].curr_height = 0.0;
                entity_funcs[object_id].direction = 1;
                setEntityActivity(object_id, 0);
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
    setEntityActivity(id, 0);
    
    entity_funcs[id].max_width  = 1024.0;
    entity_funcs[id].move_speed = 8.0;
    entity_funcs[id].curr_width = 0.0;
    entity_funcs[id].direction  = 1;
    
    setEntityScaling(id, 1.0, 0.0, 1.0);
    moveEntityLocal(id, 0.0, entity_funcs[id].max_width / 2, 0.0);  -- Fix position
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        return ENTITY_TRIGGERING_ACTIVATED;
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id, activator_id)
        if(entity_funcs[object_id].direction == 1) then
            if(entity_funcs[object_id].curr_width < entity_funcs[object_id].max_width) then
                entity_funcs[object_id].curr_width = entity_funcs[object_id].curr_width + entity_funcs[object_id].move_speed;
            else
                entity_funcs[object_id].curr_width = entity_funcs[object_id].max_width;
                entity_funcs[object_id].direction = 2;
                setEntityActivity(object_id, 0);
            end;
        else
            if(entity_funcs[object_id].curr_width > 0.0) then
                entity_funcs[object_id].curr_width = entity_funcs[object_id].curr_width - entity_funcs[object_id].move_speed;
            else
                entity_funcs[object_id].curr_width = 0.0;
                entity_funcs[object_id].direction = 1;
                setEntityActivity(object_id, 0);
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


function oldspike_init(id)  -- Teeth spikes

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 1);
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            
            local px,py,pz = getEntityPos(object_id);
            local lx,ly,lz = getEntityPos(activator_id);
            local ls = getEntitySpeedLinear(activator_id);
                
            if(lz > (pz + 256.0)) then
                local sx,sy,sz = getEntitySpeed(activator_id);
                if(sz < -256.0) then
                    setEntityCollision(object_id, 0);
                    setEntityAnim(activator_id, ANIM_TYPE_BASE, 149, 0);
                    setEntityPos(activator_id, lx, ly, pz);
                    setCharacterParam(activator_id, PARAM_HEALTH, 0);
                end;
            elseif(ls > 512.0) then
                changeCharacterParam(activator_id, PARAM_HEALTH, -(ls / 512.0));
            end;
        end;
    end
end


function newspike_init(id)  -- Teeth spikes (TR4-5)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    setEntityVisibility(id, 0);
    setEntityCollision(id, 0);
    
    entity_funcs[id].interval        = 150;     -- 150 frames = 2.5 seconds
    entity_funcs[id].curr_timer      = entity_funcs[id].interval;   -- This activates spikes on first call.
    
    entity_funcs[id].curr_scaling    = 0.0;     -- Scaling is done via linear function.
    entity_funcs[id].curr_subscaling = 0.0;     -- Subscaling is done via trigonometric function.
    
    entity_funcs[id].mode            = 0;       -- Movement mode.
    entity_funcs[id].waiting         = false;   -- Non-active state flag.
    
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
    
        -- This is case for spikes which were already activated before, but were set to idle state
        -- after activation - that is, it means if we're activating it again, they should be
        -- disabled. On the next activation event, however, they will be activated again.
    
        if(entity_funcs[object_id].waiting == true) then
            entity_funcs[object_id].waiting = false;
            entity_funcs[object_id].onDeactivate(object_id, activator_id);
            return ENTITY_TRIGGERING_NOT_READY;
        end;
    
        -- For teeth spikes, OCB value stores vertical rotation in first 3 bits (0x07), horizontal
        -- rotation in fourth bit (0x08) and working mode in next two bits (0x30).

        local curr_OCB  = getEntityOCB(object_id);
        local rot_value = bit32.band(curr_OCB, 0x07);
        
        -- Spikes are vertically rotated in 45 degree step, and zero rotational value actually means
        -- that spikes are pointed downwards (e. g. 180 degrees rotation).
        
        rotateEntity(object_id, 0.0, 0.0, (180.0 + rot_value * 45.0));
        if(bit32.band(curr_OCB, 0x08) ~= 0) then 
            rotateEntity(object_id, 270.0) 
        end;   -- Rotate horizontally.
        
        -- Fix teeth spikes position for right angles (except upwards case).
        
        if(rot_value == 0) then
            moveEntityLocal(object_id, 0.0, 0.0, -1024.0);
        elseif(rot_value == 2) then
            moveEntityLocal(object_id, 512.0, 0.0, -512.0);
        elseif(rot_value == 6) then
            moveEntityLocal(object_id, -512.0, 0.0, -512.0);
        end;
        
        -- Mode 0 is normal teeth spike movement (continous pop-retract), mode 1 is sticked out
        -- constantly mode, and mode 2 is single pop-retract (as seen in Coastal Ruins shooting range).
        
        entity_funcs[object_id].mode = bit32.rshift(bit32.band(curr_OCB, 0x30), 4);
        setEntityActivity(object_id, 1);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
    
        -- This non-obvious code is needed to make teeth spike retract with animation.
        -- What it basically does is overrides spike mode with single pop-retract, and also
        -- overrides current activity state, which allows spikes to successfully retract and
        -- disable themselves. Teeth spike mode will be back to normal on the next re-activation,
        -- cause OCB parsing happens right in onActivate event.
        
        setEntityActivity(object_id, 1);
        entity_funcs[object_id].mode = 2;
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
    
        -- If spike timer is less than 60 (meaning the first second of cycle), we should initiate
        -- pop-retract phase. Pop phase is done in first 10 frames, and retract phase is done after
        -- frame 50 (e.g. 60-10 = 10 frames as well). For mode 1, we stop the phase right after
        -- spikes were popped out, and send them to idle state - so they can be deactivated later.
        local time_coef = frame_time * 60.0;
        if(entity_funcs[object_id].curr_timer < 60) then
            entity_funcs[object_id].curr_subscaling = entity_funcs[object_id].curr_subscaling + 6 * time_coef;  -- Subscaling makes full turn in 1 sec.
            if(entity_funcs[object_id].curr_timer <= 10) then
                entity_funcs[object_id].curr_scaling = entity_funcs[object_id].curr_scaling + 0.1 * time_coef;
            elseif(entity_funcs[object_id].curr_timer > 50) then
                if(entity_funcs[object_id].mode == 1) then
                    entity_funcs[object_id].waiting = true;
                    setEntityActivity(object_id, 0);
                    return;
                else
                    entity_funcs[object_id].curr_scaling = entity_funcs[object_id].curr_scaling - 0.1 * time_coef;
                end;
            end;
            
        -- If spike timer is large than 60 (rest of the cycle), we force spikes to hide
        -- completely and send them to idle state. For mode 2, we also stop whole cycle, so
        -- spikes will remain in the dormant state until next activation.
            
        elseif(entity_funcs[object_id].curr_timer < entity_funcs[object_id].interval) then
            if(entity_funcs[object_id].waiting == false) then
                entity_funcs[object_id].curr_subscaling = 0;
                entity_funcs[object_id].curr_scaling = 0.0;
                setEntityVisibility(object_id, 0);
                setEntityScaling(object_id, 1.0, 1.0, 0.0);
                setEntityCollision(object_id, 0);
                entity_funcs[object_id].waiting = true;
                if(entity_funcs[object_id].mode == 2) then
                    setEntityActivity(object_id, 0);
                    return;
                end;
            end;
            
        -- When cycle has ended, we reset idle state and timer, and force spikes to be visible
        -- and material, and also play spike sound.
            
        else
            setEntityVisibility(object_id, 1);
            setEntityCollision(object_id, 1);
            entity_funcs[object_id].curr_timer = 0;
            playSound(343, object_id);
            entity_funcs[object_id].waiting = false;
        end;
        
        -- We update entity scaling (material and visible) only if entity isn't in idle state.
        -- This way, we conserve system resources in idle cycle phase.
        
        if(entity_funcs[object_id].waiting == false) then
            setEntityScaling(object_id, 1.0, 1.0, entity_funcs[object_id].curr_scaling - math.abs(math.cos(math.rad(entity_funcs[object_id].curr_subscaling))) / 4);
        end;
        
        entity_funcs[object_id].curr_timer = entity_funcs[object_id].curr_timer + 1.0 * time_coef;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
        
            -- If Lara has collided with spikes in early phase of spike cycle (i.e. popping out),
            -- we immediately kill her and play impale animation. Also, we force spike mode to 1,
            -- so that particular spikes won't retract back, or else it'll look unrealistic with
            -- Lara being "impaled" into thin air.
            
            if(entity_funcs[object_id].curr_timer <= 10) then
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 149, 0);
                setCharacterParam(activator_id, PARAM_HEALTH, 0);
                entity_funcs[object_id].mode = 1;
                return;
            end;
            
            -- Normal spikes behaviour - only difference is we're not playing impale animation if
            -- Lara is approaching from the top, but create ragdoll instead - it is needed to
            -- prevent aforementioned problem, but in this case Lara could be impaled in the
            -- retraction phase, which will look unrealistic as well.
            
            local px,py,pz = getEntityPos(object_id);
            local lx,ly,lz = getEntityPos(activator_id);
            local ls = getEntitySpeedLinear(activator_id);
                
            if(lz > (pz + 256.0)) then
                local sx,sy,sz = getEntitySpeed(activator_id);
                if(sz < -256.0) then
                    addEntityRagdoll(activator_id, RD_TYPE_LARA);
                    setCharacterParam(activator_id, PARAM_HEALTH, 0);
                    playSound(SOUND_IMPALE, activator_id);
                end;
            elseif(ls > 512.0) then
                changeCharacterParam(activator_id, PARAM_HEALTH, -(ls / 512.0));
            else
                changeCharacterParam(activator_id, PARAM_HEALTH, -4);
            end;
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].interval        = nil;
        entity_funcs[object_id].curr_timer      = nil;
        entity_funcs[object_id].curr_scaling    = nil;
        entity_funcs[object_id].curr_subscaling = nil;
        entity_funcs[object_id].waiting         = nil;
        entity_funcs[object_id].mode            = nil;
    end
    
    prepareEntity(id);
end


function spikewall_init(id)      -- Spike wall

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
        stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        local ver = getLevelVersion();
        local scan_distance = 32.0;
        if(ver < TR_II) then scan_distance = 1536.0 end; -- TR1's lava mass has different floor scan distance.
        
        if(similarSector(object_id, 0.0, scan_distance, 0.0, false)) then
            moveEntityLocal(object_id, 0.0, 8.0 * 60.0 * frame_time, 0.0);
            playSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        else
            setEntityActivity(object_id, 0);    -- Stop
            stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityModelID(activator_id) == 0) then
            local curr_st = getEntityMoveType(activator_id);
            if((curr_st == MOVE_CLIMBING) or (curr_st == MOVE_MONKEYSWING)) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 28, 0);
                playSound(SOUND_LARAINJURY, activator_id);
            elseif(curr_st == MOVE_WALLS_CLIMB) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 30, 0);
                playSound(SOUND_LARAINJURY, activator_id);
            end;
            
            if(getCharacterParam(activator_id, PARAM_HEALTH) > 0) then
                changeCharacterParam(activator_id, PARAM_HEALTH, -20);
                playSound(getGlobalSound(getLevelVersion(), GLOBALID_SPIKEHIT), activator_id);
                if(getCharacterParam(activator_id, PARAM_HEALTH) <= 0) then
                    addEntityRagdoll(activator_id, RD_TYPE_LARA);
                    playSound(SOUND_GEN_DEATH, activator_id);
                    setEntityActivity(object_id, 0);
                    stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
                end;
            end;
        end;
    end
end


function spikeceiling_init(id)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
        stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        local px, py, pz = getEntityPos(object_id);

        if(pz > (getSectorHeight(object_id) + 512.0)) then
            moveEntityLocal(object_id, 0.0, 0.0, -4.0 * 60.0 * frame_time);
            playSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        else
            setEntityActivity(object_id, 0);    -- Stop
            stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityModelID(activator_id) == 0) then
            local curr_st = getEntityMoveType(activator_id);
            if((curr_st == MOVE_CLIMBING) or (curr_st == MOVE_MONKEYSWING)) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 28, 0);
                playSound(SOUND_LARAINJURY, activator_id);
            elseif(curr_st == MOVE_WALLS_CLIMB) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 30, 0);
                playSound(SOUND_LARAINJURY, activator_id);
            end;
        
            local px,py,pz = getEntityPos(object_id);
            local lx,ly,lz = getEntityPos(activator_id);
                
            if(getCharacterParam(activator_id, PARAM_HEALTH) > 0) then
                if(lz > (pz - 256.0)) then
                    addEntityRagdoll(activator_id, RD_TYPE_LARA);
                    setCharacterParam(activator_id, PARAM_HEALTH, 0);
                    playSound(SOUND_IMPALE, activator_id);
                else
                    changeCharacterParam(activator_id, PARAM_HEALTH, -20);
                    playSound(getGlobalSound(getLevelVersion(), GLOBALID_SPIKEHIT), activator_id);
                    if(getCharacterParam(activator_id, PARAM_HEALTH) <= 0) then
                        addEntityRagdoll(activator_id, RD_TYPE_LARA);
                        playSound(SOUND_GEN_DEATH, activator_id);
                        setEntityActivity(object_id, 0);
                        stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
                    end;
                end;
            end;
        end;
    end
end


function cleaner_init(id)      -- Thames Wharf machine (aka cleaner)

    disableEntity(id);
    
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    
    -- Initialize specific cleaner parameters.
    
    entity_funcs[id].rotating = 0;                  -- Specifies if cleaner is in rotating phase or not.
    entity_funcs[id].distance_traveled = 1024.0;    -- Each 1024 units, cleaner checks floor state.
                                                    -- Must be 1024 by default to process initial floor state correctly!
    entity_funcs[id].current_angle = 0.0;           -- Current cleaner rotating angle.
    entity_funcs[id].target_angle  = 0.0;           -- Desired angle value, after which cleaner stops rotating.
    
    entity_funcs[id].turn_rot_speed  = 2.0;         -- Rotation speed for free left-turn.
    entity_funcs[id].stuck_rot_speed = 2.0;         -- Rotation speed for face-wall right turn.
    entity_funcs[id].move_speed      = 32.0;        -- Ordinary movement speed.
    
    entity_funcs[id].loop_detector = {};            -- Loop detector struct needed to prevent cleaner stuck at flat floor.
    
    entity_funcs[id].loop_detector[1] = {x = 0.0, y = 0.0};
    entity_funcs[id].loop_detector[2] = {x = 0.0, y = 0.0};
    entity_funcs[id].loop_detector[3] = {x = 0.0, y = 0.0};
    entity_funcs[id].loop_detector[4] = {x = 0.0, y = 0.0};
    
    entity_funcs[id].move_count = 1;                -- Needed to fill loop detector struct each 4 moves.
    entity_funcs[id].loop_count = 0;                -- If loop count reaches a value of 4, cleaner will turn right, not left.
    
    entity_funcs[id].dead = 0;                      -- Needed to process fuse box collision correctly.
    
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then 
            enableEntity(object_id) 
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        playSound(131, object_id);
        stopSound(191, object_id);
        setEntityActivity(object_id, 0);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        local px,py,pz,ax = getEntityPos(object_id);
        playSound(191, object_id);
        
        if(entity_funcs[object_id].rotating == 0) then
            if(entity_funcs[object_id].distance_traveled >= 1024.0) then
                if(similarSector(object_id, -1024.0, 0.0, 0.0, false)) then  -- Rotate left
                    entity_funcs[object_id].rotating = 1;
                    entity_funcs[object_id].current_angle = 0.0;    -- DO NOT CHANGE!!!
                    entity_funcs[object_id].target_angle = 90.0;    -- Change if you want different "free" turning angle.
                    px = (math.floor((px / 512.0) + 0.5)) * 512.0;  -- round position
                    py = (math.floor((py / 512.0) + 0.5)) * 512.0;  -- round position
                    setEntityPos(object_id, px,py,pz,ax,ay,az);
                    entity_funcs[object_id].distance_traveled = 0.0;
                elseif((entity_funcs[object_id].loop_count > 3) or
                   (not similarSector(object_id, 0.0, 520.0, 0.0, false))) then   -- Rotate right
                    entity_funcs[object_id].loop_count = 0;
                    entity_funcs[object_id].rotating = 2;
                    entity_funcs[object_id].current_angle = 0.0;    -- DO NOT CHANGE!!!
                    entity_funcs[object_id].target_angle = 90.0;    -- Change if you want different "stuck" turning angle.
                    px = (math.floor((px / 512.0) + 0.5)) * 512.0;  -- round position
                    py = (math.floor((py / 512.0) + 0.5)) * 512.0;  -- round position
                    setEntityPos(object_id, px,py,pz,ax,ay,az);
                end;
                
                -- Loop detector algorithm: each turn cleaner stores its current position in an array of 4.
                -- If current array unit is equal to current position, it will increase loop counter, otherwise
                -- loop counter is reset back to zero. When loop counter reaches 4, robot is forced to turn right,
                -- not left. This prevents robot from being stuck at 2x2 flat floor section.
                
                if(entity_funcs[object_id].rotating == 0) then
                    entity_funcs[object_id].distance_traveled = 0.0;
                    moveEntityLocal(object_id, 0.0, entity_funcs[object_id].move_speed, 0.0);  -- Move forward...
                    entity_funcs[object_id].distance_traveled = entity_funcs[object_id].distance_traveled + entity_funcs[object_id].move_speed;
                end;
                
                if((entity_funcs[object_id].loop_detector[(entity_funcs[object_id].move_count)].x == px) and
                   (entity_funcs[object_id].loop_detector[(entity_funcs[object_id].move_count)].y == py)) then
                    entity_funcs[object_id].loop_count = entity_funcs[object_id].loop_count + 1;
                else
                    entity_funcs[object_id].loop_count = 0;
                end;
                
                entity_funcs[object_id].loop_detector[(entity_funcs[object_id].move_count)].x = px;
                entity_funcs[object_id].loop_detector[(entity_funcs[object_id].move_count)].y = py;
                 
                entity_funcs[object_id].move_count = entity_funcs[object_id].move_count + 1;
                if(entity_funcs[object_id].move_count > 4) then entity_funcs[object_id].move_count = 1 end;
            else
                moveEntityLocal(object_id, 0.0, entity_funcs[object_id].move_speed, 0.0);  -- Move forward...
                entity_funcs[object_id].distance_traveled = entity_funcs[object_id].distance_traveled + entity_funcs[object_id].move_speed;
            end;
        else            
            if(entity_funcs[object_id].rotating == 1) then
                entity_funcs[object_id].current_angle = entity_funcs[object_id].current_angle + entity_funcs[object_id].turn_rot_speed;
                rotateEntity(object_id, entity_funcs[object_id].turn_rot_speed);
            elseif(entity_funcs[object_id].rotating == 2) then
                entity_funcs[object_id].current_angle = entity_funcs[object_id].current_angle + entity_funcs[object_id].stuck_rot_speed;
                rotateEntity(object_id, -entity_funcs[object_id].stuck_rot_speed); -- another direction!
            end;
            
            if(entity_funcs[object_id].current_angle > entity_funcs[object_id].target_angle) then
                ax = (math.floor((ax / 90.0) + 0.5)) * 90.0;        -- round angle
                setEntityPos(object_id, px,py,pz,ax,ay,az);
                entity_funcs[object_id].rotating = 0;
                entity_funcs[object_id].current_angle = 0.0;
                entity_funcs[object_id].target_angle = 0.0;
                entity_funcs[object_id].rotating = 0;
            end;
        end;
    end;
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityActivity(object_id)) then
            a_model = getEntityModelID(activator_id);
            if((a_model == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then  -- Lara
                setEntityActivity(object_id, 0);
                addEntityRagdoll(activator_id, RD_TYPE_LARA);
                setCharacterParam(activator_id, PARAM_HEALTH, 0);
                stopSound(191, object_id);
                playSound(SOUND_GEN_DEATH, activator_id);
                playSound(127, activator_id);
                playSound(131, object_id);
            elseif(a_model == 354) then -- Fuse box - DOESN'T WORK FOR A MOMENT! (no collision callback from fusebox).
                if(not entity_funcs[object_id].dead) then
                    setEntityTypeFlag(object_id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR, 1);
                    stopSound(191, object_id);
                    entity_funcs[object_id].dead = 1;
                elseif(entity_funcs[object_id].dead) then
                    setEntityActivity(object_id, 0);
                    playSound(131, object_id);
                    setEntityCallbackFlag(object_id, ENTITY_CALLBACK_COLLISION, 0);
                end;
            end;
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].rotating            = nil;
        entity_funcs[object_id].distance_traveled   = nil;
        entity_funcs[object_id].current_angle       = nil;
        entity_funcs[object_id].target_angle        = nil;
        entity_funcs[object_id].turn_rot_speed      = nil;
        entity_funcs[object_id].stuck_rot_speed     = nil;
        entity_funcs[object_id].move_speed          = nil;
        entity_funcs[object_id].move_count          = nil;
        entity_funcs[object_id].loop_count          = nil;
        entity_funcs[object_id].dead                = nil;
        
        for k,v in pairs(entity_funcs[object_id].loop_detector) do
            entity_funcs[object_id].loop_detector[k].x = nil;
            entity_funcs[object_id].loop_detector[k].y = nil;
            entity_funcs[object_id].loop_detector[k]   = nil;
        end;
        
        entity_funcs[object_id].loop_detector       = nil;
    end
end

function damocles_init(id)      -- Sword of Damocles

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        entity_funcs[id].rot_speed = ((math.random(20) - 10) / 5) + 1;
        entity_funcs[id].falling = false;
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
        entity_funcs[id].rot_speed = 0.0;
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        rotateEntity(object_id, entity_funcs[object_id].rot_speed);
        
        if(sameRoom(player, object_id)) then
            local dx,dy,dz = getEntityVector(player, object_id);
            if((math.abs(dx) < 1024.0) and (math.abs(dy) < 1024.0) and (entity_funcs[object_id].falling == false)) then
                entity_funcs[object_id].falling = true;
                addTask(
                function()
                    moveEntityToEntity(object_id, player, 32.0, true);
                    if(dropEntity(object_id, frame_time, true)) then
                        playSound(103, object_id);
                        setEntityActivity(object_id, 0);
                        entity_funcs[object_id].falling = false;
                        return false;
                    end;
                    return true;
                end);
            end;
        end
    end    
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((entity_funcs[object_id].falling == true) and (getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            setCharacterParam(activator_id, PARAM_HEALTH, 0);
            playSound(SOUND_GEN_DEATH, activator_id);
            playSound(103, object_id);
            addEntityRagdoll(activator_id, RD_TYPE_LARA);
            setEntityActivity(object_id, 0);
            setEntityBodyMass(object_id, 1, 15.0);
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].rot_speed = nil;
        entity_funcs[object_id].falling = nil;
    end
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
