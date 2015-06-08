-- OPENTOMB SOUNDTRACK MAPPER SCRIPT
-- by Lwmte, Apr 2014

--------------------------------------------------------------------------------
-- TR1/2 CD triggers contain indexes that refer to PSX version CD tracks;
-- however, in PC version, tracks were drastically rearranged. Additionally,
-- certain TR1 PC versions have extra ambience tracks and remapped secret
-- audiotrack, which also complicates things a lot. For all these reasons,
-- we need to remap audiotracks via script.
-- As for TR3-5, current OpenTomb audio engine supports only OGG streaming,
-- so we also need to remap these games to ogg tracks.

-- Additionally, soundtrack remapping is needed because OpenTomb uses three-
-- channel player, which sends chat tracks (Lara voice, etc.) to chat channel,
-- while one-shot and BGM tracks uses their own channels.

-- P.S.: It is recommended to replace original TR1 and TR3 soundtracks with
-- beautiful remastered versions by tomekkobialka.
-- TR1 remastered: http://www.tombraiderforums.com/showthread.php?t=187800
-- TR3 remastered: http://www.tombraiderforums.com/showthread.php?t=185513

--------------------------------------------------------------------------------
--  IF YOU WANT TO USE ORIGINAL TR3 TRACK NAME BINDINGS INSTEAD OF REMASTER, SET
--  UNDERLYING VARIABLE TO ZERO.
--------------------------------------------------------------------------------
USE_TR3_REMASTER = 1;
--------------------------------------------------------------------------------

-- Channel modes: 

LOOP = 0x0000;  -- Looped (background)
ONCE = 0x0001;  -- One-shot (action music)
CHAT = 0x0002;  -- Voice (Lara chatting, etc.)

-- Load methods:

OGG  = 0x0000;  -- OGG Vorbis tracks
WAD  = 0x0001;  -- WAD file (CDAUDIO.WAD), not yet supported.
WAV  = 0x0002;  -- MS-ADPCM WAV tracks, not yet supported.

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

tr1_track_tbl[002] = {file = "002.ogg", mode = LOOP}; -- Main theme
tr1_track_tbl[003] = {file = "011.ogg", mode = ONCE}; -- Poseidon
tr1_track_tbl[004] = {file = "012.ogg", mode = ONCE}; -- Main theme, short
tr1_track_tbl[005] = {file = "003.ogg", mode = LOOP}; -- Caves ambience
tr1_track_tbl[006] = {file = "013.ogg", mode = ONCE}; -- Thor
tr1_track_tbl[007] = {file = "014.ogg", mode = ONCE}; -- St. Francis
tr1_track_tbl[008] = {file = "015.ogg", mode = ONCE}; -- Danger
tr1_track_tbl[009] = {file = "016.ogg", mode = ONCE}; -- Stairs
tr1_track_tbl[010] = {file = "017.ogg", mode = ONCE}; -- Midas
tr1_track_tbl[011] = {file = "018.ogg", mode = ONCE}; -- Lever
tr1_track_tbl[012] = {file = "019.ogg", mode = ONCE}; -- Hmm...
tr1_track_tbl[013] = {file = "060.ogg", mode = ONCE}; -- Secret theme 
tr1_track_tbl[014] = {file = "020.ogg", mode = ONCE}; -- Big secret theme
tr1_track_tbl[015] = {file = "021.ogg", mode = ONCE}; -- Raiders
tr1_track_tbl[016] = {file = "022.ogg", mode = ONCE}; -- Wolf
tr1_track_tbl[017] = {file = "023.ogg", mode = ONCE}; -- Awe
tr1_track_tbl[018] = {file = "024.ogg", mode = ONCE}; -- Gods
tr1_track_tbl[019] = {file = "025.ogg", mode = ONCE}; -- Main theme, reprise
tr1_track_tbl[020] = {file = "026.ogg", mode = ONCE}; -- Mummy
tr1_track_tbl[021] = {file = "027.ogg", mode = ONCE}; -- Midas, reprise
tr1_track_tbl[022] = {file = "007.ogg", mode = ONCE}; -- Natla cutscene
tr1_track_tbl[023] = {file = "008.ogg", mode = ONCE}; -- Larson cutscene
tr1_track_tbl[024] = {file = "009.ogg", mode = ONCE}; -- Natla scion cutscene
tr1_track_tbl[025] = {file = "010.ogg", mode = ONCE}; -- Tihocan cutscene
tr1_track_tbl[026] = {file = "029.ogg", mode = CHAT}; -- Home block begin
tr1_track_tbl[027] = {file = "030.ogg", mode = CHAT};
tr1_track_tbl[028] = {file = "031.ogg", mode = CHAT};
tr1_track_tbl[029] = {file = "032.ogg", mode = CHAT};
tr1_track_tbl[030] = {file = "033.ogg", mode = CHAT};
tr1_track_tbl[031] = {file = "034.ogg", mode = CHAT};
tr1_track_tbl[032] = {file = "035.ogg", mode = CHAT};
tr1_track_tbl[033] = {file = "036.ogg", mode = CHAT};
tr1_track_tbl[034] = {file = "037.ogg", mode = CHAT};
tr1_track_tbl[035] = {file = "038.ogg", mode = CHAT};
tr1_track_tbl[036] = {file = "039.ogg", mode = CHAT};
tr1_track_tbl[037] = {file = "040.ogg", mode = CHAT};
tr1_track_tbl[038] = {file = "041.ogg", mode = CHAT};
tr1_track_tbl[039] = {file = "042.ogg", mode = CHAT};
tr1_track_tbl[040] = {file = "043.ogg", mode = CHAT};
tr1_track_tbl[041] = {file = "044.ogg", mode = CHAT};
tr1_track_tbl[042] = {file = "045.ogg", mode = CHAT};
tr1_track_tbl[043] = {file = "046.ogg", mode = CHAT};
tr1_track_tbl[044] = {file = "047.ogg", mode = CHAT};
tr1_track_tbl[045] = {file = "048.ogg", mode = CHAT};
tr1_track_tbl[046] = {file = "049.ogg", mode = CHAT};
tr1_track_tbl[047] = {file = "050.ogg", mode = CHAT};
tr1_track_tbl[048] = {file = "051.ogg", mode = CHAT};
tr1_track_tbl[049] = {file = "052.ogg", mode = CHAT};
tr1_track_tbl[050] = {file = "053.ogg", mode = CHAT};
tr1_track_tbl[051] = {file = "054.ogg", mode = CHAT}; -- Baddy 1
tr1_track_tbl[052] = {file = "055.ogg", mode = CHAT}; -- Baddy 2
tr1_track_tbl[053] = {file = "056.ogg", mode = CHAT}; -- Larson
tr1_track_tbl[054] = {file = "057.ogg", mode = CHAT}; -- Natla
tr1_track_tbl[055] = {file = "058.ogg", mode = CHAT}; -- Pierre
tr1_track_tbl[056] = {file = "059.ogg", mode = CHAT}; -- Skateboard kid
tr1_track_tbl[057] = {file = "028.ogg", mode = ONCE}; -- Silence
tr1_track_tbl[058] = {file = "004.ogg", mode = LOOP}; -- PC ONLY: Water ambience
tr1_track_tbl[059] = {file = "005.ogg", mode = LOOP}; -- PC ONLY: Wind  ambience
tr1_track_tbl[060] = {file = "006.ogg", mode = LOOP}; -- PC ONLY: Pulse ambience

--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 2 ---------------------------------
--------------------------------------------------------------------------------

tr2_num_soundtracks = 66;
tr2_track_tbl       = {};

