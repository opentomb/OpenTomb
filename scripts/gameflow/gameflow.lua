-- Gameflow Script for OpenTomb
-- General script extraction module
-- Version: 1.0
-- By: Lwmte

print("Gameflow path script loaded");

---------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define global constants
---------------------------------------------------------------------------------------------------------------------------------------
gameflow_lara_home_index   =  99; -- used to load Lara's Home.
---------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Define script path information
---------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths = {};
---------------------------------------------------------------------------------------------------------------------------------------
-- 3 - Assign script path information
---------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[1.0] = {script = "scripts/gameflow/TR1.lua", 	 title = "data/tr1/pix/AMERTIT.jpg"		 };
gameflow_paths[1.1] = {script = "scripts/gameflow/TR1_demo.lua", title = "data/tr1/pix/AMERTIT.jpg"		 };
gameflow_paths[1.5] = {script = "scripts/gameflow/TR1_gold.lua", title = "data/tr1_gold/pix/TITLE.jpg"	 };
gameflow_paths[2.0] = {script = "scripts/gameflow/TR2.lua", 	 title = "data/tr2/pix/TITLE.jpg"		 };
gameflow_paths[2.1] = {script = "scripts/gameflow/TR2_demo.lua", title = "data/tr2/pix/TITLE.jpg"		 };
gameflow_paths[2.5] = {script = "scripts/gameflow/TR2_gold.lua", title = "data/tr2_gold/pix/TITLE.jpg"	 };
gameflow_paths[3.0] = {script = "scripts/gameflow/TR3.lua", 	 title = "graphics/tr3_title.png"		 };
gameflow_paths[3.5] = {script = "scripts/gameflow/TR3_gold.lua", title = "data/tr3_gold/pix/Titleuk.bmp" };
gameflow_paths[4.0] = {script = "scripts/gameflow/TR4.lua", 	 title = "graphics/tr4_title.png"		 };		-- No TR4 title screen!
gameflow_paths[4.1] = {script = "scripts/gameflow/TR4_demo.lua", title = "graphics/tr4_title.png"		 };		-- No TR4 title screen!
gameflow_paths[5.0] = {script = "scripts/gameflow/TR5.lua", 	 title = "graphics/tr5_title.png"		 };		-- No TR5 title screen!
---------------------------------------------------------------------------------------------------------------------------------------
-- This function gets current game script path.
---------------------------------------------------------------------------------------------------------------------------------------
function GetGameflowScriptPath(game_version)
	if(game_version <= 5.0) then
		cvars.game_script = gameflow_paths[game_version].script;
	else
		cvars.game_script = "NULL";
	end;
end;
---------------------------------------------------------------------------------------------------------------------------------------
-- Get game's title screen.
---------------------------------------------------------------------------------------------------------------------------------------
function GetTitleScreen(game_version)
	if(game_version <= 5.0) then
		return gameflow_paths[game_version].title;
	else
		return "NULL";
	end;
end;
---------------------------------------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------------------------------------
-- This function is a general function to get next level number. Used for both GetNextLevel and GetLoadingScreen functions.
---------------------------------------------------------------------------------------------------------------------------------------
function NextLevelNum(currentlevel, operand)
	local nextlevel;
	
	if(operand == 0) then    -- If zero we just get the next level (TR1/2/3/5)
		nextlevel = currentlevel + 1;
	else
		nextlevel = operand; -- Or we load the level id from the level end triggers operand (TR4)
	end;

	if((nextlevel == gameflow_lara_home_index) and (level[0] ~= nil)) then	-- Load Lara's Home level, if exist.
		nextlevel = 0;
	elseif(nextlevel > numlevels) then -- No Lara's Home, load first level instead.
		nextlevel = 1;
	end;
	
	return nextlevel;
end;
---------------------------------------------------------------------------------------------------------------------------------------
-- Get next level.
---------------------------------------------------------------------------------------------------------------------------------------
function GetNextLevel(currentlevel, operand)
	local level_number = NextLevelNum(currentlevel, operand);
	if(level[level_number] ~= nil) then
		return level[level_number].filepath, level[level_number].name, level_number;
	else
		return "none", "none", -1;
	end;
end;
---------------------------------------------------------------------------------------------------------------------------------------
-- Get next level's loading screen.
---------------------------------------------------------------------------------------------------------------------------------------
function GetLoadingScreen(currentlevel, operand)
	local level_number = NextLevelNum(currentlevel, operand);
	return level[level_number].picpath;
end;