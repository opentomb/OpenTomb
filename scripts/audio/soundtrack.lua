-- OPENTOMB SOUNDTRACK MAPPER SCRIPT
-- by Lwmte, Apr 2014

--------------------------------------------------------------------------------
-- TR1/2 CD triggers contain indexes that refer to PSX version CD tracks;
-- however, in PC version, tracks were drastically rearranged. Additionally,
-- certain TR1 PC versions have extra ambience tracks and remapped secret
-- audiotrack, which also complicates things a lot. For all these reasons,
-- we need to remap audiotracks via script.

-- For TR3, CDAUDIO.WAD support is forthcoming. Simply copy AUDIO folder from
-- your TR3 directory into OpenTomb/data/tr3/ directory.

-- For TR4, do the same thing.

-- P.S.: It is recommended to replace original TR1 and TR3 soundtracks with
-- beautiful remastered versions by tomekkobialka.
-- TR1 remastered: http://www.tombraiderforums.com/showthread.php?t=187800
-- TR3 remastered: http://www.tombraiderforums.com/showthread.php?t=185513

--------------------------------------------------------------------------------
--  IF YOU WANT TO USE ORIGINAL TR3 TRACK NAME BINDINGS INSTEAD OF REMASTER, SET
--  UNDERLYING VARIABLE TO ZERO.
--------------------------------------------------------------------------------
USE_TR3_REMASTER = 0;
--------------------------------------------------------------------------------

-- Channel modes: 

LOOP = 0x0000;  -- Looped (background)
ONCE = 0x0001;  -- One-shot (action music)
CHAT = 0x0002;  -- Voice (Lara chatting, etc.)

-- Load methods:

TRACK  = 0x0000;  -- Separate audio track.
WAD    = 0x0001;  -- WAD file (CDAUDIO.WAD).

-- Secret track constants(used with SECRET trigger):

SECRET_TR1  = 13 ;
SECRET_TR2  = 47 ;
SECRET_TR3  = 122;
SECRET_TR4  = 5  ;
SECRET_TR5  = 6  ;


print("Soundtrack mapper script loaded");

--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 1 ---------------------------------
--------------------------------------------------------------------------------

tr1_num_soundtracks = 61;
tr1_track_tbl       = {};

tr1_track_tbl[002] = {file = "002", mode = LOOP}; -- Main theme
tr1_track_tbl[003] = {file = "011", mode = ONCE}; -- Poseidon
tr1_track_tbl[004] = {file = "012", mode = ONCE}; -- Main theme, short
tr1_track_tbl[005] = {file = "003", mode = LOOP}; -- Caves ambience
tr1_track_tbl[006] = {file = "013", mode = ONCE}; -- Thor
tr1_track_tbl[007] = {file = "014", mode = ONCE}; -- St. Francis
tr1_track_tbl[008] = {file = "015", mode = ONCE}; -- Danger
tr1_track_tbl[009] = {file = "016", mode = ONCE}; -- Stairs
tr1_track_tbl[010] = {file = "017", mode = ONCE}; -- Midas
tr1_track_tbl[011] = {file = "018", mode = ONCE}; -- Lever
tr1_track_tbl[012] = {file = "019", mode = ONCE}; -- Hmm...
tr1_track_tbl[013] = {file = "060", mode = ONCE}; -- Secret theme 
tr1_track_tbl[014] = {file = "020", mode = ONCE}; -- Big secret theme
tr1_track_tbl[015] = {file = "021", mode = ONCE}; -- Raiders
tr1_track_tbl[016] = {file = "022", mode = ONCE}; -- Wolf
tr1_track_tbl[017] = {file = "023", mode = ONCE}; -- Awe
tr1_track_tbl[018] = {file = "024", mode = ONCE}; -- Gods
tr1_track_tbl[019] = {file = "025", mode = ONCE}; -- Main theme, reprise
tr1_track_tbl[020] = {file = "026", mode = ONCE}; -- Mummy
tr1_track_tbl[021] = {file = "027", mode = ONCE}; -- Midas, reprise
tr1_track_tbl[022] = {file = "007", mode = ONCE}; -- Natla cutscene
tr1_track_tbl[023] = {file = "008", mode = ONCE}; -- Larson cutscene
tr1_track_tbl[024] = {file = "009", mode = ONCE}; -- Natla scion cutscene
tr1_track_tbl[025] = {file = "010", mode = ONCE}; -- Tihocan cutscene
tr1_track_tbl[026] = {file = "029", mode = CHAT}; -- Home block begin
tr1_track_tbl[027] = {file = "030", mode = CHAT};
tr1_track_tbl[028] = {file = "031", mode = CHAT};
tr1_track_tbl[029] = {file = "032", mode = CHAT};
tr1_track_tbl[030] = {file = "033", mode = CHAT};
tr1_track_tbl[031] = {file = "034", mode = CHAT};
tr1_track_tbl[032] = {file = "035", mode = CHAT};
tr1_track_tbl[033] = {file = "036", mode = CHAT};
tr1_track_tbl[034] = {file = "037", mode = CHAT};
tr1_track_tbl[035] = {file = "038", mode = CHAT};
tr1_track_tbl[036] = {file = "039", mode = CHAT};
tr1_track_tbl[037] = {file = "040", mode = CHAT};
tr1_track_tbl[038] = {file = "041", mode = CHAT};
tr1_track_tbl[039] = {file = "042", mode = CHAT};
tr1_track_tbl[040] = {file = "043", mode = CHAT};
tr1_track_tbl[041] = {file = "044", mode = CHAT};
tr1_track_tbl[042] = {file = "045", mode = CHAT};
tr1_track_tbl[043] = {file = "046", mode = CHAT};
tr1_track_tbl[044] = {file = "047", mode = CHAT};
tr1_track_tbl[045] = {file = "048", mode = CHAT};
tr1_track_tbl[046] = {file = "049", mode = CHAT};
tr1_track_tbl[047] = {file = "050", mode = CHAT};
tr1_track_tbl[048] = {file = "051", mode = CHAT};
tr1_track_tbl[049] = {file = "052", mode = CHAT};
tr1_track_tbl[050] = {file = "053", mode = CHAT};
tr1_track_tbl[051] = {file = "054", mode = CHAT}; -- Baddy 1
tr1_track_tbl[052] = {file = "055", mode = CHAT}; -- Baddy 2
tr1_track_tbl[053] = {file = "056", mode = CHAT}; -- Larson
tr1_track_tbl[054] = {file = "057", mode = CHAT}; -- Natla
tr1_track_tbl[055] = {file = "058", mode = CHAT}; -- Pierre
tr1_track_tbl[056] = {file = "059", mode = CHAT}; -- Skateboard kid
tr1_track_tbl[057] = {file = "028", mode = ONCE}; -- Silence
tr1_track_tbl[058] = {file = "004", mode = LOOP}; -- PC ONLY: Water ambience
tr1_track_tbl[059] = {file = "005", mode = LOOP}; -- PC ONLY: Wind  ambience
tr1_track_tbl[060] = {file = "006", mode = LOOP}; -- PC ONLY: Pulse ambience

