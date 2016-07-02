-- OPENTOMB HELPER TRIGGER FUNCTION SCRIPT
-- by Lwmte, May 2015

--------------------------------------------------------------------------------
-- This script contains various helper functions which are used in entity
-- script.
--------------------------------------------------------------------------------

-- Template which is called for specific entity types at level start-up.

function prepareEntity(object_id)
    local object_mask = getEntityMask(object_id);
    if(object_mask == 0x1F) then
        activateEntity(object_id, 0, 0, 0, 0, 0);
        setEntityTriggerLayout(object_id, 0x00, 0, 0); -- Reset activation mask and event.
    end;
end;

function swapEntityState(object_id, state_1, state_2)
    local current_state = getEntityAnimState(object_id, ANIM_TYPE_BASE);
    if(current_state == state_1) then 
        setEntityAnimState(object_id, ANIM_TYPE_BASE, state_2); 
    elseif(current_state == state_2) then
        setEntityAnimState(object_id, ANIM_TYPE_BASE, state_1);
    end;
end;

function swapEntityEvent(object_id)
    local current_event = getEntityEvent(object_id);
    setEntityEvent(object_id, bit32.bnot(current_event));
    --setEntityEvent(object_id, not current_event);
end;

function swapEntityActivity(object_id)
    local current_activity = getEntityActivity(object_id);
    setEntityActivity(object_id, not current_activity);
end;

function swapEntityEnability(object_id)
    local current_enability = getEntityEnability(object_id);
    if(current_enability) then 
        disableEntity(object_id);
    else 
        enableEntity(object_id);
    end;
end;
