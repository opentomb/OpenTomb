-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: II Gold
-- Version: 1.2
-- By: Lmwte

------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_2_5] = {};
gameflow_params[GAME_2_5].numlevels = 5;
gameflow_params[GAME_2_5].title = "data/tr2_gold/pix/TITLE";
gameflow_params[GAME_2_5].levels = {};
------------------------------------------------------------------------------------------------------------------------------------------------------
-- Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                                 [1]    Level Name                 [2] Level File Path                             [3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_2_5].levels[01] = { name = "The Cold War",            filepath = "data/tr2_gold/data/LEVEL1.TR2",       picpath = "data/tr2_gold/pix/title" };
gameflow_params[GAME_2_5].levels[02] = { name = "Fool's Gold",             filepath = "data/tr2_gold/data/LEVEL2.TR2",       picpath = "data/tr2_gold/pix/title" };
gameflow_params[GAME_2_5].levels[03] = { name = "Furnace of the Gods",     filepath = "data/tr2_gold/data/LEVEL3.TR2",       picpath = "data/tr2_gold/pix/title" };
gameflow_params[GAME_2_5].levels[04] = { name = "Kingdom",                 filepath = "data/tr2_gold/data/LEVEL4.TR2",       picpath = "data/tr2_gold/pix/title" };
gameflow_params[GAME_2_5].levels[05] = { name = "Nightmare in Vegas",      filepath = "data/tr2_gold/data/LEVEL5.TR2",       picpath = "data/tr2_gold/pix/title" };
------------------------------------------------------------------------------------------------------------------------------------------------------