--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 2 ---------------------------------
--------------------------------------------------------------------------------

tr2_num_soundtracks = 66;
tr2_track_tbl       = {};

tr2_track_tbl[002] = {file = "002", mode = ONCE}; -- Cutscene Gates
tr2_track_tbl[003] = {file = "002", mode = ONCE}; -- Cutscene Gates
tr2_track_tbl[004] = {file = "003", mode = ONCE}; -- Cutscene Plane
tr2_track_tbl[005] = {file = "004", mode = ONCE}; -- Cutscene Monk
tr2_track_tbl[006] = {file = "005", mode = CHAT}; -- Home block begin
tr2_track_tbl[007] = {file = "006", mode = CHAT};
tr2_track_tbl[008] = {file = "007", mode = CHAT};
tr2_track_tbl[009] = {file = "008", mode = CHAT}; 
tr2_track_tbl[010] = {file = "009", mode = CHAT};
tr2_track_tbl[011] = {file = "010", mode = CHAT};
tr2_track_tbl[012] = {file = "011", mode = CHAT};
tr2_track_tbl[013] = {file = "012", mode = CHAT};
tr2_track_tbl[014] = {file = "013", mode = CHAT};
tr2_track_tbl[015] = {file = "014", mode = CHAT};
tr2_track_tbl[016] = {file = "015", mode = CHAT};
tr2_track_tbl[017] = {file = "016", mode = CHAT};
tr2_track_tbl[018] = {file = "017", mode = CHAT};
tr2_track_tbl[019] = {file = "018", mode = CHAT};
tr2_track_tbl[020] = {file = "018", mode = CHAT};
tr2_track_tbl[021] = {file = "018", mode = CHAT};
tr2_track_tbl[022] = {file = "019", mode = CHAT};
tr2_track_tbl[023] = {file = "020", mode = CHAT};
tr2_track_tbl[024] = {file = "021", mode = CHAT};
tr2_track_tbl[025] = {file = "022", mode = CHAT};
tr2_track_tbl[026] = {file = "023", mode = ONCE}; -- Lara shower (Endgame)
tr2_track_tbl[027] = {file = "023", mode = ONCE}; -- Lara shower (Endgame)
tr2_track_tbl[028] = {file = "024", mode = ONCE}; -- Dragon death
tr2_track_tbl[029] = {file = "025", mode = CHAT}; -- Home - addon
tr2_track_tbl[030] = {file = "026", mode = ONCE}; -- Cutscene Bartoli stab
tr2_track_tbl[031] = {file = "027", mode = LOOP}; -- Caves ambience
tr2_track_tbl[032] = {file = "028", mode = LOOP}; -- Water ambience
tr2_track_tbl[033] = {file = "029", mode = LOOP}; -- Wind  ambience
tr2_track_tbl[034] = {file = "030", mode = LOOP}; -- Pulse ambience
tr2_track_tbl[035] = {file = "031", mode = ONCE}; -- Danger 1
tr2_track_tbl[036] = {file = "032", mode = ONCE}; -- Danger 2
tr2_track_tbl[037] = {file = "033", mode = ONCE}; -- Danger 3
tr2_track_tbl[038] = {file = "034", mode = ONCE}; -- Sacred
tr2_track_tbl[039] = {file = "035", mode = ONCE}; -- Awe
tr2_track_tbl[040] = {file = "036", mode = ONCE}; -- Venice Violins
tr2_track_tbl[041] = {file = "037", mode = ONCE}; -- End level
tr2_track_tbl[042] = {file = "038", mode = ONCE}; -- Mystical
tr2_track_tbl[043] = {file = "039", mode = ONCE}; -- Revelation
tr2_track_tbl[044] = {file = "040", mode = ONCE}; -- Careful
tr2_track_tbl[045] = {file = "041", mode = ONCE}; -- Guitar TR
tr2_track_tbl[046] = {file = "042", mode = ONCE}; -- Drama
tr2_track_tbl[047] = {file = "043", mode = ONCE}; -- Secret theme
tr2_track_tbl[048] = {file = "044", mode = ONCE}; -- It's coming!
tr2_track_tbl[049] = {file = "045", mode = ONCE}; -- It's coming 2!
tr2_track_tbl[050] = {file = "046", mode = ONCE}; -- Warning!
tr2_track_tbl[051] = {file = "047", mode = ONCE}; -- Warning 2!
tr2_track_tbl[052] = {file = "048", mode = ONCE}; -- Techno TR
tr2_track_tbl[053] = {file = "049", mode = ONCE}; -- Percussion
tr2_track_tbl[054] = {file = "050", mode = ONCE}; -- Pads
tr2_track_tbl[055] = {file = "051", mode = ONCE}; -- Super-revelation
tr2_track_tbl[056] = {file = "052", mode = ONCE}; -- Hmm...
tr2_track_tbl[057] = {file = "053", mode = ONCE}; -- Long way up
tr2_track_tbl[058] = {file = "054", mode = LOOP}; -- Industrial ambience
tr2_track_tbl[059] = {file = "055", mode = LOOP}; -- Spooky ambience
tr2_track_tbl[060] = {file = "056", mode = ONCE}; -- Barkhang theme
tr2_track_tbl[061] = {file = "057", mode = ONCE}; -- Super-revelation short
tr2_track_tbl[062] = {file = "058", mode = CHAT}; -- Monk beaten
tr2_track_tbl[063] = {file = "059", mode = ONCE}; -- Home sweet home
tr2_track_tbl[064] = {file = "060", mode = LOOP}; -- Main theme
tr2_track_tbl[065] = {file = "061", mode = ONCE}; -- Dummy track


