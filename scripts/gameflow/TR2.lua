-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: II
-- Version: 1.0
-- By: Gh0stBlade

-----------------------------------------------------------------------------------------------------------------------------------------------

basepath = "data/tr2/";
numlevels = 18;

-----------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
-----------------------------------------------------------------------------------------------------------------------------------------------
level = {};
-----------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
-----------------------------------------------------------------------------------------------------------------------------------------------
--Array		[1]	Level Name								[2] Level File Path										[3] Level Load Screen Path
-----------------------------------------------------------------------------------------------------------------------------------------------
level[00] = { name = "Lara's Home", 				filepath = basepath .. "data/ASSAULT.TR2",  	picpath = basepath .. "pix/mansion.png"	 };
level[01] = { name = "The Great Wall",				filepath = basepath .. "data/WALL.TR2",         picpath = basepath .. "pix/china.png"    };
level[02] = { name = "Venice",						filepath = basepath .. "data/BOAT.TR2",  	    picpath = basepath .. "pix/venice.png" 	 };
level[03] = { name = "Bartoli's Hideout",			filepath = basepath .. "data/venice.TR2",  	    picpath = basepath .. "pix/venice.png" 	 };
level[04] = { name = "Opera House", 				filepath = basepath .. "data/OPERA.TR2",  	    picpath = basepath .. "pix/venice.png"   };
level[05] = { name = "Offshore Rig", 	    		filepath = basepath .. "data/RIG.TR2",  		picpath = basepath .. "pix/rig.png"    	 };
level[06] = { name = "Diving Area",            		filepath = basepath .. "data/platform.TR2",  	picpath = basepath .. "pix/rig.png"    	 };
level[07] = { name = "40 Fathoms",           		filepath = basepath .. "data/unwater.TR2",  	picpath = basepath .. "pix/titan.png"    };
level[08] = { name = "Wreck of the Maria Doria",	filepath = basepath .. "data/keel.TR2",     	picpath = basepath .. "pix/titan.png"    };
level[09] = { name = "Living Quarters",           	filepath = basepath .. "data/living.TR2",  	    picpath = basepath .. "pix/titan.png"    };
level[10] = { name = "The Deck",                	filepath = basepath .. "data/deck.TR2",  	    picpath = basepath .. "pix/titan.png"    };
level[11] = { name = "Tibetan Foothills",          	filepath = basepath .. "data/SKIDOO.TR2",  	    picpath = basepath .. "pix/tibet.png"    };
level[12] = { name = "Barkhang Monastery",         	filepath = basepath .. "data/MONASTRY.TR2",  	picpath = basepath .. "pix/tibet.png"    };
level[13] = { name = "Catacombs of the Talion",    	filepath = basepath .. "data/catacomb.TR2",  	picpath = basepath .. "pix/tibet.png"    };
level[14] = { name = "Ice Palace",    				filepath = basepath .. "data/ICECAVE.TR2",  	picpath = basepath .. "pix/tibet.png"    };
level[15] = { name = "Temple of Xian",    			filepath = basepath .. "data/Emprtomb.tr2",  	picpath = basepath .. "pix/china.png"    };
level[16] = { name = "Floating Islands",    		filepath = basepath .. "data/FLOATING.TR2",  	picpath = basepath .. "pix/china.png"    };
level[17] = { name = "The Dragon's Lair",    		filepath = basepath .. "data/xian.TR2",  	   	picpath = basepath .. "pix/china.png"    };
level[18] = { name = "Home Sweet Home",    			filepath = basepath .. "data/house.TR2",  	    picpath = basepath .. "pix/mansion.png"  };
-----------------------------------------------------------------------------------------------------------------------------------------------



-----------------------------------------------------------------------------------------------------------------------------------------------
-- 3 - Define our fmv array to store our fmv information
-----------------------------------------------------------------------------------------------------------------------------------------------
fmv = {};
-----------------------------------------------------------------------------------------------------------------------------------------------
-- 4 - Assign our fmv information
-----------------------------------------------------------------------------------------------------------------------------------------------
--Array		[1]	FMV Name (Future Use)					[2]	FMV File Path
-----------------------------------------------------------------------------------------------------------------------------------------------
fmv[0] =  { name = "Logo", 								filepath = basepath .. "fmv/LOGO.RPL"												 };
fmv[1] =  { name = "Ancient War",						filepath = basepath .. "fmv/ANCIENT.RPL"											 };
fmv[2] =  { name = "Helicopter Lara",					filepath = basepath .. "FMV/MODERN.RPL"												 };
fmv[3] =  { name = "Getting to Offshore Rig",			filepath = basepath .. "FMV/LANDING.RPL"  											 };
fmv[4] =  { name = "Diving", 							filepath = basepath .. "FMV/MS.RPL"  												 };
fmv[5] =  { name = "Arriving at Tibet", 	    		filepath = basepath .. "FMV/CRASH.RPL"  											 };
fmv[6] =  { name = "Chinese chase",             		filepath = basepath .. "FMV/JEEP.RPL"  												 };
fmv[7] =  { name = "End game",             				filepath = basepath .. "FMV/END.RPL"  												 };
-----------------------------------------------------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------------------------------------------------
-- 5 - Define our cutscene array to store our cutscene information
-----------------------------------------------------------------------------------------------------------------------------------------------
cutscene = {};
-----------------------------------------------------------------------------------------------------------------------------------------------
-- 6 - Assign our cutscene information
-----------------------------------------------------------------------------------------------------------------------------------------------
--Array		[1]	Cutscene Name (Future Use)				[2]	Cutscene Path
-----------------------------------------------------------------------------------------------------------------------------------------------
cutscene[1] =  { name = "Cutscene 1", 					filepath = basepath .. "data/CUT1.TR2"												  };
cutscene[2] =  { name = "Cutscene 2",					filepath = basepath .. "data/CUT2.TR2"												  };
cutscene[3] =  { name = "Cutscene 3",					filepath = basepath .. "data/CUT3.TR2"												  };
cutscene[4] =  { name = "Cutscene 4",					filepath = basepath .. "data/CUT4.TR2" 												  };
-----------------------------------------------------------------------------------------------------------------------------------------------