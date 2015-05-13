-- OPENTOMB STATIC MESH SCRIPT
-- By Lwmte, Apr 2015

--------------------------------------------------------------------------------
-- This script defines static mesh property table, as well as functions related
-- to this table processing.
-- Please note that static mesh properties may vary from level to level, so
-- static table is frequently redefined on a per-level basis.

-- [ hide ] flag values:

--   0x00    - Object is visible.
--   0x01    - Object is invisible.

-- [ coll ] flag values:

COLL_NONE = 0x00;  -- Object has no collisions
COLL_MESH = 0x01;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x02;  -- Object uses bounding box for collision.

--------------------------------------------------------------------------------

static_tbl = {};    -- Define static mesh property table.

-- Get static mesh flags from property table.

function getStaticMeshFlags(id)
    if((static_tbl == nil) or (static_tbl[id] == nil)) then
        return nil, nil;
    else
        local coll, hide;
        if(static_tbl[id].hide ~= nil) then hide = static_tbl[id].hide else hide = 0 end;
        if(static_tbl[id].coll ~= nil) then coll = static_tbl[id].coll else coll = COLL_NONE end;
        return coll, hide;
    end;
end;
