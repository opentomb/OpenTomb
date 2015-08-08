-- OPENTOMB STATIC PROPERTY SCRIPT
-- By Lwmte, Apr 2015

--------------------------------------------------------------------------------
-- This script defines static mesh property table, as well as functions related
-- to this table processing.
-- Please note that static mesh properties may vary from level to level, so
-- static table is frequently redefined on a per-level basis.


--------------------------------------------------------------------------------

static_tbl = {};    -- Define static mesh property table.

-- Get static mesh flags from property table.

function getStaticMeshProperties(id)
    if((static_tbl == nil) or (static_tbl[id] == nil)) then
        return -1;
    else
        local coll, shape, hide;
        if(static_tbl[id].hide ~= nil) then hide = static_tbl[id].hide else hide = false end;
        if(static_tbl[id].coll ~= nil) then coll = static_tbl[id].coll else coll = COLLISION_TYPE_NONE end;
        if(static_tbl[id].shape ~= nil) then shape = static_tbl[id].shape else shape = COLLISION_SHAPE_BOX end;
        return coll, shape, hide;
    end;
end;
