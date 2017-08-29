-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: V
-- Version: 1.2
-- By: Gh0stBlade

----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_5] = {};
gameflow_params[GAME_5].numlevels = 17;
gameflow_params[GAME_5].title = "graphics/tr5_title";
gameflow_params[GAME_5].levels = {};
----------------------------------------------------------------------------------------------------------------------------------------------
-- Assign our level information
----------------------------------------------------------------------------------------------------------------------------------------------
-- Array                               [1]    Level Name                 [2] Level File Path                          [3] Level Load Screen Path
----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_5].levels[01] = { name = "Streets of Rome",         filepath = "data/tr5/data/Andrea1.trc",      picpath = "data/tr5/pix/screen001" };
gameflow_params[GAME_5].levels[02] = { name = "Trajan's markets",        filepath = "data/tr5/data/Andrea2.trc",      picpath = "data/tr5/pix/screen002" };
gameflow_params[GAME_5].levels[03] = { name = "The Colosseum",           filepath = "data/tr5/data/Andrea3.trc",      picpath = "data/tr5/pix/screen003" };
gameflow_params[GAME_5].levels[04] = { name = "The Base",                filepath = "data/tr5/data/joby2.trc",        picpath = "data/tr5/pix/screen004" };
gameflow_params[GAME_5].levels[05] = { name = "The Submarine",           filepath = "data/tr5/data/joby3.trc",        picpath = "data/tr5/pix/screen005" };
gameflow_params[GAME_5].levels[06] = { name = "Deepsea Dive",            filepath = "data/tr5/data/joby4.trc",        picpath = "data/tr5/pix/screen006" };
gameflow_params[GAME_5].levels[07] = { name = "Sinking Submarine",       filepath = "data/tr5/data/joby5.trc",        picpath = "data/tr5/pix/screen007" };
gameflow_params[GAME_5].levels[08] = { name = "Gallows Tree",            filepath = "data/tr5/data/andy1.trc",        picpath = "data/tr5/pix/screen008" };
gameflow_params[GAME_5].levels[09] = { name = "Labyrinth",               filepath = "data/tr5/data/Andy2.trc",        picpath = "data/tr5/pix/screen009" };
gameflow_params[GAME_5].levels[10] = { name = "Old Mill",                filepath = "data/tr5/data/andy3.trc",        picpath = "data/tr5/pix/screen010" };
gameflow_params[GAME_5].levels[11] = { name = "The 13th Floor",          filepath = "data/tr5/data/rich1.trc",        picpath = "data/tr5/pix/screen011" };
gameflow_params[GAME_5].levels[12] = { name = "Escape with the Iris",    filepath = "data/tr5/data/rich2.trc",        picpath = "data/tr5/pix/screen012" };
gameflow_params[GAME_5].levels[13] = { name = "Security Breach",         filepath = "data/tr5/data/richcut2.trc",     picpath = "data/tr5/pix/screen013" };
gameflow_params[GAME_5].levels[14] = { name = "Red Alert!",              filepath = "data/tr5/data/rich3.trc",        picpath = "data/tr5/pix/screen013" };

gameflow_params[GAME_5].levels[15] = { name = "Gibby's level",           filepath = "data/tr5/data/gibby.trc",        picpath = "data/tr5/pix/screen013" };
gameflow_params[GAME_5].levels[16] = { name = "Del's level",             filepath = "data/tr5/data/del.trc",          picpath = "data/tr5/pix/screen013" };
gameflow_params[GAME_5].levels[17] = { name = "Tom's level",             filepath = "data/tr5/data/tom.trc",          picpath = "data/tr5/pix/screen013" };

----------------------------------------------------------------------------------------------------------------------------------------------
