-- OPENTOMB STATIC PROPERTY SCRIPT
-- By Lwmte, Apr 2015

--------------------------------------------------------------------------------
-- This script defines static mesh property table, as well as functions related
-- to this table processing.
-- Please note that static mesh properties may vary from level to level, so
-- static table is frequently redefined on a per-level basis.


-- [ coll ] flag values:

-- [ coll ] flag values:
-- COLLISION_GROUP_ALL                     
-- COLLISION_GROUP_STATIC_ROOM                     // room mesh
-- COLLISION_GROUP_STATIC_OBLECT                   // room static object
-- COLLISION_GROUP_KINEMATIC                       // doors, blocks, static animated entityes
-- COLLISION_GROUP_GHOST                           // probe objects
-- COLLISION_GROUP_TRIGGERS                        // probe objects
-- COLLISION_GROUP_CHARACTERS                      // Lara, enemies, friends, creatures
-- COLLISION_GROUP_VEHICLE                         // car, moto, bike
-- COLLISION_GROUP_BULLETS                         // bullets, rockets, grenades, arrows...
-- COLLISION_GROUP_DYNAMICS                        // test balls, warious

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
        return COLLISION_GROUP_STATIC_OBLECT, COLLISION_SHAPE_BOX, nil;
    else
        local coll, shape, hide;
        if(static_tbl[id].hide ~= nil) then hide = static_tbl[id].hide else hide = 0 end;
        if(static_tbl[id].coll ~= nil) then coll = static_tbl[id].coll else coll = COLLISION_NONE end;
        if(static_tbl[id].shape ~= nil) then shape = static_tbl[id].shape else shape = COLLISION_SHAPE_BOX end;
        return coll, shape, hide;
    end;
end;