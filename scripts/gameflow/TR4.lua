-- Gameflow Script for OpenTomb
-- Game: Tomb Raider: IV
-- Version: 1.0
-- By: Gh0stBlade

---------------------------------------------------------------------------------------------------------------------------------------------------------

basepath = "DATA/TR4/";
numlevels = 38;

---------------------------------------------------------------------------------------------------------------------------------------------------------
-- 1 - Define our level array to store our level information
---------------------------------------------------------------------------------------------------------------------------------------------------------
level = {};
---------------------------------------------------------------------------------------------------------------------------------------------------------
-- 2 - Assign our level information
---------------------------------------------------------------------------------------------------------------------------------------------------------
--Array		[1]	Level Name							[2] Level File Path									[3] Level Load Screen Path
---------------------------------------------------------------------------------------------------------------------------------------------------------
level[01] = { name = "Angkor Wat",					filepath = basepath .. "data/angkor1.tr4",         	picpath = basepath .. "pix/cambodia.png"       	};
level[02] = { name = "Race For The Iris",			filepath = basepath .. "data/ang_race.tr4",  	    picpath = basepath .. "pix/cambodia.png"        };
level[03] = { name = "The Tomb Of Seth",			filepath = basepath .. "data/settomb1.tr4",  	    picpath = basepath .. "pix/settomb.png" 	 	};
level[04] = { name = "Burial Chambers", 			filepath = basepath .. "data/settomb2.tr4",  	    picpath = basepath .. "pix/settomb.png" 	    };
level[05] = { name = "Valley Of The Kings", 	    filepath = basepath .. "data/jeepchas.tr4",  		picpath = basepath .. "pix/valley.png"      	};
level[06] = { name = "KV5",             			filepath = basepath .. "data/jeepchs2.tr4",  		picpath = basepath .. "pix/valley.png"    	    };
level[07] = { name = "Temple Of Karnak",           	filepath = basepath .. "data/karnak1.tr4",  	    picpath = basepath .. "pix/karnak.png"      	};
level[08] = { name = "The Great Hypostyle Hall",    filepath = basepath .. "data/hall.tr4",     		picpath = basepath .. "pix/karnak.png"      	};
level[09] = { name = "Sacred Lake",           		filepath = basepath .. "data/lake.tr4",  	    	picpath = basepath .. "pix/karnak.png"      	};
level[10] = { name = "Tomb Of Semerkhet",           filepath = basepath .. "data/semer.tr4",  	    	picpath = basepath .. "pix/semer.png"       	};
level[11] = { name = "Guardian Of Semerkhet",       filepath = basepath .. "data/semer2.tr4",  	    	picpath = basepath .. "pix/semer.png"       	};
level[12] = { name = "Desert Railroad",             filepath = basepath .. "data/train.tr4",  	    	picpath = basepath .. "pix/train.png"       	};
level[13] = { name = "Alexandria",          		filepath = basepath .. "data/alexhub.tr4",  	    picpath = basepath .. "pix/alexandria.png"     	};
level[14] = { name = "Coastal Ruins",          		filepath = basepath .. "data/alexhub2.tr4",  	    picpath = basepath .. "pix/alexandria.png"     	};
level[15] = { name = "Pharos, Temple Of Isis",     	filepath = basepath .. "data/palaces.tr4",  	    picpath = basepath .. "pix/cleopal.png"     	};
level[16] = { name = "Cleopatra's Palaces",        	filepath = basepath .. "data/palaces2.tr4",  	    picpath = basepath .. "pix/cleopal.png"      	};
level[17] = { name = "Catacombs",          			filepath = basepath .. "data/csplit1.tr4",  	    picpath = basepath .. "pix/catacomb.png"      	};
level[18] = { name = "Temple Of Poseidon",          filepath = basepath .. "data/csplit2.tr4",  	    picpath = basepath .. "pix/catacomb.png"      	};
level[19] = { name = "The Lost Library",          	filepath = basepath .. "data/library.tr4",  	    picpath = basepath .. "pix/catacomb.png"      	};
level[20] = { name = "Hall Of Demetrius",          	filepath = basepath .. "data/libend.tr4",  	    	picpath = basepath .. "pix/catacomb.png"      	};
level[21] = { name = "City Of The Dead",          	filepath = basepath .. "data/bikebit.tr4",  	    picpath = basepath .. "pix/cairo.png"       	};
level[22] = { name = "Trenches",          			filepath = basepath .. "data/nutrench.tr4",  	    picpath = basepath .. "pix/cairo.png"       	};
level[23] = { name = "Chambers Of Tulun",          	filepath = basepath .. "data/cortyard.tr4",  	    picpath = basepath .. "pix/cairo.png"       	};
level[24] = { name = "Street Bazaar",          		filepath = basepath .. "data/lowstrt.tr4",  	    picpath = basepath .. "pix/cairo.png"       	};
level[25] = { name = "Citadel Gate",          		filepath = basepath .. "data/highstrt.tr4",  	    picpath = basepath .. "pix/cairo.png"       	};
level[26] = { name = "Citadel",          			filepath = basepath .. "data/citnew.tr4",  	    	picpath = basepath .. "pix/cairo.png"       	};
level[27] = { name = "The Sphinx Complex",          filepath = basepath .. "data/joby1a.tr4",  	    	picpath = basepath .. "pix/pyramid.png"      	};
level[28] = { name = "Underneath The Sphinx",       filepath = basepath .. "data/joby2.tr4",  	    	picpath = basepath .. "pix/pyramid.png"      	};
level[29] = { name = "Menkaure's Pyramid",          filepath = basepath .. "data/joby3a.tr4",  	    	picpath = basepath .. "pix/pyramid.png"      	};
level[30] = { name = "Inside Menkaure's Pyramid",   filepath = basepath .. "data/joby3b.tr4",  	    	picpath = basepath .. "pix/pyramid.png"      	};
level[31] = { name = "The Mastabas",          		filepath = basepath .. "data/joby4a.tr4",  	    	picpath = basepath .. "pix/pyramid.png"      	};
level[32] = { name = "The Great Pyramid",          	filepath = basepath .. "data/joby4b.tr4",  	    	picpath = basepath .. "pix/pyramid.png"      	};
level[33] = { name = "Khufu's Queens Pyramids",     filepath = basepath .. "data/joby4c.tr4",  	    	picpath = basepath .. "pix/pyramid.png"      	};
level[34] = { name = "Inside The Great Pyramid",    filepath = basepath .. "data/joby5a.tr4",  	    	picpath = basepath .. "pix/pyramid.png"      	};
level[35] = { name = "Temple Of Horus",          	filepath = basepath .. "data/joby5b.tr4",  	    	picpath = basepath .. "pix/horus.png"       	};
level[36] = { name = "Temple Of Horus",          	filepath = basepath .. "data/joby5c.tr4",  	    	picpath = basepath .. "pix/horus.png"       	};
level[37] = { name = "The Times Exclusive",         filepath = basepath .. "data/times.tr4",  	    	picpath = basepath .. "pix/times.png"       	};

level[38] = { name = "The Valley Temple",         	filepath = basepath .. "data/joby1b.tr4",  	    	picpath = basepath .. "pix/pyramid.png"      	};
---------------------------------------------------------------------------------------------------------------------------------------------------------