-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: V
-- Version: 1.0
-- By: Gh0stBlade

----------------------------------------------------------------------------------------------------------------------------------------------

basepath = "data/tr5/";
numlevels = 17;

----------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
----------------------------------------------------------------------------------------------------------------------------------------------
level = {};
----------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
----------------------------------------------------------------------------------------------------------------------------------------------
--Array		[1]	Level Name						[2] Level File Path								[3] Level Load Screen Path
----------------------------------------------------------------------------------------------------------------------------------------------
level[01] = { name = "Streets of Rome",			filepath = basepath .. "data/Andrea1.trc",      picpath = basepath .. "pix/screen001.jpg"	};
level[02] = { name = "Trajan's markets",		filepath = basepath .. "data/Andrea2.trc",  	picpath = basepath .. "pix/screen002.jpg"  	};
level[03] = { name = "The Colosseum",			filepath = basepath .. "data/Andrea3.trc",  	picpath = basepath .. "pix/screen003.jpg"  	};
level[04] = { name = "The Base", 				filepath = basepath .. "data/joby2.trc",  	    picpath = basepath .. "pix/screen004.jpg"   };
level[05] = { name = "The Submarine", 	    	filepath = basepath .. "data/joby3.trc",  		picpath = basepath .. "pix/screen005.jpg"	};
level[06] = { name = "Deepsea Dive",            filepath = basepath .. "data/joby4.trc",  		picpath = basepath .. "pix/screen006.jpg"	};
level[07] = { name = "Sinking Submarine",       filepath = basepath .. "data/joby5.trc",  	    picpath = basepath .. "pix/screen007.jpg" 	};
level[08] = { name = "Gallows Tree",        	filepath = basepath .. "data/andy1.trc",     	picpath = basepath .. "pix/screen008.jpg"   };
level[09] = { name = "Labyrinth",           	filepath = basepath .. "data/Andy2.trc",  	    picpath = basepath .. "pix/screen009.jpg"   };
level[10] = { name = "Old Mill",           		filepath = basepath .. "data/andy3.trc",  	    picpath = basepath .. "pix/screen010.jpg"   };
level[11] = { name = "The 13th Floor",          filepath = basepath .. "data/rich1.trc",  	    picpath = basepath .. "pix/screen011.jpg"   };
level[12] = { name = "Escape with the Iris",    filepath = basepath .. "data/rich2.trc",  	    picpath = basepath .. "pix/screen012.jpg"   };
level[13] = { name = "Security Breach",         filepath = basepath .. "data/richcut2.trc",  	picpath = basepath .. "pix/screen013.jpg"   };
level[14] = { name = "Red Alert!",          	filepath = basepath .. "data/rich3.trc",  	    picpath = basepath .. "pix/screen013.jpg"   };

level[15] = { name = "Gibby's level",          	filepath = basepath .. "data/gibby.trc",  	    picpath = basepath .. "pix/screen013.jpg" 	};
level[16] = { name = "Del's level",    			filepath = basepath .. "data/del.trc",  	    picpath = basepath .. "pix/screen013.jpg"   };
level[17] = { name = "Tom's level",        		filepath = basepath .. "data/tom.trc",  	    picpath = basepath .. "pix/screen013.jpg"   };

----------------------------------------------------------------------------------------------------------------------------------------------