tr2_track_tbl[002] = {file = "002.ogg", mode = ONCE}; -- Cutscene Gates
tr2_track_tbl[003] = {file = "002.ogg", mode = ONCE}; -- Cutscene Gates
tr2_track_tbl[004] = {file = "003.ogg", mode = ONCE}; -- Cutscene Plane
tr2_track_tbl[005] = {file = "004.ogg", mode = ONCE}; -- Cutscene Monk
tr2_track_tbl[006] = {file = "005.ogg", mode = CHAT}; -- Home block begin
tr2_track_tbl[007] = {file = "006.ogg", mode = CHAT};
tr2_track_tbl[008] = {file = "007.ogg", mode = CHAT};
tr2_track_tbl[009] = {file = "008.ogg", mode = CHAT}; 
tr2_track_tbl[010] = {file = "009.ogg", mode = CHAT};
tr2_track_tbl[011] = {file = "010.ogg", mode = CHAT};
tr2_track_tbl[012] = {file = "011.ogg", mode = CHAT};
tr2_track_tbl[013] = {file = "012.ogg", mode = CHAT};
tr2_track_tbl[014] = {file = "013.ogg", mode = CHAT};
tr2_track_tbl[015] = {file = "014.ogg", mode = CHAT};
tr2_track_tbl[016] = {file = "015.ogg", mode = CHAT};
tr2_track_tbl[017] = {file = "016.ogg", mode = CHAT};
tr2_track_tbl[018] = {file = "017.ogg", mode = CHAT};
tr2_track_tbl[019] = {file = "018.ogg", mode = CHAT};
tr2_track_tbl[020] = {file = "018.ogg", mode = CHAT};
tr2_track_tbl[021] = {file = "018.ogg", mode = CHAT};
tr2_track_tbl[022] = {file = "019.ogg", mode = CHAT};
tr2_track_tbl[023] = {file = "020.ogg", mode = CHAT};
tr2_track_tbl[024] = {file = "021.ogg", mode = CHAT};
tr2_track_tbl[025] = {file = "022.ogg", mode = CHAT};
tr2_track_tbl[026] = {file = "023.ogg", mode = ONCE}; -- Lara shower (Endgame)
tr2_track_tbl[027] = {file = "023.ogg", mode = ONCE}; -- Lara shower (Endgame)
tr2_track_tbl[028] = {file = "024.ogg", mode = ONCE}; -- Dragon death
tr2_track_tbl[029] = {file = "025.ogg", mode = CHAT}; -- Home - addon
tr2_track_tbl[030] = {file = "026.ogg", mode = ONCE}; -- Cutscene Bartoli stab
tr2_track_tbl[031] = {file = "027.ogg", mode = LOOP}; -- Caves ambience
tr2_track_tbl[032] = {file = "028.ogg", mode = LOOP}; -- Water ambience
tr2_track_tbl[033] = {file = "029.ogg", mode = LOOP}; -- Wind  ambience
tr2_track_tbl[034] = {file = "030.ogg", mode = LOOP}; -- Pulse ambience
tr2_track_tbl[035] = {file = "031.ogg", mode = ONCE}; -- Danger 1
tr2_track_tbl[036] = {file = "032.ogg", mode = ONCE}; -- Danger 2
tr2_track_tbl[037] = {file = "033.ogg", mode = ONCE}; -- Danger 3
tr2_track_tbl[038] = {file = "034.ogg", mode = ONCE}; -- Sacred
tr2_track_tbl[039] = {file = "035.ogg", mode = ONCE}; -- Awe
tr2_track_tbl[040] = {file = "036.ogg", mode = ONCE}; -- Venice Violins
tr2_track_tbl[041] = {file = "037.ogg", mode = ONCE}; -- End level
tr2_track_tbl[042] = {file = "038.ogg", mode = ONCE}; -- Mystical
tr2_track_tbl[043] = {file = "039.ogg", mode = ONCE}; -- Revelation
tr2_track_tbl[044] = {file = "040.ogg", mode = ONCE}; -- Careful
tr2_track_tbl[045] = {file = "041.ogg", mode = ONCE}; -- Guitar TR
tr2_track_tbl[046] = {file = "042.ogg", mode = ONCE}; -- Drama
tr2_track_tbl[047] = {file = "043.ogg", mode = ONCE}; -- Secret theme
tr2_track_tbl[048] = {file = "044.ogg", mode = ONCE}; -- It's coming!
tr2_track_tbl[049] = {file = "045.ogg", mode = ONCE}; -- It's coming 2!
tr2_track_tbl[050] = {file = "046.ogg", mode = ONCE}; -- Warning!
tr2_track_tbl[051] = {file = "047.ogg", mode = ONCE}; -- Warning 2!
tr2_track_tbl[052] = {file = "048.ogg", mode = ONCE}; -- Techno TR
tr2_track_tbl[053] = {file = "049.ogg", mode = ONCE}; -- Percussion
tr2_track_tbl[054] = {file = "050.ogg", mode = ONCE}; -- Pads
tr2_track_tbl[055] = {file = "051.ogg", mode = ONCE}; -- Super-revelation
tr2_track_tbl[056] = {file = "052.ogg", mode = ONCE}; -- Hmm...
tr2_track_tbl[057] = {file = "053.ogg", mode = ONCE}; -- Long way up
tr2_track_tbl[058] = {file = "054.ogg", mode = LOOP}; -- Industrial ambience
tr2_track_tbl[059] = {file = "055.ogg", mode = LOOP}; -- Spooky ambience
tr2_track_tbl[060] = {file = "056.ogg", mode = ONCE}; -- Barkhang theme
tr2_track_tbl[061] = {file = "057.ogg", mode = ONCE}; -- Super-revelation short
tr2_track_tbl[062] = {file = "058.ogg", mode = CHAT}; -- Monk beaten
tr2_track_tbl[063] = {file = "059.ogg", mode = ONCE}; -- Home sweet home
tr2_track_tbl[064] = {file = "060.ogg", mode = LOOP}; -- Main theme
tr2_track_tbl[065] = {file = "061.ogg", mode = ONCE}; -- Dummy track


--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 3 ---------------------------------
--------------------------------------------------------------------------------

tr3_num_soundtracks = 124;
tr3_new_track_tbl   = {};

-- NAME BINDINGS FOR TR3 REMASTERED SOUNDTRACK BY TOMEKKOBIALKA.

