-- OPENTOMB ITEM COMBINE MAPPINGS
-- by Lwmte, Oct 2014

--------------------------------------------------------------------------------
-- Since OpenTomb uses internal item enumeration which is not related to local
-- version-specific TR item index, we need to remap certain item combinations.
-- This file serves such purpose, as well as provides mappings for TR4/TR5 item
-- combining patterns in future.
--------------------------------------------------------------------------------

tr1_key = {};       -- Technically, both puzzle pieces and keys are keys.
tr2_key = {};
tr3_key = {};
tr4_key = {};
tr5_key = {};

tr4_combine  = {};  -- Combine and separate maps for TR4-5.
tr5_combine  = {};


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


-- Combine / separate maps for TR4.

tr4_combine[0]  = {a = ITEM_PUZZLE_1_COMBO_A, b = ITEM_PUZZLE_1_COMBO_B, result = ITEM_PUZZLE_1};
tr4_combine[1]  = {a = ITEM_PUZZLE_2_COMBO_A, b = ITEM_PUZZLE_2_COMBO_B, result = ITEM_PUZZLE_2};
tr4_combine[2]  = {a = ITEM_PUZZLE_3_COMBO_A, b = ITEM_PUZZLE_3_COMBO_B, result = ITEM_PUZZLE_3};
tr4_combine[3]  = {a = ITEM_PUZZLE_4_COMBO_A, b = ITEM_PUZZLE_4_COMBO_B, result = ITEM_PUZZLE_4};
tr4_combine[4]  = {a = ITEM_PUZZLE_5_COMBO_A, b = ITEM_PUZZLE_5_COMBO_B, result = ITEM_PUZZLE_5};
tr4_combine[5]  = {a = ITEM_PUZZLE_6_COMBO_A, b = ITEM_PUZZLE_6_COMBO_B, result = ITEM_PUZZLE_6};
tr4_combine[6]  = {a = ITEM_PUZZLE_7_COMBO_A, b = ITEM_PUZZLE_7_COMBO_B, result = ITEM_PUZZLE_7};
tr4_combine[7]  = {a = ITEM_PUZZLE_8_COMBO_A, b = ITEM_PUZZLE_8_COMBO_B, result = ITEM_PUZZLE_8};
tr4_combine[8]  = {a = ITEM_KEY_1_COMBO_A, b = ITEM_KEY_1_COMBO_B, result = ITEM_KEY_1};
tr4_combine[9]  = {a = ITEM_KEY_2_COMBO_A, b = ITEM_KEY_2_COMBO_B, result = ITEM_KEY_2};
tr4_combine[10] = {a = ITEM_KEY_3_COMBO_A, b = ITEM_KEY_3_COMBO_B, result = ITEM_KEY_3};
tr4_combine[11] = {a = ITEM_KEY_4_COMBO_A, b = ITEM_KEY_4_COMBO_B, result = ITEM_KEY_4};
tr4_combine[12] = {a = ITEM_KEY_5_COMBO_A, b = ITEM_KEY_5_COMBO_B, result = ITEM_KEY_5};
tr4_combine[13] = {a = ITEM_KEY_6_COMBO_A, b = ITEM_KEY_6_COMBO_B, result = ITEM_KEY_6};
tr4_combine[14] = {a = ITEM_KEY_7_COMBO_A, b = ITEM_KEY_7_COMBO_B, result = ITEM_KEY_7};
tr4_combine[15] = {a = ITEM_KEY_8_COMBO_A, b = ITEM_KEY_8_COMBO_B, result = ITEM_KEY_8};
tr4_combine[16] = {a = ITEM_PICKUP_1_COMBO_A, b = ITEM_PICKUP_1_COMBO_B, result = ITEM_PICKUP_1};
tr4_combine[17] = {a = ITEM_PICKUP_2_COMBO_A, b = ITEM_PICKUP_2_COMBO_B, result = ITEM_PICKUP_2};
tr4_combine[18] = {a = ITEM_PICKUP_3_COMBO_A, b = ITEM_PICKUP_3_COMBO_B, result = ITEM_PICKUP_3};
tr4_combine[19] = {a = ITEM_PICKUP_4_COMBO_A, b = ITEM_PICKUP_4_COMBO_B, result = ITEM_PICKUP_4};