--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 3 ---------------------------------
--------------------------------------------------------------------------------

tr3_num_soundtracks = 124;
tr3_track_tbl   = {};

-- NAME BINDINGS FOR TR3 REMASTERED SOUNDTRACK BY TOMEKKOBIALKA.

tr3_track_tbl[002] = {file = "puzzle element v2", mode = ONCE};
tr3_track_tbl[003] = {file = "no waiting around v3", mode = ONCE};
tr3_track_tbl[004] = {file = "something spooky v1", mode = ONCE};
tr3_track_tbl[005] = {file = "lara's theme v2", mode = LOOP};
tr3_track_tbl[006] = {file = "cavern sewers v1", mode = ONCE};
tr3_track_tbl[007] = {file = "geordie bob v1", mode = ONCE};
tr3_track_tbl[008] = {file = "tony the loon v1", mode = ONCE};
tr3_track_tbl[009] = {file = "no waiting around 2 v2", mode = ONCE};
tr3_track_tbl[010] = {file = "greedy mob v1", mode = ONCE};
tr3_track_tbl[011] = {file = "long way up v1", mode = ONCE};
tr3_track_tbl[012] = {file = "no waiting around 3 v1", mode = ONCE};
tr3_track_tbl[013] = {file = "here be butterflies 2 v1", mode = ONCE};
tr3_track_tbl[014] = {file = "she's cool", mode = ONCE};
tr3_track_tbl[015] = {file = "mond the gap 2", mode = ONCE};
tr3_track_tbl[016] = {file = "around the corner 2", mode = ONCE};
tr3_track_tbl[017] = {file = "around the corner 1", mode = ONCE};
tr3_track_tbl[018] = {file = "kneel and pray", mode = ONCE};
tr3_track_tbl[019] = {file = "around the corner 4", mode = ONCE};
tr3_track_tbl[020] = {file = "around the corner 3", mode = ONCE};
tr3_track_tbl[021] = {file = "seeing is believeing 1", mode = ONCE};
tr3_track_tbl[022] = {file = "looky we here 3", mode = ONCE};
tr3_track_tbl[023] = {file = "here be butterfiles 4", mode = ONCE};
tr3_track_tbl[024] = {file = "stone crows 10", mode = ONCE};
tr3_track_tbl[025] = {file = "butterfiles 3", mode = ONCE};
tr3_track_tbl[026] = {file = "meteorite cavern", mode = LOOP};
tr3_track_tbl[027] = {file = "steady", mode = LOOP};
tr3_track_tbl[028] = {file = "28_Antarctica", mode = LOOP};
tr3_track_tbl[029] = {file = "29_Things", mode = LOOP};
tr3_track_tbl[030] = {file = "30_Anyone_There", mode = LOOP};
tr3_track_tbl[031] = {file = "31_Grotto", mode = LOOP};
tr3_track_tbl[032] = {file = "32_On_The_Beach", mode = LOOP};
tr3_track_tbl[033] = {file = "33_Gamma_Pals", mode = LOOP};
tr3_track_tbl[034] = {file = "34_In_The_Jungle", mode = LOOP};
tr3_track_tbl[035] = {file = "35_Piranha_Waters", mode = LOOP};
tr3_track_tbl[036] = {file = "36_The_Rapids", mode = LOOP};
tr3_track_tbl[037] = {file = "37_Supper_Time", mode = ONCE};
tr3_track_tbl[038] = {file = "look out pt 5", mode = ONCE};
tr3_track_tbl[039] = {file = "looky pt 1", mode = ONCE};
tr3_track_tbl[040] = {file = "around the corner 5", mode = ONCE};
tr3_track_tbl[041] = {file = "seeing is believing 2", mode = ONCE};
tr3_track_tbl[042] = {file = "stone the crows 9", mode = ONCE};
tr3_track_tbl[043] = {file = "look out 8", mode = ONCE};
tr3_track_tbl[044] = {file = "look out 4", mode = ONCE};
tr3_track_tbl[045] = {file = "stone crows 7", mode = ONCE};
tr3_track_tbl[046] = {file = "stone crows 3", mode = ONCE};
tr3_track_tbl[047] = {file = "stone crows 8", mode = ONCE};
tr3_track_tbl[048] = {file = "looky here 2", mode = ONCE};
tr3_track_tbl[049] = {file = "stone crows 4", mode = ONCE};
tr3_track_tbl[050] = {file = "stone crows 6", mode = ONCE};
tr3_track_tbl[051] = {file = "look out 3", mode = ONCE};
tr3_track_tbl[052] = {file = "look out 1", mode = ONCE};
tr3_track_tbl[053] = {file = "there be butterflies here 1", mode = ONCE};
tr3_track_tbl[054] = {file = "stone crows 1", mode = ONCE};
tr3_track_tbl[055] = {file = "stone crows 5", mode = ONCE};
tr3_track_tbl[056] = {file = "mind the gap 1", mode = ONCE};
tr3_track_tbl[057] = {file = "butteflies 5", mode = ONCE};
tr3_track_tbl[058] = {file = "look out 2", mode = ONCE};
tr3_track_tbl[059] = {file = "look out 7", mode = ONCE};
tr3_track_tbl[060] = {file = "stone the crows 2", mode = ONCE};
tr3_track_tbl[061] = {file = "look out 6", mode = ONCE};
tr3_track_tbl[062] = {file = "scotts hut", mode = ONCE};
tr3_track_tbl[063] = {file = "cavern sewers cutscene", mode = ONCE};
tr3_track_tbl[064] = {file = "jungle camp cutscene", mode = ONCE};
tr3_track_tbl[065] = {file = "temple cutscene", mode = ONCE};
tr3_track_tbl[066] = {file = "cavern cutscene", mode = ONCE};
tr3_track_tbl[067] = {file = "rooftops cutscene", mode = ONCE};
tr3_track_tbl[068] = {file = "68_Tree-Shack_(English)", mode = ONCE};
tr3_track_tbl[069] = {file = "temple exit cutscene", mode = ONCE};
tr3_track_tbl[070] = {file = "delivery trcuk", mode = ONCE};
tr3_track_tbl[071] = {file = "penthouse cutscene", mode = ONCE};
tr3_track_tbl[072] = {file = "ravine cutscene", mode = ONCE};
tr3_track_tbl[073] = {file = "73_Old_Smokey", mode = LOOP};
tr3_track_tbl[074] = {file = "74_Under_Smokey", mode = LOOP};
tr3_track_tbl[075] = {file = "75_Refining_Plant", mode = LOOP};
tr3_track_tbl[076] = {file = "76_Rumble_Sub", mode = LOOP};
tr3_track_tbl[077] = {file = "77_Quake", mode = LOOP};
tr3_track_tbl[078] = {file = "78_Blank", mode = ONCE};
tr3_track_tbl[082] = {file = "82", mode = CHAT};             -- Home block begin
tr3_track_tbl[083] = {file = "83", mode = CHAT};
tr3_track_tbl[084] = {file = "84", mode = CHAT};
tr3_track_tbl[085] = {file = "85", mode = CHAT};
tr3_track_tbl[086] = {file = "86", mode = CHAT};
tr3_track_tbl[087] = {file = "87", mode = CHAT};
tr3_track_tbl[088] = {file = "88", mode = CHAT};
tr3_track_tbl[089] = {file = "89", mode = CHAT};
tr3_track_tbl[090] = {file = "90", mode = CHAT};
tr3_track_tbl[091] = {file = "91", mode = CHAT};
tr3_track_tbl[092] = {file = "92", mode = CHAT};
tr3_track_tbl[093] = {file = "93", mode = CHAT};
tr3_track_tbl[094] = {file = "94", mode = CHAT};
tr3_track_tbl[095] = {file = "95", mode = CHAT};
tr3_track_tbl[096] = {file = "96", mode = CHAT};
tr3_track_tbl[097] = {file = "97", mode = CHAT};
tr3_track_tbl[098] = {file = "98", mode = CHAT};
tr3_track_tbl[099] = {file = "99", mode = CHAT};
tr3_track_tbl[100] = {file = "100", mode = CHAT};
tr3_track_tbl[101] = {file = "101", mode = CHAT};
tr3_track_tbl[102] = {file = "102", mode = CHAT};
tr3_track_tbl[103] = {file = "103", mode = CHAT};
tr3_track_tbl[104] = {file = "104", mode = CHAT};
tr3_track_tbl[105] = {file = "105", mode = CHAT};
tr3_track_tbl[106] = {file = "106", mode = CHAT};
tr3_track_tbl[107] = {file = "107", mode = CHAT};
tr3_track_tbl[108] = {file = "108", mode = CHAT};
tr3_track_tbl[109] = {file = "109", mode = CHAT};
tr3_track_tbl[110] = {file = "110", mode = CHAT};
tr3_track_tbl[111] = {file = "111", mode = CHAT};
tr3_track_tbl[112] = {file = "112", mode = CHAT};
tr3_track_tbl[113] = {file = "113", mode = CHAT};
tr3_track_tbl[114] = {file = "114", mode = CHAT};
tr3_track_tbl[115] = {file = "115", mode = CHAT};
tr3_track_tbl[116] = {file = "116", mode = CHAT};
tr3_track_tbl[117] = {file = "117", mode = CHAT};
tr3_track_tbl[118] = {file = "118", mode = CHAT};
tr3_track_tbl[119] = {file = "119", mode = CHAT};              -- Home block end
tr3_track_tbl[120] = {file = "120_In_The_Hut", mode = LOOP};
tr3_track_tbl[121] = {file = "and so on", mode = ONCE};
tr3_track_tbl[122] = {file = "secret", mode = ONCE};
tr3_track_tbl[123] = {file = "secret", mode = ONCE};

