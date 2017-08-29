-- Gameflow Script for OpenTomb
-- General script extraction module
-- Version: 1.2
-- By: Lwmte
-- Rewritten by: TeslaRus

---------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define global constants
---------------------------------------------------------------------------------------------------------------------------------------
--GAME_1
--GAME_1_1
--GAME_1_5
--GAME_2
--GAME_2_1
--GAME_2_5
--GAME_3
--GAME_3_5
--GAME_4
--GAME_4_1
--GAME_5
-- other custom games constants
GAME_MAX = GAME_5;  -- or last custom game id

gameflow_params = {};

dofile(base_path .. "scripts/gameflow/TR1.lua");
--dofile(base_path .. "scripts/gameflow/TR1_demo.lua");
dofile(base_path .. "scripts/gameflow/TR1_gold.lua");
dofile(base_path .. "scripts/gameflow/TR2.lua");
--dofile(base_path .. "scripts/gameflow/TR2_demo.lua");
dofile(base_path .. "scripts/gameflow/TR2_gold.lua");
dofile(base_path .. "scripts/gameflow/TR3.lua");
dofile(base_path .. "scripts/gameflow/TR3_gold.lua");
dofile(base_path .. "scripts/gameflow/TR4.lua");
--dofile(base_path .. "scripts/gameflow/TR4_demo.lua");
dofile(base_path .. "scripts/gameflow/TR5.lua");


--function getTitleScreen(game_version)
--function getNextLevelNum(currentgame, currentlevel, operand)
--function getNextLevel(currentgame, currentlevel, operand)
--function getLoadingScreen(currentgame, currentlevel, operand)

function setgamef(game_id, level_id)
    if(game_id == 1.0) then
        setGame(GAME_1, level_id);
    elseif(game_id == 1.1) then
        setGame(GAME_1_1, level_id);
    elseif(game_id == 1.5) then
        setGame(GAME_1_5, level_id);
    elseif(game_id == 2.0) then
        setGame(GAME_2, level_id);
    elseif(game_id == 2.1) then
        setGame(GAME_2_1, level_id);
    elseif(game_id == 2.5) then
        setGame(GAME_2_5, level_id);
    elseif(game_id == 3.0) then
        setGame(GAME_3, level_id);
    elseif(game_id == 3.5) then
        setGame(GAME_3_5, level_id);
    elseif(game_id == 4.0) then
        setGame(GAME_4, level_id);
    elseif(game_id == 4.1) then
        setGame(GAME_4_1, level_id);
    elseif(game_id == 5.0) then
        setGame(GAME_5, level_id);
    end;
end;

print("Gameflow path script loaded");
