-- OPENTOMB STATIC PROPERTY SCRIPT
-- By Lwmte, Apr 2015

--------------------------------------------------------------------------------
-- This script defines static mesh property table, as well as functions related
-- to this table processing.
-- Please note that static mesh properties may vary from level to level, so
-- static table is frequently redefined on a per-level basis.


-- [ coll ] flag values:

-- [ coll ] flag values:
COLLISION_TYPE_NONE                    = 0x0000;
COLLISION_TYPE_STATIC                  = 0x0001;     -- static object - never moved
COLLISION_TYPE_KINEMATIC               = 0x0003;     -- doors and other moveable statics
COLLISION_TYPE_DYNAMIC                 = 0x0005;     -- hellow full physics interaction
COLLISION_TYPE_ACTOR                   = 0x0007;     -- actor, enemies, NPC, animals
COLLISION_TYPE_VEHICLE                 = 0x0009;     -- car, moto, bike
COLLISION_TYPE_GHOST                   = 0x000B;     -- no fix character position, but works in collision callbacks and interacts with dynamic objects

-- [ shape ] flag values:
COLLISION_SHAPE_BOX                    = 0x0001;
COLLISION_SHAPE_BOX_BASE               = 0x0002;
COLLISION_SHAPE_SPHERE                 = 0x0003;
COLLISION_SHAPE_TRIMESH                = 0x0004;
COLLISION_SHAPE_TRIMESH_CONVEX         = 0x0005;
--------------------------------------------------------------------------------

static_tbl = {};    -- Define static mesh property table.

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