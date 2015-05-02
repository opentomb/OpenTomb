-- OPENTOMB ADDITIONAL DOOR SCRIPT
-- By TeslaRus, Lwmte 2014

--------------------------------------------------------------------------------
-- This script sets the behaviour of door items across all different TR versions.
-- Currently, door behaviour is hardcoded, but later it will be assigned via
-- item typization.
--------------------------------------------------------------------------------

tr_doors = {};  -- Overridden door array, if needed (currently not used).

-- op - open animation number, cl - close animation number
-- tr_doors[<door_id>] = {op = 3, cl = 1};
-- tr_doors[59] = {op = 2, cl = 1};

--------------------------------------------------------------------------------
function door_activate(object_id)
    local m_id = getModelID(object_id);
    
    if(m_id == nil or m_id < 0) then
        return;
    end
    
    local state = 0;
    setEntityActivity(object_id, 1);
    
    -- Open door, if mask is full.
    if(getEntityActivationMask(object_id) == 0x1F) then state = 1 end;
    
    if(tr_doors[m_id] ~= nil) then
        if((state == 1) and (getEntityActivityLock(object_id) ~= 1)) then
            setEntityAnim(object_id, tr_doors[m_id].op, 0);
        else   -- Locked doors can be closed, but not opened again!
            setEntityAnim(object_id, tr_doors[m_id].cl, 0);
        end
    else
        if(getEntityActivityLock(object_id) ~= 1) then
            setEntityState(object_id, state);
        end
    end
end

