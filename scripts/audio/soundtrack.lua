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

tr1_track_tbl[002] = {file = "002", mode = StreamType.Background}; -- Main theme
tr1_track_tbl[003] = {file = "011", mode = StreamType.Oneshot}; -- Poseidon
tr1_track_tbl[004] = {file = "012", mode = StreamType.Oneshot}; -- Main theme, short
tr1_track_tbl[005] = {file = "003", mode = StreamType.Background}; -- Caves ambience
tr1_track_tbl[006] = {file = "013", mode = StreamType.Oneshot}; -- Thor
tr1_track_tbl[007] = {file = "014", mode = StreamType.Oneshot}; -- St. Francis
tr1_track_tbl[008] = {file = "015", mode = StreamType.Oneshot}; -- Danger
tr1_track_tbl[009] = {file = "016", mode = StreamType.Oneshot}; -- Stairs
tr1_track_tbl[010] = {file = "017", mode = StreamType.Oneshot}; -- Midas
tr1_track_tbl[011] = {file = "018", mode = StreamType.Oneshot}; -- Lever
tr1_track_tbl[012] = {file = "019", mode = StreamType.Oneshot}; -- Hmm...
tr1_track_tbl[013] = {file = "060", mode = StreamType.Oneshot}; -- Secret theme
tr1_track_tbl[014] = {file = "020", mode = StreamType.Oneshot}; -- Big secret theme
tr1_track_tbl[015] = {file = "021", mode = StreamType.Oneshot}; -- Raiders
tr1_track_tbl[016] = {file = "022", mode = StreamType.Oneshot}; -- Wolf
tr1_track_tbl[017] = {file = "023", mode = StreamType.Oneshot}; -- Awe
tr1_track_tbl[018] = {file = "024", mode = StreamType.Oneshot}; -- Gods
tr1_track_tbl[019] = {file = "025", mode = StreamType.Oneshot}; -- Main theme, reprise
tr1_track_tbl[020] = {file = "026", mode = StreamType.Oneshot}; -- Mummy
tr1_track_tbl[021] = {file = "027", mode = StreamType.Oneshot}; -- Midas, reprise
tr1_track_tbl[022] = {file = "007", mode = StreamType.Oneshot}; -- Natla cutscene
tr1_track_tbl[023] = {file = "008", mode = StreamType.Oneshot}; -- Larson cutscene
tr1_track_tbl[024] = {file = "009", mode = StreamType.Oneshot}; -- Natla scion cutscene
tr1_track_tbl[025] = {file = "010", mode = StreamType.Oneshot}; -- Tihocan cutscene
tr1_track_tbl[026] = {file = "029", mode = StreamType.Chat}; -- Home block begin
tr1_track_tbl[027] = {file = "030", mode = StreamType.Chat};
tr1_track_tbl[028] = {file = "031", mode = StreamType.Chat};
tr1_track_tbl[029] = {file = "032", mode = StreamType.Chat};
tr1_track_tbl[030] = {file = "033", mode = StreamType.Chat};
tr1_track_tbl[031] = {file = "034", mode = StreamType.Chat};
tr1_track_tbl[032] = {file = "035", mode = StreamType.Chat};
tr1_track_tbl[033] = {file = "036", mode = StreamType.Chat};
tr1_track_tbl[034] = {file = "037", mode = StreamType.Chat};
tr1_track_tbl[035] = {file = "038", mode = StreamType.Chat};
tr1_track_tbl[036] = {file = "039", mode = StreamType.Chat};
tr1_track_tbl[037] = {file = "040", mode = StreamType.Chat};
tr1_track_tbl[038] = {file = "041", mode = StreamType.Chat};
tr1_track_tbl[039] = {file = "042", mode = StreamType.Chat};
tr1_track_tbl[040] = {file = "043", mode = StreamType.Chat};
tr1_track_tbl[041] = {file = "044", mode = StreamType.Chat};
tr1_track_tbl[042] = {file = "045", mode = StreamType.Chat};
tr1_track_tbl[043] = {file = "046", mode = StreamType.Chat};
tr1_track_tbl[044] = {file = "047", mode = StreamType.Chat};
tr1_track_tbl[045] = {file = "048", mode = StreamType.Chat};
tr1_track_tbl[046] = {file = "049", mode = StreamType.Chat};
tr1_track_tbl[047] = {file = "050", mode = StreamType.Chat};
tr1_track_tbl[048] = {file = "051", mode = StreamType.Chat};
tr1_track_tbl[049] = {file = "052", mode = StreamType.Chat};
tr1_track_tbl[050] = {file = "053", mode = StreamType.Chat};
tr1_track_tbl[051] = {file = "054", mode = StreamType.Chat}; -- Baddy 1
tr1_track_tbl[052] = {file = "055", mode = StreamType.Chat}; -- Baddy 2
tr1_track_tbl[053] = {file = "056", mode = StreamType.Chat}; -- Larson
tr1_track_tbl[054] = {file = "057", mode = StreamType.Chat}; -- Natla
tr1_track_tbl[055] = {file = "058", mode = StreamType.Chat}; -- Pierre
tr1_track_tbl[056] = {file = "059", mode = StreamType.Chat}; -- Skateboard kid
tr1_track_tbl[057] = {file = "028", mode = StreamType.Oneshot}; -- Silence
tr1_track_tbl[058] = {file = "004", mode = StreamType.Background}; -- PC ONLY: Water ambience
tr1_track_tbl[059] = {file = "005", mode = StreamType.Background}; -- PC ONLY: Wind  ambience
tr1_track_tbl[060] = {file = "006", mode = StreamType.Background}; -- PC ONLY: Pulse ambience

--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 2 ---------------------------------
--------------------------------------------------------------------------------

tr2_num_soundtracks = 66;
tr2_track_tbl       = {};

