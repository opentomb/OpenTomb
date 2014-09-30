-- OPENTOMB DOOR ENTITY SCRIPT
-- By TeslaRus, Lwmte 2014

--------------------------------------------------------------------------------
-- This script sets the behaviour of door items across all different TR versions.
-- Currently, door behaviour is hardcoded, but later it will be assigned via
-- item typization.
--------------------------------------------------------------------------------

tr_doors = {};

-- op - open animation number, cl - close animation number
-- tr_doors[<door_id>] = {op = 3, cl = 1};
-- tr_doors[59] = {op = 2, cl = 1};

--------------------------------------------------------------------------------
function door_activate(id, mask)
    local m_id = getModelID(id);
    
    local door_mask = getEntityActivationMask(id);
    door_mask = bit32.bxor(door_mask, mask);
    setEntityActivationMask(id, door_mask);
    
    if(door_mask ~= 0x1F) then
        state = 0;    -- Close the door in any case, if mask is not full.
    else
        state = 1;    -- Open door, if mask is full.
    end
    
    if(m_id == nil or m_id < 0) then
        return;
    end
    
    local overriden = false;
    local op = 3;
    local cl = 1;

    if(tr_doors[m_id] ~= nil) then
        op = tr_doors[m_id].op;
        cl = tr_doors[m_id].cl;
        overriden = true;
    end

    if(overriden) then
        if(state == 1) then
            setEntityAnim(id, op, 0);
            setEntityActivity(id, 1);
        else
            setEntityAnim(id, cl, 0);
            setEntityActivity(id, 1);
        end
    else
        setEntityState(id, state);
    end
end

