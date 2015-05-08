-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: IV
-- Version: 1.0
-- By: Gh0stBlade

---------------------------------------------------------------------------------------------------------------------------------------------------------

gameflow_paths[GAME_4].numlevels = 38;

---------------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
---------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_4].level = {};
---------------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
---------------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                             [1]    Level Name                      [2] Level File Path                          [3] Level Load Screen Path
---------------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_4].level[01] = { name = "Angkor Wat",                   filepath = "data/tr4/data/angkor1.tr4",      picpath = "data/tr4/pix/cambodia.png" };
gameflow_paths[GAME_4].level[02] = { name = "Race For The Iris",            filepath = "data/tr4/data/ang_race.tr4",     picpath = "data/tr4/pix/cambodia.png" };
gameflow_paths[GAME_4].level[03] = { name = "The Tomb Of Seth",             filepath = "data/tr4/data/settomb1.tr4",     picpath = "data/tr4/pix/settomb.png" };
gameflow_paths[GAME_4].level[04] = { name = "Burial Chambers",              filepath = "data/tr4/data/settomb2.tr4",     picpath = "data/tr4/pix/settomb.png" };
gameflow_paths[GAME_4].level[05] = { name = "Valley Of The Kings",          filepath = "data/tr4/data/jeepchas.tr4",     picpath = "data/tr4/pix/valley.png" };
gameflow_paths[GAME_4].level[06] = { name = "KV5",                          filepath = "data/tr4/data/jeepchs2.tr4",     picpath = "data/tr4/pix/valley.png" };
gameflow_paths[GAME_4].level[07] = { name = "Temple Of Karnak",             filepath = "data/tr4/data/karnak1.tr4",      picpath = "data/tr4/pix/karnak.png" };
gameflow_paths[GAME_4].level[08] = { name = "The Great Hypostyle Hall",     filepath = "data/tr4/data/hall.tr4",         picpath = "data/tr4/pix/karnak.png" };
gameflow_paths[GAME_4].level[09] = { name = "Sacred Lake",                  filepath = "data/tr4/data/lake.tr4",         picpath = "data/tr4/pix/karnak.png" };
gameflow_paths[GAME_4].level[10] = { name = "Yes",                       filepath = "data/tr4/data/lake.tr4",         picpath = "data/tr4/pix/karnak.png" }; -- YES - NoLevel
gameflow_paths[GAME_4].level[11] = { name = "Tomb Of Semerkhet",            filepath = "data/tr4/data/semer.tr4",        picpath = "data/tr4/pix/semer.png" };
gameflow_paths[GAME_4].level[12] = { name = "Guardian Of Semerkhet",        filepath = "data/tr4/data/semer2.tr4",       picpath = "data/tr4/pix/semer.png" };
gameflow_paths[GAME_4].level[13] = { name = "Desert Railroad",              filepath = "data/tr4/data/train.tr4",        picpath = "data/tr4/pix/train.png" };
gameflow_paths[GAME_4].level[14] = { name = "Alexandria",                   filepath = "data/tr4/data/alexhub.tr4",      picpath = "data/tr4/pix/alexandria.png" };
gameflow_paths[GAME_4].level[15] = { name = "Coastal Ruins",                filepath = "data/tr4/data/alexhub2.tr4",     picpath = "data/tr4/pix/alexandria.png" };
gameflow_paths[GAME_4].level[16] = { name = "Pharos, Temple Of Isis",       filepath = "data/tr4/data/palaces.tr4",      picpath = "data/tr4/pix/cleopal.png" };
gameflow_paths[GAME_4].level[17] = { name = "Cleopatra's Palaces",          filepath = "data/tr4/data/palaces2.tr4",     picpath = "data/tr4/pix/cleopal.png" };
gameflow_paths[GAME_4].level[18] = { name = "Catacombs",                    filepath = "data/tr4/data/csplit1.tr4",      picpath = "data/tr4/pix/catacomb.png" };
gameflow_paths[GAME_4].level[19] = { name = "Temple Of Poseidon",           filepath = "data/tr4/data/csplit2.tr4",      picpath = "data/tr4/pix/catacomb.png" };
gameflow_paths[GAME_4].level[20] = { name = "The Lost Library",             filepath = "data/tr4/data/library.tr4",      picpath = "data/tr4/pix/catacomb.png" };
gameflow_paths[GAME_4].level[21] = { name = "Hall Of Demetrius",            filepath = "data/tr4/data/libend.tr4",       picpath = "data/tr4/pix/catacomb.png" };
gameflow_paths[GAME_4].level[22] = { name = "City Of The Dead",             filepath = "data/tr4/data/bikebit.tr4",      picpath = "data/tr4/pix/cairo.png" };
gameflow_paths[GAME_4].level[23] = { name = "Trenches",                     filepath = "data/tr4/data/nutrench.tr4",     picpath = "data/tr4/pix/cairo.png" };
gameflow_paths[GAME_4].level[24] = { name = "Chambers Of Tulun",            filepath = "data/tr4/data/cortyard.tr4",     picpath = "data/tr4/pix/cairo.png" };
gameflow_paths[GAME_4].level[25] = { name = "Street Bazaar",                filepath = "data/tr4/data/lowstrt.tr4",      picpath = "data/tr4/pix/cairo.png" };
gameflow_paths[GAME_4].level[26] = { name = "Citadel Gate",                 filepath = "data/tr4/data/highstrt.tr4",     picpath = "data/tr4/pix/cairo.png" };
gameflow_paths[GAME_4].level[27] = { name = "Citadel",                      filepath = "data/tr4/data/citnew.tr4",       picpath = "data/tr4/pix/cairo.png" };
gameflow_paths[GAME_4].level[28] = { name = "The Sphinx Complex",           filepath = "data/tr4/data/joby1a.tr4",       picpath = "data/tr4/pix/pyramid.png" };
gameflow_paths[GAME_4].level[29] = { name = "No",                        filepath = "data/tr4/data/joby1a.tr4",       picpath = "data/tr4/pix/pyramid.png" };  -- NO - NoLevel
gameflow_paths[GAME_4].level[30] = { name = "Underneath The Sphinx",        filepath = "data/tr4/data/joby2.tr4",        picpath = "data/tr4/pix/pyramid.png" };
gameflow_paths[GAME_4].level[31] = { name = "Menkaure's Pyramid",           filepath = "data/tr4/data/joby3a.tr4",       picpath = "data/tr4/pix/pyramid.png" };
gameflow_paths[GAME_4].level[32] = { name = "Inside Menkaure's Pyramid",    filepath = "data/tr4/data/joby3b.tr4",       picpath = "data/tr4/pix/pyramid.png" };
gameflow_paths[GAME_4].level[33] = { name = "The Mastabas",                 filepath = "data/tr4/data/joby4a.tr4",       picpath = "data/tr4/pix/pyramid.png" };
gameflow_paths[GAME_4].level[34] = { name = "The Great Pyramid",            filepath = "data/tr4/data/joby4b.tr4",       picpath = "data/tr4/pix/pyramid.png" };
gameflow_paths[GAME_4].level[35] = { name = "Khufu's Queens Pyramids",      filepath = "data/tr4/data/joby4c.tr4",       picpath = "data/tr4/pix/pyramid.png" };
gameflow_paths[GAME_4].level[36] = { name = "Inside The Great Pyramid",     filepath = "data/tr4/data/joby5a.tr4",       picpath = "data/tr4/pix/pyramid.png" };
gameflow_paths[GAME_4].level[37] = { name = "Temple Of Horus",              filepath = "data/tr4/data/joby5b.tr4",       picpath = "data/tr4/pix/horus.png" };
gameflow_paths[GAME_4].level[38] = { name = "Temple Of Horus",              filepath = "data/tr4/data/joby5c.tr4",       picpath = "data/tr4/pix/horus.png" };
--gameflow_paths[GAME_4].level[38] = { name = "The Times Exclusive",          filepath = "data/tr4/data/times.tr4",        picpath = "data/tr4/pix/times.png" };
--gameflow_paths[GAME_4].level[39] = { name = "The Valley Temple",            filepath = "data/tr4/data/joby1b.tr4",       picpath = "data/tr4/pix/pyramid.png" };
---------------------------------------------------------------------------------------------------------------------------------------------------------
