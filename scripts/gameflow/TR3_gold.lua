-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: III Gold
-- Version: 1.2
-- By: Lwmte

------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_3_5] = {};
gameflow_params[GAME_3_5].numlevels = 6;
gameflow_params[GAME_3_5].title = "data/tr3_gold/pix/Titleuk"
gameflow_params[GAME_3_5].levels = {};
------------------------------------------------------------------------------------------------------------------------------------------------------
-- Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                                 [1]    Level Name                 [2] Level File Path                             [3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_3_5].levels[00] = { name = "Lara's Home",             filepath = "data/tr3_gold/data/HOUSE.TR2",        picpath = "data/tr3_gold/pix/HOUSE" };
gameflow_params[GAME_3_5].levels[01] = { name = "Highland Fling",          filepath = "data/tr3_gold/data/scotland.tr2",     picpath = "data/tr3_gold/pix/Highland" };
gameflow_params[GAME_3_5].levels[02] = { name = "Willard's Lair",          filepath = "data/tr3_gold/data/willsden.tr2",     picpath = "data/tr3_gold/pix/Willard" };
gameflow_params[GAME_3_5].levels[03] = { name = "Shakespeare Cliff",       filepath = "data/tr3_gold/data/chunnel.tr2",      picpath = "data/tr3_gold/pix/Chunnel" };
gameflow_params[GAME_3_5].levels[04] = { name = "Sleeping with the Fishes",filepath = "data/tr3_gold/data/undersea.tr2",     picpath = "data/tr3_gold/pix/undersea"};
gameflow_params[GAME_3_5].levels[05] = { name = "It's a Madhouse!",        filepath = "data/tr3_gold/data/zoo.tr2",          picpath = "data/tr3_gold/pix/zoo" };
gameflow_params[GAME_3_5].levels[06] = { name = "Reunion",                 filepath = "data/tr3_gold/data/slinc.tr2",        picpath = "data/tr3_gold/pix/Slinc" };
------------------------------------------------------------------------------------------------------------------------------------------------------
