-- Gameflow Script for OpenTomb
-- General script extraction module
-- Version: 1.0
-- By: Lwmte

print("Gameflow path script loaded");

--------------------------------------------------------------
-- 1 - Define global constants
--------------------------------------------------------------
gameflow_lara_home_index   =  99; -- used to load Lara's Home.
--------------------------------------------------------------
-- 2 - Define script path information
--------------------------------------------------------------
gameflow_script_paths = {};
--------------------------------------------------------------
-- 3 - Assign script path information
--------------------------------------------------------------
gameflow_script_paths[0]   = "scripts/gameflow/TR1.lua";
gameflow_script_paths[1]   = "scripts/gameflow/TR1_demo.lua";
gameflow_script_paths[2]   = "scripts/gameflow/TR1_gold.lua";
gameflow_script_paths[3]   = "scripts/gameflow/TR2.lua";
gameflow_script_paths[4]   = "scripts/gameflow/TR2_demo.lua";
gameflow_script_paths[5]   = "scripts/gameflow/TR3.lua";
gameflow_script_paths[6]   = "scripts/gameflow/TR4.lua";
gameflow_script_paths[7]   = "scripts/gameflow/TR4_demo.lua";
gameflow_script_paths[8]   = "scripts/gameflow/TR5.lua";

-- Gold games require seperate script entry.

gameflow_script_paths[9]   = "scripts/gameflow/TR2_gold.lua";
gameflow_script_paths[10]  = "scripts/gameflow/TR3_gold.lua";
--------------------------------------------------------------
-- This function sets current game, according to passed string
--------------------------------------------------------------
function GetGameflowScriptPath(game_name)
	if(game_name == "TR1")  then 
		cvars.game_script = gameflow_script_paths[0];
	elseif(game_name == "TR2") then
		cvars.game_script = gameflow_script_paths[3];
	elseif(game_name == "TR3") then
		cvars.game_script = gameflow_script_paths[5];
	elseif(game_name == "TR4") then
		cvars.game_script = gameflow_script_paths[6];
	elseif(game_name == "TR5") then
		cvars.game_script = gameflow_script_paths[8];
	elseif(game_name == "TR2GOLD") then
		cvars.game_script = gameflow_script_paths[9];
	elseif(game_name == "TR3GOLD") then
		cvars.game_script = gameflow_script_paths[10];
	else
		cvars.game_script = "NULL";
	end;
end;

function GetNextLevel(currentlevel, operand)
	if(operand == 0) then -- If zero we just get the next level (TR1/2/3/5)
		nextlevel = currentlevel + 1;
	else
		nextlevel = operand; -- Or we load the level id from the level end triggers operand (TR4)
	end;

	if((nextlevel == gameflow_lara_home_index) and (level[0] ~= nil)) then
		nextlevel = 0;
	elseif(nextlevel > numlevels) then -- If End level load first
		nextlevel = 1;
	end;
	
	return level[nextlevel].filepath, level[nextlevel].name, nextlevel;
end;
--------------------------------------------------------------