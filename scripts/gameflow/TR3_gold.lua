-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: III Gold
-- Version: 1.0
-- By: Lwmte

------------------------------------------------------------------------------------------------------------------------------------------------------

gameflow_paths[GAME_3_5].numlevels = 6;

------------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_3_5].level = {};
------------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                               [1]    Level Name                 [2] Level File Path                             [3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_3_5].level[00] = { name = "Lara's Home",             filepath = "data/tr3_gold/data/HOUSE.TR2",        picpath = "data/tr3_gold/pix/HOUSE.BMP" };
gameflow_paths[GAME_3_5].level[01] = { name = "Highland Fling",          filepath = "data/tr3_gold/data/scotland.tr2",     picpath = "data/tr3_gold/pix/Highland.bmp" };
gameflow_paths[GAME_3_5].level[02] = { name = "Willard's Lair",          filepath = "data/tr3_gold/data/willsden.tr2",     picpath = "data/tr3_gold/pix/Willard.bmp" };
gameflow_paths[GAME_3_5].level[03] = { name = "Shakespeare Cliff",       filepath = "data/tr3_gold/data/chunnel.tr2",      picpath = "data/tr3_gold/pix/Chunnel.bmp" };
gameflow_paths[GAME_3_5].level[04] = { name = "Sleeping with the Fishes",filepath = "data/tr3_gold/data/undersea.tr2",     picpath = "data/tr3_gold/pix/undersea.bmp"};
gameflow_paths[GAME_3_5].level[05] = { name = "It's a Madhouse!",        filepath = "data/tr3_gold/data/zoo.tr2",          picpath = "data/tr3_gold/pix/zoo.bmp" };
gameflow_paths[GAME_3_5].level[06] = { name = "Reunion",                 filepath = "data/tr3_gold/data/slinc.tr2",        picpath = "data/tr3_gold/pix/Slinc.bmp" };
------------------------------------------------------------------------------------------------------------------------------------------------------
