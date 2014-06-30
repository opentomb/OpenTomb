-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: I
-- Version: 1.0
-- By: Gh0stBlade

------------------------------------------------------------------------------------------------------------------------------------------------------------------

basepath = "data/tr1/";
numlevels = 15;

------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
------------------------------------------------------------------------------------------------------------------------------------------------------------------
level = {};
------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------------------
--Array		[1]	Level Name								[2] Level File Path										[3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------------------
level[00] = { name = "Lara's Home",				filepath = basepath .. "data/GYM.PHD",         	picpath = basepath .. "pix/gymloa.png"   };
level[01] = { name = "Caves",					filepath = basepath .. "data/LEVEL1.PHD",       picpath = basepath .. "pix/aztecloa.png" };
level[02] = { name = "City of Vilcabamba",		filepath = basepath .. "data/LEVEL2.PHD",  	    picpath = basepath .. "pix/aztecloa.png" };
level[03] = { name = "Lost Valley",				filepath = basepath .. "data/LEVEL3A.PHD",  	picpath = basepath .. "pix/aztecloa.png" };
level[04] = { name = "Tomb of Qualopec", 		filepath = basepath .. "data/LEVEL3B.PHD",  	picpath = basepath .. "pix/aztecloa.png" };
level[05] = { name = "St. Francis' Folly", 		filepath = basepath .. "data/LEVEL4.PHD",  		picpath = basepath .. "pix/greekloa.png" };
level[06] = { name = "Colosseum",            	filepath = basepath .. "data/LEVEL5.PHD",  		picpath = basepath .. "pix/greekloa.png" };
level[07] = { name = "Palace Midas",           	filepath = basepath .. "data/LEVEL6.PHD",  	    picpath = basepath .. "pix/greekloa.png" };
level[08] = { name = "The Cistern",        		filepath = basepath .. "data/LEVEL7A.PHD",     	picpath = basepath .. "pix/greekloa.png" };
level[09] = { name = "Tomb of Tihocan",         filepath = basepath .. "data/LEVEL7B.PHD",  	picpath = basepath .. "pix/greekloa.png" };
level[10] = { name = "City of Khamoon",         filepath = basepath .. "data/LEVEL8A.PHD",  	picpath = basepath .. "pix/egyptloa.png" };
level[11] = { name = "Obelisk of Khamoon",      filepath = basepath .. "data/LEVEL8B.PHD",  	picpath = basepath .. "pix/egyptloa.png" };
level[12] = { name = "Sanctuary of the Scion",  filepath = basepath .. "data/LEVEL8C.PHD",  	picpath = basepath .. "pix/egyptloa.png" };
level[13] = { name = "Natla's Mines",           filepath = basepath .. "data/LEVEL10A.PHD",  	picpath = basepath .. "pix/atlanloa.png" };
level[14] = { name = "Atlantis",          		filepath = basepath .. "data/LEVEL10B.PHD",  	picpath = basepath .. "pix/atlanloa.png" };
level[15] = { name = "The Great Pyramid",       filepath = basepath .. "data/LEVEL10C.PHD",  	picpath = basepath .. "pix/atlanloa.png" };
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
