-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: I Gold
-- Version: 1.2
-- By: Lmwte

------------------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_1_5] = {};
gameflow_params[GAME_1_5].numlevels = 4;
gameflow_params[GAME_1_5].title = "data/tr1_gold/pix/TITLE";
gameflow_params[GAME_1_5].levels = {};
------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                                 [1]    Level Name                 [2] Level File Path                               [3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_1_5].levels[01] = { name = "Return to Egypt",         filepath = "data/tr1_gold/data/EGYPT.TUB",        picpath = "data/tr1_gold/pix/egyptloa" };
gameflow_params[GAME_1_5].levels[02] = { name = "Temple of the Cat",       filepath = "data/tr1_gold/data/CAT.TUB",          picpath = "data/tr1_gold/pix/egyptloa" };
gameflow_params[GAME_1_5].levels[03] = { name = "Atlantean Stronghold",    filepath = "data/tr1_gold/data/END.TUB",          picpath = "data/tr1_gold/pix/atlanloa" };
gameflow_params[GAME_1_5].levels[04] = { name = "The Hive",                filepath = "data/tr1_gold/data/END2.TUB",         picpath = "data/tr1_gold/pix/atlanloa" };
------------------------------------------------------------------------------------------------------------------------------------------------------------------
