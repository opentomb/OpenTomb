-- OPENTOMB INVENTORY ITEM LIST
-- by Lwmte, Sep 2014

--------------------------------------------------------------------------------
-- CLASSIFICATION OF ITEMS
-- Basically, all TR items could be separated into three big groups - supplies
-- (weapons, ammo, medkits etc.), quest items (keys, puzzle pieces, artifacts)
-- and system items (passport, settings, load/save items). This classification
-- is needed for further implementation of ring inventory categorization.
--------------------------------------------------------------------------------

ITEM_TYPE_SYSTEM = 0
ITEM_TYPE_SUPPLY = 1
ITEM_TYPE_QUEST  = 2

--------------------------------------------------------------------------------
-- Defines menu items.
-- In first three TR games, menu items were integrated into inventory ring.
-- As TR community tends to like ring menu more than TR4-5 system, we opt for
-- recreating this menu type, thus reserving these menu slots for future use.
--------------------------------------------------------------------------------

ITEM_COMPASS = 1    -- Compass is an omnipresent menu / inventory item.
ITEM_PASSPORT = 2   -- Not used in TR4 (as diary).
ITEM_LARAHOME = 3
ITEM_VIDEO = 4
ITEM_AUDIO = 5
ITEM_CONTROLS = 6
ITEM_LOAD = 7       -- Only used in TR4-5, technically the same as passport.
ITEM_SAVE = 8       -- Only used in TR4-5, technically the same as passport.
ITEM_MAP = 9        -- Not used in TR1-3, but exist as model. Not used in TR4.

--------------------------------------------------------------------------------
-- Defines global inventory items.
-- Since different versions of TR feature different weapons yet originating from
-- the previous one (e. g., Automags originating from Magnums, MP5 originating
-- from M16, etc.), it unifies similar weapons from various games under ancestor
-- name. For example, ITEM_MAGNUM count either for magnums, automags, desert
-- eagle and revolver.
--------------------------------------------------------------------------------

ITEM_PISTOL = 10
ITEM_SHOTGUN = 11
ITEM_MAGNUM = 12        -- TR2: Automags, TR3/5: Desert eagle, TR4/5: Revolver
ITEM_UZI = 13
ITEM_M16 = 14           -- TR3: MP5, TR5: H&K
ITEM_GRENADEGUN = 15
ITEM_ROCKETGUN = 16
ITEM_HARPOONGUN = 17
ITEM_CROSSBOW = 18      -- TR5: Grappling gun

ITEM_MAGNUM_LASERSIGHT       =  1220    -- Extra item index for revolver + lasersight (TR4-5)
ITEM_CROSSBOW_LASERSIGHT     =  1820    -- Extra item index for crossbow + lasersight (TR4)
ITEM_M16_LASERSIGHT          =  1420    -- Extra item index for H&K gun + lasersight (TR5)

ITEM_LASERSIGHT = 20    -- Exist only in TR4-5.
ITEM_BINOCULARS = 21    -- Exist only in TR4-5.
ITEM_SILENCER   = 22    -- Exist only in TR5, not used.

ITEM_PISTOL_AMMO = 30
ITEM_SHOTGUN_NORMAL_AMMO = 31       -- Equal to generic shotgun ammo in TR1-3.
ITEM_SHOTGUN_WIDESHOT_AMMO = 32
ITEM_MAGNUM_AMMO = 33
ITEM_UZI_AMMO = 34
ITEM_M16_AMMO = 35
ITEM_GRENADEGUN_NORMAL_AMMO = 36    -- Equal to generic grenadegun ammo in TR3.
ITEM_GRENADEGUN_SUPER_AMMO = 37
ITEM_GRENADEGUN_FLASH_AMMO = 38
ITEM_ROCKETGUN_AMMO = 39            -- Equal to generic grenadegun ammo in TR2.
ITEM_HARPOONGUN_AMMO = 40
ITEM_CROSSBOW_NORMAL_AMMO = 41
ITEM_CROSSBOW_POISON_AMMO = 42
ITEM_CROSSBOW_EXPLOSIVE_AMMO = 43

ITEM_FLARES = 45
ITEM_SINGLE_FLARE = 46
ITEM_TORCH = 47             -- Only exists in TR4-5.

ITEM_SMALL_MEDIPACK = 50
ITEM_LARGE_MEDIPACK = 51

--------------------------------------------------------------------------------
-- Defines level-specific inventory items.
-- Despite the fact that these items change across level versions, they usually
-- share the same slots under some common names listed in objects.h file. On a
-- later stage, names are specified for each level via script. Hence, we can
-- safely redefine these items here and assign names to them via per-level
-- script file later.
--------------------------------------------------------------------------------

ITEM_KEY_1  = 60
ITEM_KEY_2  = 61
ITEM_KEY_3  = 62
ITEM_KEY_4  = 63     -- This is the last key slot for TR1-3.
ITEM_KEY_5  = 64
ITEM_KEY_6  = 65
ITEM_KEY_7  = 66
ITEM_KEY_8  = 67
ITEM_KEY_9  = 68
ITEM_KEY_10 = 69
ITEM_KEY_11 = 70
ITEM_KEY_12 = 71     -- This is the last key slot for TR4-5.

ITEM_PUZZLE_1  = 80
ITEM_PUZZLE_2  = 81
ITEM_PUZZLE_3  = 82
ITEM_PUZZLE_4  = 83  -- This is the last puzzle slot for TR1-3.
ITEM_PUZZLE_5  = 84
ITEM_PUZZLE_6  = 85
ITEM_PUZZLE_7  = 86
ITEM_PUZZLE_8  = 87
ITEM_PUZZLE_9  = 88
ITEM_PUZZLE_10 = 89
ITEM_PUZZLE_11 = 90
ITEM_PUZZLE_12 = 91  -- This is the last puzzle slot for TR4-5.