--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 4 ---------------------------------
--------------------------------------------------------------------------------

tr4_num_soundtracks = 112;
tr4_track_tbl       = {};

tr4_track_tbl[000] = {file = "044_attack_part_i", mode = ONCE}; 
tr4_track_tbl[001] = {file = "008_voncroy9a", mode = CHAT}; 
tr4_track_tbl[002] = {file = "100_attack_part_ii", mode = ONCE}; 
tr4_track_tbl[003] = {file = "010_voncroy10", mode = CHAT}; 
tr4_track_tbl[004] = {file = "015_voncroy14", mode = CHAT}; 
tr4_track_tbl[005] = {file = "073_secret", mode = ONCE}; 
tr4_track_tbl[006] = {file = "109_lyre_01", mode = CHAT}; 
tr4_track_tbl[007] = {file = "042_action_part_iv", mode = ONCE}; 
tr4_track_tbl[008] = {file = "043_action_part_v", mode = ONCE}; 
tr4_track_tbl[009] = {file = "030_voncroy30", mode = CHAT}; 
tr4_track_tbl[010] = {file = "012_voncroy11b", mode = CHAT}; 
tr4_track_tbl[011] = {file = "011_voncroy11a", mode = CHAT}; 
tr4_track_tbl[012] = {file = "063_misc_inc_01", mode = ONCE}; 
tr4_track_tbl[013] = {file = "014_voncroy13b", mode = CHAT}; 
tr4_track_tbl[014] = {file = "111_charmer", mode = CHAT}; 
tr4_track_tbl[015] = {file = "025_voncroy24b", mode = CHAT}; 
tr4_track_tbl[016] = {file = "023_voncroy23", mode = CHAT}; 
tr4_track_tbl[017] = {file = "006_voncroy7", mode = CHAT}; 
tr4_track_tbl[018] = {file = "024_voncroy24a", mode = CHAT}; 
tr4_track_tbl[019] = {file = "110_lyre_02", mode = ONCE}; 
tr4_track_tbl[020] = {file = "020_voncroy19", mode = CHAT}; 
tr4_track_tbl[021] = {file = "034_voncroy34", mode = CHAT}; 
tr4_track_tbl[022] = {file = "054_general_part_ii", mode = ONCE}; 
tr4_track_tbl[023] = {file = "036_voncroy36", mode = CHAT}; 
tr4_track_tbl[024] = {file = "004_voncroy5", mode = CHAT}; 
tr4_track_tbl[025] = {file = "035_voncroy35", mode = CHAT}; 
tr4_track_tbl[026] = {file = "027_voncroy27", mode = CHAT}; 
tr4_track_tbl[027] = {file = "053_general_part_i", mode = ONCE}; 
tr4_track_tbl[028] = {file = "022_voncroy22b", mode = CHAT}; 
tr4_track_tbl[029] = {file = "028_voncroy28_l11", mode = CHAT}; 
tr4_track_tbl[030] = {file = "003_voncroy4", mode = CHAT}; 
tr4_track_tbl[031] = {file = "001_voncroy2", mode = CHAT}; 
tr4_track_tbl[032] = {file = "041_action_part_iii", mode = ONCE}; 
tr4_track_tbl[033] = {file = "057_general_part_v", mode = ONCE}; 
tr4_track_tbl[034] = {file = "018_voncroy17", mode = CHAT}; 
tr4_track_tbl[035] = {file = "064_misc_inc_02", mode = ONCE}; 
tr4_track_tbl[036] = {file = "033_voncroy33", mode = CHAT}; 
tr4_track_tbl[037] = {file = "031_voncroy31_l12", mode = CHAT}; 
tr4_track_tbl[038] = {file = "032_voncroy32_l13", mode = CHAT}; 
tr4_track_tbl[039] = {file = "016_voncroy15", mode = CHAT}; 
tr4_track_tbl[040] = {file = "065_misc_inc_03", mode = ONCE}; 
tr4_track_tbl[041] = {file = "040_action_part_ii", mode = ONCE}; 
tr4_track_tbl[042] = {file = "112_gods_part_iv", mode = ONCE}; 
tr4_track_tbl[043] = {file = "029_voncroy29", mode = CHAT}; 
tr4_track_tbl[044] = {file = "007_voncroy8", mode = CHAT}; 
tr4_track_tbl[045] = {file = "013_voncroy12_13a_lara4", mode = CHAT}; 
tr4_track_tbl[046] = {file = "009_voncroy9b_lara3", mode = CHAT}; 
tr4_track_tbl[047] = {file = "081_dig", mode = ONCE}; 
tr4_track_tbl[048] = {file = "085_intro", mode = ONCE}; 
tr4_track_tbl[049] = {file = "071_ominous_part_i", mode = ONCE}; 
tr4_track_tbl[050] = {file = "095_phildoor", mode = ONCE}; 
tr4_track_tbl[051] = {file = "061_in_the_pyramid_part_i", mode = ONCE}; 
tr4_track_tbl[052] = {file = "050_underwater_find_part_i", mode = ONCE}; 
tr4_track_tbl[053] = {file = "058_gods_part_i", mode = ONCE}; 
tr4_track_tbl[054] = {file = "005_voncroy6_lara2", mode = CHAT}; 
tr4_track_tbl[055] = {file = "045_authentic_tr", mode = ONCE}; 
tr4_track_tbl[056] = {file = "060_gods_part_iii", mode = ONCE}; 
tr4_track_tbl[057] = {file = "055_general_part_iii", mode = ONCE}; 
tr4_track_tbl[058] = {file = "059_gods_part_ii", mode = ONCE}; 
tr4_track_tbl[059] = {file = "068_mystery_part_ii", mode = ONCE}; 
tr4_track_tbl[060] = {file = "076_captain2", mode = ONCE}; 
tr4_track_tbl[061] = {file = "019_lara6_voncroy18", mode = ONCE}; 
tr4_track_tbl[062] = {file = "002_voncroy3", mode = CHAT}; 
tr4_track_tbl[063] = {file = "066_misc_inc_04", mode = ONCE}; 
tr4_track_tbl[064] = {file = "067_mystery_part_i", mode = ONCE}; 
tr4_track_tbl[065] = {file = "038_a_short_01", mode = ONCE}; 
tr4_track_tbl[066] = {file = "088_key", mode = ONCE}; 
tr4_track_tbl[067] = {file = "017_voncroy16_lara5", mode = CHAT}; 
tr4_track_tbl[068] = {file = "026_vc25_l9_vc26_l10", mode = CHAT}; 
tr4_track_tbl[069] = {file = "056_general_part_iv", mode = ONCE}; 
tr4_track_tbl[070] = {file = "021_vc20_l7_vc21_l8_vc22a", mode = CHAT}; 
tr4_track_tbl[071] = {file = "096_sarcoph", mode = ONCE}; 
tr4_track_tbl[072] = {file = "087_jeepb", mode = ONCE}; 
tr4_track_tbl[073] = {file = "091_minilib1", mode = ONCE}; 
tr4_track_tbl[074] = {file = "086_jeepa", mode = ONCE}; 
tr4_track_tbl[075] = {file = "051_egyptian_mood_part_i", mode = ONCE}; 
tr4_track_tbl[076] = {file = "078_croywon", mode = ONCE}; 
tr4_track_tbl[077] = {file = "092_minilib2", mode = ONCE}; 
tr4_track_tbl[078] = {file = "083_horus", mode = ONCE}; 
tr4_track_tbl[079] = {file = "049_close_to_the_end_part_ii", mode = ONCE}; 
tr4_track_tbl[080] = {file = "037_vc37_l15_vc38", mode = CHAT}; 
tr4_track_tbl[081] = {file = "097_scorpion", mode = ONCE}; 
tr4_track_tbl[082] = {file = "089_larawon", mode = ONCE}; 
tr4_track_tbl[083] = {file = "094_minilib4", mode = ONCE}; 
tr4_track_tbl[084] = {file = "098_throne", mode = ONCE}; 
tr4_track_tbl[085] = {file = "048_close_to_the_end", mode = ONCE}; 
tr4_track_tbl[086] = {file = "070_mystery_part_iv", mode = ONCE}; 
tr4_track_tbl[087] = {file = "093_minilib3", mode = ONCE}; 
tr4_track_tbl[088] = {file = "072_puzzle_part_i", mode = ONCE}; 
tr4_track_tbl[089] = {file = "074_backpack", mode = ONCE}; 
tr4_track_tbl[090] = {file = "069_mystery_part_iii", mode = ONCE}; 
tr4_track_tbl[091] = {file = "052_egyptian_mood_part_ii", mode = ONCE}; 
tr4_track_tbl[092] = {file = "084_inscrip", mode = ONCE}; 
tr4_track_tbl[093] = {file = "099_whouse", mode = ONCE}; 
tr4_track_tbl[094] = {file = "047_boss_02", mode = ONCE}; 
tr4_track_tbl[095] = {file = "080_crypt2", mode = ONCE}; 
tr4_track_tbl[096] = {file = "090_libend", mode = ONCE}; 
tr4_track_tbl[097] = {file = "046_boss_01", mode = ONCE}; 
tr4_track_tbl[098] = {file = "062_jeep_thrills_max", mode = ONCE}; 
tr4_track_tbl[099] = {file = "079_crypt1", mode = ONCE}; 
tr4_track_tbl[100] = {file = "082_finale", mode = ONCE}; 
tr4_track_tbl[101] = {file = "075_captain1", mode = ONCE}; 
tr4_track_tbl[102] = {file = "105_a5_battle", mode = LOOP}; 
tr4_track_tbl[103] = {file = "077_crocgod", mode = ONCE}; 
tr4_track_tbl[104] = {file = "039_tr4_title_q10", mode = LOOP}; 
tr4_track_tbl[105] = {file = "108_a8_coastal", mode = LOOP}; 
tr4_track_tbl[106] = {file = "107_a7_train+", mode = LOOP}; 
tr4_track_tbl[107] = {file = "101_a1_in_dark", mode = LOOP}; 
tr4_track_tbl[108] = {file = "102_a2_in_drips", mode = LOOP}; 
tr4_track_tbl[109] = {file = "104_a4_weird1", mode = LOOP}; 
tr4_track_tbl[110] = {file = "106_a6_out_day", mode = LOOP}; 
tr4_track_tbl[111] = {file = "103_a3_out_night", mode = LOOP}; 


