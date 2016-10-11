-- OPENTOMB SCRIPT LOADING ORDER
-- By Lwmte, Jun 2015 

--------------------------------------------------------------------------------
-- This is the core of Lua script loading, all script files should be loaded via
-- this file.
-- DO NOT CHANGE SCRIPT ORDER, unless you're completely sure what you're doing!
--------------------------------------------------------------------------------

-- Pre OpenGL/SDL init script loading.

function loadscript_pre()
    dofile(base_path .. "scripts/strings/getstring.lua");
    dofile(base_path .. "scripts/system/sys_scripts.lua");
    dofile(base_path .. "scripts/system/debug.lua");
    dofile(base_path .. "scripts/gameflow/gameflow.lua");
    dofile(base_path .. "scripts/trigger/trigger_functions.lua");
    dofile(base_path .. "scripts/trigger/helper_functions.lua");
    dofile(base_path .. "scripts/entity/entity_functions.lua");
    dofile(base_path .. "scripts/character/hair.lua");
    dofile(base_path .. "scripts/character/ragdoll.lua");
    dofile(base_path .. "scripts/config/control_constants.lua");
    dofile(base_path .. "scripts/audio/common_sounds.lua");
    dofile(base_path .. "scripts/audio/soundtrack.lua");
    dofile(base_path .. "scripts/audio/sample_override.lua");
    
    dofile(base_path .. "scripts/character/character.lua")
    dofile(base_path .. "scripts/inventory/item_list.lua")
    dofile(base_path .. "scripts/inventory/item_combine.lua")
    dofile(base_path .. "scripts/inventory/items.lua")
end


-- Post OpenGL/SDL init script loading.

function loadscript_post()
    dofile(base_path .. "scripts/gui/fonts.lua")
end
