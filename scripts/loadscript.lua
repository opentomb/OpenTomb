-- OPENTOMB SCRIPT LOADING ORDER
-- By Lwmte, Jun 2015 

--------------------------------------------------------------------------------
-- This is the core of Lua script loading, all script files should be loaded via
-- this file.
-- DO NOT CHANGE SCRIPT ORDER, unless you're completely sure what you're doing!
--------------------------------------------------------------------------------

-- Pre OpenGL/SDL init script loading.

function loadscript_pre()
    dofile("scripts/system/constants.lua");
    
    dofile("scripts/strings/getstring.lua");
    dofile("scripts/system/sys_scripts.lua");
    dofile("scripts/system/debug.lua");
    dofile("scripts/gameflow/gameflow.lua");
    dofile("scripts/trigger/trigger_functions.lua");
    dofile("scripts/trigger/helper_functions.lua");
    dofile("scripts/entity/entity_model_ID_override.lua");
    dofile("scripts/entity/entity_properties.lua");
    dofile("scripts/entity/entity_functions.lua");
    dofile("scripts/staticmesh/staticmesh.lua");
    dofile("scripts/character/hair.lua");
    dofile("scripts/character/ragdoll.lua");
    dofile("scripts/config/control_constants.lua");
    dofile("scripts/audio/common_sounds.lua");
    dofile("scripts/audio/soundtrack.lua");
    dofile("scripts/audio/sample_override.lua");
    
    dofile("scripts/character/character.lua")
    dofile("scripts/inventory/item_list.lua")
    dofile("scripts/inventory/item_combine.lua")
    dofile("scripts/inventory/items.lua")
end


-- Post OpenGL/SDL init script loading.

function loadscript_post()
    dofile("scripts/gui/fonts.lua")
end
