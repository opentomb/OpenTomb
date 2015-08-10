-- Gameflow Script for OpenTomb
-- General script extraction module
-- Version: 1.0
-- By: Lwmte
-- Rewritten by: TeslaRus

-- other custom games constants
GAME_MAX = TR_V;  -- or last custom game id
---------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Define script path information
---------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths = {};
---------------------------------------------------------------------------------------------------------------------------------------
-- 3 - Assign script path information
---------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[TR_I]   = {script = "scripts/gameflow/TR1.lua",      title = "data/tr1/pix/AMERTIT.jpg" };
gameflow_paths[TR_I_DEMO] = {script = "scripts/gameflow/TR1_demo.lua", title = "data/tr1/pix/AMERTIT.jpg" };
gameflow_paths[TR_I_UB] = {script = "scripts/gameflow/TR1_gold.lua", title = "data/tr1_gold/pix/TITLE.jpg" };
gameflow_paths[TR_II]   = {script = "scripts/gameflow/TR2.lua",      title = "data/tr2/pix/TITLE.jpg" };
gameflow_paths[TR_II_DEMO] = {script = "scripts/gameflow/TR2_demo.lua", title = "data/tr2/pix/TITLE.jpg" };
gameflow_paths[TR_II_GOLD] = {script = "scripts/gameflow/TR2_gold.lua", title = "data/tr2_gold/pix/TITLE.jpg" };
gameflow_paths[TR_III]   = {script = "scripts/gameflow/TR3.lua",      title = "data/tr3/pix/TITLEUK.BMP" };
gameflow_paths[TR_III_GOLD] = {script = "scripts/gameflow/TR3_gold.lua", title = "data/tr3_gold/pix/Titleuk.bmp" };
gameflow_paths[TR_IV]   = {script = "scripts/gameflow/TR4.lua",      title = "graphics/tr4_title.png" };        -- No TR4 title screen!
gameflow_paths[TR_IV_DEMO] = {script = "scripts/gameflow/TR4_demo.lua", title = "graphics/tr4_title.png" };        -- No TR4 title screen!
gameflow_paths[TR_V]   = {script = "scripts/gameflow/TR5.lua",      title = "graphics/tr5_title.png" };        -- No TR5 title screen!

dofile(gameflow_paths[TR_I].script);
--dofile(gameflow_paths[TR_I_DEMO].script);
dofile(gameflow_paths[TR_I_UB].script);
dofile(gameflow_paths[TR_II].script);
--dofile(gameflow_paths[TR_II_DEMO].script);
dofile(gameflow_paths[TR_II_GOLD].script);
dofile(gameflow_paths[TR_III].script);
dofile(gameflow_paths[TR_III_GOLD].script);
dofile(gameflow_paths[TR_IV].script);
--dofile(gameflow_paths[TR_IV_DEMO].script);
dofile(gameflow_paths[TR_V].script);


---------------------------------------------------------------------------------------------------------------------------------------
-- Get game's title screen.
---------------------------------------------------------------------------------------------------------------------------------------
function getTitleScreen(game_version)
    if(game_version <= GAME_MAX) then
        return gameflow_paths[game_version].title;
    else
        return "NULL";
    end;
end;
---------------------------------------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------------------------------------
-- This function is a general function to get next level number. Used for both GetNextLevel and GetLoadingScreen functions.
---------------------------------------------------------------------------------------------------------------------------------------
function getNextLevelNum(currentgame, currentlevel, operand)
    local nextlevel;

    if(operand == 0) then    -- If zero we just get the next level (TR1/2/3/5)
        nextlevel = currentlevel + 1;
    else
        nextlevel = operand; -- Or we load the level id from the level end triggers operand (TR4)
    end;

    if(gameflow_paths[currentgame] ~= nil) then
        if((nextlevel == 99) and (gameflow_paths[currentgame].level[0] ~= nil)) then      -- Load Lara's Home level (99), if exist.
            nextlevel = 0;
        elseif(nextlevel > gameflow_paths[currentgame].numlevels) then -- No Lara's Home, load first level instead.
            nextlevel = 1;
        end;
    end;

    return nextlevel;
end;
---------------------------------------------------------------------------------------------------------------------------------------
-- Get next level.
---------------------------------------------------------------------------------------------------------------------------------------
function getNextLevel(currentgame, currentlevel, operand)
    local level_number = getNextLevelNum(currentgame, currentlevel, operand);
    if(gameflow_paths[currentgame].level[level_number] ~= nil) then
        return gameflow_paths[currentgame].level[level_number].filepath, gameflow_paths[currentgame].level[level_number].name, level_number;
    else
        return "none", "none", -1;
    end;
end;
---------------------------------------------------------------------------------------------------------------------------------------
-- Get next level's loading screen.
---------------------------------------------------------------------------------------------------------------------------------------
function getLoadingScreen(currentgame, currentlevel, operand)
    local level_number = getNextLevelNum(currentgame, currentlevel, operand);
    if(gameflow_paths[currentgame].level[level_number] ~= nil) then
        return gameflow_paths[currentgame].level[level_number].picpath;
    else
        return "NULL";
    end;
end;

function setgamef(game_id, level_id)
    if(game_id == 1.0) then
        setgame(TR_I, level_id);
    elseif(game_id == 1.1) then
        setgame(TR_I_DEMO, level_id);
    elseif(game_id == 1.5) then
        setgame(TR_I_UB, level_id);
    elseif(game_id == 2.0) then
        setgame(TR_II, level_id);
    elseif(game_id == 2.1) then
        setgame(TR_II_DEMO, level_id);
    elseif(game_id == 2.5) then
        setgame(TR_II_GOLD, level_id);
    elseif(game_id == 3.0) then
        setgame(TR_III, level_id);
    elseif(game_id == 3.5) then
        setgame(TR_III_GOLD, level_id);
    elseif(game_id == 4.0) then
        setgame(TR_IV, level_id);
    elseif(game_id == 4.1) then
        setgame(TR_IV_DEMO, level_id);
    elseif(game_id == 5.0) then
        setgame(TR_V, level_id);
    end;
end;

SETGAMEF = setgamef;

print("Gameflow path script loaded");