tr3_new_track_tbl[002] = {file = "puzzle element v2.ogg", mode = ONCE};
tr3_new_track_tbl[003] = {file = "no waiting around v3.ogg", mode = ONCE};
tr3_new_track_tbl[004] = {file = "something spooky v1.ogg", mode = ONCE};
tr3_new_track_tbl[005] = {file = "lara's theme v2.ogg", mode = LOOP};
tr3_new_track_tbl[006] = {file = "cavern sewers v1.ogg", mode = ONCE};
tr3_new_track_tbl[007] = {file = "geordie bob v1.ogg", mode = ONCE};
tr3_new_track_tbl[008] = {file = "tony the loon v1.ogg", mode = ONCE};
tr3_new_track_tbl[009] = {file = "no waiting around 2 v2.ogg", mode = ONCE};
tr3_new_track_tbl[010] = {file = "greedy mob v1.ogg", mode = ONCE};
tr3_new_track_tbl[011] = {file = "long way up v1.ogg", mode = ONCE};
tr3_new_track_tbl[012] = {file = "no waiting around 3 v1.ogg", mode = ONCE};
tr3_new_track_tbl[013] = {file = "here be butterflies 2 v1.ogg", mode = ONCE};
tr3_new_track_tbl[014] = {file = "she's cool.ogg", mode = ONCE};
tr3_new_track_tbl[015] = {file = "mond the gap 2.ogg", mode = ONCE};
tr3_new_track_tbl[016] = {file = "around the corner 2.ogg", mode = ONCE};
tr3_new_track_tbl[017] = {file = "around the corner 1.ogg", mode = ONCE};
tr3_new_track_tbl[018] = {file = "kneel and pray.ogg", mode = ONCE};
tr3_new_track_tbl[019] = {file = "around the corner 4.ogg", mode = ONCE};
tr3_new_track_tbl[020] = {file = "around the corner 3.ogg", mode = ONCE};
tr3_new_track_tbl[021] = {file = "seeing is believeing 1.ogg", mode = ONCE};
tr3_new_track_tbl[022] = {file = "looky we here 3.ogg", mode = ONCE};
tr3_new_track_tbl[023] = {file = "here be butterfiles 4.ogg", mode = ONCE};
tr3_new_track_tbl[024] = {file = "stone crows 10.ogg", mode = ONCE};
tr3_new_track_tbl[025] = {file = "butterfiles 3.ogg", mode = ONCE};
tr3_new_track_tbl[026] = {file = "meteorite cavern.ogg", mode = LOOP};
tr3_new_track_tbl[027] = {file = "steady.ogg", mode = LOOP};
tr3_new_track_tbl[028] = {file = "28_Antarctica.ogg", mode = LOOP};
tr3_new_track_tbl[029] = {file = "29_Things.ogg", mode = LOOP};
tr3_new_track_tbl[030] = {file = "30_Anyone_There.ogg", mode = LOOP};
tr3_new_track_tbl[031] = {file = "31_Grotto.ogg", mode = LOOP};
tr3_new_track_tbl[032] = {file = "32_On_The_Beach.ogg", mode = LOOP};
tr3_new_track_tbl[033] = {file = "33_Gamma_Pals.ogg", mode = LOOP};
tr3_new_track_tbl[034] = {file = "34_In_The_Jungle.ogg", mode = LOOP};
tr3_new_track_tbl[035] = {file = "35_Piranha_Waters.ogg", mode = LOOP};
tr3_new_track_tbl[036] = {file = "36_The_Rapids.ogg", mode = LOOP};
tr3_new_track_tbl[037] = {file = "37_Supper_Time.ogg", mode = ONCE};
tr3_new_track_tbl[038] = {file = "look out pt 5.ogg", mode = ONCE};
tr3_new_track_tbl[039] = {file = "looky pt 1.ogg", mode = ONCE};
tr3_new_track_tbl[040] = {file = "around the corner 5.ogg", mode = ONCE};
tr3_new_track_tbl[041] = {file = "seeing is believing 2.ogg", mode = ONCE};
tr3_new_track_tbl[042] = {file = "stone the crows 9.ogg", mode = ONCE};
tr3_new_track_tbl[043] = {file = "look out 8.ogg", mode = ONCE};
tr3_new_track_tbl[044] = {file = "look out 4.ogg", mode = ONCE};
tr3_new_track_tbl[045] = {file = "stone crows 7.ogg", mode = ONCE};
tr3_new_track_tbl[046] = {file = "stone crows 3.ogg", mode = ONCE};
tr3_new_track_tbl[047] = {file = "stone crows 8.ogg", mode = ONCE};
tr3_new_track_tbl[048] = {file = "looky here 2.ogg", mode = ONCE};
tr3_new_track_tbl[049] = {file = "stone crows 4.ogg", mode = ONCE};
tr3_new_track_tbl[050] = {file = "stone crows 6.ogg", mode = ONCE};
tr3_new_track_tbl[051] = {file = "look out 3.ogg", mode = ONCE};
tr3_new_track_tbl[052] = {file = "look out 1.ogg", mode = ONCE};
tr3_new_track_tbl[053] = {file = "there be butterflies here 1.ogg", mode = ONCE};
tr3_new_track_tbl[054] = {file = "stone crows 1.ogg", mode = ONCE};
tr3_new_track_tbl[055] = {file = "stone crows 5.ogg", mode = ONCE};
tr3_new_track_tbl[056] = {file = "mind the gap 1.ogg", mode = ONCE};
tr3_new_track_tbl[057] = {file = "butteflies 5.ogg", mode = ONCE};
tr3_new_track_tbl[058] = {file = "look out 2.ogg", mode = ONCE};
tr3_new_track_tbl[059] = {file = "look out 7.ogg", mode = ONCE};
tr3_new_track_tbl[060] = {file = "stone the crows 2.ogg", mode = ONCE};
tr3_new_track_tbl[061] = {file = "look out 6.ogg", mode = ONCE};
tr3_new_track_tbl[062] = {file = "scotts hut.ogg", mode = ONCE};
tr3_new_track_tbl[063] = {file = "cavern sewers cutscene.ogg", mode = ONCE};
tr3_new_track_tbl[064] = {file = "jungle camp cutscene.ogg", mode = ONCE};
tr3_new_track_tbl[065] = {file = "temple cutscene.ogg", mode = ONCE};
tr3_new_track_tbl[066] = {file = "cavern cutscene.ogg", mode = ONCE};
tr3_new_track_tbl[067] = {file = "rooftops cutscene.ogg", mode = ONCE};
tr3_new_track_tbl[068] = {file = "68_Tree-Shack_(English).ogg", mode = ONCE};
tr3_new_track_tbl[069] = {file = "temple exit cutscene.ogg", mode = ONCE};
tr3_new_track_tbl[070] = {file = "delivery trcuk.ogg", mode = ONCE};
tr3_new_track_tbl[071] = {file = "penthouse cutscene.ogg", mode = ONCE};
tr3_new_track_tbl[072] = {file = "ravine cutscene.ogg", mode = ONCE};
tr3_new_track_tbl[073] = {file = "73_Old_Smokey.ogg", mode = LOOP};
tr3_new_track_tbl[074] = {file = "74_Under_Smokey.ogg", mode = LOOP};
tr3_new_track_tbl[075] = {file = "75_Refining_Plant.ogg", mode = LOOP};
tr3_new_track_tbl[076] = {file = "76_Rumble_Sub.ogg", mode = LOOP};
tr3_new_track_tbl[077] = {file = "77_Quake.ogg", mode = LOOP};
tr3_new_track_tbl[078] = {file = "78_Blank.ogg", mode = ONCE};
tr3_new_track_tbl[082] = {file = "82.ogg", mode = CHAT};        -- Home block begin
tr3_new_track_tbl[083] = {file = "83.ogg", mode = CHAT};
tr3_new_track_tbl[084] = {file = "84.ogg", mode = CHAT};
tr3_new_track_tbl[085] = {file = "85.ogg", mode = CHAT};
tr3_new_track_tbl[086] = {file = "86.ogg", mode = CHAT};
tr3_new_track_tbl[087] = {file = "87.ogg", mode = CHAT};
tr3_new_track_tbl[088] = {file = "88.ogg", mode = CHAT};
tr3_new_track_tbl[089] = {file = "89.ogg", mode = CHAT};
tr3_new_track_tbl[090] = {file = "90.ogg", mode = CHAT};
tr3_new_track_tbl[091] = {file = "91.ogg", mode = CHAT};
tr3_new_track_tbl[092] = {file = "92.ogg", mode = CHAT};
tr3_new_track_tbl[093] = {file = "93.ogg", mode = CHAT};
tr3_new_track_tbl[094] = {file = "94.ogg", mode = CHAT};
tr3_new_track_tbl[095] = {file = "95.ogg", mode = CHAT};
tr3_new_track_tbl[096] = {file = "96.ogg", mode = CHAT};
tr3_new_track_tbl[097] = {file = "97.ogg", mode = CHAT};
tr3_new_track_tbl[098] = {file = "98.ogg", mode = CHAT};
tr3_new_track_tbl[099] = {file = "99.ogg", mode = CHAT};
tr3_new_track_tbl[100] = {file = "100.ogg", mode = CHAT};
tr3_new_track_tbl[101] = {file = "101.ogg", mode = CHAT};
tr3_new_track_tbl[102] = {file = "102.ogg", mode = CHAT};
tr3_new_track_tbl[103] = {file = "103.ogg", mode = CHAT};
tr3_new_track_tbl[104] = {file = "104.ogg", mode = CHAT};
tr3_new_track_tbl[105] = {file = "105.ogg", mode = CHAT};
tr3_new_track_tbl[106] = {file = "106.ogg", mode = CHAT};
tr3_new_track_tbl[107] = {file = "107.ogg", mode = CHAT};
tr3_new_track_tbl[108] = {file = "108.ogg", mode = CHAT};
tr3_new_track_tbl[109] = {file = "109.ogg", mode = CHAT};
tr3_new_track_tbl[110] = {file = "110.ogg", mode = CHAT};
tr3_new_track_tbl[111] = {file = "111.ogg", mode = CHAT};
tr3_new_track_tbl[112] = {file = "112.ogg", mode = CHAT};
tr3_new_track_tbl[113] = {file = "113.ogg", mode = CHAT};
tr3_new_track_tbl[114] = {file = "114.ogg", mode = CHAT};
tr3_new_track_tbl[115] = {file = "115.ogg", mode = CHAT};
tr3_new_track_tbl[116] = {file = "116.ogg", mode = CHAT};
tr3_new_track_tbl[117] = {file = "117.ogg", mode = CHAT};
tr3_new_track_tbl[118] = {file = "118.ogg", mode = CHAT};
tr3_new_track_tbl[119] = {file = "119.ogg", mode = CHAT};        -- Home block end
tr3_new_track_tbl[120] = {file = "120_In_The_Hut.ogg", mode = LOOP};
tr3_new_track_tbl[121] = {file = "and so on.ogg", mode = ONCE};
tr3_new_track_tbl[122] = {file = "secret.ogg", mode = ONCE};
tr3_new_track_tbl[123] = {file = "secret.ogg", mode = ONCE};


