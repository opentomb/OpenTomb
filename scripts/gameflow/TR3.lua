-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: III
-- Version: 1.0
-- By: Gh0stBlade

----------------------------------------------------------------------------------------------------------------------------------------------------

basepath = "data/tr3/";
numlevels = 20;

----------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
----------------------------------------------------------------------------------------------------------------------------------------------------
level = {};
----------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
----------------------------------------------------------------------------------------------------------------------------------------------------
--Array		[1]	Level Name						[2] Level File Path								[3] Level Load Screen Path
----------------------------------------------------------------------------------------------------------------------------------------------------
level[00] = { name = "Lara's House", 			filepath = basepath .. "data/HOUSE.TR2",  		picpath = basepath .. "pix/HOUSE.BMP"			  };
level[01] = { name = "Jungle",					filepath = basepath .. "data/JUNGLE.TR2",       picpath = basepath .. "pix/INDIA.BMP"        	  };
level[02] = { name = "Temple Ruins",			filepath = basepath .. "data/TEMPLE.TR2",  	    picpath = basepath .. "pix/INDIA.BMP" 	   		  };
level[03] = { name = "The River Ganges",		filepath = basepath .. "data/QUADCHAS.TR2",  	picpath = basepath .. "pix/INDIA.BMP" 	   		  };
level[04] = { name = "Caves Of Kaliya", 		filepath = basepath .. "data/TONYBOSS.TR2",  	picpath = basepath .. "pix/INDIA.BMP" 	    	  };
level[05] = { name = "Coastal Village", 	    filepath = basepath .. "data/SHORE.TR2",  		picpath = basepath .. "pix/SOUTHPAC.BMP"    	  };
level[06] = { name = "Crash Site",             	filepath = basepath .. "data/CRASH.TR2",  		picpath = basepath .. "pix/SOUTHPAC.BMP"    	  };
level[07] = { name = "Madubu Gorge",           	filepath = basepath .. "data/RAPIDS.TR2",  	    picpath = basepath .. "pix/SOUTHPAC.BMP"    	  };
level[08] = { name = "Temple Of Puna",         	filepath = basepath .. "data/TRIBOSS.TR2",     	picpath = basepath .. "pix/SOUTHPAC.BMP"    	  };
level[09] = { name = "Thames Wharf",           	filepath = basepath .. "data/ROOFS.TR2",  	    picpath = basepath .. "pix/LONDON.BMP"      	  };
level[10] = { name = "Aldwych",                	filepath = basepath .. "data/SEWER.TR2",  	    picpath = basepath .. "pix/LONDON.BMP"      	  };
level[11] = { name = "Lud's Gate",             	filepath = basepath .. "data/TOWER.TR2",  	    picpath = basepath .. "pix/LONDON.BMP"      	  };
level[12] = { name = "City",                   	filepath = basepath .. "data/OFFICE.TR2",  	    picpath = basepath .. "pix/LONDON.BMP"      	  };
level[13] = { name = "Nevada Desert",          	filepath = basepath .. "data/NEVADA.TR2",  	    picpath = basepath .. "pix/NEVADA.BMP"      	  };
level[14] = { name = "High Security Compound",  filepath = basepath .. "data/COMPOUND.TR2",  	picpath = basepath .. "pix/NEVADA.BMP"      	  };
level[15] = { name = "Area 51",          		filepath = basepath .. "data/AREA51.TR2",  	    picpath = basepath .. "pix/NEVADA.BMP"      	  };
level[16] = { name = "Antarctica",          	filepath = basepath .. "data/ANTARC.TR2",  	    picpath = basepath .. "pix/ANTARC.BMP"      	  };
level[17] = { name = "RX-Tech Mines",          	filepath = basepath .. "data/MINES.TR2",  	    picpath = basepath .. "pix/ANTARC.BMP"      	  };
level[18] = { name = "Lost City Of Tinnos",     filepath = basepath .. "data/CITY.TR2",  	    picpath = basepath .. "pix/ANTARC.BMP"      	  };
level[19] = { name = "Meteorite Cavern",        filepath = basepath .. "data/CHAMBER.TR2",  	picpath = basepath .. "pix/ANTARC.BMP"      	  };
level[20] = { name = "All Hallows",          	filepath = basepath .. "data/STPAUL.TR2",  	    picpath = basepath .. "pix/LONDON.BMP"      	  };
----------------------------------------------------------------------------------------------------------------------------------------------------


----------------------------------------------------------------------------------------------------------------------------------------------------
-- 3 - Define our fmv array to store our fmv information
----------------------------------------------------------------------------------------------------------------------------------------------------
fmv = {};
----------------------------------------------------------------------------------------------------------------------------------------------------
-- 4 - Assign our fmv information
----------------------------------------------------------------------------------------------------------------------------------------------------
--Array		[1]	FMV Name (Future Use)					[2]	FMV File Path
----------------------------------------------------------------------------------------------------------------------------------------------------
fmv[0] =  { name = "Logo", 								filepath = basepath .. "fmv/logo.rpl"													  };
fmv[1] =  { name = "Intro",								filepath = basepath .. "fmv/Intr_Eng.rpl"												  };
fmv[4] =  { name = "Sailing Beagle", 					filepath = basepath .. "fmv/Sail_Eng.rpl"  												  };
fmv[5] =  { name = "Antarctica Crash", 	    			filepath = basepath .. "fmv/Crsh_Eng.rpl"  												  };
fmv[6] =  { name = "End game",             				filepath = basepath .. "fmv/Endgame.rpl"  												  };
----------------------------------------------------------------------------------------------------------------------------------------------------


----------------------------------------------------------------------------------------------------------------------------------------------------
-- 5 - Define our cutscene array to store our cutscene information
----------------------------------------------------------------------------------------------------------------------------------------------------
cutscene = {};
----------------------------------------------------------------------------------------------------------------------------------------------------
-- 6 - Assign our cutscene information
----------------------------------------------------------------------------------------------------------------------------------------------------
--Array			[1]	Cutscene Name (Future Use)			[2]	Cutscene Path
----------------------------------------------------------------------------------------------------------------------------------------------------
cutscene[0] =  { name = "Cutscene 1", 					filepath = basepath .. "CUTS/CUT6.TR2"			   										  };
cutscene[1] =  { name = "Cutscene 2",					filepath = basepath .. "CUTS/CUT9.TR2"											      	  };
cutscene[2] =  { name = "Cutscene 3",					filepath = basepath .. "CUTS/CUT1.TR2"									          		  };	
cutscene[3] =  { name = "Cutscene 4",					filepath = basepath .. "CUTS/CUT4.TR2" 											  		  };
cutscene[4] =  { name = "Cutscene 5", 					filepath = basepath .. "CUTS/CUT2.TR2"  										  		  };
cutscene[5] =  { name = "Cutscene 6", 	    			filepath = basepath .. "CUTS/CUT5.TR2"	  										 		  };
cutscene[6] =  { name = "Cutscene 7",          			filepath = basepath .. "CUTS/CUT11.TR2"	  										  		  };
----------------------------------------------------------------------------------------------------------------------------------------------------