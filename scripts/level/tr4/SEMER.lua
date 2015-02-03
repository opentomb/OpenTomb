COLL_NONE = 0x00;  -- Object has no collisions
COLL_MESH = 0x01;  -- Object uses real mesh data for collision.
COLL_BBOX = 0x02;  -- Object uses bounding box for collision.

-- STATIC COLLISION FLAGS ------------------------------------------------------
--------------------------------------------------------------------------------

static_tbl = {};

-- PLANT statics (as listed in OBJECTS.H from TRLE)

static_tbl[11] = {coll = COLL_NONE};            -- Stairs


function trGetStaticMeshFlags(ver, id)
    if(static_tbl[id] == nil) then
        return nil, nil;
    else
        return static_tbl[id].coll, static_tbl[id].hide;
    end;
end;