-- NAME BINDINGS FOR ORIGINAL CDAUDIO.WAD SOUNDTRACK.

tr3_old_track_tbl = {};

tr3_old_track_tbl[002] = {file = "02_The_Puzzle_Element.ogg", mode = ONCE};
tr3_old_track_tbl[003] = {file = "03_No_Waiting_Around_pt-I.ogg", mode = ONCE};
tr3_old_track_tbl[004] = {file = "04_Something_Spooky_Is_In_That_Jungle.ogg", mode = ONCE};
tr3_old_track_tbl[005] = {file = "05_Lara's_Themes.ogg", mode = LOOP};
tr3_old_track_tbl[006] = {file = "06_The_Cavern_Sewers.ogg", mode = ONCE};
tr3_old_track_tbl[007] = {file = "07_Geordie_Bob.ogg", mode = ONCE};
tr3_old_track_tbl[008] = {file = "08_Tony (The Loon).ogg", mode = ONCE};
tr3_old_track_tbl[009] = {file = "09_No_Waiting_Around_pt-II.ogg", mode = ONCE};
tr3_old_track_tbl[010] = {file = "10_The_Greedy_Mob.ogg", mode = ONCE};
tr3_old_track_tbl[011] = {file = "11_A_Long_Way_Up.ogg", mode = ONCE};
tr3_old_track_tbl[012] = {file = "12_No_Waiting_Around_pt-III.ogg", mode = ONCE};
tr3_old_track_tbl[013] = {file = "13_There_Be_Butterflies_Here_pt-II.ogg", mode = ONCE};
tr3_old_track_tbl[014] = {file = "14_She's_Cool.ogg", mode = ONCE};
tr3_old_track_tbl[015] = {file = "15_Mind_The_Gap_pt-II.ogg", mode = ONCE};
tr3_old_track_tbl[016] = {file = "16_Around_The_Corner_pt-II.ogg", mode = ONCE};
tr3_old_track_tbl[017] = {file = "17_Around_The_Corner_pt-I.ogg", mode = ONCE};
tr3_old_track_tbl[018] = {file = "18_Kneel_And_Pray.ogg", mode = ONCE};
tr3_old_track_tbl[019] = {file = "19_Around_The_Corner_pt-IV.ogg", mode = ONCE};
tr3_old_track_tbl[020] = {file = "20_Around_The_Corner_pt-III.ogg", mode = ONCE};
tr3_old_track_tbl[021] = {file = "21_Seeing_Is_Believing_pt-I.ogg", mode = ONCE};
tr3_old_track_tbl[022] = {file = "22_Looky_What_We_Have_Here_pt-III.ogg", mode = ONCE};
tr3_old_track_tbl[023] = {file = "23_There_Be_Butterflies_Here_pt-IV.ogg", mode = ONCE};
tr3_old_track_tbl[024] = {file = "24_Stone_The_Crows_pt-X.ogg", mode = ONCE};
tr3_old_track_tbl[025] = {file = "25_There_Be_Butterflies_Here_pt-III.ogg", mode = ONCE};
tr3_old_track_tbl[026] = {file = "26_Meteorite_Cavern.ogg", mode = LOOP};
tr3_old_track_tbl[027] = {file = "27_Steady.ogg", mode = LOOP};
tr3_old_track_tbl[028] = {file = "28_Antarctica.ogg", mode = LOOP};
tr3_old_track_tbl[029] = {file = "29_Things.ogg", mode = LOOP};
tr3_old_track_tbl[030] = {file = "30_Anyone_There.ogg", mode = LOOP};
tr3_old_track_tbl[031] = {file = "31_Grotto.ogg", mode = LOOP};
tr3_old_track_tbl[032] = {file = "32_On_The_Beach.ogg", mode = LOOP};
tr3_old_track_tbl[033] = {file = "33_Gamma_Pals.ogg", mode = LOOP};
tr3_old_track_tbl[034] = {file = "34_In_The_Jungle.ogg", mode = LOOP};
tr3_old_track_tbl[035] = {file = "35_Piranha_Waters.ogg", mode = LOOP};
tr3_old_track_tbl[036] = {file = "36_The_Rapids.ogg", mode = LOOP};
tr3_old_track_tbl[037] = {file = "37_Supper_Time.ogg", mode = ONCE};
tr3_old_track_tbl[038] = {file = "38_Look_Out_pt-V.ogg", mode = ONCE};
tr3_old_track_tbl[039] = {file = "39_Looky_What_We_Have_Here_pt-I.ogg", mode = ONCE};
tr3_old_track_tbl[040] = {file = "40_Around_The_Corner_pt-V.ogg", mode = ONCE};
tr3_old_track_tbl[041] = {file = "41_Seeing_Is_Believing_pt-II.ogg", mode = ONCE};
tr3_old_track_tbl[042] = {file = "42_Stone_The_Crows_pt-IX.ogg", mode = ONCE};
tr3_old_track_tbl[043] = {file = "43_Look_Out_pt-VIII.ogg", mode = ONCE};
tr3_old_track_tbl[044] = {file = "44_Look_Out_pt-IV.ogg", mode = ONCE};
tr3_old_track_tbl[045] = {file = "45_Stone_The_Crows_pt-VII.ogg", mode = ONCE};
tr3_old_track_tbl[046] = {file = "46_Stone_The_Crows_pt-III.ogg", mode = ONCE};
tr3_old_track_tbl[047] = {file = "47_Stone_The_Crows_pt-VIII.ogg", mode = ONCE};
tr3_old_track_tbl[048] = {file = "48_Looky_What_We_Have_Here_pt-II.ogg", mode = ONCE};
tr3_old_track_tbl[049] = {file = "49_Stone_The_Crows_pt-IV.ogg", mode = ONCE};
tr3_old_track_tbl[050] = {file = "50_Stone_The_Crows_pt-VI.ogg", mode = ONCE};
tr3_old_track_tbl[051] = {file = "51_Look_Out_pt-III.ogg", mode = ONCE};
tr3_old_track_tbl[052] = {file = "52_Look_Out_pt-I.ogg", mode = ONCE};
tr3_old_track_tbl[053] = {file = "53_There_Be_Butterflies_Here_pt-I.ogg", mode = ONCE};
tr3_old_track_tbl[054] = {file = "54_Stone_The_Crows_pt-I.ogg", mode = ONCE};
tr3_old_track_tbl[055] = {file = "55_Stone_The_Crows_pt-V.ogg", mode = ONCE};
tr3_old_track_tbl[056] = {file = "56_Mind_The_Gap_pt-I.ogg", mode = ONCE};
tr3_old_track_tbl[057] = {file = "57_There_Be_Butterflies_Here_pt-V.ogg", mode = ONCE};
tr3_old_track_tbl[058] = {file = "58_Look_Out_pt-II.ogg", mode = ONCE};
tr3_old_track_tbl[059] = {file = "59_Look_Out_pt-VII.ogg", mode = ONCE};
tr3_old_track_tbl[060] = {file = "60_Stone_The_Crows_pt-II.ogg", mode = ONCE};
tr3_old_track_tbl[061] = {file = "61_Look_Out_pt-VI.ogg", mode = ONCE};
tr3_old_track_tbl[062] = {file = "62_Scotts-Hut_(English).ogg", mode = ONCE};
tr3_old_track_tbl[063] = {file = "63_Cavern-Sewers_(English).ogg", mode = ONCE};
tr3_old_track_tbl[064] = {file = "64_Jungle-Camp_(English).ogg", mode = ONCE};
tr3_old_track_tbl[065] = {file = "65_Worship-Room_(English).ogg", mode = ONCE};
tr3_old_track_tbl[066] = {file = "66_Cavern_(English).ogg", mode = ONCE};
tr3_old_track_tbl[067] = {file = "67_Rooftops_(English).ogg", mode = ONCE};
tr3_old_track_tbl[068] = {file = "68_Tree-Shack_(English).ogg", mode = ONCE};
tr3_old_track_tbl[069] = {file = "69_Temple-Exit_(Generic).ogg", mode = ONCE};
tr3_old_track_tbl[070] = {file = "70_Delivery-Truck_(English).ogg", mode = ONCE};
tr3_old_track_tbl[071] = {file = "71_Penthouse_(English).ogg", mode = ONCE};
tr3_old_track_tbl[072] = {file = "72_Ravine_(English).ogg", mode = ONCE};
tr3_old_track_tbl[073] = {file = "73_Old_Smokey.ogg", mode = LOOP};
tr3_old_track_tbl[074] = {file = "74_Under_Smokey.ogg", mode = LOOP};
tr3_old_track_tbl[075] = {file = "75_Refining_Plant.ogg", mode = LOOP};
tr3_old_track_tbl[076] = {file = "76_Rumble_Sub.ogg", mode = LOOP};
tr3_old_track_tbl[077] = {file = "77_Quake.ogg", mode = LOOP};
tr3_old_track_tbl[078] = {file = "78_Blank.ogg", mode = ONCE};
tr3_old_track_tbl[082] = {file = "82.ogg", mode = CHAT};        -- Home block begin
tr3_old_track_tbl[083] = {file = "83.ogg", mode = CHAT};
tr3_old_track_tbl[084] = {file = "84.ogg", mode = CHAT};
tr3_old_track_tbl[085] = {file = "85.ogg", mode = CHAT};
tr3_old_track_tbl[086] = {file = "86.ogg", mode = CHAT};
tr3_old_track_tbl[087] = {file = "87.ogg", mode = CHAT};
tr3_old_track_tbl[088] = {file = "88.ogg", mode = CHAT};
tr3_old_track_tbl[089] = {file = "89.ogg", mode = CHAT};
tr3_old_track_tbl[090] = {file = "90.ogg", mode = CHAT};
tr3_old_track_tbl[091] = {file = "91.ogg", mode = CHAT};
tr3_old_track_tbl[092] = {file = "92.ogg", mode = CHAT};
tr3_old_track_tbl[093] = {file = "93.ogg", mode = CHAT};
tr3_old_track_tbl[094] = {file = "94.ogg", mode = CHAT};
tr3_old_track_tbl[095] = {file = "95.ogg", mode = CHAT};
tr3_old_track_tbl[096] = {file = "96.ogg", mode = CHAT};
tr3_old_track_tbl[097] = {file = "97.ogg", mode = CHAT};
tr3_old_track_tbl[098] = {file = "98.ogg", mode = CHAT};
tr3_old_track_tbl[099] = {file = "99.ogg", mode = CHAT};
tr3_old_track_tbl[100] = {file = "100.ogg", mode = CHAT};
tr3_old_track_tbl[101] = {file = "101.ogg", mode = CHAT};
tr3_old_track_tbl[102] = {file = "102.ogg", mode = CHAT};
tr3_old_track_tbl[103] = {file = "103.ogg", mode = CHAT};
tr3_old_track_tbl[104] = {file = "104.ogg", mode = CHAT};
tr3_old_track_tbl[105] = {file = "105.ogg", mode = CHAT};
tr3_old_track_tbl[106] = {file = "106.ogg", mode = CHAT};
tr3_old_track_tbl[107] = {file = "107.ogg", mode = CHAT};
tr3_old_track_tbl[108] = {file = "108.ogg", mode = CHAT};
tr3_old_track_tbl[109] = {file = "109.ogg", mode = CHAT};
tr3_old_track_tbl[110] = {file = "110.ogg", mode = CHAT};
tr3_old_track_tbl[111] = {file = "111.ogg", mode = CHAT};
tr3_old_track_tbl[112] = {file = "112.ogg", mode = CHAT};
tr3_old_track_tbl[113] = {file = "113.ogg", mode = CHAT};
tr3_old_track_tbl[114] = {file = "114.ogg", mode = CHAT};
tr3_old_track_tbl[115] = {file = "115.ogg", mode = CHAT};
tr3_old_track_tbl[116] = {file = "116.ogg", mode = CHAT};
tr3_old_track_tbl[117] = {file = "117.ogg", mode = CHAT};
tr3_old_track_tbl[118] = {file = "118.ogg", mode = CHAT};
tr3_old_track_tbl[119] = {file = "119.ogg", mode = CHAT};        -- Home block end
tr3_old_track_tbl[120] = {file = "120_In_The_Hut.ogg", mode = LOOP};
tr3_old_track_tbl[121] = {file = "121_And_So_On.ogg", mode = ONCE};
tr3_old_track_tbl[122] = {file = "122_secret.ogg", mode = ONCE};
tr3_old_track_tbl[123] = {file = "123_secret.ogg", mode = ONCE};