tr4_combine[20] = {a = ITEM_WATERSKIN_LARGE_5, b = ITEM_WATERSKIN_SMALL_0, result = ITEM_WATERSKIN_LARGE_2, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[21] = {a = ITEM_WATERSKIN_LARGE_5, b = ITEM_WATERSKIN_SMALL_1, result = ITEM_WATERSKIN_LARGE_3, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[22] = {a = ITEM_WATERSKIN_LARGE_5, b = ITEM_WATERSKIN_SMALL_2, result = ITEM_WATERSKIN_LARGE_4, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[23] = {a = ITEM_WATERSKIN_LARGE_4, b = ITEM_WATERSKIN_SMALL_0, result = ITEM_WATERSKIN_LARGE_1, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[24] = {a = ITEM_WATERSKIN_LARGE_4, b = ITEM_WATERSKIN_SMALL_1, result = ITEM_WATERSKIN_LARGE_2, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[25] = {a = ITEM_WATERSKIN_LARGE_4, b = ITEM_WATERSKIN_SMALL_2, result = ITEM_WATERSKIN_LARGE_3, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[26] = {a = ITEM_WATERSKIN_LARGE_3, b = ITEM_WATERSKIN_SMALL_0, result = ITEM_WATERSKIN_LARGE_0, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[27] = {a = ITEM_WATERSKIN_LARGE_3, b = ITEM_WATERSKIN_SMALL_1, result = ITEM_WATERSKIN_LARGE_1, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[28] = {a = ITEM_WATERSKIN_LARGE_3, b = ITEM_WATERSKIN_SMALL_2, result = ITEM_WATERSKIN_LARGE_2, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[29] = {a = ITEM_WATERSKIN_LARGE_2, b = ITEM_WATERSKIN_SMALL_0, result = ITEM_WATERSKIN_LARGE_0, result2 = ITEM_WATERSKIN_SMALL_2};
tr4_combine[30] = {a = ITEM_WATERSKIN_LARGE_2, b = ITEM_WATERSKIN_SMALL_1, result = ITEM_WATERSKIN_LARGE_0, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[31] = {a = ITEM_WATERSKIN_LARGE_2, b = ITEM_WATERSKIN_SMALL_2, result = ITEM_WATERSKIN_LARGE_1, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[32] = {a = ITEM_WATERSKIN_LARGE_1, b = ITEM_WATERSKIN_SMALL_0, result = ITEM_WATERSKIN_LARGE_0, result2 = ITEM_WATERSKIN_SMALL_1};
tr4_combine[33] = {a = ITEM_WATERSKIN_LARGE_1, b = ITEM_WATERSKIN_SMALL_1, result = ITEM_WATERSKIN_LARGE_0, result2 = ITEM_WATERSKIN_SMALL_2};
tr4_combine[34] = {a = ITEM_WATERSKIN_LARGE_1, b = ITEM_WATERSKIN_SMALL_2, result = ITEM_WATERSKIN_LARGE_0, result2 = ITEM_WATERSKIN_SMALL_3};
tr4_combine[35] = {a = ITEM_WATERSKIN_SMALL_3, b = ITEM_WATERSKIN_LARGE_0, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_3};
tr4_combine[36] = {a = ITEM_WATERSKIN_SMALL_3, b = ITEM_WATERSKIN_LARGE_1, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_4};
tr4_combine[37] = {a = ITEM_WATERSKIN_SMALL_3, b = ITEM_WATERSKIN_LARGE_2, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_5};
tr4_combine[38] = {a = ITEM_WATERSKIN_SMALL_3, b = ITEM_WATERSKIN_LARGE_3, result = ITEM_WATERSKIN_SMALL_1, result2 = ITEM_WATERSKIN_LARGE_5};
tr4_combine[39] = {a = ITEM_WATERSKIN_SMALL_3, b = ITEM_WATERSKIN_LARGE_4, result = ITEM_WATERSKIN_SMALL_2, result2 = ITEM_WATERSKIN_LARGE_5};
tr4_combine[40] = {a = ITEM_WATERSKIN_SMALL_2, b = ITEM_WATERSKIN_LARGE_0, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_2};
tr4_combine[41] = {a = ITEM_WATERSKIN_SMALL_2, b = ITEM_WATERSKIN_LARGE_1, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_3};
tr4_combine[42] = {a = ITEM_WATERSKIN_SMALL_2, b = ITEM_WATERSKIN_LARGE_2, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_4};
tr4_combine[43] = {a = ITEM_WATERSKIN_SMALL_2, b = ITEM_WATERSKIN_LARGE_3, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_5};
tr4_combine[44] = {a = ITEM_WATERSKIN_SMALL_2, b = ITEM_WATERSKIN_LARGE_4, result = ITEM_WATERSKIN_SMALL_1, result2 = ITEM_WATERSKIN_LARGE_5};
tr4_combine[45] = {a = ITEM_WATERSKIN_SMALL_1, b = ITEM_WATERSKIN_LARGE_0, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_1};
tr4_combine[46] = {a = ITEM_WATERSKIN_SMALL_1, b = ITEM_WATERSKIN_LARGE_1, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_2};
tr4_combine[47] = {a = ITEM_WATERSKIN_SMALL_1, b = ITEM_WATERSKIN_LARGE_2, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_3};
tr4_combine[48] = {a = ITEM_WATERSKIN_SMALL_1, b = ITEM_WATERSKIN_LARGE_3, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_4};
tr4_combine[49] = {a = ITEM_WATERSKIN_SMALL_1, b = ITEM_WATERSKIN_LARGE_4, result = ITEM_WATERSKIN_SMALL_0, result2 = ITEM_WATERSKIN_LARGE_5};

tr4_combine[50] = {a = ITEM_MAGNUM, b = ITEM_LASERSIGHT, result = ITEM_MAGNUM_LASERSIGHT};
tr4_combine[51] = {a = ITEM_CROSSBOW, b = ITEM_LASERSIGHT, result = ITEM_CROSSBOW_LASERSIGHT};


-- Combine / separate maps for TR5.

tr5_combine[0]  = {a = ITEM_PUZZLE_1_COMBO_A, b = ITEM_PUZZLE_1_COMBO_B, result = ITEM_PUZZLE_1};
tr5_combine[1]  = {a = ITEM_PUZZLE_2_COMBO_A, b = ITEM_PUZZLE_2_COMBO_B, result = ITEM_PUZZLE_2};
tr5_combine[2]  = {a = ITEM_PUZZLE_3_COMBO_A, b = ITEM_PUZZLE_3_COMBO_B, result = ITEM_PUZZLE_3};
tr5_combine[3]  = {a = ITEM_PUZZLE_4_COMBO_A, b = ITEM_PUZZLE_4_COMBO_B, result = ITEM_PUZZLE_4};
tr5_combine[4]  = {a = ITEM_PUZZLE_5_COMBO_A, b = ITEM_PUZZLE_5_COMBO_B, result = ITEM_PUZZLE_5};
tr5_combine[5]  = {a = ITEM_PUZZLE_6_COMBO_A, b = ITEM_PUZZLE_6_COMBO_B, result = ITEM_PUZZLE_6};
tr5_combine[6]  = {a = ITEM_PUZZLE_7_COMBO_A, b = ITEM_PUZZLE_7_COMBO_B, result = ITEM_PUZZLE_7};
tr5_combine[7]  = {a = ITEM_PUZZLE_8_COMBO_A, b = ITEM_PUZZLE_8_COMBO_B, result = ITEM_PUZZLE_8};
tr5_combine[8]  = {a = ITEM_KEY_1_COMBO_A, b = ITEM_KEY_1_COMBO_B, result = ITEM_KEY_1};
tr5_combine[9]  = {a = ITEM_KEY_2_COMBO_A, b = ITEM_KEY_2_COMBO_B, result = ITEM_KEY_2};
tr5_combine[10] = {a = ITEM_KEY_3_COMBO_A, b = ITEM_KEY_3_COMBO_B, result = ITEM_KEY_3};
tr5_combine[11] = {a = ITEM_KEY_4_COMBO_A, b = ITEM_KEY_4_COMBO_B, result = ITEM_KEY_4};
tr5_combine[12] = {a = ITEM_KEY_5_COMBO_A, b = ITEM_KEY_5_COMBO_B, result = ITEM_KEY_5};
tr5_combine[13] = {a = ITEM_KEY_6_COMBO_A, b = ITEM_KEY_6_COMBO_B, result = ITEM_KEY_6};
tr5_combine[14] = {a = ITEM_KEY_7_COMBO_A, b = ITEM_KEY_7_COMBO_B, result = ITEM_KEY_7};
tr5_combine[15] = {a = ITEM_KEY_8_COMBO_A, b = ITEM_KEY_8_COMBO_B, result = ITEM_KEY_8};
tr5_combine[16] = {a = ITEM_PICKUP_1_COMBO_A, b = ITEM_PICKUP_1_COMBO_B, result = ITEM_PICKUP_1};
tr5_combine[17] = {a = ITEM_PICKUP_2_COMBO_A, b = ITEM_PICKUP_2_COMBO_B, result = ITEM_PICKUP_2};
tr5_combine[18] = {a = ITEM_PICKUP_3_COMBO_A, b = ITEM_PICKUP_3_COMBO_B, result = ITEM_PICKUP_3};
tr5_combine[19] = {a = ITEM_PICKUP_4_COMBO_A, b = ITEM_PICKUP_4_COMBO_B, result = ITEM_PICKUP_4};

tr5_combine[20] = {a = ITEM_MAGNUM, b = ITEM_LASERSIGHT, result = ITEM_MAGNUM_LASERSIGHT};
tr5_combine[21] = {a = ITEM_M16, b = ITEM_LASERSIGHT, result = ITEM_M16_LASERSIGHT};

function combineItems(a, b, oneway)
    
    oneway    = oneway or 0;    
    local ver = getLevelVersion();
    local map = {};
    local i   = 0;
    local j   = 0;
    
    if(ver < TR_IV) then
        print("Combine: wrong game version.");
        return;
    elseif(ver < TR_V) then
        map = tr4_combine;
    elseif(ver == TR_V) then
        map = tr5_combine;
    else
        print("Combine: unknown game version.");
        return;
    end;
    
    if(getItemsCount(player, a) == 0 or getItemsCount(player, b) == 0) then
        print("Combine: no specified item(s) in inventory!");
        return;
    end;
    
    while(map[i] ~= nil) do
        if(((a == map[i].a) and (b == map[i].b)) or
           ((b == map[i].a) and (a == map[i].b) and (oneway == 0))) then
            removeItem(player, a, 1);
            removeItem(player, b, 1);
            addItem(player, map[i].result, 1);
            if(map[i].result2 ~= nil) then
                addItem(player, map[i].result2, 1);
            end;
            playSound(getGlobalSound(ver, GLOBALID_MENUCLANG));
            print("Combine: items successfully combined!");
            return;
        end;
        i = i + 1;
    end;
    
    print("Combine: no items combined. Wrong combo or item name(s)?");
    
end

function separateItems(a)
    
    local ver = getLevelVersion();
    local map = {};
    local i   = 0;
    local j   = 0;
    
    if(ver < TR_IV) then
        print("Wrong game version.");
        return;
    elseif(ver < TR_V) then
        map = tr4_combine;
    elseif(ver == TR_V) then
        map = tr5_combine;
    end;
    
    if(getItemsCount(player, a) <= 0) then
        print("Separate: no specified item in inventory!");
        return;
    end;
    
    while(map[i] ~= nil) do
        if(a == map[i].result) then
            addItem(player, map[i].a, 1);
            addItem(player, map[i].b, 1);
            removeItem(player, a, 1);
            playSound(getGlobalSound(ver, GLOBALID_MENUCLANG));
            print("Separate: items successfully separated!");
            return;
        end;
        i = i + 1;
    end;
    
    print("Separate: no items separated. Wrong item name(s)?");
    
end