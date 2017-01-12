-- OPENTOMB TRIGGER FUNCTION SCRIPT
-- by Lwmte, April 2015

--------------------------------------------------------------------------------
-- This file contains core trigger routines which are used to initialize, run
-- and do other actions related to trigger array.
-- Trigger array itself is generated on the fly from each level file and is not
-- visible for user. You can turn on debug output of trigger array via config
-- command "system->output_triggers = 1".
--------------------------------------------------------------------------------

dofile(base_path .. "scripts/trigger/flipeffects.lua")  -- Initialize flipeffects.

-- Clear dead enemies, if they have CLEAR BODY flag specified.

function clearBodies()
    print("CLEAR BODIES");
end

-- Plays specified cutscene. Only valid in retail TR4-5.

function playCutscene(cutscene_index)
    if(getLevelVersion() < TR_IV) then return 0 end;
    print("CUTSCENE: index = " .. cutscene_index);
end