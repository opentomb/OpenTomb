-- OPENTOMB STATIC PROPERTY SCRIPT
-- By Lwmte, Apr 2015

--------------------------------------------------------------------------------
-- This script defines static mesh property table, as well as functions related
-- to this table processing.
-- Please note that static mesh properties may vary from level to level, so
-- static table is frequently redefined on a per-level basis.
--------------------------------------------------------------------------------

static_tbl = {};    -- Define static mesh property table.

-- Clear whole static mesh property table. Must be called on each level loading.

function st_Clear()
    for k,v in pairs(static_tbl) do
        static_tbl[k].coll  = nil;
        static_tbl[k].hide  = nil;
        static_tbl[k].shape = nil;
        static_tbl[k] = nil;
    end;
    print("Static mesh table cleaned");
end;

-- Get static mesh flags from property table.

function getStaticMeshProperties(id)
    if((static_tbl == nil) or (static_tbl[id] == nil)) then
        return COLLISION_TYPE_STATIC, COLLISION_SHAPE_BOX, nil;
    else
        local coll, shape, hide;
        if(static_tbl[id].hide ~= nil) then hide = static_tbl[id].hide else hide = 0 end;
        if(static_tbl[id].coll ~= nil) then coll = static_tbl[id].coll else coll = COLLISION_TYPE_NONE end;
        if(static_tbl[id].shape ~= nil) then shape = static_tbl[id].shape else shape = COLLISION_SHAPE_BOX end;
        return coll, shape, hide;
    end;
end;