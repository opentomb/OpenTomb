-- OPENTOMB LEVEL PRELOAD SCRIPT
-- By Lwmte, Apr 2015
print("level_preload->loaded !");

--------------------------------------------------------------------------------
-- This script defines static mesh property table, as well as functions related
-- to this table processing.
-- Here are level preload routine. 
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

dofile(base_path .. "scripts/entity/entity_properties.lua");
dofile(base_path .. "scripts/entity/entity_model_id_override.lua");

static_tbl = {};    -- Define static mesh property table.

-- Get static mesh flags from property table.

function getStaticMeshProperties(id)
    local coll = COLLISION_GROUP_STATIC_OBLECT;
    local shape = COLLISION_SHAPE_BOX;
    local hide = 0;

    if((static_tbl ~= nil) and (static_tbl[id] ~= nil)) then
        if(static_tbl[id].coll ~= nil) then 
            coll = static_tbl[id].coll;
        end;

        if(static_tbl[id].shape ~= nil) then 
            shape = static_tbl[id].shape;
        end;

        if(static_tbl[id].hide ~= nil) then 
            hide = static_tbl[id].hide;
        end;
    end;

    return coll, shape, hide;
end;

level_PostLoad = function()
    print("level_PostLoad DEFAULT");
end;

level_PreLoad = function()
    print("level_PreLoad DEFAULT");
end;