--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 5 ---------------------------------
--------------------------------------------------------------------------------

tr5_num_soundtracks = 136;
tr5_track_tbl       = {};

tr5_track_tbl[000] = {file = "xa1_TL_10B", mode = CHAT};
tr5_track_tbl[001] = {file = "xa1_Z_10", mode = CHAT};
tr5_track_tbl[002] = {file = "xa1_TL_05", mode = CHAT};
tr5_track_tbl[003] = {file = "xa1_TL_08", mode = CHAT};
tr5_track_tbl[004] = {file = "xa1_TL_11", mode = CHAT};
tr5_track_tbl[005] = {file = "xa1_ANDYPEW", mode = ONCE};
tr5_track_tbl[006] = {file = "xa1_SECRET", mode = ONCE};
tr5_track_tbl[007] = {file = "xa1_TL_02", mode = CHAT};
tr5_track_tbl[008] = {file = "xa2_HMMM05", mode = ONCE};
tr5_track_tbl[009] = {file = "xa2_TL_01", mode = CHAT};
tr5_track_tbl[010] = {file = "xa2_ATTACK04", mode = ONCE};
tr5_track_tbl[011] = {file = "xa2_UWATER2B", mode = ONCE};
tr5_track_tbl[012] = {file = "xa2_SPOOKY2A", mode = ONCE};
tr5_track_tbl[013] = {file = "xa2_TL_10A", mode = CHAT};
tr5_track_tbl[014] = {file = "xa2_HMMM02", mode = ONCE};
tr5_track_tbl[015] = {file = "xa2_TOMS01", mode = ONCE};
tr5_track_tbl[016] = {file = "xa3_Attack03", mode = ONCE};
tr5_track_tbl[017] = {file = "xa3_Attack02", mode = ONCE};
tr5_track_tbl[018] = {file = "xa3_Hmmm01", mode = ONCE};
tr5_track_tbl[019] = {file = "xa3_Stealth1", mode = ONCE};
tr5_track_tbl[020] = {file = "xa3_Stealth2", mode = ONCE};
tr5_track_tbl[021] = {file = "xa3_Attack01", mode = ONCE};
tr5_track_tbl[022] = {file = "xa3_TL_06", mode = CHAT};
tr5_track_tbl[023] = {file = "xa3_TL_03", mode = CHAT};
tr5_track_tbl[024] = {file = "xa4_hmmm06", mode = ONCE};
tr5_track_tbl[025] = {file = "xa4_mil01", mode = ONCE};
tr5_track_tbl[026] = {file = "xa4_Z_03", mode = CHAT};
tr5_track_tbl[027] = {file = "xa4_hit01", mode = ONCE};
tr5_track_tbl[028] = {file = "xa4_spooky05", mode = ONCE};
tr5_track_tbl[029] = {file = "xa4_drama01", mode = ONCE};
tr5_track_tbl[030] = {file = "xa4_stealth4", mode = ONCE};
tr5_track_tbl[031] = {file = "xa4_mil05", mode = ONCE};
tr5_track_tbl[032] = {file = "xa5_HMMM04", mode = ONCE};
tr5_track_tbl[033] = {file = "xa5_MIL06", mode = ONCE};
tr5_track_tbl[034] = {file = "xa5_SPOOKY02", mode = ONCE};
tr5_track_tbl[035] = {file = "xa5_TL_12", mode = CHAT};
tr5_track_tbl[036] = {file = "xa5_MIL02A", mode = ONCE};
tr5_track_tbl[037] = {file = "xa5_HMMM03", mode = ONCE};
tr5_track_tbl[038] = {file = "xa5_MIL02", mode = ONCE};
tr5_track_tbl[039] = {file = "xa5_TL_04", mode = CHAT};
tr5_track_tbl[040] = {file = "xa6_Mil04", mode = ONCE};
tr5_track_tbl[041] = {file = "xa6_Solo01", mode = ONCE};
tr5_track_tbl[042] = {file = "xa6_Z12", mode = CHAT};
tr5_track_tbl[043] = {file = "xa6_Stealth3", mode = ONCE};
tr5_track_tbl[044] = {file = "xa6_AuthSolo", mode = ONCE};
tr5_track_tbl[045] = {file = "xa6_Spooky03", mode = ONCE};
tr5_track_tbl[046] = {file = "xa6_Z13", mode = CHAT};
tr5_track_tbl[047] = {file = "xa6_Z_04anim", mode = CHAT};
tr5_track_tbl[048] = {file = "xa7_z_06a", mode = CHAT};
tr5_track_tbl[049] = {file = "xa7_andyoooh", mode = ONCE};
tr5_track_tbl[050] = {file = "xa7_andyooer", mode = ONCE};
tr5_track_tbl[051] = {file = "xa7_tl_07", mode = CHAT};
tr5_track_tbl[052] = {file = "xa7_z_02", mode = CHAT};
tr5_track_tbl[053] = {file = "xa7_evibes01", mode = ONCE};
tr5_track_tbl[054] = {file = "xa7_z_06", mode = CHAT};
tr5_track_tbl[055] = {file = "xa7_authtr", mode = ONCE};
tr5_track_tbl[056] = {file = "xa8_mil03", mode = ONCE};
tr5_track_tbl[057] = {file = "xa8_fightsc", mode = ONCE};
tr5_track_tbl[058] = {file = "xa8_richcut3", mode = ONCE};
tr5_track_tbl[059] = {file = "xa8_z_13", mode = CHAT};
tr5_track_tbl[060] = {file = "xa8_z_08", mode = CHAT};
tr5_track_tbl[061] = {file = "xa8_uwater2a", mode = ONCE};
tr5_track_tbl[062] = {file = "xa8_jobyalrm", mode = ONCE};
tr5_track_tbl[063] = {file = "xa8_mil02b", mode = ONCE};
tr5_track_tbl[064] = {file = "xa9_swampy", mode = ONCE};
tr5_track_tbl[065] = {file = "xa9_evibes02", mode = ONCE};
tr5_track_tbl[066] = {file = "xa9_gods01", mode = ONCE};
tr5_track_tbl[067] = {file = "xa9_z_03", mode = CHAT};
tr5_track_tbl[068] = {file = "xa9_richcut4", mode = ONCE};
tr5_track_tbl[069] = {file = "xa9_title4", mode = ONCE};
tr5_track_tbl[070] = {file = "xa9_spooky01", mode = ONCE};
tr5_track_tbl[071] = {file = "xa9_chopin01", mode = ONCE};
tr5_track_tbl[072] = {file = "xa10_echoir01", mode = ONCE};
tr5_track_tbl[073] = {file = "xa10_title3", mode = ONCE};
tr5_track_tbl[074] = {file = "xa10_perc01", mode = ONCE};
tr5_track_tbl[075] = {file = "xa10_vc01", mode = ONCE};
tr5_track_tbl[076] = {file = "xa10_title2", mode = ONCE};
tr5_track_tbl[077] = {file = "xa10_z_09", mode = CHAT};
tr5_track_tbl[078] = {file = "xa10_spooky04", mode = ONCE};
tr5_track_tbl[079] = {file = "xa10_z_10", mode = CHAT};
tr5_track_tbl[080] = {file = "xa11_vc01atv", mode = ONCE};
tr5_track_tbl[081] = {file = "xa11_andy3", mode = ONCE};
tr5_track_tbl[082] = {file = "xa11_title1", mode = ONCE};
tr5_track_tbl[083] = {file = "xa11_flyby1", mode = ONCE};
tr5_track_tbl[084] = {file = "xa11_monk_2", mode = ONCE};
tr5_track_tbl[085] = {file = "xa11_andy4", mode = ONCE};
tr5_track_tbl[086] = {file = "xa11_flyby3", mode = ONCE};
tr5_track_tbl[087] = {file = "xa11_flyby2", mode = ONCE};
tr5_track_tbl[088] = {file = "xa12_moses01", mode = ONCE};
tr5_track_tbl[089] = {file = "xa12_andy4b", mode = ONCE};
tr5_track_tbl[090] = {file = "xa12_z_10", mode = CHAT};
tr5_track_tbl[091] = {file = "xa12_flyby4", mode = ONCE};
tr5_track_tbl[092] = {file = "xa12_richcut1", mode = ONCE};
tr5_track_tbl[093] = {file = "xa12_andy5", mode = ONCE};
tr5_track_tbl[094] = {file = "xa12_z_05", mode = CHAT};
tr5_track_tbl[095] = {file = "xa12_z_01", mode = CHAT};
tr5_track_tbl[096] = {file = "xa13_Joby3", mode = ONCE};
tr5_track_tbl[097] = {file = "xa13_Andy7", mode = ONCE};
tr5_track_tbl[098] = {file = "xa13_Andrea3B", mode = ONCE};
tr5_track_tbl[099] = {file = "xa13_cossack", mode = ONCE};
tr5_track_tbl[100] = {file = "xa13_Z_07", mode = CHAT};
tr5_track_tbl[101] = {file = "xa13_Andy6", mode = ONCE};
tr5_track_tbl[102] = {file = "xa13_Andrea3", mode = ONCE};
tr5_track_tbl[103] = {file = "xa13_Joby7", mode = ONCE};
tr5_track_tbl[104] = {file = "xa14_uwater1", mode = ONCE};
tr5_track_tbl[105] = {file = "xa14_joby1", mode = ONCE};
tr5_track_tbl[106] = {file = "xa14_andy10", mode = ONCE};
tr5_track_tbl[107] = {file = "xa14_richcut2", mode = ONCE};
tr5_track_tbl[108] = {file = "xa14_andrea1", mode = ONCE};
tr5_track_tbl[109] = {file = "xa14_andy8", mode = ONCE};
tr5_track_tbl[110] = {file = "xa14_joby6", mode = ONCE};
tr5_track_tbl[111] = {file = "xa14_ecredits", mode = LOOP};
tr5_track_tbl[112] = {file = "xa15_boss_01", mode = ONCE};
tr5_track_tbl[113] = {file = "xa15_joby2", mode = ONCE};
tr5_track_tbl[114] = {file = "xa15_joby4", mode = ONCE};
tr5_track_tbl[115] = {file = "xa15_joby5", mode = ONCE};
tr5_track_tbl[116] = {file = "xa15_joby9", mode = ONCE};
tr5_track_tbl[117] = {file = "xa15_a_andy", mode = LOOP};
tr5_track_tbl[118] = {file = "xa15_a_rome", mode = LOOP};
tr5_track_tbl[119] = {file = "xa15_andy2", mode = ONCE};
tr5_track_tbl[120] = {file = "xa16_Joby8", mode = ONCE};
tr5_track_tbl[121] = {file = "xa16_A_Sub_Amb", mode = LOOP};
tr5_track_tbl[122] = {file = "xa16_Joby10", mode = ONCE};
tr5_track_tbl[123] = {file = "xa16_A_Harbour_out", mode = LOOP};
tr5_track_tbl[124] = {file = "xa16_A_Andy_Out_Norm", mode = LOOP};
tr5_track_tbl[125] = {file = "xa16_A_Andy_Out_Spooky", mode = LOOP};
tr5_track_tbl[126] = {file = "xa16_A_Rome_Dag", mode = LOOP}
tr5_track_tbl[127] = {file = "xa16_A_Underwater", mode = LOOP};
tr5_track_tbl[128] = {file = "xa17_A_Rome_Night", mode = LOOP};
tr5_track_tbl[129] = {file = "xa17_A_VC_Saga", mode = LOOP};
tr5_track_tbl[130] = {file = "xa17_A_Industrg", mode = LOOP}
tr5_track_tbl[131] = {file = "xa17_Andrea2", mode = ONCE};
tr5_track_tbl[132] = {file = "xa17_Andy1", mode = ONCE};
tr5_track_tbl[133] = {file = "xa17_Andrea4", mode = ONCE};
tr5_track_tbl[134] = {file = "xa17_Andy9", mode = ONCE};
tr5_track_tbl[135] = {file = "xa17_Andy11", mode = ONCE};