--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 4 ---------------------------------
--------------------------------------------------------------------------------

tr4_num_soundtracks = 112;
tr4_track_tbl       = {};

tr4_track_tbl[000] = {file = "044_attack_part_i.ogg", mode = ONCE}; 
tr4_track_tbl[001] = {file = "008_voncroy9a.ogg", mode = CHAT}; 
tr4_track_tbl[002] = {file = "100_attack_part_ii.ogg", mode = ONCE}; 
tr4_track_tbl[003] = {file = "010_voncroy10.ogg", mode = CHAT}; 
tr4_track_tbl[004] = {file = "015_voncroy14.ogg", mode = CHAT}; 
tr4_track_tbl[005] = {file = "073_secret.ogg", mode = ONCE}; 
tr4_track_tbl[006] = {file = "109_lyre_01.ogg", mode = CHAT}; 
tr4_track_tbl[007] = {file = "042_action_part_iv.ogg", mode = ONCE}; 
tr4_track_tbl[008] = {file = "043_action_part_v.ogg", mode = ONCE}; 
tr4_track_tbl[009] = {file = "030_voncroy30.ogg", mode = CHAT}; 
tr4_track_tbl[010] = {file = "012_voncroy11b.ogg", mode = CHAT}; 
tr4_track_tbl[011] = {file = "011_voncroy11a.ogg", mode = CHAT}; 
tr4_track_tbl[012] = {file = "063_misc_inc_01.ogg", mode = ONCE}; 
tr4_track_tbl[013] = {file = "014_voncroy13b.ogg", mode = CHAT}; 
tr4_track_tbl[014] = {file = "111_charmer.ogg", mode = CHAT}; 
tr4_track_tbl[015] = {file = "025_voncroy24b.ogg", mode = CHAT}; 
tr4_track_tbl[016] = {file = "023_voncroy23.ogg", mode = CHAT}; 
tr4_track_tbl[017] = {file = "006_voncroy7.ogg", mode = CHAT}; 
tr4_track_tbl[018] = {file = "024_voncroy24a.ogg", mode = CHAT}; 
tr4_track_tbl[019] = {file = "110_lyre_02.ogg", mode = ONCE}; 
tr4_track_tbl[020] = {file = "020_voncroy19.ogg", mode = CHAT}; 
tr4_track_tbl[021] = {file = "034_voncroy34.ogg", mode = CHAT}; 
tr4_track_tbl[022] = {file = "054_general_part_ii.ogg", mode = ONCE}; 
tr4_track_tbl[023] = {file = "036_voncroy36.ogg", mode = CHAT}; 
tr4_track_tbl[024] = {file = "004_voncroy5.ogg", mode = CHAT}; 
tr4_track_tbl[025] = {file = "035_voncroy35.ogg", mode = CHAT}; 
tr4_track_tbl[026] = {file = "027_voncroy27.ogg", mode = CHAT}; 
tr4_track_tbl[027] = {file = "053_general_part_i.ogg", mode = ONCE}; 
tr4_track_tbl[028] = {file = "022_voncroy22b.ogg", mode = CHAT}; 
tr4_track_tbl[029] = {file = "028_voncroy28_l11.ogg", mode = CHAT}; 
tr4_track_tbl[030] = {file = "003_voncroy4.ogg", mode = CHAT}; 
tr4_track_tbl[031] = {file = "001_voncroy2.ogg", mode = CHAT}; 
tr4_track_tbl[032] = {file = "041_action_part_iii.ogg", mode = ONCE}; 
tr4_track_tbl[033] = {file = "057_general_part_v.ogg", mode = ONCE}; 
tr4_track_tbl[034] = {file = "018_voncroy17.ogg", mode = CHAT}; 
tr4_track_tbl[035] = {file = "064_misc_inc_02.ogg", mode = ONCE}; 
tr4_track_tbl[036] = {file = "033_voncroy33.ogg", mode = CHAT}; 
tr4_track_tbl[037] = {file = "031_voncroy31_l12.ogg", mode = CHAT}; 
tr4_track_tbl[038] = {file = "032_voncroy32_l13.ogg", mode = CHAT}; 
tr4_track_tbl[039] = {file = "016_voncroy15.ogg", mode = CHAT}; 
tr4_track_tbl[040] = {file = "065_misc_inc_03.ogg", mode = ONCE}; 
tr4_track_tbl[041] = {file = "040_action_part_ii.ogg", mode = ONCE}; 
tr4_track_tbl[042] = {file = "112_gods_part_iv.ogg", mode = ONCE}; 
tr4_track_tbl[043] = {file = "029_voncroy29.ogg", mode = CHAT}; 
tr4_track_tbl[044] = {file = "007_voncroy8.ogg", mode = CHAT}; 
tr4_track_tbl[045] = {file = "013_voncroy12_13a_lara4.ogg", mode = CHAT}; 
tr4_track_tbl[046] = {file = "009_voncroy9b_lara3.ogg", mode = CHAT}; 
tr4_track_tbl[047] = {file = "081_dig.ogg", mode = ONCE}; 
tr4_track_tbl[048] = {file = "085_intro.ogg", mode = ONCE}; 
tr4_track_tbl[049] = {file = "071_ominous_part_i.ogg", mode = ONCE}; 
tr4_track_tbl[050] = {file = "095_phildoor.ogg", mode = ONCE}; 
tr4_track_tbl[051] = {file = "061_in_the_pyramid_part_i.ogg", mode = ONCE}; 
tr4_track_tbl[052] = {file = "050_underwater_find_part_i.ogg", mode = ONCE}; 
tr4_track_tbl[053] = {file = "058_gods_part_i.ogg", mode = ONCE}; 
tr4_track_tbl[054] = {file = "005_voncroy6_lara2.ogg", mode = CHAT}; 
tr4_track_tbl[055] = {file = "045_authentic_tr.ogg", mode = ONCE}; 
tr4_track_tbl[056] = {file = "060_gods_part_iii.ogg", mode = ONCE}; 
tr4_track_tbl[057] = {file = "055_general_part_iii.ogg", mode = ONCE}; 
tr4_track_tbl[058] = {file = "059_gods_part_ii.ogg", mode = ONCE}; 
tr4_track_tbl[059] = {file = "068_mystery_part_ii.ogg", mode = ONCE}; 
tr4_track_tbl[060] = {file = "076_captain2.ogg", mode = ONCE}; 
tr4_track_tbl[061] = {file = "019_lara6_voncroy18.ogg", mode = ONCE}; 
tr4_track_tbl[062] = {file = "002_voncroy3.ogg", mode = CHAT}; 
tr4_track_tbl[063] = {file = "066_misc_inc_04.ogg", mode = ONCE}; 
tr4_track_tbl[064] = {file = "067_mystery_part_i.ogg", mode = ONCE}; 
tr4_track_tbl[065] = {file = "038_a_short_01.ogg", mode = ONCE}; 
tr4_track_tbl[066] = {file = "088_key.ogg", mode = ONCE}; 
tr4_track_tbl[067] = {file = "017_voncroy16_lara5.ogg", mode = CHAT}; 
tr4_track_tbl[068] = {file = "026_vc25_l9_vc26_l10.ogg", mode = CHAT}; 
tr4_track_tbl[069] = {file = "056_general_part_iv.ogg", mode = ONCE}; 
tr4_track_tbl[070] = {file = "021_vc20_l7_vc21_l8_vc22a.ogg", mode = CHAT}; 
tr4_track_tbl[071] = {file = "096_sarcoph.ogg", mode = ONCE}; 
tr4_track_tbl[072] = {file = "087_jeepb.ogg", mode = ONCE}; 
tr4_track_tbl[073] = {file = "091_minilib1.ogg", mode = ONCE}; 
tr4_track_tbl[074] = {file = "086_jeepa.ogg", mode = ONCE}; 
tr4_track_tbl[075] = {file = "051_egyptian_mood_part_i.ogg", mode = ONCE}; 
tr4_track_tbl[076] = {file = "078_croywon.ogg", mode = ONCE}; 
tr4_track_tbl[077] = {file = "092_minilib2.ogg", mode = ONCE}; 
tr4_track_tbl[078] = {file = "083_horus.ogg", mode = ONCE}; 
tr4_track_tbl[079] = {file = "049_close_to_the_end_part_ii.ogg", mode = ONCE}; 
tr4_track_tbl[080] = {file = "037_vc37_l15_vc38.ogg", mode = CHAT}; 
tr4_track_tbl[081] = {file = "097_scorpion.ogg", mode = ONCE}; 
tr4_track_tbl[082] = {file = "089_larawon.ogg", mode = ONCE}; 
tr4_track_tbl[083] = {file = "094_minilib4.ogg", mode = ONCE}; 
tr4_track_tbl[084] = {file = "098_throne.ogg", mode = ONCE}; 
tr4_track_tbl[085] = {file = "048_close_to_the_end.ogg", mode = ONCE}; 
tr4_track_tbl[086] = {file = "070_mystery_part_iv.ogg", mode = ONCE}; 
tr4_track_tbl[087] = {file = "093_minilib3.ogg", mode = ONCE}; 
tr4_track_tbl[088] = {file = "072_puzzle_part_i.ogg", mode = ONCE}; 
tr4_track_tbl[089] = {file = "074_backpack.ogg", mode = ONCE}; 
tr4_track_tbl[090] = {file = "069_mystery_part_iii.ogg", mode = ONCE}; 
tr4_track_tbl[091] = {file = "052_egyptian_mood_part_ii.ogg", mode = ONCE}; 
tr4_track_tbl[092] = {file = "084_inscrip.ogg", mode = ONCE}; 
tr4_track_tbl[093] = {file = "099_whouse.ogg", mode = ONCE}; 
tr4_track_tbl[094] = {file = "047_boss_02.ogg", mode = ONCE}; 
tr4_track_tbl[095] = {file = "080_crypt2.ogg", mode = ONCE}; 
tr4_track_tbl[096] = {file = "090_libend.ogg", mode = ONCE}; 
tr4_track_tbl[097] = {file = "046_boss_01.ogg", mode = ONCE}; 
tr4_track_tbl[098] = {file = "062_jeep_thrills_max.ogg", mode = ONCE}; 
tr4_track_tbl[099] = {file = "079_crypt1.ogg", mode = ONCE}; 
tr4_track_tbl[100] = {file = "082_finale.ogg", mode = ONCE}; 
tr4_track_tbl[101] = {file = "075_captain1.ogg", mode = ONCE}; 
tr4_track_tbl[102] = {file = "105_a5_battle.ogg", mode = LOOP}; 
tr4_track_tbl[103] = {file = "077_crocgod.ogg", mode = ONCE}; 
tr4_track_tbl[104] = {file = "039_tr4_title_q10.ogg", mode = LOOP}; 
tr4_track_tbl[105] = {file = "108_a8_coastal.ogg", mode = LOOP}; 
tr4_track_tbl[106] = {file = "107_a7_train+.ogg", mode = LOOP}; 
tr4_track_tbl[107] = {file = "101_a1_in_dark.ogg", mode = LOOP}; 
tr4_track_tbl[108] = {file = "102_a2_in_drips.ogg", mode = LOOP}; 
tr4_track_tbl[109] = {file = "104_a4_weird1.ogg", mode = LOOP}; 
tr4_track_tbl[110] = {file = "106_a6_out_day.ogg", mode = LOOP}; 
tr4_track_tbl[111] = {file = "103_a3_out_night.ogg", mode = LOOP}; 


