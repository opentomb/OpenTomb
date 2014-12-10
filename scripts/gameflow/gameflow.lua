-- Gameflow Script for OpenTomb
-- General script extraction module
-- Version: 1.0
-- By: Lwmte
-- Rewritten by: TeslaRus

---------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define global constants
---------------------------------------------------------------------------------------------------------------------------------------
gameflow_lara_home_index   =  99; -- used to load Lara's Home.

GAME_1      = 0;
GAME_1_1    = 1;
GAME_1_5    = 2;
GAME_2      = 3;
GAME_2_1    = 4;
GAME_2_5    = 5;
GAME_3      = 6;
GAME_3_5    = 7;
GAME_4      = 8;
GAME_4_1    = 9;
GAME_5      = 10;
-- other custom games constants
GAME_MAX = GAME_5;  -- or last custom game id
---------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Define script path information
---------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths = {};
---------------------------------------------------------------------------------------------------------------------------------------
-- 3 - Assign script path information
---------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_1]   = {script = "scripts/gameflow/TR1.lua",      title = "data/tr1/pix/AMERTIT.jpg" };
gameflow_paths[GAME_1_1] = {script = "scripts/gameflow/TR1_demo.lua", title = "data/tr1/pix/AMERTIT.jpg" };
gameflow_paths[GAME_1_5] = {script = "scripts/gameflow/TR1_gold.lua", title = "data/tr1_gold/pix/TITLE.jpg" };
gameflow_paths[GAME_2]   = {script = "scripts/gameflow/TR2.lua",      title = "data/tr2/pix/TITLE.jpg" };
gameflow_paths[GAME_2_1] = {script = "scripts/gameflow/TR2_demo.lua", title = "data/tr2/pix/TITLE.jpg" };
gameflow_paths[GAME_2_5] = {script = "scripts/gameflow/TR2_gold.lua", title = "data/tr2_gold/pix/TITLE.jpg" };
gameflow_paths[GAME_3]   = {script = "scripts/gameflow/TR3.lua",      title = "graphics/tr3_title.png" };
gameflow_paths[GAME_3_5] = {script = "scripts/gameflow/TR3_gold.lua", title = "data/tr3_gold/pix/Titleuk.bmp" };
gameflow_paths[GAME_4]   = {script = "scripts/gameflow/TR4.lua",      title = "graphics/tr4_title.png" };        -- No TR4 title screen!
gameflow_paths[GAME_4_1] = {script = "scripts/gameflow/TR4_demo.lua", title = "graphics/tr4_title.png" };        -- No TR4 title screen!
gameflow_paths[GAME_5]   = {script = "scripts/gameflow/TR5.lua",      title = "graphics/tr5_title.png" };        -- No TR5 title screen!

dofile(gameflow_paths[GAME_1].script);
--dofile(gameflow_paths[GAME_1_1].script);
dofile(gameflow_paths[GAME_1_5].script);
dofile(gameflow_paths[GAME_2].script);
--dofile(gameflow_paths[GAME_2_1].script);
dofile(gameflow_paths[GAME_2_5].script);
dofile(gameflow_paths[GAME_3].script);
dofile(gameflow_paths[GAME_3_5].script);
dofile(gameflow_paths[GAME_4].script);
--dofile(gameflow_paths[GAME_4_1].script);
dofile(gameflow_paths[GAME_5].script);


---------------------------------------------------------------------------------------------------------------------------------------
-- Get game's title screen.
---------------------------------------------------------------------------------------------------------------------------------------
function GetTitleScreen(game_version)
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
function NextLevelNum(currentgame, currentlevel, operand)
    local nextlevel;

    if(operand == 0) then    -- If zero we just get the next level (TR1/2/3/5)
        nextlevel = currentlevel + 1;
    else
        nextlevel = operand; -- Or we load the level id from the level end triggers operand (TR4)
    end;

    if(gameflow_paths[currentgame] ~= nil) then
        if((nextlevel == gameflow_lara_home_index) and (gameflow_paths[currentgame].level[0] ~= nil)) then      -- Load Lara's Home level, if exist.
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
function GetNextLevel(currentgame, currentlevel, operand)
    local level_number = NextLevelNum(currentgame, currentlevel, operand);
    if(gameflow_paths[currentgame].level[level_number] ~= nil) then
        return gameflow_paths[currentgame].level[level_number].filepath, gameflow_paths[currentgame].level[level_number].name, level_number;
    else
        return "none", "none", -1;
    end;
end;
---------------------------------------------------------------------------------------------------------------------------------------
-- Get next level's loading screen.
---------------------------------------------------------------------------------------------------------------------------------------
function GetLoadingScreen(currentgame, currentlevel, operand)
    if(level ~= nil) then
        local level_number = NextLevelNum(currentgame, currentlevel, operand);
        return gameflow_paths[currentgame].level[level_number].picpath;
    else
        return "NULL";
    end;
end;

print("Gameflow path script loaded");