tr2_track_tbl[002] = {file = "002", mode = StreamType.Oneshot}; -- Cutscene Gates
tr2_track_tbl[003] = {file = "002", mode = StreamType.Oneshot}; -- Cutscene Gates
tr2_track_tbl[004] = {file = "003", mode = StreamType.Oneshot}; -- Cutscene Plane
tr2_track_tbl[005] = {file = "004", mode = StreamType.Oneshot}; -- Cutscene Monk
tr2_track_tbl[006] = {file = "005", mode = StreamType.Chat}; -- Home block begin
tr2_track_tbl[007] = {file = "006", mode = StreamType.Chat};
tr2_track_tbl[008] = {file = "007", mode = StreamType.Chat};
tr2_track_tbl[009] = {file = "008", mode = StreamType.Chat};
tr2_track_tbl[010] = {file = "009", mode = StreamType.Chat};
tr2_track_tbl[011] = {file = "010", mode = StreamType.Chat};
tr2_track_tbl[012] = {file = "011", mode = StreamType.Chat};
tr2_track_tbl[013] = {file = "012", mode = StreamType.Chat};
tr2_track_tbl[014] = {file = "013", mode = StreamType.Chat};
tr2_track_tbl[015] = {file = "014", mode = StreamType.Chat};
tr2_track_tbl[016] = {file = "015", mode = StreamType.Chat};
tr2_track_tbl[017] = {file = "016", mode = StreamType.Chat};
tr2_track_tbl[018] = {file = "017", mode = StreamType.Chat};
tr2_track_tbl[019] = {file = "018", mode = StreamType.Chat};
tr2_track_tbl[020] = {file = "018", mode = StreamType.Chat};
tr2_track_tbl[021] = {file = "018", mode = StreamType.Chat};
tr2_track_tbl[022] = {file = "019", mode = StreamType.Chat};
tr2_track_tbl[023] = {file = "020", mode = StreamType.Chat};
tr2_track_tbl[024] = {file = "021", mode = StreamType.Chat};
tr2_track_tbl[025] = {file = "022", mode = StreamType.Chat};
tr2_track_tbl[026] = {file = "023", mode = StreamType.Oneshot}; -- Lara shower (Endgame)
tr2_track_tbl[027] = {file = "023", mode = StreamType.Oneshot}; -- Lara shower (Endgame)
tr2_track_tbl[028] = {file = "024", mode = StreamType.Oneshot}; -- Dragon death
tr2_track_tbl[029] = {file = "025", mode = StreamType.Chat}; -- Home - addon
tr2_track_tbl[030] = {file = "026", mode = StreamType.Oneshot}; -- Cutscene Bartoli stab
tr2_track_tbl[031] = {file = "027", mode = StreamType.Background}; -- Caves ambience
tr2_track_tbl[032] = {file = "028", mode = StreamType.Background}; -- Water ambience
tr2_track_tbl[033] = {file = "029", mode = StreamType.Background}; -- Wind  ambience
tr2_track_tbl[034] = {file = "030", mode = StreamType.Background}; -- Pulse ambience
tr2_track_tbl[035] = {file = "031", mode = StreamType.Oneshot}; -- Danger 1
tr2_track_tbl[036] = {file = "032", mode = StreamType.Oneshot}; -- Danger 2
tr2_track_tbl[037] = {file = "033", mode = StreamType.Oneshot}; -- Danger 3
tr2_track_tbl[038] = {file = "034", mode = StreamType.Oneshot}; -- Sacred
tr2_track_tbl[039] = {file = "035", mode = StreamType.Oneshot}; -- Awe
tr2_track_tbl[040] = {file = "036", mode = StreamType.Oneshot}; -- Venice Violins
tr2_track_tbl[041] = {file = "037", mode = StreamType.Oneshot}; -- End level
tr2_track_tbl[042] = {file = "038", mode = StreamType.Oneshot}; -- Mystical
tr2_track_tbl[043] = {file = "039", mode = StreamType.Oneshot}; -- Revelation
tr2_track_tbl[044] = {file = "040", mode = StreamType.Oneshot}; -- Careful
tr2_track_tbl[045] = {file = "041", mode = StreamType.Oneshot}; -- Guitar TR
tr2_track_tbl[046] = {file = "042", mode = StreamType.Oneshot}; -- Drama
tr2_track_tbl[047] = {file = "043", mode = StreamType.Oneshot}; -- Secret theme
tr2_track_tbl[048] = {file = "044", mode = StreamType.Oneshot}; -- It's coming!
tr2_track_tbl[049] = {file = "045", mode = StreamType.Oneshot}; -- It's coming 2!
tr2_track_tbl[050] = {file = "046", mode = StreamType.Oneshot}; -- Warning!
tr2_track_tbl[051] = {file = "047", mode = StreamType.Oneshot}; -- Warning 2!
tr2_track_tbl[052] = {file = "048", mode = StreamType.Oneshot}; -- Techno TR
tr2_track_tbl[053] = {file = "049", mode = StreamType.Oneshot}; -- Percussion
tr2_track_tbl[054] = {file = "050", mode = StreamType.Oneshot}; -- Pads
tr2_track_tbl[055] = {file = "051", mode = StreamType.Oneshot}; -- Super-revelation
tr2_track_tbl[056] = {file = "052", mode = StreamType.Oneshot}; -- Hmm...
tr2_track_tbl[057] = {file = "053", mode = StreamType.Oneshot}; -- Long way up
tr2_track_tbl[058] = {file = "054", mode = StreamType.Background}; -- Industrial ambience
tr2_track_tbl[059] = {file = "055", mode = StreamType.Background}; -- Spooky ambience
tr2_track_tbl[060] = {file = "056", mode = StreamType.Oneshot}; -- Barkhang theme
tr2_track_tbl[061] = {file = "057", mode = StreamType.Oneshot}; -- Super-revelation short
tr2_track_tbl[062] = {file = "058", mode = StreamType.Chat}; -- Monk beaten
tr2_track_tbl[063] = {file = "059", mode = StreamType.Oneshot}; -- Home sweet home
tr2_track_tbl[064] = {file = "060", mode = StreamType.Background}; -- Main theme
tr2_track_tbl[065] = {file = "061", mode = StreamType.Oneshot}; -- Dummy track


--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 3 ---------------------------------
--------------------------------------------------------------------------------

tr3_num_soundtracks = 124;
tr3_track_tbl   = {};

-- NAME BINDINGS FOR TR3 REMASTERED SOUNDTRACK BY TOMEKKOBIALKA.

