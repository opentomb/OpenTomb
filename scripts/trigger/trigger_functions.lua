-- OPENTOMB TRIGGER FUNCTION SCRIPT
-- by Lwmte, April 2015

--------------------------------------------------------------------------------
-- This file contains core trigger routines which are used to initialize, run
-- and do other actions related to trigger array.
-- Trigger array itself is generated on the fly from each level file and is not
-- visible for user. You can turn on debug output of trigger array via config
-- command "system->output_triggers = 1".
--------------------------------------------------------------------------------

dofile("scripts/trigger/flipeffects.lua")  -- Initialize flipeffects.

trigger_list = {};  -- Initialize trigger array.

-- Run trigger. Called when desired entity is on trigger sector.

function tlist_RunTrigger(index, activator_type, activator)
    if((trigger_list[index] ~= nil) and (trigger_list[index].func ~= nil) and (trigger_list[index].activator_type == activator_type)) then
        return trigger_list[index].func(activator);
    else
        return 0;
    end;
end;

-- Erase single trigger.

function tlist_EraseTrigger(index)
    if(trigger_list[index] ~= nil) then
        trigger_list[index].func = nil;
        trigger_list[index].activator_type = nil;
        trigger_list[index] = nil;
    end;
end;

-- Clear whole trigger array. Must be called on each level loading.

function tlist_Clear()
    for k,v in pairs(trigger_list) do
        tlist_EraseTrigger(k);
    end;
    print("Trigger table cleaned");
end;


--------------------------------------------------------------------------------
-- Next goes implementation of all possible trigger functions which are called
-- from trigger array elements.
--------------------------------------------------------------------------------

-- Tries to activate entity.

function activateEntity(object_id, activator_id, trigger_mask, trigger_op, trigger_lock, trigger_timer)

    -- Get current trigger layout.
    
    local mask, event, lock = getEntityTriggerLayout(object_id);
    
    -- TR2+ ONLY: If mask was already set and lock flag is set, bypass activation.
    
    if((getLevelVersion() >= TR_II) and (lock == true) and (mask == 0x1F)) then return end;
    
    -- Apply trigger mask to entity mask.

    if(trigger_op == TRIGGER_OP_XOR) then
        mask = bit32.bxor(mask, trigger_mask);   -- Switch cases
    else
        mask = bit32.bor(mask, trigger_mask);    -- Other cases
    end;
    
    -- Apply trigger lock to entity lock.
    
    lock = (lock or trigger_lock);
    
    -- Full entity mask (11111) is always a reason to activate an entity.
    -- If mask is not full, entity won't activate - no exclusions.
        
    if((mask == 0x1F) and (not event)) then
        execEntity(ENTITY_CALLBACK_ACTIVATE, object_id, activator_id);
        event = true;
    elseif((mask ~= 0x1F) and event) then
        execEntity(ENTITY_CALLBACK_DEACTIVATE, object_id, activator_id);
        event = false;
    end;
    
    -- Update trigger layout.
    
    setEntityTriggerLayout(object_id, mask, event, lock);
    setEntityTimer(object_id, trigger_timer);                   -- Engage timer.
end;


-- Tries to deactivate entity. Doesn't work with certain kinds of entities (like enemies).

function deactivateEntity(object_id, activator_id, trigger_lock)

    -- Get current trigger layout.
    
    local mask, event, lock = getEntityTriggerLayout(object_id);
    
    -- TR2+ ONLY: It seems that antitrigger event DOES NOT check lock state - however, it copies
    -- its trigger lock flag to entity lock flag, so next activation event will be executed once.
    -- In TR1, antitriggers are ignored for locked entities.
    
    if((getLevelVersion() < TR_II) and (lock)) then return end;
    
    lock = (lock or trigger_lock);
    
    -- Execute entity deactivation function, only if activation was previously set.
    
    if(event) then
        execEntity(ENTITY_CALLBACK_DEACTIVATE, object_id, activator_id);
        
        -- Activation mask and timer are forced to zero when entity is deactivated.
        -- Activity lock is ignored, since it can't be raised by antitriggers.

        setEntityTriggerLayout(object_id, 0x00, false, lock);
        setEntityTimer(object_id, 0.0);
    end;    
end


-- Set current camera.

function setCamera(camera_index, timer, once, zoom)
    print("CAMERA: index = " .. camera_index .. ", timer = " .. timer .. ", once = " .. once .. ", zoom = " .. zoom);
end


-- Moves desired entity to specified sink.

function moveToSink(entity_index, sink_index)
    local movetype = getEntityMoveType(entity_index);
    if(movetype == 5) then  -- Dive, if on water.
        if(getEntityAnim(entity_index) ~= 113) then
            setEntityAnim(entity_index, 113);
            setEntityMoveType(entity_index, 6);
        end;
    elseif(movetype == 6) then
        moveEntityToSink(entity_index, sink_index);
    end;
end


-- Specifies an entity to look at (with player camera)

function setCamTarget(entity_index, timer)
    print("CAMERA TARGET: index = " .. entity_index .. ", timer = " .. timer);
end


-- Does specified flipeffect.

function doEffect(effect_index, extra_parameter) -- extra parameter is usually the timer field
    if(flipeffects[effect_index] ~= nil) then
        return flipeffects[effect_index](parameter);
    else
        return nil; -- Add hardcoded flipeffect routine here
    end;
end


-- Sets specified secret index as found and plays audiotrack with pop-up notification.

function findSecret(secret_number)
    if(not getSecretStatus(secret_number)) then
        setSecretStatus(secret_number, true);  -- Set actual secret status
        playStream(getSecretTrackNumber(getLevelVersion()));   -- Play audiotrack
        --showNotify("You have found a secret!", NOTIFY_ACHIEVEMENT);
    end;
end


-- Clear dead enemies, if they have CLEAR BODY flag specified.

function clearBodies()
    print("CLEAR BODIES");
end


-- Plays specified flyby. Only valid in TR4-5.

function playFlyby(flyby_index, once)
    if(getLevelVersion() < TR_IV) then return 0 end;
    print("FLYBY: index = " .. flyby_index .. " once = " .. once);
end


-- Plays specified cutscene. Only valid in retail TR4-5.

function playCutscene(cutscene_index)
    if(getLevelVersion() < TR_IV) then return 0 end;
    print("CUTSCENE: index = " .. cutscene_index);
end
