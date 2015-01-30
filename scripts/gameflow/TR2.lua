-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: II
-- Version: 1.0
-- By: Gh0stBlade

-----------------------------------------------------------------------------------------------------------------------------------------------

gameflow_paths[GAME_2].numlevels = 18;

-----------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
-----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_2].level = {};
-----------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
-----------------------------------------------------------------------------------------------------------------------------------------------
-- Array                             [1]     Level Name                [2] Level File Path                          [3] Level Load Screen Path
-----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_2].level[00] = { name = "Lara's Home",             filepath = "data/tr2/data/ASSAULT.TR2",      picpath = "data/tr2/pix/mansion.png" };
gameflow_paths[GAME_2].level[01] = { name = "The Great Wall",          filepath = "data/tr2/data/WALL.TR2",         picpath = "data/tr2/pix/china.png" };
gameflow_paths[GAME_2].level[02] = { name = "Venice",                  filepath = "data/tr2/data/BOAT.TR2",         picpath = "data/tr2/pix/venice.png" };
gameflow_paths[GAME_2].level[03] = { name = "Bartoli's Hideout",       filepath = "data/tr2/data/venice.TR2",       picpath = "data/tr2/pix/venice.png" };
gameflow_paths[GAME_2].level[04] = { name = "Opera House",             filepath = "data/tr2/data/OPERA.TR2",        picpath = "data/tr2/pix/venice.png" };
gameflow_paths[GAME_2].level[05] = { name = "Offshore Rig",            filepath = "data/tr2/data/RIG.TR2",          picpath = "data/tr2/pix/rig.png" };
gameflow_paths[GAME_2].level[06] = { name = "Diving Area",             filepath = "data/tr2/data/platform.TR2",     picpath = "data/tr2/pix/rig.png" };
gameflow_paths[GAME_2].level[07] = { name = "40 Fathoms",              filepath = "data/tr2/data/unwater.TR2",      picpath = "data/tr2/pix/titan.png" };
gameflow_paths[GAME_2].level[08] = { name = "Wreck of the Maria Doria",filepath = "data/tr2/data/keel.TR2",         picpath = "data/tr2/pix/titan.png" };
gameflow_paths[GAME_2].level[09] = { name = "Living Quarters",         filepath = "data/tr2/data/living.TR2",       picpath = "data/tr2/pix/titan.png" };
gameflow_paths[GAME_2].level[10] = { name = "The Deck",                filepath = "data/tr2/data/deck.TR2",         picpath = "data/tr2/pix/titan.png" };
gameflow_paths[GAME_2].level[11] = { name = "Tibetan Foothills",       filepath = "data/tr2/data/SKIDOO.TR2",       picpath = "data/tr2/pix/tibet.png" };
gameflow_paths[GAME_2].level[12] = { name = "Barkhang Monastery",      filepath = "data/tr2/data/MONASTRY.TR2",     picpath = "data/tr2/pix/tibet.png" };
gameflow_paths[GAME_2].level[13] = { name = "Catacombs of the Talion", filepath = "data/tr2/data/catacomb.TR2",     picpath = "data/tr2/pix/tibet.png" };
gameflow_paths[GAME_2].level[14] = { name = "Ice Palace",              filepath = "data/tr2/data/ICECAVE.TR2",      picpath = "data/tr2/pix/tibet.png" };
gameflow_paths[GAME_2].level[15] = { name = "Temple of Xian",          filepath = "data/tr2/data/Emprtomb.tr2",     picpath = "data/tr2/pix/china.png" };
gameflow_paths[GAME_2].level[16] = { name = "Floating Islands",        filepath = "data/tr2/data/FLOATING.TR2",     picpath = "data/tr2/pix/china.png" };
gameflow_paths[GAME_2].level[17] = { name = "The Dragon's Lair",       filepath = "data/tr2/data/xian.TR2",         picpath = "data/tr2/pix/china.png" };
gameflow_paths[GAME_2].level[18] = { name = "Home Sweet Home",         filepath = "data/tr2/data/house.TR2",        picpath = "data/tr2/pix/mansion.png" };
-----------------------------------------------------------------------------------------------------------------------------------------------



-----------------------------------------------------------------------------------------------------------------------------------------------
-- 3 - Define our fmv array to store our fmv information
-----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_2].fmv = {};
-----------------------------------------------------------------------------------------------------------------------------------------------
-- 4 - Assign our fmv information
-----------------------------------------------------------------------------------------------------------------------------------------------
-- Array                           [1]    FMV Name (Future Use)                [2]    FMV File Path
-----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_2].fmv[0] =  { name = "Logo",                              filepath = "data/tr2/fmv/LOGO.RPL" };
gameflow_paths[GAME_2].fmv[1] =  { name = "Ancient War",                       filepath = "data/tr2/fmv/ANCIENT.RPL" };
gameflow_paths[GAME_2].fmv[2] =  { name = "Helicopter Lara",                   filepath = "data/tr2/FMV/MODERN.RPL" };
gameflow_paths[GAME_2].fmv[3] =  { name = "Getting to Offshore Rig",           filepath = "data/tr2/FMV/LANDING.RPL" };
gameflow_paths[GAME_2].fmv[4] =  { name = "Diving",                            filepath = "data/tr2/FMV/MS.RPL" };
gameflow_paths[GAME_2].fmv[5] =  { name = "Arriving at Tibet",                 filepath = "data/tr2/FMV/CRASH.RPL" };
gameflow_paths[GAME_2].fmv[6] =  { name = "Chinese chase",                     filepath = "data/tr2/FMV/JEEP.RPL" };
gameflow_paths[GAME_2].fmv[7] =  { name = "End game",                          filepath = "data/tr2/FMV/END.RPL" };
-----------------------------------------------------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------------------------------------------------
-- 5 - Define our cutscene array to store our cutscene information
-----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_2].cutscene = {};
-----------------------------------------------------------------------------------------------------------------------------------------------
-- 6 - Assign our cutscene information
-----------------------------------------------------------------------------------------------------------------------------------------------
-- Array                                [1]    Cutscene Name (Future Use)       [2]    Cutscene Path
-----------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[GAME_2].cutscene[1] =  { name = "Cutscene 1",                    filepath = "data/tr2/data/CUT1.TR2" };
gameflow_paths[GAME_2].cutscene[2] =  { name = "Cutscene 2",                    filepath = "data/tr2/data/CUT2.TR2" };
gameflow_paths[GAME_2].cutscene[3] =  { name = "Cutscene 3",                    filepath = "data/tr2/data/CUT3.TR2" };
gameflow_paths[GAME_2].cutscene[4] =  { name = "Cutscene 4",                    filepath = "data/tr2/data/CUT4.TR2" };
-----------------------------------------------------------------------------------------------------------------------------------------------
