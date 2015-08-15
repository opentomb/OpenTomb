-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: III
-- Version: 1.0
-- By: Gh0stBlade

----------------------------------------------------------------------------------------------------------------------------------------------------

gameflow_paths[Game.III].numlevels = 20;

----------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
----------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[Game.III].level = {};
----------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
----------------------------------------------------------------------------------------------------------------------------------------------------
--Array                              [1]    Level Name                 [2] Level File Path                          [3] Level Load Screen Path
----------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[Game.III].level[00] = { name = "Lara's House",            filepath = "data/tr3/data/HOUSE.TR2",        picpath = "data/tr3/pix/HOUSE.BMP" };
gameflow_paths[Game.III].level[01] = { name = "Jungle",                  filepath = "data/tr3/data/JUNGLE.TR2",       picpath = "data/tr3/pix/INDIA.BMP" };
gameflow_paths[Game.III].level[02] = { name = "Temple Ruins",            filepath = "data/tr3/data/TEMPLE.TR2",       picpath = "data/tr3/pix/INDIA.BMP" };
gameflow_paths[Game.III].level[03] = { name = "The River Ganges",        filepath = "data/tr3/data/QUADCHAS.TR2",     picpath = "data/tr3/pix/INDIA.BMP" };
gameflow_paths[Game.III].level[04] = { name = "Caves Of Kaliya",         filepath = "data/tr3/data/TONYBOSS.TR2",     picpath = "data/tr3/pix/INDIA.BMP" };
gameflow_paths[Game.III].level[05] = { name = "Coastal Village",         filepath = "data/tr3/data/SHORE.TR2",        picpath = "data/tr3/pix/SOUTHPAC.BMP" };
gameflow_paths[Game.III].level[06] = { name = "Crash Site",              filepath = "data/tr3/data/CRASH.TR2",        picpath = "data/tr3/pix/SOUTHPAC.BMP" };
gameflow_paths[Game.III].level[07] = { name = "Madubu Gorge",            filepath = "data/tr3/data/RAPIDS.TR2",       picpath = "data/tr3/pix/SOUTHPAC.BMP" };
gameflow_paths[Game.III].level[08] = { name = "Temple Of Puna",          filepath = "data/tr3/data/TRIBOSS.TR2",      picpath = "data/tr3/pix/SOUTHPAC.BMP" };
gameflow_paths[Game.III].level[09] = { name = "Thames Wharf",            filepath = "data/tr3/data/ROOFS.TR2",        picpath = "data/tr3/pix/LONDON.BMP" };
gameflow_paths[Game.III].level[10] = { name = "Aldwych",                 filepath = "data/tr3/data/SEWER.TR2",        picpath = "data/tr3/pix/LONDON.BMP" };
gameflow_paths[Game.III].level[11] = { name = "Lud's Gate",              filepath = "data/tr3/data/TOWER.TR2",        picpath = "data/tr3/pix/LONDON.BMP" };
gameflow_paths[Game.III].level[12] = { name = "City",                    filepath = "data/tr3/data/OFFICE.TR2",       picpath = "data/tr3/pix/LONDON.BMP" };
gameflow_paths[Game.III].level[13] = { name = "Nevada Desert",           filepath = "data/tr3/data/NEVADA.TR2",       picpath = "data/tr3/pix/NEVADA.BMP" };
gameflow_paths[Game.III].level[14] = { name = "High Security Compound",  filepath = "data/tr3/data/COMPOUND.TR2",     picpath = "data/tr3/pix/NEVADA.BMP" };
gameflow_paths[Game.III].level[15] = { name = "Area 51",                 filepath = "data/tr3/data/AREA51.TR2",       picpath = "data/tr3/pix/NEVADA.BMP" };
gameflow_paths[Game.III].level[16] = { name = "Antarctica",              filepath = "data/tr3/data/ANTARC.TR2",       picpath = "data/tr3/pix/ANTARC.BMP" };
gameflow_paths[Game.III].level[17] = { name = "RX-Tech Mines",           filepath = "data/tr3/data/MINES.TR2",        picpath = "data/tr3/pix/ANTARC.BMP" };
gameflow_paths[Game.III].level[18] = { name = "Lost City Of Tinnos",     filepath = "data/tr3/data/CITY.TR2",         picpath = "data/tr3/pix/ANTARC.BMP" };
gameflow_paths[Game.III].level[19] = { name = "Meteorite Cavern",        filepath = "data/tr3/data/CHAMBER.TR2",      picpath = "data/tr3/pix/ANTARC.BMP" };
gameflow_paths[Game.III].level[20] = { name = "All Hallows",             filepath = "data/tr3/data/STPAUL.TR2",       picpath = "data/tr3/pix/LONDON.BMP" };
----------------------------------------------------------------------------------------------------------------------------------------------------


----------------------------------------------------------------------------------------------------------------------------------------------------
-- 3 - Define our fmv array to store our fmv information
----------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[Game.III].fmv = {};
----------------------------------------------------------------------------------------------------------------------------------------------------
-- 4 - Assign our fmv information
----------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                           [1]    FMV Name (Future Use)                 [2]        FMV File Path
----------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[Game.III].fmv[0] =  { name = "Logo",                               filepath = "data/tr3/fmv/logo.rpl" };
gameflow_paths[Game.III].fmv[1] =  { name = "Intro",                              filepath = "data/tr3/fmv/Intr_Eng.rpl" };
gameflow_paths[Game.III].fmv[4] =  { name = "Sailing Beagle",                     filepath = "data/tr3/fmv/Sail_Eng.rpl" };
gameflow_paths[Game.III].fmv[5] =  { name = "Antarctica Crash",                   filepath = "data/tr3/fmv/Crsh_Eng.rpl" };
gameflow_paths[Game.III].fmv[6] =  { name = "End game",                           filepath = "data/tr3/fmv/Endgame.rpl" };
----------------------------------------------------------------------------------------------------------------------------------------------------


----------------------------------------------------------------------------------------------------------------------------------------------------
-- 5 - Define our cutscene array to store our cutscene information
----------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[Game.III].cutscene = {};
----------------------------------------------------------------------------------------------------------------------------------------------------
-- 6 - Assign our cutscene information
----------------------------------------------------------------------------------------------------------------------------------------------------
-- Array                                [1]    Cutscene Name (Future Use)       [2]    Cutscene Path
----------------------------------------------------------------------------------------------------------------------------------------------------
gameflow_paths[Game.III].cutscene[0] =  { name = "Cutscene 1",                    filepath = "data/tr3/CUTS/CUT6.TR2" };
gameflow_paths[Game.III].cutscene[1] =  { name = "Cutscene 2",                    filepath = "data/tr3/CUTS/CUT9.TR2" };
gameflow_paths[Game.III].cutscene[2] =  { name = "Cutscene 3",                    filepath = "data/tr3/CUTS/CUT1.TR2" };    
gameflow_paths[Game.III].cutscene[3] =  { name = "Cutscene 4",                    filepath = "data/tr3/CUTS/CUT4.TR2" };
gameflow_paths[Game.III].cutscene[4] =  { name = "Cutscene 5",                    filepath = "data/tr3/CUTS/CUT2.TR2" };
gameflow_paths[Game.III].cutscene[5] =  { name = "Cutscene 6",                    filepath = "data/tr3/CUTS/CUT5.TR2" };
gameflow_paths[Game.III].cutscene[6] =  { name = "Cutscene 7",                    filepath = "data/tr3/CUTS/CUT11.TR2" };
----------------------------------------------------------------------------------------------------------------------------------------------------
