-- OPENTOMB ITEM COMBINE MAPPINGS
-- by Lwmte, Oct 2014

--------------------------------------------------------------------------------
-- Since OpenTomb uses internal item enumeration which is not related to local
-- version-specific TR item index, we need to remap certain item combinations.
-- This file serves such purpose, as well as provides mappings for TR4/TR5 item
-- combining patterns in future.
--------------------------------------------------------------------------------

tr1_key = {};   -- Technically, both puzzle pieces and keys are keys.
tr2_key = {};
tr3_key = {};
tr4_key = {};
tr5_key = {};

tr1_key[118] = ITEM_PUZZLE_1;           -- TR1 puzzleholes
tr1_key[119] = ITEM_PUZZLE_2;
tr1_key[120] = ITEM_PUZZLE_3;
tr1_key[121] = ITEM_PUZZLE_4;
tr1_key[137] = ITEM_KEY_1;              -- TR1 keyholes
tr1_key[138] = ITEM_KEY_2;
tr1_key[139] = ITEM_KEY_3;
tr1_key[140] = ITEM_KEY_4;

tr2_key[182] = ITEM_PUZZLE_1;           -- TR2 puzzleholes
tr2_key[183] = ITEM_PUZZLE_2;
tr2_key[184] = ITEM_PUZZLE_3;
tr2_key[185] = ITEM_PUZZLE_4;
tr2_key[201] = ITEM_KEY_1;              -- TR2 keyholes
tr2_key[202] = ITEM_KEY_2;
tr2_key[203] = ITEM_KEY_3;
tr2_key[204] = ITEM_KEY_4;

tr3_key[213] = ITEM_PUZZLE_1;           -- TR3 puzzleholes
tr3_key[214] = ITEM_PUZZLE_2;
tr3_key[215] = ITEM_PUZZLE_3;
tr3_key[216] = ITEM_PUZZLE_4;
tr3_key[232] = ITEM_KEY_1;              -- TR3 keyholes
tr3_key[233] = ITEM_KEY_2;
tr3_key[234] = ITEM_KEY_3;
tr3_key[235] = ITEM_KEY_4;

tr4_key[260] = ITEM_PUZZLE_1;           -- TR4 puzzleholes
tr4_key[261] = ITEM_PUZZLE_2;
tr4_key[262] = ITEM_PUZZLE_3;
tr4_key[263] = ITEM_PUZZLE_4;
tr4_key[264] = ITEM_PUZZLE_5;
tr4_key[265] = ITEM_PUZZLE_6;
tr4_key[266] = ITEM_PUZZLE_7;
tr4_key[267] = ITEM_PUZZLE_8;
tr4_key[268] = ITEM_PUZZLE_9;
tr4_key[269] = ITEM_PUZZLE_10;
tr4_key[270] = ITEM_PUZZLE_11;
tr4_key[271] = ITEM_PUZZLE_12;
tr4_key[284] = ITEM_KEY_1;              -- TR4 keyholes
tr4_key[285] = ITEM_KEY_1;
tr4_key[286] = ITEM_KEY_3;
tr4_key[287] = ITEM_KEY_4;
tr4_key[288] = ITEM_KEY_5;
tr4_key[289] = ITEM_KEY_6;
tr4_key[290] = ITEM_KEY_7;
tr4_key[291] = ITEM_KEY_8;
tr4_key[292] = ITEM_KEY_9;
tr4_key[293] = ITEM_KEY_10;
tr4_key[294] = ITEM_KEY_11;
tr4_key[295] = ITEM_KEY_12;

tr4_key[260] = ITEM_PUZZLE_1;           -- TR4 puzzleholes
tr4_key[261] = ITEM_PUZZLE_2;
tr4_key[262] = ITEM_PUZZLE_3;
tr4_key[263] = ITEM_PUZZLE_4;
tr4_key[264] = ITEM_PUZZLE_5;
tr4_key[265] = ITEM_PUZZLE_6;
tr4_key[266] = ITEM_PUZZLE_7;
tr4_key[267] = ITEM_PUZZLE_8;
tr4_key[268] = ITEM_PUZZLE_9;
tr4_key[269] = ITEM_PUZZLE_10;
tr4_key[270] = ITEM_PUZZLE_11;
tr4_key[271] = ITEM_PUZZLE_12;
tr4_key[284] = ITEM_KEY_1;              -- TR4 keyholes
tr4_key[285] = ITEM_KEY_2;
tr4_key[286] = ITEM_KEY_3;
tr4_key[287] = ITEM_KEY_4;
tr4_key[288] = ITEM_KEY_5;
tr4_key[289] = ITEM_KEY_6;
tr4_key[290] = ITEM_KEY_7;
tr4_key[291] = ITEM_KEY_8;
tr4_key[292] = ITEM_KEY_9;
tr4_key[293] = ITEM_KEY_10;
tr4_key[294] = ITEM_KEY_11;
tr4_key[295] = ITEM_KEY_12;

tr5_key[242] = ITEM_PUZZLE_1;           -- TR5 puzzleholes
tr5_key[243] = ITEM_PUZZLE_2;
tr5_key[244] = ITEM_PUZZLE_3;
tr5_key[245] = ITEM_PUZZLE_4;
tr5_key[246] = ITEM_PUZZLE_5;
tr5_key[247] = ITEM_PUZZLE_6;
tr5_key[248] = ITEM_PUZZLE_7;
tr5_key[249] = ITEM_PUZZLE_8;
tr5_key[258] = ITEM_KEY_1;              -- TR5 keyholes
tr5_key[259] = ITEM_KEY_2;
tr5_key[260] = ITEM_KEY_3;
tr5_key[261] = ITEM_KEY_4;
tr5_key[262] = ITEM_KEY_5;
tr5_key[263] = ITEM_KEY_6;
tr5_key[264] = ITEM_KEY_7;
tr5_key[265] = ITEM_KEY_8;