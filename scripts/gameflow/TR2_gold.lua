-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: II Gold
-- Version: 1.0
-- By: Lmwte

------------------------------------------------------------------------------------------------------------------------------------------------------

gameflow_paths[Game.II_GOLD].numlevels = 5;

------------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[Game.II_GOLD].level = {};
------------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                               [1]    Level Name                 [2] Level File Path                             [3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[Game.II_GOLD].level[01] = { name = "The Cold War",            filepath = "data/tr2_gold/data/level1.tr2",       picpath = "data/tr2_gold/pix/TITLE.jpg" };
gameflow_paths[Game.II_GOLD].level[02] = { name = "Fool's Gold",             filepath = "data/tr2_gold/data/level2.tr2",       picpath = "data/tr2_gold/pix/TITLE.jpg" };
gameflow_paths[Game.II_GOLD].level[03] = { name = "Furnace of the Gods",     filepath = "data/tr2_gold/data/level3.tr2",       picpath = "data/tr2_gold/pix/TITLE.jpg" };
gameflow_paths[Game.II_GOLD].level[04] = { name = "Kingdom",                 filepath = "data/tr2_gold/data/level4.tr2",       picpath = "data/tr2_gold/pix/TITLE.jpg" };
gameflow_paths[Game.II_GOLD].level[05] = { name = "Nightmare in Vegas",      filepath = "data/tr2_gold/data/level5.tr2",       picpath = "data/tr2_gold/pix/TITLE.jpg" };
------------------------------------------------------------------------------------------------------------------------------------------------------
