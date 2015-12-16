-- OPENTOMB STATIC PROPERTY SCRIPT
-- By Lwmte, Apr 2015

--------------------------------------------------------------------------------
-- This script defines static mesh property table, as well as functions related
-- to this table processing.
-- Please note that static mesh properties may vary from level to level, so
-- static table is frequently redefined on a per-level basis.


-- [ coll ] flag values:

-- [ coll ] flag values:
-- COLLISION_TYPE_NONE
-- COLLISION_TYPE_STATIC                  -- static object - never moved
-- COLLISION_TYPE_KINEMATIC               -- doors and other moveable statics
-- COLLISION_TYPE_DYNAMIC                 -- hellow full physics interaction
-- COLLISION_TYPE_ACTOR                   -- actor, enemies, NPC, animals
-- COLLISION_TYPE_VEHICLE                 -- car, moto, bike
-- COLLISION_TYPE_GHOST                   -- no fix character position, but works in collision callbacks and interacts with dynamic objects

-- [ shape ] flag values:
-- COLLISION_SHAPE_BOX
-- COLLISION_SHAPE_BOX_BASE
-- COLLISION_SHAPE_SPHERE
-- COLLISION_SHAPE_TRIMESH
-- COLLISION_SHAPE_TRIMESH_CONVEX
-- COLLISION_SHAPE_SINGLE_BOX
-- COLLISION_SHAPE_SINGLE_SPHERE
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