tr3_track_tbl[002] = {file = "puzzle element v2", mode = StreamType.Oneshot};
tr3_track_tbl[003] = {file = "no waiting around v3", mode = StreamType.Oneshot};
tr3_track_tbl[004] = {file = "something spooky v1", mode = StreamType.Oneshot};
tr3_track_tbl[005] = {file = "lara's theme v2", mode = StreamType.Background};
tr3_track_tbl[006] = {file = "cavern sewers v1", mode = StreamType.Oneshot};
tr3_track_tbl[007] = {file = "geordie bob v1", mode = StreamType.Oneshot};
tr3_track_tbl[008] = {file = "tony the loon v1", mode = StreamType.Oneshot};
tr3_track_tbl[009] = {file = "no waiting around 2 v2", mode = StreamType.Oneshot};
tr3_track_tbl[010] = {file = "greedy mob v1", mode = StreamType.Oneshot};
tr3_track_tbl[011] = {file = "long way up v1", mode = StreamType.Oneshot};
tr3_track_tbl[012] = {file = "no waiting around 3 v1", mode = StreamType.Oneshot};
tr3_track_tbl[013] = {file = "here be butterflies 2 v1", mode = StreamType.Oneshot};
tr3_track_tbl[014] = {file = "she's cool", mode = StreamType.Oneshot};
tr3_track_tbl[015] = {file = "mond the gap 2", mode = StreamType.Oneshot};
tr3_track_tbl[016] = {file = "around the corner 2", mode = StreamType.Oneshot};
tr3_track_tbl[017] = {file = "around the corner 1", mode = StreamType.Oneshot};
tr3_track_tbl[018] = {file = "kneel and pray", mode = StreamType.Oneshot};
tr3_track_tbl[019] = {file = "around the corner 4", mode = StreamType.Oneshot};
tr3_track_tbl[020] = {file = "around the corner 3", mode = StreamType.Oneshot};
tr3_track_tbl[021] = {file = "seeing is believeing 1", mode = StreamType.Oneshot};
tr3_track_tbl[022] = {file = "looky we here 3", mode = StreamType.Oneshot};
tr3_track_tbl[023] = {file = "here be butterfiles 4", mode = StreamType.Oneshot};
tr3_track_tbl[024] = {file = "stone crows 10", mode = StreamType.Oneshot};
tr3_track_tbl[025] = {file = "butterfiles 3", mode = StreamType.Oneshot};
tr3_track_tbl[026] = {file = "meteorite cavern", mode = StreamType.Background};
tr3_track_tbl[027] = {file = "steady", mode = StreamType.Background};
tr3_track_tbl[028] = {file = "28_Antarctica", mode = StreamType.Background};
tr3_track_tbl[029] = {file = "29_Things", mode = StreamType.Background};
tr3_track_tbl[030] = {file = "30_Anyone_There", mode = StreamType.Background};
tr3_track_tbl[031] = {file = "31_Grotto", mode = StreamType.Background};
tr3_track_tbl[032] = {file = "32_On_The_Beach", mode = StreamType.Background};
tr3_track_tbl[033] = {file = "33_Gamma_Pals", mode = StreamType.Background};
tr3_track_tbl[034] = {file = "34_In_The_Jungle", mode = StreamType.Background};
tr3_track_tbl[035] = {file = "35_Piranha_Waters", mode = StreamType.Background};
tr3_track_tbl[036] = {file = "36_The_Rapids", mode = StreamType.Background};
tr3_track_tbl[037] = {file = "37_Supper_Time", mode = StreamType.Oneshot};
tr3_track_tbl[038] = {file = "look out pt 5", mode = StreamType.Oneshot};
tr3_track_tbl[039] = {file = "looky pt 1", mode = StreamType.Oneshot};
tr3_track_tbl[040] = {file = "around the corner 5", mode = StreamType.Oneshot};
tr3_track_tbl[041] = {file = "seeing is believing 2", mode = StreamType.Oneshot};
tr3_track_tbl[042] = {file = "stone the crows 9", mode = StreamType.Oneshot};
tr3_track_tbl[043] = {file = "look out 8", mode = StreamType.Oneshot};
tr3_track_tbl[044] = {file = "look out 4", mode = StreamType.Oneshot};
tr3_track_tbl[045] = {file = "stone crows 7", mode = StreamType.Oneshot};
tr3_track_tbl[046] = {file = "stone crows 3", mode = StreamType.Oneshot};
tr3_track_tbl[047] = {file = "stone crows 8", mode = StreamType.Oneshot};
tr3_track_tbl[048] = {file = "looky here 2", mode = StreamType.Oneshot};
tr3_track_tbl[049] = {file = "stone crows 4", mode = StreamType.Oneshot};
tr3_track_tbl[050] = {file = "stone crows 6", mode = StreamType.Oneshot};
tr3_track_tbl[051] = {file = "look out 3", mode = StreamType.Oneshot};
tr3_track_tbl[052] = {file = "look out 1", mode = StreamType.Oneshot};
tr3_track_tbl[053] = {file = "there be butterflies here 1", mode = StreamType.Oneshot};
tr3_track_tbl[054] = {file = "stone crows 1", mode = StreamType.Oneshot};
tr3_track_tbl[055] = {file = "stone crows 5", mode = StreamType.Oneshot};
tr3_track_tbl[056] = {file = "mind the gap 1", mode = StreamType.Oneshot};
tr3_track_tbl[057] = {file = "butteflies 5", mode = StreamType.Oneshot};
tr3_track_tbl[058] = {file = "look out 2", mode = StreamType.Oneshot};
tr3_track_tbl[059] = {file = "look out 7", mode = StreamType.Oneshot};
tr3_track_tbl[060] = {file = "stone the crows 2", mode = StreamType.Oneshot};
tr3_track_tbl[061] = {file = "look out 6", mode = StreamType.Oneshot};
tr3_track_tbl[062] = {file = "scotts hut", mode = StreamType.Oneshot};
tr3_track_tbl[063] = {file = "cavern sewers cutscene", mode = StreamType.Oneshot};
tr3_track_tbl[064] = {file = "jungle camp cutscene", mode = StreamType.Oneshot};
tr3_track_tbl[065] = {file = "temple cutscene", mode = StreamType.Oneshot};
tr3_track_tbl[066] = {file = "cavern cutscene", mode = StreamType.Oneshot};
tr3_track_tbl[067] = {file = "rooftops cutscene", mode = StreamType.Oneshot};
tr3_track_tbl[068] = {file = "68_Tree-Shack_(English)", mode = StreamType.Oneshot};
tr3_track_tbl[069] = {file = "temple exit cutscene", mode = StreamType.Oneshot};
tr3_track_tbl[070] = {file = "delivery trcuk", mode = StreamType.Oneshot};
tr3_track_tbl[071] = {file = "penthouse cutscene", mode = StreamType.Oneshot};
tr3_track_tbl[072] = {file = "ravine cutscene", mode = StreamType.Oneshot};
tr3_track_tbl[073] = {file = "73_Old_Smokey", mode = StreamType.Background};
tr3_track_tbl[074] = {file = "74_Under_Smokey", mode = StreamType.Background};
tr3_track_tbl[075] = {file = "75_Refining_Plant", mode = StreamType.Background};
tr3_track_tbl[076] = {file = "76_Rumble_Sub", mode = StreamType.Background};
tr3_track_tbl[077] = {file = "77_Quake", mode = StreamType.Background};
tr3_track_tbl[078] = {file = "78_Blank", mode = StreamType.Oneshot};
tr3_track_tbl[082] = {file = "82", mode = StreamType.Chat};             -- Home block begin
tr3_track_tbl[083] = {file = "83", mode = StreamType.Chat};
tr3_track_tbl[084] = {file = "84", mode = StreamType.Chat};
tr3_track_tbl[085] = {file = "85", mode = StreamType.Chat};
tr3_track_tbl[086] = {file = "86", mode = StreamType.Chat};
tr3_track_tbl[087] = {file = "87", mode = StreamType.Chat};
tr3_track_tbl[088] = {file = "88", mode = StreamType.Chat};
tr3_track_tbl[089] = {file = "89", mode = StreamType.Chat};
tr3_track_tbl[090] = {file = "90", mode = StreamType.Chat};
tr3_track_tbl[091] = {file = "91", mode = StreamType.Chat};
tr3_track_tbl[092] = {file = "92", mode = StreamType.Chat};
tr3_track_tbl[093] = {file = "93", mode = StreamType.Chat};
tr3_track_tbl[094] = {file = "94", mode = StreamType.Chat};
tr3_track_tbl[095] = {file = "95", mode = StreamType.Chat};
tr3_track_tbl[096] = {file = "96", mode = StreamType.Chat};
tr3_track_tbl[097] = {file = "97", mode = StreamType.Chat};
tr3_track_tbl[098] = {file = "98", mode = StreamType.Chat};
tr3_track_tbl[099] = {file = "99", mode = StreamType.Chat};
tr3_track_tbl[100] = {file = "100", mode = StreamType.Chat};
tr3_track_tbl[101] = {file = "101", mode = StreamType.Chat};
tr3_track_tbl[102] = {file = "102", mode = StreamType.Chat};
tr3_track_tbl[103] = {file = "103", mode = StreamType.Chat};
tr3_track_tbl[104] = {file = "104", mode = StreamType.Chat};
tr3_track_tbl[105] = {file = "105", mode = StreamType.Chat};
tr3_track_tbl[106] = {file = "106", mode = StreamType.Chat};
tr3_track_tbl[107] = {file = "107", mode = StreamType.Chat};
tr3_track_tbl[108] = {file = "108", mode = StreamType.Chat};
tr3_track_tbl[109] = {file = "109", mode = StreamType.Chat};
tr3_track_tbl[110] = {file = "110", mode = StreamType.Chat};
tr3_track_tbl[111] = {file = "111", mode = StreamType.Chat};
tr3_track_tbl[112] = {file = "112", mode = StreamType.Chat};
tr3_track_tbl[113] = {file = "113", mode = StreamType.Chat};
tr3_track_tbl[114] = {file = "114", mode = StreamType.Chat};
tr3_track_tbl[115] = {file = "115", mode = StreamType.Chat};
tr3_track_tbl[116] = {file = "116", mode = StreamType.Chat};
tr3_track_tbl[117] = {file = "117", mode = StreamType.Chat};
tr3_track_tbl[118] = {file = "118", mode = StreamType.Chat};
tr3_track_tbl[119] = {file = "119", mode = StreamType.Chat};              -- Home block end
tr3_track_tbl[120] = {file = "120_In_The_Hut", mode = StreamType.Background};
tr3_track_tbl[121] = {file = "and so on", mode = StreamType.Oneshot};
tr3_track_tbl[122] = {file = "secret", mode = StreamType.Oneshot};
tr3_track_tbl[123] = {file = "secret", mode = StreamType.Oneshot};

