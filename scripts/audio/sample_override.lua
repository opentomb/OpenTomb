-- OPENTOMB SAMPLE OVERRIDE SCRIPT
-- by noname, June 2014

--------------------------------------------------------------------------------
-- This script allows to override native sound effects from level files or
-- MAIN.SFX file with custom samples. Note that you can specify it on a
-- per-game basis only, and generally, it is needed for first two games which 
-- used low-quality samples. You won't need to replace samples for late games
-- (like TR3, TR4 or TR5), since they already have high-quality samples.
--------------------------------------------------------------------------------

print("Sample remapper script loaded");

tr1_sound      = {};

tr1_sound[000] = {sample = 0,   count = 004};
tr1_sound[001] = {sample = 4,   count = 001};
tr1_sound[002] = {sample = 5,   count = 001};
tr1_sound[003] = {sample = 6,   count = 001};
tr1_sound[004] = {sample = 7,   count = 001};
tr1_sound[005] = {sample = 8,   count = 001};
tr1_sound[006] = {sample = 9,   count = 001};
tr1_sound[007] = {sample = 10,  count = 001};
tr1_sound[008] = {sample = 11,  count = 001};
tr1_sound[009] = {sample = 12,  count = 001};
tr1_sound[010] = {sample = 13,  count = 002};
tr1_sound[011] = {sample = 15,  count = 001};
tr1_sound[012] = {sample = 16,  count = 001};
tr1_sound[013] = {sample = 17,  count = 001};
tr1_sound[014] = {sample = 18,  count = 002};
tr1_sound[016] = {sample = 20,  count = 001};
tr1_sound[018] = {sample = 21,  count = 001};
tr1_sound[019] = {sample = 22,  count = 002};
tr1_sound[020] = {sample = 24,  count = 001};
tr1_sound[022] = {sample = 25,  count = 002};
tr1_sound[024] = {sample = 27,  count = 001};
tr1_sound[025] = {sample = 28,  count = 002};
tr1_sound[026] = {sample = 30,  count = 001};
tr1_sound[027] = {sample = 31,  count = 002};
tr1_sound[028] = {sample = 33,  count = 002};
tr1_sound[029] = {sample = 35,  count = 001};
tr1_sound[030] = {sample = 36,  count = 001};
tr1_sound[031] = {sample = 37,  count = 002};
tr1_sound[032] = {sample = 39,  count = 001};
tr1_sound[033] = {sample = 40,  count = 001};
tr1_sound[034] = {sample = 41,  count = 001};
tr1_sound[035] = {sample = 42,  count = 001};
tr1_sound[036] = {sample = 43,  count = 001};
tr1_sound[037] = {sample = 44,  count = 001};
tr1_sound[038] = {sample = 45,  count = 001};
tr1_sound[039] = {sample = 46,  count = 001};
tr1_sound[040] = {sample = 47,  count = 001};
tr1_sound[041] = {sample = 48,  count = 001};
tr1_sound[042] = {sample = 49,  count = 001};
tr1_sound[043] = {sample = 50,  count = 001};
tr1_sound[044] = {sample = 51,  count = 001};
tr1_sound[045] = {sample = 52,  count = 001};
tr1_sound[046] = {sample = 53,  count = 001};
tr1_sound[047] = {sample = 54,  count = 003};
tr1_sound[048] = {sample = 57,  count = 001};
tr1_sound[050] = {sample = 58,  count = 001};
tr1_sound[051] = {sample = 59,  count = 001};
tr1_sound[052] = {sample = 60,  count = 001};
tr1_sound[053] = {sample = 61,  count = 001};
tr1_sound[054] = {sample = 62,  count = 001};
tr1_sound[055] = {sample = 63,  count = 001};
tr1_sound[056] = {sample = 64,  count = 001};
tr1_sound[057] = {sample = 65,  count = 001};
tr1_sound[058] = {sample = 66,  count = 001};
tr1_sound[059] = {sample = 67,  count = 001};
tr1_sound[060] = {sample = 68,  count = 001};
tr1_sound[061] = {sample = 69,  count = 001};
tr1_sound[063] = {sample = 70,  count = 001};
tr1_sound[064] = {sample = 71,  count = 001};
tr1_sound[065] = {sample = 72,  count = 001};
tr1_sound[066] = {sample = 73,  count = 001};
tr1_sound[067] = {sample = 74,  count = 001};
tr1_sound[068] = {sample = 75,  count = 001};
tr1_sound[069] = {sample = 76,  count = 001};
tr1_sound[070] = {sample = 77,  count = 001};
tr1_sound[071] = {sample = 78,  count = 001};
tr1_sound[072] = {sample = 79,  count = 001};
tr1_sound[073] = {sample = 80,  count = 001};
tr1_sound[074] = {sample = 81,  count = 001};
tr1_sound[075] = {sample = 82,  count = 002};
tr1_sound[076] = {sample = 84,  count = 001};
tr1_sound[077] = {sample = 85,  count = 001};
tr1_sound[078] = {sample = 86,  count = 001};
tr1_sound[079] = {sample = 87,  count = 001};
tr1_sound[080] = {sample = 88,  count = 001};
tr1_sound[081] = {sample = 89,  count = 001};
tr1_sound[082] = {sample = 90,  count = 001};
tr1_sound[083] = {sample = 91,  count = 001};
tr1_sound[084] = {sample = 92,  count = 001};
tr1_sound[085] = {sample = 93,  count = 001};
tr1_sound[086] = {sample = 94,  count = 002};
tr1_sound[087] = {sample = 96,  count = 001};
tr1_sound[088] = {sample = 97,  count = 001};
tr1_sound[089] = {sample = 98,  count = 001};
tr1_sound[090] = {sample = 99,  count = 001};
tr1_sound[091] = {sample = 100, count = 001};
tr1_sound[092] = {sample = 101, count = 001};
tr1_sound[093] = {sample = 102, count = 002};
tr1_sound[094] = {sample = 104, count = 001};
tr1_sound[095] = {sample = 105, count = 001};
tr1_sound[096] = {sample = 106, count = 001};
tr1_sound[097] = {sample = 107, count = 001};
tr1_sound[098] = {sample = 108, count = 001};
tr1_sound[099] = {sample = 109, count = 001};
tr1_sound[100] = {sample = 110, count = 001};
tr1_sound[101] = {sample = 111, count = 004};
tr1_sound[102] = {sample = 115, count = 001};
tr1_sound[103] = {sample = 116, count = 001};
tr1_sound[104] = {sample = 117, count = 001};
tr1_sound[108] = {sample = 118, count = 001};
tr1_sound[109] = {sample = 119, count = 001};
tr1_sound[110] = {sample = 120, count = 001};
tr1_sound[111] = {sample = 121, count = 001};
tr1_sound[112] = {sample = 122, count = 001};
tr1_sound[113] = {sample = 123, count = 001};
tr1_sound[114] = {sample = 124, count = 001};
tr1_sound[115] = {sample = 125, count = 001};
tr1_sound[116] = {sample = 126, count = 001};
tr1_sound[117] = {sample = 127, count = 001};
tr1_sound[118] = {sample = 128, count = 001};
tr1_sound[119] = {sample = 129, count = 001};
tr1_sound[120] = {sample = 130, count = 002};
tr1_sound[121] = {sample = 132, count = 001};
tr1_sound[122] = {sample = 133, count = 001};
tr1_sound[123] = {sample = 134, count = 001};
tr1_sound[124] = {sample = 135, count = 001};
tr1_sound[125] = {sample = 136, count = 001};
tr1_sound[126] = {sample = 137, count = 004};
tr1_sound[127] = {sample = 141, count = 001};
tr1_sound[128] = {sample = 142, count = 001};
tr1_sound[129] = {sample = 143, count = 001};
tr1_sound[130] = {sample = 144, count = 001};
tr1_sound[131] = {sample = 145, count = 001};
tr1_sound[132] = {sample = 146, count = 001};
tr1_sound[133] = {sample = 147, count = 001};
tr1_sound[134] = {sample = 148, count = 001};
tr1_sound[135] = {sample = 149, count = 001};
tr1_sound[136] = {sample = 150, count = 001};
tr1_sound[137] = {sample = 151, count = 001};
tr1_sound[138] = {sample = 152, count = 001};
tr1_sound[139] = {sample = 153, count = 001};
tr1_sound[140] = {sample = 154, count = 001};
tr1_sound[141] = {sample = 155, count = 001};
tr1_sound[142] = {sample = 156, count = 001};
tr1_sound[143] = {sample = 157, count = 004};
tr1_sound[144] = {sample = 161, count = 001};
tr1_sound[145] = {sample = 162, count = 001};
tr1_sound[146] = {sample = 163, count = 001};
tr1_sound[147] = {sample = 164, count = 001};
tr1_sound[148] = {sample = 165, count = 001};
tr1_sound[149] = {sample = 166, count = 001};
tr1_sound[150] = {sample = 167, count = 001};
tr1_sound[151] = {sample = 168, count = 001};
tr1_sound[152] = {sample = 169, count = 001};
tr1_sound[153] = {sample = 170, count = 001};
tr1_sound[154] = {sample = 171, count = 001};
tr1_sound[155] = {sample = 172, count = 001};
tr1_sound[156] = {sample = 173, count = 001};
tr1_sound[157] = {sample = 174, count = 001};
tr1_sound[158] = {sample = 175, count = 001};
tr1_sound[159] = {sample = 176, count = 001};
tr1_sound[160] = {sample = 177, count = 001};
tr1_sound[161] = {sample = 178, count = 001};
tr1_sound[162] = {sample = 179, count = 001};
tr1_sound[163] = {sample = 180, count = 001};
tr1_sound[164] = {sample = 181, count = 001};
tr1_sound[165] = {sample = 182, count = 001};
tr1_sound[166] = {sample = 183, count = 001};
tr1_sound[167] = {sample = 184, count = 001};
tr1_sound[168] = {sample = 185, count = 001};
tr1_sound[169] = {sample = 186, count = 001};
tr1_sound[170] = {sample = 187, count = 001};
tr1_sound[171] = {sample = 188, count = 001};
tr1_sound[172] = {sample = 189, count = 001};


tr_sound_info = {};

tr_sound_info[0] = { num_samples       = 195,
                     num_sounds        = 165,
                     sample_name_mask  = "data/tr1/samples/SFX_%04d.wav",
                     sample_table      = tr1_sound };


function GetOverridedSample(ver, level_id, sound_id)
    if((tr_sound_info[ver] ~= nil) and (tr_sound_info[ver].sample_table[sound_id] ~= nil)) then
        return tr_sound_info[ver].sample_table[sound_id].sample, tr_sound_info[ver].sample_table[sound_id].count;
    else
        return -1, -1;
    end
end;

function GetOverridedSamplesInfo(ver)
    if(tr_sound_info[ver] ~= nil) then
        return tr_sound_info[ver].num_samples, tr_sound_info[ver].num_sounds, tr_sound_info[ver].sample_name_mask;
    else
        return -1, -1, "NONE";
    end;
end;