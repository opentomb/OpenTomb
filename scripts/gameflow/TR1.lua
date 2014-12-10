-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: I
-- Version: 1.0
-- By: Gh0stBlade

------------------------------------------------------------------------------------------------------------------------------------------------------------------

gameflow_paths[GAME_1].numlevels = 15;

------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
------------------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_1].level = {};
------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                             [1]    Level Name                 [2] Level File Path                          [3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_1].level[00] = { name = "Lara's Home",             filepath = "data/tr1/data/GYM.PHD",          picpath = "data/tr1/pix/gymloa.png"   };
gameflow_paths[GAME_1].level[01] = { name = "Caves",                   filepath = "data/tr1/data/LEVEL1.PHD",       picpath = "data/tr1/pix/aztecloa.png" };
gameflow_paths[GAME_1].level[02] = { name = "City of Vilcabamba",      filepath = "data/tr1/data/LEVEL2.PHD",       picpath = "data/tr1/pix/aztecloa.png" };
gameflow_paths[GAME_1].level[03] = { name = "Lost Valley",             filepath = "data/tr1/data/LEVEL3A.PHD",      picpath = "data/tr1/pix/aztecloa.png" };
gameflow_paths[GAME_1].level[04] = { name = "Tomb of Qualopec",        filepath = "data/tr1/data/LEVEL3B.PHD",      picpath = "data/tr1/pix/aztecloa.png" };
gameflow_paths[GAME_1].level[05] = { name = "St. Francis' Folly",      filepath = "data/tr1/data/LEVEL4.PHD",       picpath = "data/tr1/pix/greekloa.png" };
gameflow_paths[GAME_1].level[06] = { name = "Colosseum",               filepath = "data/tr1/data/LEVEL5.PHD",       picpath = "data/tr1/pix/greekloa.png" };
gameflow_paths[GAME_1].level[07] = { name = "Palace Midas",            filepath = "data/tr1/data/LEVEL6.PHD",       picpath = "data/tr1/pix/greekloa.png" };
gameflow_paths[GAME_1].level[08] = { name = "The Cistern",             filepath = "data/tr1/data/LEVEL7A.PHD",      picpath = "data/tr1/pix/greekloa.png" };
gameflow_paths[GAME_1].level[09] = { name = "Tomb of Tihocan",         filepath = "data/tr1/data/LEVEL7B.PHD",      picpath = "data/tr1/pix/greekloa.png" };
gameflow_paths[GAME_1].level[10] = { name = "City of Khamoon",         filepath = "data/tr1/data/LEVEL8A.PHD",      picpath = "data/tr1/pix/egyptloa.png" };
gameflow_paths[GAME_1].level[11] = { name = "Obelisk of Khamoon",      filepath = "data/tr1/data/LEVEL8B.PHD",      picpath = "data/tr1/pix/egyptloa.png" };
gameflow_paths[GAME_1].level[12] = { name = "Sanctuary of the Scion",  filepath = "data/tr1/data/LEVEL8C.PHD",      picpath = "data/tr1/pix/egyptloa.png" };
gameflow_paths[GAME_1].level[13] = { name = "Natla's Mines",           filepath = "data/tr1/data/LEVEL10A.PHD",     picpath = "data/tr1/pix/atlanloa.png" };
gameflow_paths[GAME_1].level[14] = { name = "Atlantis",                filepath = "data/tr1/data/LEVEL10B.PHD",     picpath = "data/tr1/pix/atlanloa.png" };
gameflow_paths[GAME_1].level[15] = { name = "The Great Pyramid",       filepath = "data/tr1/data/LEVEL10C.PHD",     picpath = "data/tr1/pix/atlanloa.png" };
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