--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 4 ---------------------------------
--------------------------------------------------------------------------------

tr4_num_soundtracks = 112;
tr4_track_tbl       = {};

tr4_track_tbl[000] = {file = "044_attack_part_i", mode = StreamType.Oneshot};
tr4_track_tbl[001] = {file = "008_voncroy9a", mode = StreamType.Chat};
tr4_track_tbl[002] = {file = "100_attack_part_ii", mode = StreamType.Oneshot};
tr4_track_tbl[003] = {file = "010_voncroy10", mode = StreamType.Chat};
tr4_track_tbl[004] = {file = "015_voncroy14", mode = StreamType.Chat};
tr4_track_tbl[005] = {file = "073_secret", mode = StreamType.Oneshot};
tr4_track_tbl[006] = {file = "109_lyre_01", mode = StreamType.Chat};
tr4_track_tbl[007] = {file = "042_action_part_iv", mode = StreamType.Oneshot};
tr4_track_tbl[008] = {file = "043_action_part_v", mode = StreamType.Oneshot};
tr4_track_tbl[009] = {file = "030_voncroy30", mode = StreamType.Chat};
tr4_track_tbl[010] = {file = "012_voncroy11b", mode = StreamType.Chat};
tr4_track_tbl[011] = {file = "011_voncroy11a", mode = StreamType.Chat};
tr4_track_tbl[012] = {file = "063_misc_inc_01", mode = StreamType.Oneshot};
tr4_track_tbl[013] = {file = "014_voncroy13b", mode = StreamType.Chat};
tr4_track_tbl[014] = {file = "111_charmer", mode = StreamType.Chat};
tr4_track_tbl[015] = {file = "025_voncroy24b", mode = StreamType.Chat};
tr4_track_tbl[016] = {file = "023_voncroy23", mode = StreamType.Chat};
tr4_track_tbl[017] = {file = "006_voncroy7", mode = StreamType.Chat};
tr4_track_tbl[018] = {file = "024_voncroy24a", mode = StreamType.Chat};
tr4_track_tbl[019] = {file = "110_lyre_02", mode = StreamType.Oneshot};
tr4_track_tbl[020] = {file = "020_voncroy19", mode = StreamType.Chat};
tr4_track_tbl[021] = {file = "034_voncroy34", mode = StreamType.Chat};
tr4_track_tbl[022] = {file = "054_general_part_ii", mode = StreamType.Oneshot};
tr4_track_tbl[023] = {file = "036_voncroy36", mode = StreamType.Chat};
tr4_track_tbl[024] = {file = "004_voncroy5", mode = StreamType.Chat};
tr4_track_tbl[025] = {file = "035_voncroy35", mode = StreamType.Chat};
tr4_track_tbl[026] = {file = "027_voncroy27", mode = StreamType.Chat};
tr4_track_tbl[027] = {file = "053_general_part_i", mode = StreamType.Oneshot};
tr4_track_tbl[028] = {file = "022_voncroy22b", mode = StreamType.Chat};
tr4_track_tbl[029] = {file = "028_voncroy28_l11", mode = StreamType.Chat};
tr4_track_tbl[030] = {file = "003_voncroy4", mode = StreamType.Chat};
tr4_track_tbl[031] = {file = "001_voncroy2", mode = StreamType.Chat};
tr4_track_tbl[032] = {file = "041_action_part_iii", mode = StreamType.Oneshot};
tr4_track_tbl[033] = {file = "057_general_part_v", mode = StreamType.Oneshot};
tr4_track_tbl[034] = {file = "018_voncroy17", mode = StreamType.Chat};
tr4_track_tbl[035] = {file = "064_misc_inc_02", mode = StreamType.Oneshot};
tr4_track_tbl[036] = {file = "033_voncroy33", mode = StreamType.Chat};
tr4_track_tbl[037] = {file = "031_voncroy31_l12", mode = StreamType.Chat};
tr4_track_tbl[038] = {file = "032_voncroy32_l13", mode = StreamType.Chat};
tr4_track_tbl[039] = {file = "016_voncroy15", mode = StreamType.Chat};
tr4_track_tbl[040] = {file = "065_misc_inc_03", mode = StreamType.Oneshot};
tr4_track_tbl[041] = {file = "040_action_part_ii", mode = StreamType.Oneshot};
tr4_track_tbl[042] = {file = "112_gods_part_iv", mode = StreamType.Oneshot};
tr4_track_tbl[043] = {file = "029_voncroy29", mode = StreamType.Chat};
tr4_track_tbl[044] = {file = "007_voncroy8", mode = StreamType.Chat};
tr4_track_tbl[045] = {file = "013_voncroy12_13a_lara4", mode = StreamType.Chat};
tr4_track_tbl[046] = {file = "009_voncroy9b_lara3", mode = StreamType.Chat};
tr4_track_tbl[047] = {file = "081_dig", mode = StreamType.Oneshot};
tr4_track_tbl[048] = {file = "085_intro", mode = StreamType.Oneshot};
tr4_track_tbl[049] = {file = "071_ominous_part_i", mode = StreamType.Oneshot};
tr4_track_tbl[050] = {file = "095_phildoor", mode = StreamType.Oneshot};
tr4_track_tbl[051] = {file = "061_in_the_pyramid_part_i", mode = StreamType.Oneshot};
tr4_track_tbl[052] = {file = "050_underwater_find_part_i", mode = StreamType.Oneshot};
tr4_track_tbl[053] = {file = "058_gods_part_i", mode = StreamType.Oneshot};
tr4_track_tbl[054] = {file = "005_voncroy6_lara2", mode = StreamType.Chat};
tr4_track_tbl[055] = {file = "045_authentic_tr", mode = StreamType.Oneshot};
tr4_track_tbl[056] = {file = "060_gods_part_iii", mode = StreamType.Oneshot};
tr4_track_tbl[057] = {file = "055_general_part_iii", mode = StreamType.Oneshot};
tr4_track_tbl[058] = {file = "059_gods_part_ii", mode = StreamType.Oneshot};
tr4_track_tbl[059] = {file = "068_mystery_part_ii", mode = StreamType.Oneshot};
tr4_track_tbl[060] = {file = "076_captain2", mode = StreamType.Oneshot};
tr4_track_tbl[061] = {file = "019_lara6_voncroy18", mode = StreamType.Oneshot};
tr4_track_tbl[062] = {file = "002_voncroy3", mode = StreamType.Chat};
tr4_track_tbl[063] = {file = "066_misc_inc_04", mode = StreamType.Oneshot};
tr4_track_tbl[064] = {file = "067_mystery_part_i", mode = StreamType.Oneshot};
tr4_track_tbl[065] = {file = "038_a_short_01", mode = StreamType.Oneshot};
tr4_track_tbl[066] = {file = "088_key", mode = StreamType.Oneshot};
tr4_track_tbl[067] = {file = "017_voncroy16_lara5", mode = StreamType.Chat};
tr4_track_tbl[068] = {file = "026_vc25_l9_vc26_l10", mode = StreamType.Chat};
tr4_track_tbl[069] = {file = "056_general_part_iv", mode = StreamType.Oneshot};
tr4_track_tbl[070] = {file = "021_vc20_l7_vc21_l8_vc22a", mode = StreamType.Chat};
tr4_track_tbl[071] = {file = "096_sarcoph", mode = StreamType.Oneshot};
tr4_track_tbl[072] = {file = "087_jeepb", mode = StreamType.Oneshot};
tr4_track_tbl[073] = {file = "091_minilib1", mode = StreamType.Oneshot};
tr4_track_tbl[074] = {file = "086_jeepa", mode = StreamType.Oneshot};
tr4_track_tbl[075] = {file = "051_egyptian_mood_part_i", mode = StreamType.Oneshot};
tr4_track_tbl[076] = {file = "078_croywon", mode = StreamType.Oneshot};
tr4_track_tbl[077] = {file = "092_minilib2", mode = StreamType.Oneshot};
tr4_track_tbl[078] = {file = "083_horus", mode = StreamType.Oneshot};
tr4_track_tbl[079] = {file = "049_close_to_the_end_part_ii", mode = StreamType.Oneshot};
tr4_track_tbl[080] = {file = "037_vc37_l15_vc38", mode = StreamType.Chat};
tr4_track_tbl[081] = {file = "097_scorpion", mode = StreamType.Oneshot};
tr4_track_tbl[082] = {file = "089_larawon", mode = StreamType.Oneshot};
tr4_track_tbl[083] = {file = "094_minilib4", mode = StreamType.Oneshot};
tr4_track_tbl[084] = {file = "098_throne", mode = StreamType.Oneshot};
tr4_track_tbl[085] = {file = "048_close_to_the_end", mode = StreamType.Oneshot};
tr4_track_tbl[086] = {file = "070_mystery_part_iv", mode = StreamType.Oneshot};
tr4_track_tbl[087] = {file = "093_minilib3", mode = StreamType.Oneshot};
tr4_track_tbl[088] = {file = "072_puzzle_part_i", mode = StreamType.Oneshot};
tr4_track_tbl[089] = {file = "074_backpack", mode = StreamType.Oneshot};
tr4_track_tbl[090] = {file = "069_mystery_part_iii", mode = StreamType.Oneshot};
tr4_track_tbl[091] = {file = "052_egyptian_mood_part_ii", mode = StreamType.Oneshot};
tr4_track_tbl[092] = {file = "084_inscrip", mode = StreamType.Oneshot};
tr4_track_tbl[093] = {file = "099_whouse", mode = StreamType.Oneshot};
tr4_track_tbl[094] = {file = "047_boss_02", mode = StreamType.Oneshot};
tr4_track_tbl[095] = {file = "080_crypt2", mode = StreamType.Oneshot};
tr4_track_tbl[096] = {file = "090_libend", mode = StreamType.Oneshot};
tr4_track_tbl[097] = {file = "046_boss_01", mode = StreamType.Oneshot};
tr4_track_tbl[098] = {file = "062_jeep_thrills_max", mode = StreamType.Oneshot};
tr4_track_tbl[099] = {file = "079_crypt1", mode = StreamType.Oneshot};
tr4_track_tbl[100] = {file = "082_finale", mode = StreamType.Oneshot};
tr4_track_tbl[101] = {file = "075_captain1", mode = StreamType.Oneshot};
tr4_track_tbl[102] = {file = "105_a5_battle", mode = StreamType.Background};
tr4_track_tbl[103] = {file = "077_crocgod", mode = StreamType.Oneshot};
tr4_track_tbl[104] = {file = "039_tr4_title_q10", mode = StreamType.Background};
tr4_track_tbl[105] = {file = "108_a8_coastal", mode = StreamType.Background};
tr4_track_tbl[106] = {file = "107_a7_train+", mode = StreamType.Background};
tr4_track_tbl[107] = {file = "101_a1_in_dark", mode = StreamType.Background};
tr4_track_tbl[108] = {file = "102_a2_in_drips", mode = StreamType.Background};
tr4_track_tbl[109] = {file = "104_a4_weird1", mode = StreamType.Background};
tr4_track_tbl[110] = {file = "106_a6_out_day", mode = StreamType.Background};
tr4_track_tbl[111] = {file = "103_a3_out_night", mode = StreamType.Background};