function getTrackInfo(ver, id)
    local tbl = {};
    local path, method, ext;
    
    if(ver < 3) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        tbl    = tr1_track_tbl;
        path   = "data/tr1/audio/";
        method = TRACK;
        ext    = ".ogg";
    elseif(ver < 5) then                -- TR_II, TR_II_DEMO
        tbl    = tr2_track_tbl;
        path   = "data/tr2/audio/";
        method = TRACK;
        ext    = ".ogg";
    elseif(ver < 6) then
        tbl    = tr3_track_tbl;
        if(USE_TR3_REMASTER == 1) then  -- TR_III
            path   = "data/tr3/audio/";
            method = TRACK;
            ext    = ".ogg";
        else
            path   = "data/tr3/audio/cdaudio.wad";
            method = WAD;
        end
    elseif(ver < 8) then                -- TR_IV, TR_IV_DEMO
        tbl    = tr4_track_tbl;
        path   = "data/tr4/audio/";
        method = TRACK;
        ext    = ".wav";
    elseif(ver < 9) then                -- TR_V
        tbl    = tr5_track_tbl;
        path   = "data/tr5/audio/";
        method = TRACK;
        ext    = ".wav";
    else
        return "NONE", -1, -1;
    end;
    
    if(tbl[id] == nil) then
        return "NONE", -1, -1;
    else
        if(method == TRACK) then
            return (path .. tbl[id].file .. ext), tbl[id].mode, method;
        else
            return path, tbl[id].mode, method;
        end;
    end;
end;

function getSecretTrackNumber(ver)
    if(ver < 3) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        return SECRET_TR1;
    elseif(ver < 5) then                -- TR_II, TR_II_DEMO
        return SECRET_TR2;
    elseif(ver < 6) then                
        return SECRET_TR3;
    elseif(ver < 8) then                -- TR_IV, TR_IV_DEMO
        return SECRET_TR4;
    elseif(ver < 9) then                -- TR_V
        return SECRET_TR5;
    else
        return 0;
    end;
end;

function getNumTracks(ver)
    if(ver < 3) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        return tr1_num_soundtracks;
    elseif(ver < 5) then                -- TR_II, TR_II_DEMO
        return tr2_num_soundtracks;
    elseif(ver < 6) then                
        return tr3_num_soundtracks;
    elseif(ver < 8) then                -- TR_IV, TR_IV_DEMO
        return tr4_num_soundtracks;
    elseif(ver < 9) then                -- TR_V
        return tr5_num_soundtracks;
    else
        return 0;
    end;
end;