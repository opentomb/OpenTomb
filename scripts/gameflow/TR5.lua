-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: V
-- Version: 1.0
-- By: Gh0stBlade

----------------------------------------------------------------------------------------------------------------------------------------------

gameflow_paths[GAME_5].numlevels = 17;

----------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_5].level = {};
----------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
----------------------------------------------------------------------------------------------------------------------------------------------
-- Array                             [1]    Level Name                 [2] Level File Path                          [3] Level Load Screen Path
----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_5].level[01] = { name = "Streets of Rome",         filepath = "data/tr5/data/Andrea1.trc",      picpath = "data/tr5/pix/screen001.jpg" };
gameflow_paths[GAME_5].level[02] = { name = "Trajan's markets",        filepath = "data/tr5/data/Andrea2.trc",      picpath = "data/tr5/pix/screen002.jpg" };
gameflow_paths[GAME_5].level[03] = { name = "The Colosseum",           filepath = "data/tr5/data/Andrea3.trc",      picpath = "data/tr5/pix/screen003.jpg" };
gameflow_paths[GAME_5].level[04] = { name = "The Base",                filepath = "data/tr5/data/joby2.trc",        picpath = "data/tr5/pix/screen004.jpg" };
gameflow_paths[GAME_5].level[05] = { name = "The Submarine",           filepath = "data/tr5/data/joby3.trc",        picpath = "data/tr5/pix/screen005.jpg" };
gameflow_paths[GAME_5].level[06] = { name = "Deepsea Dive",            filepath = "data/tr5/data/joby4.trc",        picpath = "data/tr5/pix/screen006.jpg" };
gameflow_paths[GAME_5].level[07] = { name = "Sinking Submarine",       filepath = "data/tr5/data/joby5.trc",        picpath = "data/tr5/pix/screen007.jpg" };
gameflow_paths[GAME_5].level[08] = { name = "Gallows Tree",            filepath = "data/tr5/data/andy1.trc",        picpath = "data/tr5/pix/screen008.jpg" };
gameflow_paths[GAME_5].level[09] = { name = "Labyrinth",               filepath = "data/tr5/data/Andy2.trc",        picpath = "data/tr5/pix/screen009.jpg" };
gameflow_paths[GAME_5].level[10] = { name = "Old Mill",                filepath = "data/tr5/data/andy3.trc",        picpath = "data/tr5/pix/screen010.jpg" };
gameflow_paths[GAME_5].level[11] = { name = "The 13th Floor",          filepath = "data/tr5/data/rich1.trc",        picpath = "data/tr5/pix/screen011.jpg" };
gameflow_paths[GAME_5].level[12] = { name = "Escape with the Iris",    filepath = "data/tr5/data/rich2.trc",        picpath = "data/tr5/pix/screen012.jpg" };
gameflow_paths[GAME_5].level[13] = { name = "Security Breach",         filepath = "data/tr5/data/richcut2.trc",     picpath = "data/tr5/pix/screen013.jpg" };
gameflow_paths[GAME_5].level[14] = { name = "Red Alert!",              filepath = "data/tr5/data/rich3.trc",        picpath = "data/tr5/pix/screen013.jpg" };

gameflow_paths[GAME_5].level[15] = { name = "Gibby's level",           filepath = "data/tr5/data/gibby.trc",        picpath = "data/tr5/pix/screen013.jpg" };
gameflow_paths[GAME_5].level[16] = { name = "Del's level",             filepath = "data/tr5/data/del.trc",          picpath = "data/tr5/pix/screen013.jpg" };
gameflow_paths[GAME_5].level[17] = { name = "Tom's level",             filepath = "data/tr5/data/tom.trc",          picpath = "data/tr5/pix/screen013.jpg" };

----------------------------------------------------------------------------------------------------------------------------------------------