--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 5 ---------------------------------
--------------------------------------------------------------------------------

tr5_num_soundtracks = 136;
tr5_track_tbl       = {};

tr5_track_tbl[000] = {file = "xa1_TL_10B", mode = StreamType.Chat};
tr5_track_tbl[001] = {file = "xa1_Z_10", mode = StreamType.Chat};
tr5_track_tbl[002] = {file = "xa1_TL_05", mode = StreamType.Chat};
tr5_track_tbl[003] = {file = "xa1_TL_08", mode = StreamType.Chat};
tr5_track_tbl[004] = {file = "xa1_TL_11", mode = StreamType.Chat};
tr5_track_tbl[005] = {file = "xa1_ANDYPEW", mode = StreamType.Oneshot};
tr5_track_tbl[006] = {file = "xa1_SECRET", mode = StreamType.Oneshot};
tr5_track_tbl[007] = {file = "xa1_TL_02", mode = StreamType.Chat};
tr5_track_tbl[008] = {file = "xa2_HMMM05", mode = StreamType.Oneshot};
tr5_track_tbl[009] = {file = "xa2_TL_01", mode = StreamType.Chat};
tr5_track_tbl[010] = {file = "xa2_ATTACK04", mode = StreamType.Oneshot};
tr5_track_tbl[011] = {file = "xa2_UWATER2B", mode = StreamType.Oneshot};
tr5_track_tbl[012] = {file = "xa2_SPOOKY2A", mode = StreamType.Oneshot};
tr5_track_tbl[013] = {file = "xa2_TL_10A", mode = StreamType.Chat};
tr5_track_tbl[014] = {file = "xa2_HMMM02", mode = StreamType.Oneshot};
tr5_track_tbl[015] = {file = "xa2_TOMS01", mode = StreamType.Oneshot};
tr5_track_tbl[016] = {file = "xa3_Attack03", mode = StreamType.Oneshot};
tr5_track_tbl[017] = {file = "xa3_Attack02", mode = StreamType.Oneshot};
tr5_track_tbl[018] = {file = "xa3_Hmmm01", mode = StreamType.Oneshot};
tr5_track_tbl[019] = {file = "xa3_Stealth1", mode = StreamType.Oneshot};
tr5_track_tbl[020] = {file = "xa3_Stealth2", mode = StreamType.Oneshot};
tr5_track_tbl[021] = {file = "xa3_Attack01", mode = StreamType.Oneshot};
tr5_track_tbl[022] = {file = "xa3_TL_06", mode = StreamType.Chat};
tr5_track_tbl[023] = {file = "xa3_TL_03", mode = StreamType.Chat};
tr5_track_tbl[024] = {file = "xa4_hmmm06", mode = StreamType.Oneshot};
tr5_track_tbl[025] = {file = "xa4_mil01", mode = StreamType.Oneshot};
tr5_track_tbl[026] = {file = "xa4_Z_03", mode = StreamType.Chat};
tr5_track_tbl[027] = {file = "xa4_hit01", mode = StreamType.Oneshot};
tr5_track_tbl[028] = {file = "xa4_spooky05", mode = StreamType.Oneshot};
tr5_track_tbl[029] = {file = "xa4_drama01", mode = StreamType.Oneshot};
tr5_track_tbl[030] = {file = "xa4_stealth4", mode = StreamType.Oneshot};
tr5_track_tbl[031] = {file = "xa4_mil05", mode = StreamType.Oneshot};
tr5_track_tbl[032] = {file = "xa5_HMMM04", mode = StreamType.Oneshot};
tr5_track_tbl[033] = {file = "xa5_MIL06", mode = StreamType.Oneshot};
tr5_track_tbl[034] = {file = "xa5_SPOOKY02", mode = StreamType.Oneshot};
tr5_track_tbl[035] = {file = "xa5_TL_12", mode = StreamType.Chat};
tr5_track_tbl[036] = {file = "xa5_MIL02A", mode = StreamType.Oneshot};
tr5_track_tbl[037] = {file = "xa5_HMMM03", mode = StreamType.Oneshot};
tr5_track_tbl[038] = {file = "xa5_MIL02", mode = StreamType.Oneshot};
tr5_track_tbl[039] = {file = "xa5_TL_04", mode = StreamType.Chat};
tr5_track_tbl[040] = {file = "xa6_Mil04", mode = StreamType.Oneshot};
tr5_track_tbl[041] = {file = "xa6_Solo01", mode = StreamType.Oneshot};
tr5_track_tbl[042] = {file = "xa6_Z12", mode = StreamType.Chat};
tr5_track_tbl[043] = {file = "xa6_Stealth3", mode = StreamType.Oneshot};
tr5_track_tbl[044] = {file = "xa6_AuthSolo", mode = StreamType.Oneshot};
tr5_track_tbl[045] = {file = "xa6_Spooky03", mode = StreamType.Oneshot};
tr5_track_tbl[046] = {file = "xa6_Z13", mode = StreamType.Chat};
tr5_track_tbl[047] = {file = "xa6_Z_04anim", mode = StreamType.Chat};
tr5_track_tbl[048] = {file = "xa7_z_06a", mode = StreamType.Chat};
tr5_track_tbl[049] = {file = "xa7_andyoooh", mode = StreamType.Oneshot};
tr5_track_tbl[050] = {file = "xa7_andyooer", mode = StreamType.Oneshot};
tr5_track_tbl[051] = {file = "xa7_tl_07", mode = StreamType.Chat};
tr5_track_tbl[052] = {file = "xa7_z_02", mode = StreamType.Chat};
tr5_track_tbl[053] = {file = "xa7_evibes01", mode = StreamType.Oneshot};
tr5_track_tbl[054] = {file = "xa7_z_06", mode = StreamType.Chat};
tr5_track_tbl[055] = {file = "xa7_authtr", mode = StreamType.Oneshot};
tr5_track_tbl[056] = {file = "xa8_mil03", mode = StreamType.Oneshot};
tr5_track_tbl[057] = {file = "xa8_fightsc", mode = StreamType.Oneshot};
tr5_track_tbl[058] = {file = "xa8_richcut3", mode = StreamType.Oneshot};
tr5_track_tbl[059] = {file = "xa8_z_13", mode = StreamType.Chat};
tr5_track_tbl[060] = {file = "xa8_z_08", mode = StreamType.Chat};
tr5_track_tbl[061] = {file = "xa8_uwater2a", mode = StreamType.Oneshot};
tr5_track_tbl[062] = {file = "xa8_jobyalrm", mode = StreamType.Oneshot};
tr5_track_tbl[063] = {file = "xa8_mil02b", mode = StreamType.Oneshot};
tr5_track_tbl[064] = {file = "xa9_swampy", mode = StreamType.Oneshot};
tr5_track_tbl[065] = {file = "xa9_evibes02", mode = StreamType.Oneshot};
tr5_track_tbl[066] = {file = "xa9_gods01", mode = StreamType.Oneshot};
tr5_track_tbl[067] = {file = "xa9_z_03", mode = StreamType.Chat};
tr5_track_tbl[068] = {file = "xa9_richcut4", mode = StreamType.Oneshot};
tr5_track_tbl[069] = {file = "xa9_title4", mode = StreamType.Oneshot};
tr5_track_tbl[070] = {file = "xa9_spooky01", mode = StreamType.Oneshot};
tr5_track_tbl[071] = {file = "xa9_chopin01", mode = StreamType.Oneshot};
tr5_track_tbl[072] = {file = "xa10_echoir01", mode = StreamType.Oneshot};
tr5_track_tbl[073] = {file = "xa10_title3", mode = StreamType.Oneshot};
tr5_track_tbl[074] = {file = "xa10_perc01", mode = StreamType.Oneshot};
tr5_track_tbl[075] = {file = "xa10_vc01", mode = StreamType.Oneshot};
tr5_track_tbl[076] = {file = "xa10_title2", mode = StreamType.Oneshot};
tr5_track_tbl[077] = {file = "xa10_z_09", mode = StreamType.Chat};
tr5_track_tbl[078] = {file = "xa10_spooky04", mode = StreamType.Oneshot};
tr5_track_tbl[079] = {file = "xa10_z_10", mode = StreamType.Chat};
tr5_track_tbl[080] = {file = "xa11_vc01atv", mode = StreamType.Oneshot};
tr5_track_tbl[081] = {file = "xa11_andy3", mode = StreamType.Oneshot};
tr5_track_tbl[082] = {file = "xa11_title1", mode = StreamType.Oneshot};
tr5_track_tbl[083] = {file = "xa11_flyby1", mode = StreamType.Oneshot};
tr5_track_tbl[084] = {file = "xa11_monk_2", mode = StreamType.Oneshot};
tr5_track_tbl[085] = {file = "xa11_andy4", mode = StreamType.Oneshot};
tr5_track_tbl[086] = {file = "xa11_flyby3", mode = StreamType.Oneshot};
tr5_track_tbl[087] = {file = "xa11_flyby2", mode = StreamType.Oneshot};
tr5_track_tbl[088] = {file = "xa12_moses01", mode = StreamType.Oneshot};
tr5_track_tbl[089] = {file = "xa12_andy4b", mode = StreamType.Oneshot};
tr5_track_tbl[090] = {file = "xa12_z_10", mode = StreamType.Chat};
tr5_track_tbl[091] = {file = "xa12_flyby4", mode = StreamType.Oneshot};
tr5_track_tbl[092] = {file = "xa12_richcut1", mode = StreamType.Oneshot};
tr5_track_tbl[093] = {file = "xa12_andy5", mode = StreamType.Oneshot};
tr5_track_tbl[094] = {file = "xa12_z_05", mode = StreamType.Chat};
tr5_track_tbl[095] = {file = "xa12_z_01", mode = StreamType.Chat};
tr5_track_tbl[096] = {file = "xa13_Joby3", mode = StreamType.Oneshot};
tr5_track_tbl[097] = {file = "xa13_Andy7", mode = StreamType.Oneshot};
tr5_track_tbl[098] = {file = "xa13_Andrea3B", mode = StreamType.Oneshot};
tr5_track_tbl[099] = {file = "xa13_cossack", mode = StreamType.Oneshot};
tr5_track_tbl[100] = {file = "xa13_Z_07", mode = StreamType.Chat};
tr5_track_tbl[101] = {file = "xa13_Andy6", mode = StreamType.Oneshot};
tr5_track_tbl[102] = {file = "xa13_Andrea3", mode = StreamType.Oneshot};
tr5_track_tbl[103] = {file = "xa13_Joby7", mode = StreamType.Oneshot};
tr5_track_tbl[104] = {file = "xa14_uwater1", mode = StreamType.Oneshot};
tr5_track_tbl[105] = {file = "xa14_joby1", mode = StreamType.Oneshot};
tr5_track_tbl[106] = {file = "xa14_andy10", mode = StreamType.Oneshot};
tr5_track_tbl[107] = {file = "xa14_richcut2", mode = StreamType.Oneshot};
tr5_track_tbl[108] = {file = "xa14_andrea1", mode = StreamType.Oneshot};
tr5_track_tbl[109] = {file = "xa14_andy8", mode = StreamType.Oneshot};
tr5_track_tbl[110] = {file = "xa14_joby6", mode = StreamType.Oneshot};
tr5_track_tbl[111] = {file = "xa14_ecredits", mode = StreamType.Background};
tr5_track_tbl[112] = {file = "xa15_boss_01", mode = StreamType.Oneshot};
tr5_track_tbl[113] = {file = "xa15_joby2", mode = StreamType.Oneshot};
tr5_track_tbl[114] = {file = "xa15_joby4", mode = StreamType.Oneshot};
tr5_track_tbl[115] = {file = "xa15_joby5", mode = StreamType.Oneshot};
tr5_track_tbl[116] = {file = "xa15_joby9", mode = StreamType.Oneshot};
tr5_track_tbl[117] = {file = "xa15_a_andy", mode = StreamType.Background};
tr5_track_tbl[118] = {file = "xa15_a_rome", mode = StreamType.Background};
tr5_track_tbl[119] = {file = "xa15_andy2", mode = StreamType.Oneshot};
tr5_track_tbl[120] = {file = "xa16_Joby8", mode = StreamType.Oneshot};
tr5_track_tbl[121] = {file = "xa16_A_Sub_Amb", mode = StreamType.Background};
tr5_track_tbl[122] = {file = "xa16_Joby10", mode = StreamType.Oneshot};
tr5_track_tbl[123] = {file = "xa16_A_Harbour_out", mode = StreamType.Background};
tr5_track_tbl[124] = {file = "xa16_A_Andy_Out_Norm", mode = StreamType.Background};
tr5_track_tbl[125] = {file = "xa16_A_Andy_Out_Spooky", mode = StreamType.Background};
tr5_track_tbl[126] = {file = "xa16_A_Rome_Day", mode = StreamType.Background}
tr5_track_tbl[127] = {file = "xa16_A_Underwater", mode = StreamType.Background};
tr5_track_tbl[128] = {file = "xa17_A_Rome_Night", mode = StreamType.Background};
tr5_track_tbl[129] = {file = "xa17_A_VC_Saga", mode = StreamType.Background};
tr5_track_tbl[130] = {file = "xa17_A_Industrg", mode = StreamType.Background}
tr5_track_tbl[131] = {file = "xa17_Andrea2", mode = StreamType.Oneshot};
tr5_track_tbl[132] = {file = "xa17_Andy1", mode = StreamType.Oneshot};
tr5_track_tbl[133] = {file = "xa17_Andrea4", mode = StreamType.Oneshot};
tr5_track_tbl[134] = {file = "xa17_Andy9", mode = StreamType.Oneshot};
tr5_track_tbl[135] = {file = "xa17_Andy11", mode = StreamType.Oneshot};


