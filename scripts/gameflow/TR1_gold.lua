-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: I Gold
-- Version: 1.0
-- By: Lmwte

------------------------------------------------------------------------------------------------------------------------------------------------------------------

gameflow_paths[GAME_1_5].numlevels = 4;

------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
------------------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_1_5].level = {};
------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                               [1]    Level Name                 [2] Level File Path                               [3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_1_5].level[01] = { name = "Return to Egypt",         filepath = "data/tr1_gold/data/EGYPT.TUB",        picpath = "data/tr1_gold/pix/egyptloa" };
gameflow_paths[GAME_1_5].level[02] = { name = "Temple of the Cat",       filepath = "data/tr1_gold/data/CAT.TUB",          picpath = "data/tr1_gold/pix/egyptloa" };
gameflow_paths[GAME_1_5].level[03] = { name = "Atlantean Stronghold",    filepath = "data/tr1_gold/data/END.TUB",          picpath = "data/tr1_gold/pix/atlanloa" };
gameflow_paths[GAME_1_5].level[04] = { name = "The Hive",                filepath = "data/tr1_gold/data/END2.TUB",         picpath = "data/tr1_gold/pix/atlanloa" };
------------------------------------------------------------------------------------------------------------------------------------------------------------------
