-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: I
-- Version: 1.2
-- By: Gh0stBlade

------------------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_1] = {};
gameflow_params[GAME_1].numlevels = 19;
gameflow_params[GAME_1].title = "data/tr1/pix/AMERTIT";
gameflow_params[GAME_1].levels = {};

------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                               [1]    Level Name                 [2] Level File Path                          [3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_1].levels[0]  = { name = "Lara's Home",             filepath = "data/tr1/data/GYM.PHD",          picpath = "data/tr1/pix/gymload"  };
gameflow_params[GAME_1].levels[1]  = { name = "Caves",                   filepath = "data/tr1/data/LEVEL1.PHD",       picpath = "data/tr1/pix/aztecloa" };
gameflow_params[GAME_1].levels[2]  = { name = "City of Vilcabamba",      filepath = "data/tr1/data/LEVEL2.PHD",       picpath = "data/tr1/pix/aztecloa" };
gameflow_params[GAME_1].levels[3]  = { name = "Lost Valley",             filepath = "data/tr1/data/LEVEL3A.PHD",      picpath = "data/tr1/pix/aztecloa" };
gameflow_params[GAME_1].levels[4]  = { name = "Tomb of Qualopec",        filepath = "data/tr1/data/LEVEL3B.PHD",      picpath = "data/tr1/pix/aztecloa" };
gameflow_params[GAME_1].levels[5]  = { name = "Tomb of Qualopec",        filepath = "data/tr1/data/CUT1.PHD",         picpath = "data/tr1/pix/aztecloa" };
gameflow_params[GAME_1].levels[6]  = { name = "St. Francis' Folly",      filepath = "data/tr1/data/LEVEL4.PHD",       picpath = "data/tr1/pix/greekloa" };
gameflow_params[GAME_1].levels[7]  = { name = "Colosseum",               filepath = "data/tr1/data/LEVEL5.PHD",       picpath = "data/tr1/pix/greekloa" };
gameflow_params[GAME_1].levels[8]  = { name = "Palace Midas",            filepath = "data/tr1/data/LEVEL6.PHD",       picpath = "data/tr1/pix/greekloa" };
gameflow_params[GAME_1].levels[9]  = { name = "The Cistern",             filepath = "data/tr1/data/LEVEL7A.PHD",      picpath = "data/tr1/pix/greekloa" };
gameflow_params[GAME_1].levels[10] = { name = "Tomb of Tihocan",         filepath = "data/tr1/data/LEVEL7B.PHD",      picpath = "data/tr1/pix/greekloa" };
gameflow_params[GAME_1].levels[11] = { name = "Tomb of Tihocan",         filepath = "data/tr1/data/CUT2.PHD",         picpath = "data/tr1/pix/greekloa" };
gameflow_params[GAME_1].levels[12] = { name = "City of Khamoon",         filepath = "data/tr1/data/LEVEL8A.PHD",      picpath = "data/tr1/pix/egyptloa" };
gameflow_params[GAME_1].levels[13] = { name = "Obelisk of Khamoon",      filepath = "data/tr1/data/LEVEL8B.PHD",      picpath = "data/tr1/pix/egyptloa" };
gameflow_params[GAME_1].levels[14] = { name = "Sanctuary of the Scion",  filepath = "data/tr1/data/LEVEL8C.PHD",      picpath = "data/tr1/pix/egyptloa" };
gameflow_params[GAME_1].levels[15] = { name = "Natla's Mines",           filepath = "data/tr1/data/LEVEL10A.PHD",     picpath = "data/tr1/pix/atlanloa" };
gameflow_params[GAME_1].levels[16] = { name = "Atlantis",                filepath = "data/tr1/data/CUT3.PHD",         picpath = "data/tr1/pix/atlanloa" };
gameflow_params[GAME_1].levels[17] = { name = "Atlantis",                filepath = "data/tr1/data/LEVEL10B.PHD",     picpath = "data/tr1/pix/atlanloa" };
gameflow_params[GAME_1].levels[18] = { name = "Atlantis",                filepath = "data/tr1/data/CUT4.PHD",         picpath = "data/tr1/pix/atlanloa" };
gameflow_params[GAME_1].levels[19] = { name = "The Great Pyramid",       filepath = "data/tr1/data/LEVEL10C.PHD",     picpath = "data/tr1/pix/atlanloa" };
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
