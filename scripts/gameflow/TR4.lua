-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: IV
-- Version: 1.2
-- By: Gh0stBlade

---------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_4] = {};
gameflow_params[GAME_4].numlevels = 38;
gameflow_params[GAME_4].title = "graphics/tr4_title";
gameflow_params[GAME_4].levels = {};
---------------------------------------------------------------------------------------------------------------------------------------------------------
-- Assign our level information
---------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                               [1]    Level Name                      [2] Level File Path                          [3] Level Load Screen Path
---------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_params[GAME_4].levels[01] = { name = "Angkor Wat",                   filepath = "data/tr4/data/angkor1.tr4",      picpath = "data/tr4/pix/cambodia" };
gameflow_params[GAME_4].levels[02] = { name = "Race For The Iris",            filepath = "data/tr4/data/ang_race.tr4",     picpath = "data/tr4/pix/cambodia" };
gameflow_params[GAME_4].levels[03] = { name = "The Tomb Of Seth",             filepath = "data/tr4/data/settomb1.tr4",     picpath = "data/tr4/pix/settomb" };
gameflow_params[GAME_4].levels[04] = { name = "Burial Chambers",              filepath = "data/tr4/data/settomb2.tr4",     picpath = "data/tr4/pix/settomb" };
gameflow_params[GAME_4].levels[05] = { name = "Valley Of The Kings",          filepath = "data/tr4/data/jeepchas.tr4",     picpath = "data/tr4/pix/valley" };
gameflow_params[GAME_4].levels[06] = { name = "KV5",                          filepath = "data/tr4/data/jeepchs2.tr4",     picpath = "data/tr4/pix/valley" };
gameflow_params[GAME_4].levels[07] = { name = "Temple Of Karnak",             filepath = "data/tr4/data/karnak1.tr4",      picpath = "data/tr4/pix/karnak" };
gameflow_params[GAME_4].levels[08] = { name = "The Great Hypostyle Hall",     filepath = "data/tr4/data/hall.tr4",         picpath = "data/tr4/pix/karnak" };
gameflow_params[GAME_4].levels[09] = { name = "Sacred Lake",                  filepath = "data/tr4/data/lake.tr4",         picpath = "data/tr4/pix/karnak" };
gameflow_params[GAME_4].levels[10] = { name = "Yes",                          filepath = "data/tr4/data/lake.tr4",         picpath = "data/tr4/pix/karnak" }; -- YES - NoLevel
gameflow_params[GAME_4].levels[11] = { name = "Tomb Of Semerkhet",            filepath = "data/tr4/data/semer.tr4",        picpath = "data/tr4/pix/semer" };
gameflow_params[GAME_4].levels[12] = { name = "Guardian Of Semerkhet",        filepath = "data/tr4/data/semer2.tr4",       picpath = "data/tr4/pix/semer" };
gameflow_params[GAME_4].levels[13] = { name = "Desert Railroad",              filepath = "data/tr4/data/train.tr4",        picpath = "data/tr4/pix/train" };
gameflow_params[GAME_4].levels[14] = { name = "Alexandria",                   filepath = "data/tr4/data/alexhub.tr4",      picpath = "data/tr4/pix/alexandria" };
gameflow_params[GAME_4].levels[15] = { name = "Coastal Ruins",                filepath = "data/tr4/data/alexhub2.tr4",     picpath = "data/tr4/pix/alexandria" };
gameflow_params[GAME_4].levels[16] = { name = "Pharos, Temple Of Isis",       filepath = "data/tr4/data/palaces.tr4",      picpath = "data/tr4/pix/cleopal" };
gameflow_params[GAME_4].levels[17] = { name = "Cleopatra's Palaces",          filepath = "data/tr4/data/palaces2.tr4",     picpath = "data/tr4/pix/cleopal" };
gameflow_params[GAME_4].levels[18] = { name = "Catacombs",                    filepath = "data/tr4/data/csplit1.tr4",      picpath = "data/tr4/pix/catacomb" };
gameflow_params[GAME_4].levels[19] = { name = "Temple Of Poseidon",           filepath = "data/tr4/data/csplit2.tr4",      picpath = "data/tr4/pix/catacomb" };
gameflow_params[GAME_4].levels[20] = { name = "The Lost Library",             filepath = "data/tr4/data/library.tr4",      picpath = "data/tr4/pix/catacomb" };
gameflow_params[GAME_4].levels[21] = { name = "Hall Of Demetrius",            filepath = "data/tr4/data/libend.tr4",       picpath = "data/tr4/pix/catacomb" };
gameflow_params[GAME_4].levels[22] = { name = "City Of The Dead",             filepath = "data/tr4/data/bikebit.tr4",      picpath = "data/tr4/pix/cairo" };
gameflow_params[GAME_4].levels[23] = { name = "Trenches",                     filepath = "data/tr4/data/nutrench.tr4",     picpath = "data/tr4/pix/cairo" };
gameflow_params[GAME_4].levels[24] = { name = "Chambers Of Tulun",            filepath = "data/tr4/data/cortyard.tr4",     picpath = "data/tr4/pix/cairo" };
gameflow_params[GAME_4].levels[25] = { name = "Street Bazaar",                filepath = "data/tr4/data/lowstrt.tr4",      picpath = "data/tr4/pix/cairo" };
gameflow_params[GAME_4].levels[26] = { name = "Citadel Gate",                 filepath = "data/tr4/data/highstrt.tr4",     picpath = "data/tr4/pix/cairo" };
gameflow_params[GAME_4].levels[27] = { name = "Citadel",                      filepath = "data/tr4/data/citnew.tr4",       picpath = "data/tr4/pix/cairo" };
gameflow_params[GAME_4].levels[28] = { name = "The Sphinx Complex",           filepath = "data/tr4/data/joby1a.tr4",       picpath = "data/tr4/pix/pyramid" };
gameflow_params[GAME_4].levels[29] = { name = "No",                           filepath = "data/tr4/data/joby1a.tr4",       picpath = "data/tr4/pix/pyramid" };  -- NO - NoLevel
gameflow_params[GAME_4].levels[30] = { name = "Underneath The Sphinx",        filepath = "data/tr4/data/joby2.tr4",        picpath = "data/tr4/pix/pyramid" };
gameflow_params[GAME_4].levels[31] = { name = "Menkaure's Pyramid",           filepath = "data/tr4/data/joby3a.tr4",       picpath = "data/tr4/pix/pyramid" };
gameflow_params[GAME_4].levels[32] = { name = "Inside Menkaure's Pyramid",    filepath = "data/tr4/data/joby3b.tr4",       picpath = "data/tr4/pix/pyramid" };
gameflow_params[GAME_4].levels[33] = { name = "The Mastabas",                 filepath = "data/tr4/data/joby4a.tr4",       picpath = "data/tr4/pix/pyramid" };
gameflow_params[GAME_4].levels[34] = { name = "The Great Pyramid",            filepath = "data/tr4/data/joby4b.tr4",       picpath = "data/tr4/pix/pyramid" };
gameflow_params[GAME_4].levels[35] = { name = "Khufu's Queens Pyramids",      filepath = "data/tr4/data/joby4c.tr4",       picpath = "data/tr4/pix/pyramid" };
gameflow_params[GAME_4].levels[36] = { name = "Inside The Great Pyramid",     filepath = "data/tr4/data/joby5a.tr4",       picpath = "data/tr4/pix/pyramid" };
gameflow_params[GAME_4].levels[37] = { name = "Temple Of Horus",              filepath = "data/tr4/data/joby5b.tr4",       picpath = "data/tr4/pix/horus" };
gameflow_params[GAME_4].levels[38] = { name = "Temple Of Horus",              filepath = "data/tr4/data/joby5c.tr4",       picpath = "data/tr4/pix/horus" };
--gameflow_params[GAME_4].levels[38] = { name = "The Times Exclusive",        filepath = "data/tr4/data/times.tr4",        picpath = "data/tr4/pix/times" };
--gameflow_params[GAME_4].levels[39] = { name = "The Valley Temple",          filepath = "data/tr4/data/joby1b.tr4",       picpath = "data/tr4/pix/pyramid" };
---------------------------------------------------------------------------------------------------------------------------------------------------------