--------------------------------------------------------------------------------
-------------------------------- TOMB RAIDER 5 ---------------------------------
--------------------------------------------------------------------------------

tr5_num_soundtracks = 136;
tr5_track_tbl       = {};

tr5_track_tbl[000] = {file = "xa1_TL_10B.ogg", mode = CHAT};
tr5_track_tbl[001] = {file = "xa1_Z_10.ogg", mode = CHAT};
tr5_track_tbl[002] = {file = "xa1_TL_05.ogg", mode = CHAT};
tr5_track_tbl[003] = {file = "xa1_TL_08.ogg", mode = CHAT};
tr5_track_tbl[004] = {file = "xa1_TL_11.ogg", mode = CHAT};
tr5_track_tbl[005] = {file = "xa1_ANDYPEW.ogg", mode = ONCE};
tr5_track_tbl[006] = {file = "xa1_SECRET.ogg", mode = ONCE};
tr5_track_tbl[007] = {file = "xa1_TL_02.ogg", mode = CHAT};
tr5_track_tbl[008] = {file = "xa2_HMMM05.ogg", mode = ONCE};
tr5_track_tbl[009] = {file = "xa2_TL_01.ogg", mode = CHAT};
tr5_track_tbl[010] = {file = "xa2_ATTACK04.ogg", mode = ONCE};
tr5_track_tbl[011] = {file = "xa2_UWATER2B.ogg", mode = ONCE};
tr5_track_tbl[012] = {file = "xa2_SPOOKY2A.ogg", mode = ONCE};
tr5_track_tbl[013] = {file = "xa2_TL_10A.ogg", mode = CHAT};
tr5_track_tbl[014] = {file = "xa2_HMMM02.ogg", mode = ONCE};
tr5_track_tbl[015] = {file = "xa2_TOMS01.ogg", mode = ONCE};
tr5_track_tbl[016] = {file = "xa3_Attack03.ogg", mode = ONCE};
tr5_track_tbl[017] = {file = "xa3_Attack02.ogg", mode = ONCE};
tr5_track_tbl[018] = {file = "xa3_Hmmm01.ogg", mode = ONCE};
tr5_track_tbl[019] = {file = "xa3_Stealth1.ogg", mode = ONCE};
tr5_track_tbl[020] = {file = "xa3_Stealth2.ogg", mode = ONCE};
tr5_track_tbl[021] = {file = "xa3_Attack01.ogg", mode = ONCE};
tr5_track_tbl[022] = {file = "xa3_TL_06.ogg", mode = CHAT};
tr5_track_tbl[023] = {file = "xa3_TL_03.ogg", mode = CHAT};
tr5_track_tbl[024] = {file = "xa4_hmmm06.ogg", mode = ONCE};
tr5_track_tbl[025] = {file = "xa4_mil01.ogg", mode = ONCE};
tr5_track_tbl[026] = {file = "xa4_Z_03.ogg", mode = CHAT};
tr5_track_tbl[027] = {file = "xa4_hit01.ogg", mode = ONCE};
tr5_track_tbl[028] = {file = "xa4_spooky05.ogg", mode = ONCE};
tr5_track_tbl[029] = {file = "xa4_drama01.ogg", mode = ONCE};
tr5_track_tbl[030] = {file = "xa4_stealth4.ogg", mode = ONCE};
tr5_track_tbl[031] = {file = "xa4_mil05.ogg", mode = ONCE};
tr5_track_tbl[032] = {file = "xa5_HMMM04.ogg", mode = ONCE};
tr5_track_tbl[033] = {file = "xa5_MIL06.ogg", mode = ONCE};
tr5_track_tbl[034] = {file = "xa5_SPOOKY02.ogg", mode = ONCE};
tr5_track_tbl[035] = {file = "xa5_TL_12.ogg", mode = CHAT};
tr5_track_tbl[036] = {file = "xa5_MIL02A.ogg", mode = ONCE};
tr5_track_tbl[037] = {file = "xa5_HMMM03.ogg", mode = ONCE};
tr5_track_tbl[038] = {file = "xa5_MIL02.ogg", mode = ONCE};
tr5_track_tbl[039] = {file = "xa5_TL_04.ogg", mode = CHAT};
tr5_track_tbl[040] = {file = "xa6_Mil04.ogg", mode = ONCE};
tr5_track_tbl[041] = {file = "xa6_Solo01.ogg", mode = ONCE};
tr5_track_tbl[042] = {file = "xa6_Z12.ogg", mode = CHAT};
tr5_track_tbl[043] = {file = "xa6_Stealth3.ogg", mode = ONCE};
tr5_track_tbl[044] = {file = "xa6_AuthSolo.ogg", mode = ONCE};
tr5_track_tbl[045] = {file = "xa6_Spooky03.ogg", mode = ONCE};
tr5_track_tbl[046] = {file = "xa6_Z13.ogg", mode = CHAT};
tr5_track_tbl[047] = {file = "xa6_Z_04anim.ogg", mode = CHAT};
tr5_track_tbl[048] = {file = "xa7_z_06a.ogg", mode = CHAT};
tr5_track_tbl[049] = {file = "xa7_andyoooh.ogg", mode = ONCE};
tr5_track_tbl[050] = {file = "xa7_andyooer.ogg", mode = ONCE};
tr5_track_tbl[051] = {file = "xa7_tl_07.ogg", mode = CHAT};
tr5_track_tbl[052] = {file = "xa7_z_02.ogg", mode = CHAT};
tr5_track_tbl[053] = {file = "xa7_evibes01.ogg", mode = ONCE};
tr5_track_tbl[054] = {file = "xa7_z_06.ogg", mode = CHAT};
tr5_track_tbl[055] = {file = "xa7_authtr.ogg", mode = ONCE};
tr5_track_tbl[056] = {file = "xa8_mil03.ogg", mode = ONCE};
tr5_track_tbl[057] = {file = "xa8_fightsc.ogg", mode = ONCE};
tr5_track_tbl[058] = {file = "xa8_richcut3.ogg", mode = ONCE};
tr5_track_tbl[059] = {file = "xa8_z_13.ogg", mode = CHAT};
tr5_track_tbl[060] = {file = "xa8_z_08.ogg", mode = CHAT};
tr5_track_tbl[061] = {file = "xa8_uwater2a.ogg", mode = ONCE};
tr5_track_tbl[062] = {file = "xa8_jobyalrm.ogg", mode = ONCE};
tr5_track_tbl[063] = {file = "xa8_mil02b.ogg", mode = ONCE};
tr5_track_tbl[064] = {file = "xa9_swampy.ogg", mode = ONCE};
tr5_track_tbl[065] = {file = "xa9_evibes02.ogg", mode = ONCE};
tr5_track_tbl[066] = {file = "xa9_gods01.ogg", mode = ONCE};
tr5_track_tbl[067] = {file = "xa9_z_03.ogg", mode = CHAT};
tr5_track_tbl[068] = {file = "xa9_richcut4.ogg", mode = ONCE};
tr5_track_tbl[069] = {file = "xa9_title4.ogg", mode = ONCE};
tr5_track_tbl[070] = {file = "xa9_spooky01.ogg", mode = ONCE};
tr5_track_tbl[071] = {file = "xa9_chopin01.ogg", mode = ONCE};
tr5_track_tbl[072] = {file = "xa10_echoir01.ogg", mode = ONCE};
tr5_track_tbl[073] = {file = "xa10_title3.ogg", mode = ONCE};
tr5_track_tbl[074] = {file = "xa10_perc01.ogg", mode = ONCE};
tr5_track_tbl[075] = {file = "xa10_vc01.ogg", mode = ONCE};
tr5_track_tbl[076] = {file = "xa10_title2.ogg", mode = ONCE};
tr5_track_tbl[077] = {file = "xa10_z_09.ogg", mode = CHAT};
tr5_track_tbl[078] = {file = "xa10_spooky04.ogg", mode = ONCE};
tr5_track_tbl[079] = {file = "xa10_z_10.ogg", mode = CHAT};
tr5_track_tbl[080] = {file = "xa11_vc01atv.ogg", mode = ONCE};
tr5_track_tbl[081] = {file = "xa11_andy3.ogg", mode = ONCE};
tr5_track_tbl[082] = {file = "xa11_title1.ogg", mode = ONCE};
tr5_track_tbl[083] = {file = "xa11_flyby1.ogg", mode = ONCE};
tr5_track_tbl[084] = {file = "xa11_monk_2.ogg", mode = ONCE};
tr5_track_tbl[085] = {file = "xa11_andy4.ogg", mode = ONCE};
tr5_track_tbl[086] = {file = "xa11_flyby3.ogg", mode = ONCE};
tr5_track_tbl[087] = {file = "xa11_flyby2.ogg", mode = ONCE};
tr5_track_tbl[088] = {file = "xa12_moses01.ogg", mode = ONCE};
tr5_track_tbl[089] = {file = "xa12_andy4b.ogg", mode = ONCE};
tr5_track_tbl[090] = {file = "xa12_z_10.ogg", mode = CHAT};
tr5_track_tbl[091] = {file = "xa12_flyby4.ogg", mode = ONCE};
tr5_track_tbl[092] = {file = "xa12_richcut1.ogg", mode = ONCE};
tr5_track_tbl[093] = {file = "xa12_andy5.ogg", mode = ONCE};
tr5_track_tbl[094] = {file = "xa12_z_05.ogg", mode = CHAT};
tr5_track_tbl[095] = {file = "xa12_z_01.ogg", mode = CHAT};
tr5_track_tbl[096] = {file = "xa13_Joby3.ogg", mode = ONCE};
tr5_track_tbl[097] = {file = "xa13_Andy7.ogg", mode = ONCE};
tr5_track_tbl[098] = {file = "xa13_Andrea3B.ogg", mode = ONCE};
tr5_track_tbl[099] = {file = "xa13_cossack.ogg", mode = ONCE};
tr5_track_tbl[100] = {file = "xa13_Z_07.ogg", mode = CHAT};
tr5_track_tbl[101] = {file = "xa13_Andy6.ogg", mode = ONCE};
tr5_track_tbl[102] = {file = "xa13_Andrea3.ogg", mode = ONCE};
tr5_track_tbl[103] = {file = "xa13_Joby7.ogg", mode = ONCE};
tr5_track_tbl[104] = {file = "xa14_uwater1.ogg", mode = ONCE};
tr5_track_tbl[105] = {file = "xa14_joby1.ogg", mode = ONCE};
tr5_track_tbl[106] = {file = "xa14_andy10.ogg", mode = ONCE};
tr5_track_tbl[107] = {file = "xa14_richcut2.ogg", mode = ONCE};
tr5_track_tbl[108] = {file = "xa14_andrea1.ogg", mode = ONCE};
tr5_track_tbl[109] = {file = "xa14_andy8.ogg", mode = ONCE};
tr5_track_tbl[110] = {file = "xa14_joby6.ogg", mode = ONCE};
tr5_track_tbl[111] = {file = "xa14_ecredits.ogg", mode = LOOP};
tr5_track_tbl[112] = {file = "xa15_boss_01.ogg", mode = ONCE};
tr5_track_tbl[113] = {file = "xa15_joby2.ogg", mode = ONCE};
tr5_track_tbl[114] = {file = "xa15_joby4.ogg", mode = ONCE};
tr5_track_tbl[115] = {file = "xa15_joby5.ogg", mode = ONCE};
tr5_track_tbl[116] = {file = "xa15_joby9.ogg", mode = ONCE};
tr5_track_tbl[117] = {file = "xa15_a_andy.ogg", mode = LOOP};
tr5_track_tbl[118] = {file = "xa15_a_rome.ogg", mode = LOOP};
tr5_track_tbl[119] = {file = "xa15_andy2.ogg", mode = ONCE};
tr5_track_tbl[120] = {file = "xa16_Joby8.ogg", mode = ONCE};
tr5_track_tbl[121] = {file = "xa16_A_Sub_Amb.ogg", mode = LOOP};
tr5_track_tbl[122] = {file = "xa16_Joby10.ogg", mode = ONCE};
tr5_track_tbl[123] = {file = "xa16_A_Harbour_out.ogg", mode = LOOP};
tr5_track_tbl[124] = {file = "xa16_A_Andy_Out_Norm.ogg", mode = LOOP};
tr5_track_tbl[125] = {file = "xa16_A_Andy_Out_Spooky.ogg", mode = LOOP};
tr5_track_tbl[126] = {file = "xa16_A_Rome_Day.ogg", mode = LOOP}
tr5_track_tbl[127] = {file = "xa16_A_Underwater.ogg", mode = LOOP};
tr5_track_tbl[128] = {file = "xa17_A_Rome_Night.ogg", mode = LOOP};
tr5_track_tbl[129] = {file = "xa17_A_VC_Saga.ogg", mode = LOOP};
tr5_track_tbl[130] = {file = "xa17_A_Industry.ogg", mode = LOOP}
tr5_track_tbl[131] = {file = "xa17_Andrea2.ogg", mode = ONCE};
tr5_track_tbl[132] = {file = "xa17_Andy1.ogg", mode = ONCE};
tr5_track_tbl[133] = {file = "xa17_Andrea4.ogg", mode = ONCE};
tr5_track_tbl[134] = {file = "xa17_Andy9.ogg", mode = ONCE};
tr5_track_tbl[135] = {file = "xa17_Andy11.ogg", mode = ONCE};


function getTrackInfo(ver, id)
    local tbl = {};
    local path, method;
    
    if(ver < 3) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        tbl    = tr1_track_tbl;
        path   = "data/tr1/audio/";
        method = OGG;
    elseif(ver < 5) then                -- TR_II, TR_II_DEMO
        tbl    = tr2_track_tbl;
        path   = "data/tr2/audio/";
        method = OGG;
    elseif(ver < 6) then                
        if(USE_TR3_REMASTER == 1) then
            tbl  = tr3_new_track_tbl;    -- TR_III (REMASTERED)
        else
            tbl  = tr3_old_track_tbl;    -- TR_III (OLD)
        end
        path   = "data/tr3/audio/";
        method = OGG;
    elseif(ver < 8) then                -- TR_IV, TR_IV_DEMO
        tbl    = tr4_track_tbl;
        path   = "data/tr4/audio/";
        method = OGG;
    elseif(ver < 9) then                -- TR_V
        tbl    = tr5_track_tbl;
        path   = "data/tr5/audio/";
        method = OGG;
    else
        return "NONE", -1, -1;
    end;
    
    if(tbl[id] == nil) then
        return "NONE", -1, -1;
    else
        return (path .. tbl[id].file), tbl[id].mode, method;
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