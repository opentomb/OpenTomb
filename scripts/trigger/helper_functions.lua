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
    local current_state = getEntityState(object_id);
    if(current_state == state_1) then current_state = state_2 else current_state = state_1 end;
    setEntityState(object_id, current_state);
end;

function swapEntityEvent(object_id)
    local current_event = getEntityEvent(object_id);
    setEntityEvent(object_id, not current_event);
end;
