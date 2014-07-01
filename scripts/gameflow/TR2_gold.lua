-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: II Gold
-- Version: 1.0
-- By: Lmwte

------------------------------------------------------------------------------------------------------------------------------------------------------

basepath = "data/tr2_gold/";
numlevels = 5;

------------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
------------------------------------------------------------------------------------------------------------------------------------------------------
level = {};
------------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
------------------------------------------------------------------------------------------------------------------------------------------------------
--Array		[1]	Level Name							[2] Level File Path									[3] Level Load Screen Path
------------------------------------------------------------------------------------------------------------------------------------------------------
level[01] =  { name = "The Cold War",   			filepath = basepath .. "data/level1.tr2",  			picpath = basepath .. "pix/aleutian.png"   	};
level[02] =  { name = "Fool's Gold",				filepath = basepath .. "data/level2.tr2",           picpath = basepath .. "pix/aleutian.png"    };
level[03] =  { name = "Furnace of the Gods",		filepath = basepath .. "data/level3.tr2",  	        picpath = basepath .. "pix/aleutian.png"  	};
level[04] =  { name = "Kingdom",					filepath = basepath .. "data/level4.tr2",  	    	picpath = basepath .. "pix/aleutian.png"  	};
level[05] =  { name = "Nightmare in Vegas", 		filepath = basepath .. "data/level5.tr2",  	    	picpath = basepath .. "pix/vegas.png"  		};
------------------------------------------------------------------------------------------------------------------------------------------------------