-- Pickup items are generic slots for items that are neither keys nor
-- puzzles.

ITEM_PICKUP_1 = 100
ITEM_PICKUP_2 = 101
ITEM_PICKUP_3 = 102
ITEM_PICKUP_4 = 103
ITEM_PICKUP_5 = 104
ITEM_PICKUP_6 = 105

-- Quest items are usually global for each game (e. g., Scion for TR1,
-- Dagger of Xian for TR2, etc.), but occasionally, same quest item
-- slot may share several items which were removed from inventory
-- before.

ITEM_QUEST_1 = 110
ITEM_QUEST_2 = 111
ITEM_QUEST_3 = 112
ITEM_QUEST_4 = 113
ITEM_QUEST_5 = 114
ITEM_QUEST_6 = 115
ITEM_QUEST_7 = 116
ITEM_QUEST_8 = 117

-- Secret items only exist in TR2 and TR5, plus they appear in first
-- level of TR4. In TR2, there are three secret items, while in TR4 and
-- TR5, there is only one secret item slot.

ITEM_SECRET_1 = 120
ITEM_SECRET_2 = 121
ITEM_SECRET_3 = 122

--------------------------------------------------------------------------------
-- Defines TR4-5-specific items.
-- TR4 and TR5 introduced a lot of unique pick-up objects, which never existed
-- in earlier games. Therefore, to ease out conditional checks, we specify all
-- these items after all other items are specified.
--------------------------------------------------------------------------------

-- Combo items only exist in TR4-5, and they produce corresponding
-- full item when combined. Both TR4-5 contain combo variations only
-- for first 8 puzzle and key items.

ITEM_PUZZLE_1_COMBO_A   = 130
ITEM_PUZZLE_1_COMBO_B   = 131
ITEM_PUZZLE_2_COMBO_A   = 132
ITEM_PUZZLE_2_COMBO_B   = 133
ITEM_PUZZLE_3_COMBO_A   = 134
ITEM_PUZZLE_3_COMBO_B   = 135
ITEM_PUZZLE_4_COMBO_A   = 136
ITEM_PUZZLE_4_COMBO_B   = 137
ITEM_PUZZLE_5_COMBO_A   = 138
ITEM_PUZZLE_5_COMBO_B   = 139
ITEM_PUZZLE_6_COMBO_A   = 140
ITEM_PUZZLE_6_COMBO_B   = 141
ITEM_PUZZLE_7_COMBO_A   = 142
ITEM_PUZZLE_7_COMBO_B   = 143
ITEM_PUZZLE_8_COMBO_A   = 144
ITEM_PUZZLE_8_COMBO_B   = 145

ITEM_KEY_1_COMBO_A      = 150
ITEM_KEY_1_COMBO_B      = 151
ITEM_KEY_2_COMBO_A      = 152
ITEM_KEY_2_COMBO_B      = 153
ITEM_KEY_3_COMBO_A      = 154
ITEM_KEY_3_COMBO_B      = 155
ITEM_KEY_4_COMBO_A      = 156
ITEM_KEY_4_COMBO_B      = 157
ITEM_KEY_5_COMBO_A      = 158
ITEM_KEY_5_COMBO_B      = 159
ITEM_KEY_6_COMBO_A      = 160
ITEM_KEY_6_COMBO_B      = 161
ITEM_KEY_7_COMBO_A      = 162
ITEM_KEY_7_COMBO_B      = 163
ITEM_KEY_8_COMBO_A      = 164
ITEM_KEY_8_COMBO_B      = 165

ITEM_PICKUP_1_COMBO_A   = 170
ITEM_PICKUP_1_COMBO_B   = 171
ITEM_PICKUP_2_COMBO_A   = 172
ITEM_PICKUP_2_COMBO_B   = 173
ITEM_PICKUP_3_COMBO_A   = 174
ITEM_PICKUP_3_COMBO_B   = 175
ITEM_PICKUP_4_COMBO_A   = 176
ITEM_PICKUP_4_COMBO_B   = 177

-- Waterskin items are kind of combo items, but with very specific
-- kind of action - you can pour the water from one to another.

ITEM_WATERSKIN_SMALL_0  = 180
ITEM_WATERSKIN_SMALL_1  = 181
ITEM_WATERSKIN_SMALL_2  = 182
ITEM_WATERSKIN_SMALL_3  = 183
ITEM_WATERSKIN_LARGE_0  = 184
ITEM_WATERSKIN_LARGE_1  = 185
ITEM_WATERSKIN_LARGE_2  = 186
ITEM_WATERSKIN_LARGE_3  = 187
ITEM_WATERSKIN_LARGE_4  = 188
ITEM_WATERSKIN_LARGE_5  = 189

-- Crowbar item is a specific pick-up for TR4-5 only. Despite the fact
-- that it also was present in TR3, it counted there as simple puzzle
-- piece (and that's the reason why it was used with usual Lara puzzle
-- insertion animation).

ITEM_CROWBAR = 190

-- Examine items are present only in TR4.

ITEM_EXAMINE_1 = 200
ITEM_EXAMINE_2 = 201
ITEM_EXAMINE_3 = 202

-- Chloroform and cloth is only present in TR5.

ITEM_CHLOROFORM       = 210
ITEM_CLOTH            = 211
ITEM_CHLOROFORM_CLOTH = 212