function getTrackInfo(engine, id)
    local tbl = {};
    local path, method, ext;
    
    if(engine == Engine.I) then
        tbl    = tr1_track_tbl;
        path   = "data/tr1/audio/";
        method = StreamMethod.Track;
        ext    = ".ogg";
    elseif(engine == Engine.II) then
        tbl    = tr2_track_tbl;
        path   = "data/tr2/audio/";
        method = StreamMethod.Track;
        ext    = ".ogg";
    elseif(engine == Engine.III) then
        tbl    = tr3_track_tbl;
        if(USE_TR3_REMASTER == 1) then
            path   = "data/tr3/audio/";
            method = StreamMethod.Track;
            ext    = ".ogg";
        else
            path   = "data/tr3/audio/cdaudio.wad";
            method = StreamMethod.WAD;
        end
    elseif(engine == Engine.IV) then
        tbl    = tr4_track_tbl;
        path   = "data/tr4/audio/";
        method = StreamMethod.Track;
        ext    = ".wav";
    elseif(engine == Engine.V) then
        tbl    = tr5_track_tbl;
        path   = "data/tr5/audio/";
        method = StreamMethod.Track;
        ext    = ".wav";
    else
        return "NONE", -1, -1;
    end;
    
    if(tbl[id] == nil) then
        return "NONE", -1, -1;
    else
        if(method == StreamMethod.Track) then
            return (path .. tbl[id].file .. ext), tbl[id].mode, method;
        else
            return path, tbl[id].mode, method;
        end;
    end;
end;

function getSecretTrackNumber(engine)
    if(engine == Engine.I) then
        return SECRET_TR1;
    elseif(engine == Engine.II) then
        return SECRET_TR2;
    elseif(engine == Engine.III) then
        return SECRET_TR3;
    elseif(engine == Engine.IV) then
        return SECRET_TR4;
    elseif(engine == Engine.V) then
        return SECRET_TR5;
    else
        return 0;
    end;
end;

function getNumTracks(engine)
    if(engine == Engine.I) then
        return tr1_num_soundtracks;
    elseif(engine == Engine.II) then
        return tr2_num_soundtracks;
    elseif(engine == Engine.III) then
        return tr3_num_soundtracks;
    elseif(engine == Engine.IV) then
        return tr4_num_soundtracks;
    elseif(engine == Engine.V) then
        return tr5_num_soundtracks;
    else
        return 0;
    end;
end;
