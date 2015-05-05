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

function activateEntity(object_id, activator_id, trigger_mask, trigger_op, object_lock, object_timer)

    -- Get current entity activity lock.
    
    local current_lock = getEntityActivityLock(object_id);
    if(current_lock ~= 0) then return end;   -- No action if object is locked.
    
    -- Apply trigger mask to entity mask.

    local object_mask = getEntityActivationMask(object_id);
    if(trigger_op == 1) then
        object_mask = bit32.bxor(object_mask, trigger_mask);   -- Switch cases
    else
        object_mask = bit32.bor(object_mask, trigger_mask);    -- Other cases
    end;

    setEntityActivationMask(object_id, object_mask);
    
    -- Full entity mask (11111) is always a reason to activate an entity.
    -- If mask is not full, entity won't activate - no exclusions.
    
    if(object_mask == 0x1F) then
        setEntityTimer(object_id, object_timer);    -- Engage timer.
        execEntity(object_id, activator_id, ENTITY_CALLBACK_ACTIVATE);
        setEntityActivityLock(object_id, bit32.bor(current_lock, object_lock));
    else
        setEntityTimer(object_id, 0.0);             -- Reset timer.
        execEntity(object_id, activator_id, ENTITY_CALLBACK_DEACTIVATE);
    end;
end;


-- Tries to deactivate entity. Doesn't work with certain kinds of entities (like enemies).

function deactivateEntity(object_id, activator_id)
    
    -- Activation mask and timer are forced to zero when entity is deactivated.
    -- Activity lock is ignored, since it can't be raised by antitriggers.
    
    setEntityActivationMask(object_id, 0x00);
    setEntityTimer(object_id, 0.0);
    
    -- Execute entity deactivation function.
    if(getEntityActivity(object_id) == 1) then
        execEntity(object_id, activator_id, ENTITY_CALLBACK_DEACTIVATE);
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
    if(getSecretStatus(secret_number) == 0) then
        setSecretStatus(secret_number, 1);  -- Set actual secret status
        playStream(getSecretTrackNumber(getLevelVersion()));   -- Play audiotrack
        --showNotify("You have found a secret!", NOTIFY_ACHIEVEMENT);
    end;
end


-- WHAT IS THIS?

function setBodybag(bodybag_index)
    print("BODYBAG: index = " .. bodybag_index);
end


-- Plays specified flyby. Only valid in TR4-5.

function playFlyby(flyby_index)
    if(getLevelVersion() < TR_IV) then return 0 end;
    print("FLYBY: index = " .. flyby_index);
end


-- Plays specified cutscene. Only valid in retail TR4-5.

function playCutscene(cutscene_index)
    if(getLevelVersion() < TR_IV) then return 0 end;
    print("CUTSCENE: index = " .. cutscene_index);
end


-- Special template which is called for specific entity types at level start-up.

function prepareEntity(object_id)
    activateEntity(object_id, 0, 0, 0, 0, 0);
end;