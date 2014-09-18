-- OPENTOMB ENTITY COLLISION AND VISIBILITY FLAG OVERRIDE
-- By TeslaRus, Lwmte 2013-2014

--------------------------------------------------------------------------------
--   0x0000  - Object has no collisions
--   0x0001  - Object uses real mesh data for collision.
--   0x0002  - Object uses bounding box for collision.

--  Note that this common script also defines default collision flags for static
--  meshes, although each level in TR have different static mesh assignment, so
--  these are purely temporary, until all level scrips are written.
--------------------------------------------------------------------------------

print("Collide / visibility flags script loaded");

--------------------------------------------------------------------------------
----------------------------- TR_I, TR_I_DEMO, TR_I_UB -------------------------
--------------------------------------------------------------------------------
tr1_static_tbl = {};                             -- TR_I, TR_I_DEMO, TR_I_UB
tr1_static_tbl[10] = {coll = 0x0001};            -- closed gate
tr1_static_tbl[11] = {coll = 0x0002};            -- trough
tr1_static_tbl[12] = {coll = 0x0001};            -- animal leather
--tr1_static_tbl[13] = {coll = 0x0001};
tr1_static_tbl[14] = {coll = 0x0001};            -- table
tr1_static_tbl[15] = {coll = 0x0001};            -- chair
tr1_static_tbl[16] = {coll = 0x0001};            -- Natla's cave desk
tr1_static_tbl[17] = {coll = 0x0001};            -- Natla's cave desk
tr1_static_tbl[18] = {coll = 0x0001};            -- Natla's cave desk
tr1_static_tbl[19] = {coll = 0x0001};            -- Natla's cave barrier
tr1_static_tbl[20] = {coll = 0x0001};            -- Natla's cave ugol ff
tr1_static_tbl[30] = {coll = 0x0001};            -- ticky statue or colonna
tr1_static_tbl[31] = {coll = 0x0001};            -- sharp
tr1_static_tbl[32] = {coll = 0x0001};            -- Natla's cave barrier
tr1_static_tbl[33] = {coll = 0x0001};            -- part of the bridge
tr1_static_tbl[34] = {coll = 0x0001};            -- part of the bridge
tr1_static_tbl[39] = {coll = 0x0001};            -- ruined wall
--tr1_static_tbl[43] = {coll = 0x0001};

--------------------------------------------------------------------------------
--------------------------------- TR_II, TR_II_DEMO ----------------------------
--------------------------------------------------------------------------------
tr2_static_tbl = {};
tr2_static_tbl[0] = {coll = 0x0001};
tr2_static_tbl[30] = {coll = 0x0001};       -- Chandelier

--------------------------------------------------------------------------------
-------------------------------------- TR_III ----------------------------------
--------------------------------------------------------------------------------
tr3_static_tbl = {};
tr3_static_tbl[10] = {coll = 0x0001};       -- stone Shiva
tr3_static_tbl[11] = {coll = 0x0001};       -- stone Shiva
tr3_static_tbl[30] = {coll = 0x0001};       -- shelf
tr3_static_tbl[31] = {coll = 0x0001};       -- shelf
tr3_static_tbl[41] = {coll = 0x0002};       -- safe
--------------------------------------------------------------------------------
--------------------------------- TR_IV, TR_IV_DEMO ----------------------------
--------------------------------------------------------------------------------
tr4_static_tbl = {};

-- PLANT statics (as listed in OBJECTS.H from TRLE)

tr4_static_tbl[0]  = {coll = 0x0002};            -- PLANT0
tr4_static_tbl[01] = {coll = 0x0002};            -- PLANT1
tr4_static_tbl[02] = {coll = 0x0002};            -- PLANT2
tr4_static_tbl[03] = {coll = 0x0002};            -- PLANT3
tr4_static_tbl[04] = {coll = 0x0002};            -- PLANT4
tr4_static_tbl[05] = {coll = 0x0002};            -- PLANT5
tr4_static_tbl[06] = {coll = 0x0002};            -- PLANT6
tr4_static_tbl[07] = {coll = 0x0002};            -- PLANT7
tr4_static_tbl[08] = {coll = 0x0002};            -- PLANT8
tr4_static_tbl[09] = {coll = 0x0002};            -- PLANT9

-- FURNITURE statics

tr4_static_tbl[10] = {coll = 0x0001};            -- column
tr4_static_tbl[11] = {coll = 0x0001};            -- big vase (library), stairs (semer) - difference models collision (stairs - ColBox=NULL)
tr4_static_tbl[12] = {coll = 0x0002};            -- bank
tr4_static_tbl[13] = {coll = 0x0001};            -- ark
tr4_static_tbl[14] = {coll = 0x0001};            -- floor light
tr4_static_tbl[15] = {coll = 0x0001};            -- balcony border (semer), vase (karnak1)
tr4_static_tbl[16] = {coll = 0x0002};
tr4_static_tbl[17] = {coll = 0x0002};            -- colonn
tr4_static_tbl[18] = {coll = 0x0001};            -- stairs
tr4_static_tbl[19] = {coll = 0x0001};            -- scroll safe (library)

-- ROCK statics

tr4_static_tbl[20] = {coll = 0x0001};            -- scroll safe (library)
tr4_static_tbl[21] = {coll = 0x0002};
tr4_static_tbl[22] = {coll = 0x0002};
tr4_static_tbl[23] = {coll = 0x0002};
tr4_static_tbl[24] = {coll = 0x0001};            -- stairs (hub.tr4)
tr4_static_tbl[25] = {coll = 0x0002};
tr4_static_tbl[26] = {coll = 0x0002};
tr4_static_tbl[27] = {coll = 0x0001};            -- palma - fixme: stick
tr4_static_tbl[28] = {coll = 0x0002};
tr4_static_tbl[29] = {coll = 0x0002};

-- ARCHITECTURE statics

tr4_static_tbl[30] = {coll = 0x0001};            -- column
tr4_static_tbl[31] = {coll = 0x0002};            -- balcony border
tr4_static_tbl[32] = {coll = 0x0002};            -- balcony border
tr4_static_tbl[33] = {coll = 0x0002};            -- balcony border
tr4_static_tbl[34] = {coll = 0x0002};            -- balcony border
tr4_static_tbl[35] = {coll = 0x0001};            -- stairs
tr4_static_tbl[36] = {coll = 0x0002};
tr4_static_tbl[37] = {coll = 0x0002};            -- corner colonna
tr4_static_tbl[38] = {coll = 0x0002};
tr4_static_tbl[39] = {coll = 0x0002};

-- DEBRIS statics

tr4_static_tbl[40] = {coll = 0x0002};
tr4_static_tbl[41] = {coll = 0x0002};
tr4_static_tbl[42] = {coll = 0x0002};
tr4_static_tbl[43] = {coll = 0x0002};  
tr4_static_tbl[44] = {coll = 0x0002};
tr4_static_tbl[45] = {coll = 0x0002};
tr4_static_tbl[46] = {coll = 0x0002};
tr4_static_tbl[47] = {coll = 0x0002};
tr4_static_tbl[48] = {coll = 0x0002};
tr4_static_tbl[49] = {coll = 0x0002};

-- SHATTER statics
-- Note: all SHATTER statics can be destroyed by SHATTER event (either on hit or something else)

tr4_static_tbl[50] = {coll = 0x0001};
tr4_static_tbl[51] = {coll = 0x0002};
tr4_static_tbl[52] = {coll = 0x0002};
tr4_static_tbl[53] = {coll = 0x0002};
tr4_static_tbl[54] = {coll = 0x0002};
tr4_static_tbl[55] = {coll = 0x0002};
tr4_static_tbl[56] = {coll = 0x0002};
tr4_static_tbl[57] = {coll = 0x0002};
tr4_static_tbl[58] = {coll = 0x0002};
tr4_static_tbl[59] = {coll = 0x0002};

--------------------------------------------------------------------------------
------------------------------------- TR_V -------------------------------------
--------------------------------------------------------------------------------
tr5_static_tbl = {};

-- Remark: TR5's leaked OBJECTS.H static section file is similar to TR4 one,
-- except 5 additional statics with MIPs added at the end.

-- PLANT statics (as listed in OBJECTS.H from TRLE)

tr5_static_tbl[00] = {coll = 0x0002};            -- PLANT0
tr5_static_tbl[01] = {coll = 0x0002};            -- PLANT1
tr5_static_tbl[02] = {coll = 0x0001};            -- PLANT2
tr5_static_tbl[03] = {coll = 0x0001};            -- PLANT3
tr5_static_tbl[04] = {coll = 0x0001};            -- PLANT4
tr5_static_tbl[05] = {coll = 0x0001};            -- PLANT5
tr5_static_tbl[06] = {coll = 0x0001};            -- PLANT6
tr5_static_tbl[07] = {coll = 0x0001};            -- PLANT7
tr5_static_tbl[08] = {coll = 0x0001};            -- PLANT8
tr5_static_tbl[09] = {coll = 0x0001};            -- PLANT9

-- FURNITURE statics

tr5_static_tbl[10] = {coll = 0x0001};            -- FURNITURE0
tr5_static_tbl[11] = {coll = 0x0001};            -- FURNITURE1
tr5_static_tbl[12] = {coll = 0x0001};            -- FURNITURE2
tr5_static_tbl[13] = {coll = 0x0002};            -- FURNITURE3
tr5_static_tbl[14] = {coll = 0x0001};            -- FURNITURE4
tr5_static_tbl[15] = {coll = 0x0002};            -- FURNITURE5
tr5_static_tbl[16] = {coll = 0x0002};            -- FURNITURE6
tr5_static_tbl[17] = {coll = 0x0001};            -- FURNITURE7
tr5_static_tbl[18] = {coll = 0x0001};            -- FURNITURE8
tr5_static_tbl[19] = {coll = 0x0001};            -- FURNITURE9

-- ROCK statics

tr5_static_tbl[20] = {coll = 0x0001};            -- ROCK0
tr5_static_tbl[21] = {coll = 0x0002};            -- ROCK1
tr5_static_tbl[22] = {coll = 0x0002};            -- ROCK2
tr5_static_tbl[23] = {coll = 0x0001};            -- ROCK3
tr5_static_tbl[24] = {coll = 0x0001};            -- ROCK4
tr5_static_tbl[25] = {coll = 0x0001};            -- ROCK5
tr5_static_tbl[26] = {coll = 0x0001};            -- ROCK6
tr5_static_tbl[27] = {coll = 0x0001};            -- ROCK7
tr5_static_tbl[28] = {coll = 0x0001};            -- ROCK8
tr5_static_tbl[29] = {coll = 0x0001};            -- ROCK9

-- ARCHITECTURE statics

tr5_static_tbl[30] = {coll = 0x0001};            -- ARCHITECTURE0
tr5_static_tbl[31] = {coll = 0x0002};            -- ARCHITECTURE1
tr5_static_tbl[32] = {coll = 0x0002};            -- ARCHITECTURE2
tr5_static_tbl[33] = {coll = 0x0002};            -- ARCHITECTURE3
tr5_static_tbl[34] = {coll = 0x0002};            -- ARCHITECTURE4
tr5_static_tbl[35] = {coll = 0x0001};            -- ARCHITECTURE5
tr5_static_tbl[36] = {coll = 0x0002};            -- ARCHITECTURE6
tr5_static_tbl[37] = {coll = 0x0001};            -- ARCHITECTURE7
tr5_static_tbl[38] = {coll = 0x0001};            -- ARCHITECTURE8
tr5_static_tbl[39] = {coll = 0x0001};            -- ARCHITECTURE9

-- DEBRIS statics

tr5_static_tbl[40] = {coll = 0x0002};            -- DEBRIS0
tr5_static_tbl[41] = {coll = 0x0002};            -- DEBRIS1
tr5_static_tbl[42] = {coll = 0x0002};            -- DEBRIS2
tr5_static_tbl[43] = {coll = 0x0002};            -- DEBRIS3
tr5_static_tbl[44] = {coll = 0x0002};            -- DEBRIS4
tr5_static_tbl[45] = {coll = 0x0002};            -- DEBRIS5
tr5_static_tbl[46] = {coll = 0x0002};            -- DEBRIS6
tr5_static_tbl[47] = {coll = 0x0002};            -- DEBRIS7
tr5_static_tbl[48] = {coll = 0x0002};            -- DEBRIS8
tr5_static_tbl[49] = {coll = 0x0002};            -- DEBRIS9

-- SHATTER statics
-- Note: all SHATTER statics can be destroyed by SHATTER event (either on hit or something else)

tr5_static_tbl[50] = {coll = 0x0001};            -- SHATTER0
tr5_static_tbl[51] = {coll = 0x0001};            -- SHATTER1
tr5_static_tbl[52] = {coll = 0x0001};            -- SHATTER2
tr5_static_tbl[53] = {coll = 0x0001};            -- SHATTER3
tr5_static_tbl[54] = {coll = 0x0001};            -- SHATTER4
tr5_static_tbl[55] = {coll = 0x0001};            -- SHATTER5
tr5_static_tbl[56] = {coll = 0x0001};            -- SHATTER6
tr5_static_tbl[57] = {coll = 0x0001};            -- SHATTER7
tr5_static_tbl[58] = {coll = 0x0001};            -- SHATTER8
tr5_static_tbl[59] = {coll = 0x0001};            -- SHATTER9

-- Extra TR5 static slots

tr5_static_tbl[60] = {coll = 0x0001};            -- STATICOBJ1
tr5_static_tbl[61] = {coll = 0x0001};            -- STATICOBJ1_MIP
tr5_static_tbl[62] = {coll = 0x0001};            -- STATICOBJ2
tr5_static_tbl[63] = {coll = 0x0001};            -- STATICOBJ2_MIP
tr5_static_tbl[64] = {coll = 0x0001};            -- STATICOBJ3
tr5_static_tbl[65] = {coll = 0x0001};            -- STATICOBJ3_MIP
tr5_static_tbl[66] = {coll = 0x0001};            -- STATICOBJ4
tr5_static_tbl[67] = {coll = 0x0001};            -- STATICOBJ4_MIP
tr5_static_tbl[68] = {coll = 0x0001};            -- STATICOBJ5
tr5_static_tbl[69] = {coll = 0x0001};            -- STATICOBJ5_MIP

function GetStaticMeshFlags(ver, id)
    tbl = {};
    if(ver < 3) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        tbl = tr1_static_tbl;
    elseif(ver < 5) then                -- TR_II, TR_II_DEMO
        tbl = tr2_static_tbl;
    elseif(ver < 6) then                -- TR_III
        tbl = tr3_static_tbl;
    elseif(ver < 8) then                -- TR_IV, TR_IV_DEMO
        tbl = tr4_static_tbl;
    elseif(ver < 9) then                -- TR_V
        tbl = tr5_static_tbl;
    else
        return 0, 0;
    end;

    if(tbl[id] == nil) then
        return nil, nil;
    else
        return tbl[id].coll, tbl[id].hide;
    end;
end;


--------------------------------------------------------------------------------
----------------------------- TR_I, TR_I_DEMO, TR_I_UB -------------------------
--------------------------------------------------------------------------------

tr1_entity_tbl = {};

-- NOTE: Objects before ID 06 are internal service objects and never appear in-game.

-- ACTORS --

tr1_entity_tbl[06] = {coll = 0x0002};                      -- Doppelgagner
tr1_entity_tbl[07] = {coll = 0x0002};                      -- Wolf
tr1_entity_tbl[08] = {coll = 0x0002};                      -- Bear
tr1_entity_tbl[09] = {coll = 0x0002};                      -- Bat
tr1_entity_tbl[10] = {coll = 0x0002};                      -- Crocodile (land)
tr1_entity_tbl[11] = {coll = 0x0002};                      -- Crocodile (water)
tr1_entity_tbl[12] = {coll = 0x0002};                      -- Lion Male
tr1_entity_tbl[13] = {coll = 0x0002};                      -- Lion Female
tr1_entity_tbl[14] = {coll = 0x0002};                      -- Puma
tr1_entity_tbl[15] = {coll = 0x0002};                      -- Gorilla
tr1_entity_tbl[16] = {coll = 0x0002};                      -- Rat (land)
tr1_entity_tbl[17] = {coll = 0x0002};                      -- Rat (water)
tr1_entity_tbl[18] = {coll = 0x0001};                      -- T-Rex
tr1_entity_tbl[19] = {coll = 0x0002};                      -- Raptor
tr1_entity_tbl[20] = {coll = 0x0001};                      -- Winged mutant
tr1_entity_tbl[21] = {coll = 0x0000, hide = 1};            -- (RESPAWN POINT?)
tr1_entity_tbl[22] = {coll = 0x0000, hide = 1};            -- (AI TARGET?)
tr1_entity_tbl[23] = {coll = 0x0001};                      -- Centaur
tr1_entity_tbl[24] = {coll = 0x0002};                      -- Mummy
tr1_entity_tbl[25] = {coll = 0x0002};                      -- DinoWarrior (UNUSED!)
tr1_entity_tbl[26] = {coll = 0x0002};                      -- Fish
tr1_entity_tbl[27] = {coll = 0x0002};                      -- Larson
tr1_entity_tbl[28] = {coll = 0x0002};                      -- Pierre
tr1_entity_tbl[29] = {coll = 0x0002};                      -- Skateboard
tr1_entity_tbl[30] = {coll = 0x0002};                      -- Skateboard Kid
tr1_entity_tbl[31] = {coll = 0x0002};                      -- Cowboy
tr1_entity_tbl[32] = {coll = 0x0002};                      -- Mr. T
tr1_entity_tbl[33] = {coll = 0x0002};                      -- Natla (winged)
tr1_entity_tbl[34] = {coll = 0x0001};                      -- Torso Boss

-- ANIMATINGS --

tr1_entity_tbl[35] = {coll = 0x0002};                      -- Falling floor
tr1_entity_tbl[36] = {coll = 0x0001};                      -- Swinging blade (Vilcabamba, etc.)
tr1_entity_tbl[37] = {coll = 0x0001};                      -- Spikes
tr1_entity_tbl[38] = {coll = 0x0001};                      -- Boulder
tr1_entity_tbl[39] = {coll = 0x0001};                      -- Dart
tr1_entity_tbl[40] = {coll = 0x0001};                      -- Dartgun
tr1_entity_tbl[41] = {coll = 0x0001};                      -- Lifting door
tr1_entity_tbl[42] = {coll = 0x0001};                      -- Slamming sawtooth doors
tr1_entity_tbl[43] = {coll = 0x0001};                      -- Sword of Damocles
tr1_entity_tbl[44] = {coll = 0x0001};                      -- Thor's hammer (handle)
tr1_entity_tbl[45] = {coll = 0x0001};                      -- Thor's hammer (block)
tr1_entity_tbl[46] = {coll = 0x0001};                      -- Thor's lightning ball
tr1_entity_tbl[47] = {coll = 0x0001};                      -- Barricade
tr1_entity_tbl[48] = {coll = 0x0002};                      -- Pushable block
tr1_entity_tbl[49] = {coll = 0x0002};                      -- Pushable block
tr1_entity_tbl[50] = {coll = 0x0002};                      -- Pushable block
tr1_entity_tbl[51] = {coll = 0x0002};                      -- Pushable block
tr1_entity_tbl[52] = {coll = 0x0001};                      -- Moving block
tr1_entity_tbl[53] = {coll = 0x0002};                      -- Falling ceiling
tr1_entity_tbl[54] = {coll = 0x0001};                      -- Sword of Damocles (unused?)
tr1_entity_tbl[55] = {coll = 0x0002};                      -- Wall switch (lever)
tr1_entity_tbl[56] = {coll = 0x0002};                      -- Underwater switch (lever)

-- DOORS --

tr1_entity_tbl[57] = {coll = 0x0001};                      -- Door
tr1_entity_tbl[58] = {coll = 0x0001};                      -- Door
tr1_entity_tbl[59] = {coll = 0x0001};                      -- Door
tr1_entity_tbl[60] = {coll = 0x0001};                      -- Door
tr1_entity_tbl[61] = {coll = 0x0001};                      -- Door
tr1_entity_tbl[62] = {coll = 0x0001};                      -- Door
tr1_entity_tbl[63] = {coll = 0x0001};                      -- Door
tr1_entity_tbl[64] = {coll = 0x0001};                      -- Door

tr1_entity_tbl[65] = {coll = 0x0001};                      -- Floor trapdoor
tr1_entity_tbl[66] = {coll = 0x0001};                      -- Floor trapdoor

-- COLLISION OBJECTS --

tr1_entity_tbl[68] = {coll = 0x0001};                      -- Bridge flat
tr1_entity_tbl[69] = {coll = 0x0001};                      -- Bridge tilt 1
tr1_entity_tbl[70] = {coll = 0x0001};                      -- Bridge tilt 2

-- MENU ITEMS AND ANIMATINGS --

tr1_entity_tbl[71] = {coll = 0x0000};                      -- Menu: Passport
tr1_entity_tbl[72] = {coll = 0x0000};                      -- Menu: Compass
tr1_entity_tbl[73] = {coll = 0x0000};                      -- Menu: Lara's Home photo

tr1_entity_tbl[74] = {coll = 0x0002};                      -- Animated cogs 1
tr1_entity_tbl[75] = {coll = 0x0002};                      -- Animated cogs 2
tr1_entity_tbl[76] = {coll = 0x0002};                      -- Animated cogs 3

tr1_entity_tbl[81] = {coll = 0x0000};                      -- Menu: Passport (closed)

tr1_entity_tbl[82] = {coll = 0x0000};                      -- Natla Logo
tr1_entity_tbl[83] = {coll = 0x0001};                      -- Savegame crystal

tr1_entity_tbl[95] = {coll = 0x0000};                      -- Menu: Sunglasses
tr1_entity_tbl[96] = {coll = 0x0000};                      -- Menu: Cassette player
tr1_entity_tbl[97] = {coll = 0x0000};                      -- Menu: Arrow keys
tr1_entity_tbl[98] = {coll = 0x0000};                      -- Menu: Flashlight (UNUSED!)

-- GENERAL MENU ITEMS --

tr1_entity_tbl[99] = {coll = 0x0002};                      -- Menu: Pistols
tr1_entity_tbl[100] = {coll = 0x0002};                     -- Menu: Shotgun
tr1_entity_tbl[101] = {coll = 0x0002};                     -- Menu: Magnums
tr1_entity_tbl[102] = {coll = 0x0002};                     -- Menu: Uzis
tr1_entity_tbl[103] = {coll = 0x0002};                     -- Menu: Pistol ammo
tr1_entity_tbl[104] = {coll = 0x0002};                     -- Menu: Shotgun ammo
tr1_entity_tbl[105] = {coll = 0x0002};                     -- Menu: Magnum ammo
tr1_entity_tbl[106] = {coll = 0x0002};                     -- Menu: Uzi ammo
tr1_entity_tbl[107] = {coll = 0x0002};                     -- Menu: Grenade ammo (UNUSED!)
tr1_entity_tbl[108] = {coll = 0x0002};                     -- Pick-up: Small medipack
tr1_entity_tbl[109] = {coll = 0x0002};                     -- Pick-up: Large medipack

-- PUZZLE PICKUPS --

tr1_entity_tbl[114] = {coll = 0x0002};                     -- Pick-up: Puzzle 1
tr1_entity_tbl[115] = {coll = 0x0002};                     -- Pick-up: Puzzle 2
tr1_entity_tbl[116] = {coll = 0x0002};                     -- Pick-up: Puzzle 3
tr1_entity_tbl[117] = {coll = 0x0002};                     -- Pick-up: Puzzle 4

-- PUZZLES, KEYS AND SLOTS FOR THEM --

tr1_entity_tbl[118] = {coll = 0x0002};                     -- Slot 1 empty
tr1_entity_tbl[119] = {coll = 0x0002};                     -- Slot 2 empty
tr1_entity_tbl[120] = {coll = 0x0002};                     -- Slot 3 empty
tr1_entity_tbl[121] = {coll = 0x0002};                     -- Slot 4 empty
tr1_entity_tbl[122] = {coll = 0x0002};                     -- Slot 1 full
tr1_entity_tbl[123] = {coll = 0x0002};                     -- Slot 2 full
tr1_entity_tbl[124] = {coll = 0x0002};                     -- Slot 3 full
tr1_entity_tbl[125] = {coll = 0x0002};                     -- Slot 4 full

tr1_entity_tbl[127] = {coll = 0x0002};                     -- Puzzle item 1
tr1_entity_tbl[128] = {coll = 0x0002, hide = 1};           -- Midas gold touch

tr1_entity_tbl[133] = {coll = 0x0002};                     -- Key 1
tr1_entity_tbl[134] = {coll = 0x0002};                     -- Key 2
tr1_entity_tbl[135] = {coll = 0x0002};                     -- Key 3
tr1_entity_tbl[136] = {coll = 0x0002};                     -- Key 4

tr1_entity_tbl[137] = {coll = 0x0002};                     -- Lock 1 
tr1_entity_tbl[138] = {coll = 0x0002};                     -- Lock 2
tr1_entity_tbl[139] = {coll = 0x0002};                     -- Lock 3
tr1_entity_tbl[140] = {coll = 0x0002};                     -- Lock 4

tr1_entity_tbl[145] = {coll = 0x0002};                     -- Scion 1
tr1_entity_tbl[146] = {coll = 0x0002};                     -- Scion 2
tr1_entity_tbl[147] = {coll = 0x0000};                     -- Scion holder
tr1_entity_tbl[150] = {coll = 0x0000};                     -- Scion piece

-- ANIMATINGS --

tr1_entity_tbl[161] = {coll = 0x0001};                     -- Centaur statue
tr1_entity_tbl[162] = {coll = 0x0001};                     -- Natla's mines cabin
tr1_entity_tbl[163] = {coll = 0x0001};                     -- Mutant egg

-- SERVICE OBJECTS --

tr1_entity_tbl[166] = {coll = 0x0000};                     -- Gunflash
tr1_entity_tbl[169] = {coll = 0x0000, hide = 0x0001};      -- Camera target
tr1_entity_tbl[170] = {coll = 0x0000, hide = 0x0001};      -- Waterfall mist

tr1_entity_tbl[172] = {coll = 0x0001};                     -- Mutant bullet
tr1_entity_tbl[173] = {coll = 0x0001};                     -- Mutant grenade

tr1_entity_tbl[177] = {coll = 0x0001, hide = 1};           -- Lava particle emitter

tr1_entity_tbl[179] = {coll = 0x0000, hide = 1};           -- Flame emitter
tr1_entity_tbl[180] = {coll = 0x0000};                     -- Moving lava mass
tr1_entity_tbl[181] = {coll = 0x0001};                     -- Mutant egg (big)
tr1_entity_tbl[182] = {coll = 0x0001};                     -- Motorboat

tr1_entity_tbl[183] = {coll = 0x0001, hide = 1};      -- [UNKNOWN YET]

--------------------------------------------------------------------------------
--------------------------------- TR_II, TR_II_DEMO ----------------------------
--------------------------------------------------------------------------------
tr2_entity_tbl = {};

-- Remark: object IDs 0-12 are used for Lara model and never show in game
-- independently.

-- VEHICLES --

tr2_entity_tbl[13] = {coll = 0x0001};                -- Red snowmobile (can go fast)
tr2_entity_tbl[14] = {coll = 0x0001};                -- Boat
tr2_entity_tbl[51] = {coll = 0x0001};                -- Black snowmobile (with guns)

-- ACTORS --

tr2_entity_tbl[15] = {coll = 0x0002};                -- Doberman
tr2_entity_tbl[16] = {coll = 0x0002};                -- Masked goon (white mask, jacket)
tr2_entity_tbl[17] = {coll = 0x0002};                -- Masked goon (white mask, vest)
tr2_entity_tbl[18] = {coll = 0x0002};                -- Masked goon (black mask)
tr2_entity_tbl[19] = {coll = 0x0002};                -- Knifethrower
tr2_entity_tbl[20] = {coll = 0x0002};                -- Shotgun goon
tr2_entity_tbl[21] = {coll = 0x0002};                -- Rat
tr2_entity_tbl[22] = {coll = 0x0002};                -- Dragon (front)
tr2_entity_tbl[23] = {coll = 0x0002};                -- Dragon (back)
tr2_entity_tbl[24] = {coll = 0x0002};                -- Gondola (Venetian boat)
tr2_entity_tbl[25] = {coll = 0x0002};                -- Shark
tr2_entity_tbl[26] = {coll = 0x0002};                -- Yellow moray eel
tr2_entity_tbl[27] = {coll = 0x0002};                -- Black moray eel
tr2_entity_tbl[28] = {coll = 0x0002};                -- Barracuda / Whiskered Fish
tr2_entity_tbl[29] = {coll = 0x0002};                -- Scuba diver
tr2_entity_tbl[30] = {coll = 0x0002};                -- Gun-wielding rig worker (khaki pants)
tr2_entity_tbl[31] = {coll = 0x0002};                -- Gun-wielding rig worker (blue jeans)
tr2_entity_tbl[32] = {coll = 0x0002};                -- Stick-wielding goon
tr2_entity_tbl[33] = {coll = 0x0002};                -- Stick-wielding goon (can't climb)
tr2_entity_tbl[34] = {coll = 0x0002};                -- Flamethrower-wielding goon
tr2_entity_tbl[36] = {coll = 0x0002};                -- Spider
tr2_entity_tbl[37] = {coll = 0x0002};                -- Giant spider
tr2_entity_tbl[38] = {coll = 0x0002};                -- Crow
tr2_entity_tbl[39] = {coll = 0x0002};                -- Tiger / Leopard
tr2_entity_tbl[40] = {coll = 0x0002};                -- Marco Bartoli
tr2_entity_tbl[41] = {coll = 0x0002};                -- Spear-wielding Xian Guard
tr2_entity_tbl[42] = {coll = 0x0002};                -- Spear-wielding Xian Guard statue
tr2_entity_tbl[43] = {coll = 0x0002};                -- Sword-wielding Xian Guard
tr2_entity_tbl[44] = {coll = 0x0002};                -- Sword-wielding Xian Guard statue
tr2_entity_tbl[45] = {coll = 0x0002};                -- Yeti
tr2_entity_tbl[46] = {coll = 0x0002};                -- Bird monster (guards Talion)
tr2_entity_tbl[47] = {coll = 0x0002};                -- Eagle
tr2_entity_tbl[48] = {coll = 0x0002};                -- Mercenary
tr2_entity_tbl[49] = {coll = 0x0002};                -- Mercenary (black ski mask, gray jacket)
tr2_entity_tbl[50] = {coll = 0x0002};                -- Mercenary (black ski mask, brown jacket)
tr2_entity_tbl[52] = {coll = 0x0002};                -- Mercenary snowmobile driver
tr2_entity_tbl[53] = {coll = 0x0002};                -- Monk with long stick
tr2_entity_tbl[54] = {coll = 0x0002};                -- Monk with knife-end stick

-- TRAPS --

tr2_entity_tbl[55] = {coll = 0x0002};                -- Collapsible floor
tr2_entity_tbl[57] = {coll = 0x0001};                -- Loose boards
tr2_entity_tbl[58] = {coll = 0x0002};                -- Swinging sandbag / spiky ball
tr2_entity_tbl[59] = {coll = 0x0002};                -- Spikes / Glass shards
tr2_entity_tbl[60] = {coll = 0x0001};                -- Boulder
tr2_entity_tbl[61] = {coll = 0x0002};                -- Disk (like dart)
tr2_entity_tbl[62] = {coll = 0x0000};                -- Wall-mounted disk shooter (like dartgun)
tr2_entity_tbl[63] = {coll = 0x0001};                -- Drawbridge
tr2_entity_tbl[64] = {coll = 0x0002};                -- Slamming door
tr2_entity_tbl[65] = {coll = 0x0001};                -- Elevator
tr2_entity_tbl[66] = {coll = 0x0002};                -- Minisub
tr2_entity_tbl[67] = {coll = 0x0001};                -- Movable cubical block (pushable)
tr2_entity_tbl[68] = {coll = 0x0001};                -- Movable cubical block (pushable)
tr2_entity_tbl[69] = {coll = 0x0001};                -- Movable cubical block (pushable)
tr2_entity_tbl[70] = {coll = 0x0001};                -- Movable cubical block (pushable)
tr2_entity_tbl[71] = {coll = 0x0002};                -- Big bowl (Ice Palace)
tr2_entity_tbl[72] = {coll = 0x0002};                -- Breakable window (can shoot out)
tr2_entity_tbl[73] = {coll = 0x0002};                -- Breakable window (must jump through)
tr2_entity_tbl[76] = {coll = 0x0001};                -- Airplane propeller
tr2_entity_tbl[77] = {coll = 0x0002};                -- Power saw
tr2_entity_tbl[78] = {coll = 0x0002};                -- Overhead pulley hook
tr2_entity_tbl[79] = {coll = 0x0002};                -- Sandbag / Ceiling fragments
tr2_entity_tbl[80] = {coll = 0x0002};                -- Rolling spindle
tr2_entity_tbl[81] = {coll = 0x0002};                -- Wall-mounted knife blade
tr2_entity_tbl[82] = {coll = 0x0002};                -- Statue with knife blade
tr2_entity_tbl[83] = {coll = 0x0002};                -- Multiple boulders / snowballs
tr2_entity_tbl[84] = {coll = 0x0002};                -- Detachable icicles
tr2_entity_tbl[85] = {coll = 0x0002};                -- Spiky movable wall
tr2_entity_tbl[86] = {coll = 0x0001};                -- Bounce pad
tr2_entity_tbl[87] = {coll = 0x0002};                -- Spiky ceiling segment
tr2_entity_tbl[88] = {coll = 0x0002};                -- Tibetan bell
tr2_entity_tbl[91] = {coll = 0x0000};                -- Lara and a snowmobile
tr2_entity_tbl[92] = {coll = 0x0000};                -- Wheel knob
tr2_entity_tbl[93] = {coll = 0x0000};                -- Switch
tr2_entity_tbl[94] = {coll = 0x0002};                -- Underwater propeller
tr2_entity_tbl[95] = {coll = 0x0002};                -- Air fan
tr2_entity_tbl[96] = {coll = 0x0002};                -- Swinging box / spiky ball
tr2_entity_tbl[101] = {coll = 0x0002};               -- Rolling storage drums
tr2_entity_tbl[102] = {coll = 0x0002};               -- Zipline handle
tr2_entity_tbl[103] = {coll = 0x0000};               -- Switch
tr2_entity_tbl[104] = {coll = 0x0000};               -- Switch
tr2_entity_tbl[105] = {coll = 0x0000};               -- Underwater switch

-- DOORS --

tr2_entity_tbl[106] = {coll = 0x0001};               -- Door
tr2_entity_tbl[107] = {coll = 0x0001};               -- Door
tr2_entity_tbl[108] = {coll = 0x0001};               -- Door
tr2_entity_tbl[109] = {coll = 0x0001};               -- Door
tr2_entity_tbl[110] = {coll = 0x0001};               -- Door
tr2_entity_tbl[111] = {coll = 0x0002};               -- Door (pulled upward in Temple of Xian)
tr2_entity_tbl[112] = {coll = 0x0002};               -- Door (pulled upward in Temple of Xian)
tr2_entity_tbl[113] = {coll = 0x0002};               -- Door (pulled upward)
tr2_entity_tbl[114] = {coll = 0x0001};               -- Trapdoor (opens downward)
tr2_entity_tbl[115] = {coll = 0x0001};               -- Trapdoor (opens downward)
tr2_entity_tbl[116] = {coll = 0x0001};               -- Trapdoor (opens downward)
tr2_entity_tbl[117] = {coll = 0x0001};               -- Bridge (flat)
tr2_entity_tbl[118] = {coll = 0x0001};               -- Bridge (slope = 1)
tr2_entity_tbl[119] = {coll = 0x0001};               -- Bridge (slope = 2)

-- MISC ITEMS --

tr2_entity_tbl[120] = {coll = 0x0000};               -- Secret #1
tr2_entity_tbl[121] = {coll = 0x0000};               -- Secret #2
tr2_entity_tbl[122] = {coll = 0x0000};               -- Lara and butler picture
tr2_entity_tbl[133] = {coll = 0x0000};               -- Secret #3
tr2_entity_tbl[134] = {coll = 0x0000};               -- Natla's logo
tr2_entity_tbl[152] = {coll = 0x0000};               -- Flare
tr2_entity_tbl[153] = {coll = 0x0000};               -- Sunglasses
tr2_entity_tbl[154] = {coll = 0x0000};               -- Portable CD player
tr2_entity_tbl[155] = {coll = 0x0000};               -- Direction keys
tr2_entity_tbl[157] = {coll = 0x0000};               -- Pistol
tr2_entity_tbl[158] = {coll = 0x0000};               -- Shotgun
tr2_entity_tbl[159] = {coll = 0x0000};               -- Auto-pistol
tr2_entity_tbl[160] = {coll = 0x0000};               -- Uzi
tr2_entity_tbl[161] = {coll = 0x0000};               -- Harpoon gun
tr2_entity_tbl[162] = {coll = 0x0000};               -- M16
tr2_entity_tbl[163] = {coll = 0x0000};               -- Grenade launcher
tr2_entity_tbl[164] = {coll = 0x0000};               -- Pistol ammo(?)
tr2_entity_tbl[165] = {coll = 0x0000};               -- Shotgun ammo
tr2_entity_tbl[166] = {coll = 0x0000};               -- Auto-pistol ammo
tr2_entity_tbl[167] = {coll = 0x0000};               -- Uzi ammo
tr2_entity_tbl[168] = {coll = 0x0000};               -- Harpoons
tr2_entity_tbl[169] = {coll = 0x0000};               -- M16 ammo
tr2_entity_tbl[170] = {coll = 0x0000};               -- Grenades
tr2_entity_tbl[171] = {coll = 0x0000};               -- Small medipack
tr2_entity_tbl[172] = {coll = 0x0000};               -- Large medipack
tr2_entity_tbl[173] = {coll = 0x0000};               -- Flares (opening box)
tr2_entity_tbl[178] = {coll = 0x0000};               -- Puzzle 1
tr2_entity_tbl[179] = {coll = 0x0000};               -- Puzzle 2
tr2_entity_tbl[180] = {coll = 0x0000};               -- Puzzle 3 ?
tr2_entity_tbl[181] = {coll = 0x0000};               -- Puzzle 4
tr2_entity_tbl[182] = {coll = 0x0000};               -- Slot 1 empty
tr2_entity_tbl[183] = {coll = 0x0000};               -- Slot 2 empty
tr2_entity_tbl[184] = {coll = 0x0000};               -- Slot 3 empty ?
tr2_entity_tbl[185] = {coll = 0x0000};               -- Slot 4 empty
tr2_entity_tbl[186] = {coll = 0x0000};               -- Slot 1 full
tr2_entity_tbl[187] = {coll = 0x0000};               -- Slot 2 full
tr2_entity_tbl[188] = {coll = 0x0000};               -- Slot 3 full ?
tr2_entity_tbl[189] = {coll = 0x0000};               -- Slot 4 full
tr2_entity_tbl[197] = {coll = 0x0000};               -- Key 1
tr2_entity_tbl[198] = {coll = 0x0000};               -- Key 2
tr2_entity_tbl[199] = {coll = 0x0000};               -- Key 3
tr2_entity_tbl[200] = {coll = 0x0000};               -- Key 4
tr2_entity_tbl[201] = {coll = 0x0000};               -- Lock 1
tr2_entity_tbl[202] = {coll = 0x0000};               -- Lock 2
tr2_entity_tbl[203] = {coll = 0x0000};               -- Lock 3
tr2_entity_tbl[204] = {coll = 0x0000};               -- Lock 4
tr2_entity_tbl[207] = {coll = 0x0000};               -- Pickup 5
tr2_entity_tbl[208] = {coll = 0x0000};               -- Pickup 6
tr2_entity_tbl[209] = {coll = 0x0000};               -- Dragon explosion effect (expanding netted bubble)
tr2_entity_tbl[210] = {coll = 0x0000};               -- Dragon explosion effect (expanding netted bubble)
tr2_entity_tbl[211] = {coll = 0x0000};               -- Dragon explosion effect (expanding solid bubble)
tr2_entity_tbl[212] = {coll = 0x0000};               -- Alarm
tr2_entity_tbl[213] = {coll = 0x0000, hide = 1};     -- Placeholder
tr2_entity_tbl[214] = {coll = 0x0002};               -- Tyrannosaur
tr2_entity_tbl[215] = {coll = 0x0000, hide = 1};     -- Singing birds
tr2_entity_tbl[216] = {coll = 0x0000, hide = 1};     -- Placeholder
tr2_entity_tbl[217] = {coll = 0x0000, hide = 1};     -- Placeholder
tr2_entity_tbl[218] = {coll = 0x0002};               -- Dragon bones (front)
tr2_entity_tbl[219] = {coll = 0x0002};               -- Dragon bones (back)
tr2_entity_tbl[222] = {coll = 0x0002};               -- Aquatic Mine (Venice)
tr2_entity_tbl[223] = {coll = 0x0000};               -- Menu background
tr2_entity_tbl[225] = {coll = 0x0000};               -- Gong-hammering animation
tr2_entity_tbl[226] = {coll = 0x0000};               -- Gong (Ice Palace)
tr2_entity_tbl[227] = {coll = 0x0000};               -- Detonator box
tr2_entity_tbl[228] = {coll = 0x0000};               -- Helicopter (Diving Area)
tr2_entity_tbl[235] = {coll = 0x0000};               -- Flare burning?
tr2_entity_tbl[240] = {coll = 0x0000};               -- Gunflare
tr2_entity_tbl[241] = {coll = 0x0000};               -- Gunflare (M16)
tr2_entity_tbl[243] = {coll = 0x0000, hide = 1};     -- Camera target
tr2_entity_tbl[244] = {coll = 0x0000, hide = 1};     -- Camera target - 2 (?)
tr2_entity_tbl[245] = {coll = 0x0001};               -- Harpoon (single)
tr2_entity_tbl[247] = {coll = 0x0001};               -- Pointer?
tr2_entity_tbl[248] = {coll = 0x0001};               -- Grenade (single)
tr2_entity_tbl[249] = {coll = 0x0001};               -- Harpoon (flying)
tr2_entity_tbl[251] = {coll = 0x0000, hide = 1};     -- Sparks
tr2_entity_tbl[253] = {coll = 0x0000, hide = 1};     -- Fire
tr2_entity_tbl[254] = {coll = 0x0000};               -- Skybox
tr2_entity_tbl[256] = {coll = 0x0001};               -- Monk
tr2_entity_tbl[257] = {coll = 0x0000, hide = 1}      -- Door bell
tr2_entity_tbl[258] = {coll = 0x0000, hide = 1}      -- Alarm bell
tr2_entity_tbl[259] = {coll = 0x0001};               -- Helicopter
tr2_entity_tbl[260] = {coll = 0x0002};               -- The butler
tr2_entity_tbl[262] = {coll = 0x0000, hide = 1};     -- Lara cutscene placement?
tr2_entity_tbl[263] = {coll = 0x0000};               -- Shotgun animation (Home Sweet Home)
tr2_entity_tbl[264] = {coll = 0x0000, hide = 1};     -- Dragon transform wave


--------------------------------------------------------------------------------
-------------------------------------- TR_III ----------------------------------
--------------------------------------------------------------------------------
tr3_entity_tbl = {};

-- NOTE: Objects before ID 12 are internal service objects and never appear in-game.

-- VEHICLES --

tr3_entity_tbl[12] = {coll = 0x0001};               -- UPV
tr3_entity_tbl[14] = {coll = 0x0001};               -- Kayak
tr3_entity_tbl[15] = {coll = 0x0001};               -- Inflatable boat
tr3_entity_tbl[16] = {coll = 0x0001};               -- Quadbike
tr3_entity_tbl[17] = {coll = 0x0001};               -- Mine cart
tr3_entity_tbl[18] = {coll = 0x0001};               -- Big gun

-- ACTORS --

tr3_entity_tbl[19] = {coll = 0x0002};               -- Hydro propeller (?)
tr3_entity_tbl[20] = {coll = 0x0002};               -- Tribesman with spiked axe
tr3_entity_tbl[21] = {coll = 0x0002};               -- Tribesman with poison-dart gun
tr3_entity_tbl[22] = {coll = 0x0002};               -- Dog
tr3_entity_tbl[23] = {coll = 0x0002};               -- Rat
tr3_entity_tbl[24] = {coll = 0x0002, hide = 1};     -- Kill All Triggers
tr3_entity_tbl[25] = {coll = 0x0002};               -- Killer whale
tr3_entity_tbl[26] = {coll = 0x0002};               -- Scuba diver
tr3_entity_tbl[27] = {coll = 0x0002};               -- Crow
tr3_entity_tbl[28] = {coll = 0x0002};               -- Tiger
tr3_entity_tbl[29] = {coll = 0x0002};               -- Vulture
tr3_entity_tbl[30] = {coll = 0x0001};               -- Assault-course target
tr3_entity_tbl[31] = {coll = 0x0002};               -- Crawler mutant in closet
tr3_entity_tbl[32] = {coll = 0x0002};               -- Crocodile (in water)
tr3_entity_tbl[34] = {coll = 0x0002};               -- Compsognathus
tr3_entity_tbl[35] = {coll = 0x0002};               -- Lizard thing
tr3_entity_tbl[36] = {coll = 0x0002};               -- Puna guy
tr3_entity_tbl[37] = {coll = 0x0002};               -- Mercenary
tr3_entity_tbl[38] = {coll = 0x0001};               -- Raptor hung by rope (fish bait)
tr3_entity_tbl[39] = {coll = 0x0002};               -- RX-Tech guy in red jacket
tr3_entity_tbl[40] = {coll = 0x0002};               -- RX-Tech guy with gun (dressed like flamethrower guy)
tr3_entity_tbl[41] = {coll = 0x0002};               -- Dog (Antarctica)
tr3_entity_tbl[42] = {coll = 0x0002};               -- Crawler mutant
tr3_entity_tbl[44] = {coll = 0x0002};               -- Tinnos wasp
tr3_entity_tbl[45] = {coll = 0x0002};               -- Tinnos monster
tr3_entity_tbl[46] = {coll = 0x0002};               -- Brute mutant (with claw)
tr3_entity_tbl[47] = {coll = 0x0002};               -- Tinnos wasp respawn point
tr3_entity_tbl[48] = {coll = 0x0002};               -- Raptor respawn point
tr3_entity_tbl[49] = {coll = 0x0002};               -- Willard spider
tr3_entity_tbl[50] = {coll = 0x0002};               -- RX-Tech flamethrower guy
tr3_entity_tbl[51] = {coll = 0x0002};               -- London goon
tr3_entity_tbl[53] = {coll = 0x0002};               -- 'Damned' stick-wielding goon
tr3_entity_tbl[56] = {coll = 0x0002};               -- London guard
tr3_entity_tbl[57] = {coll = 0x0002};               -- Sophia Lee
tr3_entity_tbl[58] = {coll = 0x0001};               -- Thames Wharf machine
tr3_entity_tbl[60] = {coll = 0x0002};               -- MP with stick
tr3_entity_tbl[61] = {coll = 0x0002};               -- MP with gun
tr3_entity_tbl[62] = {coll = 0x0002};               -- Prisoner
tr3_entity_tbl[63] = {coll = 0x0002};               -- MP with sighted gun and night sight
tr3_entity_tbl[64] = {coll = 0x0002};               -- Gun turret
tr3_entity_tbl[65] = {coll = 0x0002};               -- Dam guard
tr3_entity_tbl[66] = {coll = 0x0002, hide = 1};     -- Kind of tripwire
tr3_entity_tbl[67] = {coll = 0x0002, hide = 1};     -- Electrified wire
tr3_entity_tbl[68] = {coll = 0x0002, hide = 1};     -- Killer tripwire
tr3_entity_tbl[69] = {coll = 0x0002};               -- Cobra / Rattlesnake
tr3_entity_tbl[70] = {coll = 0x0001};               -- Temple statue
tr3_entity_tbl[71] = {coll = 0x0002};               -- Monkey
tr3_entity_tbl[73] = {coll = 0x0002};               -- Tony Firehands

-- AI OBJECTS --

tr3_entity_tbl[74] = {coll = 0x0000, hide = 1};  -- AI Guard
tr3_entity_tbl[75] = {coll = 0x0000, hide = 1};  -- AI Ambush
tr3_entity_tbl[76] = {coll = 0x0000, hide = 1};  -- AI Path
tr3_entity_tbl[77] = {coll = 0x0000, hide = 1};  -- AI Unknown #77
tr3_entity_tbl[78] = {coll = 0x0000, hide = 1};  -- AI Follow
tr3_entity_tbl[79] = {coll = 0x0000, hide = 1};  -- AI Patrol
tr3_entity_tbl[80] = {coll = 0x0000, hide = 1};  -- Unknown Id #80
tr3_entity_tbl[81] = {coll = 0x0000, hide = 1};  -- Unknown Id #81
tr3_entity_tbl[82] = {coll = 0x0000, hide = 1};  -- Unknown Id #82

-- TRAPS & DOORS --

tr3_entity_tbl[83] = {coll = 0x0001};            -- Collapsible floor
tr3_entity_tbl[86] = {coll = 0x0001};            -- Swinging thing
tr3_entity_tbl[87] = {coll = 0x0001};            -- Spikes / Barbed wire
tr3_entity_tbl[88] = {coll = 0x0001};            -- Boulder / Barrel
tr3_entity_tbl[89] = {coll = 0x0001};            -- Giant boulder (Temple of Puna)
tr3_entity_tbl[90] = {coll = 0x0001};            -- Disk (like dart)
tr3_entity_tbl[91] = {coll = 0x0001};            -- Dart shooter
tr3_entity_tbl[94] = {coll = 0x0001};            -- Spiked impaled skeleton / Slamming door
tr3_entity_tbl[97] = {coll = 0x0001};            -- Movable cubical block (pushable)
tr3_entity_tbl[98] = {coll = 0x0001};            -- Movable cubical block (pushable)
tr3_entity_tbl[101] = {coll = 0x0001};           -- Destroyable boarded-up window
tr3_entity_tbl[102] = {coll = 0x0001};           -- Destroyable boarded-up window / wall
tr3_entity_tbl[106] = {coll = 0x0001};           -- Overhead pulley hook
tr3_entity_tbl[107] = {coll = 0x0001};           -- Falling fragments
tr3_entity_tbl[108] = {coll = 0x0001};           -- Rolling spindle
tr3_entity_tbl[110] = {coll = 0x0001};           -- Subway train
tr3_entity_tbl[111] = {coll = 0x0001};           -- Wall-mounted knife blade / Knife disk
tr3_entity_tbl[113] = {coll = 0x0001};           -- Detachable stalactites
tr3_entity_tbl[114] = {coll = 0x0001};           -- Spiky movable wall
tr3_entity_tbl[116] = {coll = 0x0001};           -- Spiky movable vertical wall / Tunnel borer
tr3_entity_tbl[117] = {coll = 0x0001};           -- Valve wheel / Pulley
tr3_entity_tbl[118] = {coll = 0x0001};           -- Switch
tr3_entity_tbl[119] = {coll = 0x0001};           -- Underwater propeller / Diver sitting on block / Underwater rotating knives / Meteorite
tr3_entity_tbl[120] = {coll = 0x0001};           -- Fan
tr3_entity_tbl[121] = {coll = 0x0001};           -- Heavy stamper / Grinding drum / Underwater rotating knives
tr3_entity_tbl[122] = {coll = 0x0001};           -- Temple statue (original petrified state)
tr3_entity_tbl[123] = {coll = 0x0002};           -- Monkey with medipack
tr3_entity_tbl[124] = {coll = 0x0002};           -- Monkey with key
tr3_entity_tbl[127] = {coll = 0x0002};           -- Zipline handle
tr3_entity_tbl[128] = {coll = 0x0001};           -- Switch
tr3_entity_tbl[129] = {coll = 0x0001};           -- Switch
tr3_entity_tbl[130] = {coll = 0x0001};           -- Underwater switch
tr3_entity_tbl[131] = {coll = 0x0001};           -- Door
tr3_entity_tbl[132] = {coll = 0x0001};           -- Door
tr3_entity_tbl[133] = {coll = 0x0001};           -- Door
tr3_entity_tbl[134] = {coll = 0x0001};           -- Door
tr3_entity_tbl[135] = {coll = 0x0001};           -- Door
tr3_entity_tbl[136] = {coll = 0x0001};           -- Door
tr3_entity_tbl[137] = {coll = 0x0001};           -- Door
tr3_entity_tbl[138] = {coll = 0x0001};           -- Door
tr3_entity_tbl[139] = {coll = 0x0001};           -- Trapdoor
tr3_entity_tbl[140] = {coll = 0x0001};           -- Trapdoor
tr3_entity_tbl[141] = {coll = 0x0001};           -- Trapdoor
tr3_entity_tbl[142] = {coll = 0x0001};           -- Bridge (flat)
tr3_entity_tbl[143] = {coll = 0x0001};           -- Bridge (slope = 1)
tr3_entity_tbl[144] = {coll = 0x0001};           -- Bridge (slope = 2)

-- PICK-UPS --

tr3_entity_tbl[145] = {coll = 0x0000};           -- Passport (opening up)
tr3_entity_tbl[146] = {coll = 0x0000};           -- Stopwatch
tr3_entity_tbl[147] = {coll = 0x0000};           -- Lara's Home photo
tr3_entity_tbl[158] = {coll = 0x0000};           -- Passport (closed)
tr3_entity_tbl[159] = {coll = 0x0000};           -- Natla logo
tr3_entity_tbl[160] = {coll = 0x0000};           -- Pistols (pick-up)
tr3_entity_tbl[161] = {coll = 0x0000};           -- Shotgun (pick-up)
tr3_entity_tbl[162] = {coll = 0x0000};           -- Desert Eagle (pick-up)
tr3_entity_tbl[163] = {coll = 0x0000};           -- Uzis (pick-up)
tr3_entity_tbl[164] = {coll = 0x0000};           -- Harpoon gun (pick-up)
tr3_entity_tbl[165] = {coll = 0x0000};           -- MP5 (pick-up)
tr3_entity_tbl[166] = {coll = 0x0000};           -- Rocket launcher (pick-up)
tr3_entity_tbl[167] = {coll = 0x0000};           -- Grenade launcher (pick-up)
tr3_entity_tbl[168] = {coll = 0x0000};           -- Pistol ammo (pick-up)
tr3_entity_tbl[169] = {coll = 0x0000};           -- Shotgun ammo (pick-up)
tr3_entity_tbl[170] = {coll = 0x0000};           -- Desert Eagle ammo (pick-up)
tr3_entity_tbl[171] = {coll = 0x0000};           -- Uzi ammo (pick-up)
tr3_entity_tbl[172] = {coll = 0x0000};           -- Harpoons (pick-up)
tr3_entity_tbl[173] = {coll = 0x0000};           -- MP5 ammo (pick-up)
tr3_entity_tbl[174] = {coll = 0x0000};           -- Rockets (pick-up)
tr3_entity_tbl[175] = {coll = 0x0000};           -- Grenades (pick-up)
tr3_entity_tbl[176] = {coll = 0x0000};           -- Small medipack (pick-up)
tr3_entity_tbl[177] = {coll = 0x0000};           -- Large medipack (pick-up)
tr3_entity_tbl[178] = {coll = 0x0000};           -- Flares (pick-up)
tr3_entity_tbl[179] = {coll = 0x0000};           -- Flare (pick-up)
tr3_entity_tbl[180] = {coll = 0x0000};           -- Savegame crystal (pick-up)

-- MENU ITEMS --

tr3_entity_tbl[181] = {coll = 0x0000};           -- Sunglasses
tr3_entity_tbl[182] = {coll = 0x0000};           -- Portable CD Player
tr3_entity_tbl[183] = {coll = 0x0000};           -- Direction keys
tr3_entity_tbl[184] = {coll = 0x0000};           -- Globe (for indicating destinations)
tr3_entity_tbl[185] = {coll = 0x0000};           -- Pistols
tr3_entity_tbl[186] = {coll = 0x0000};           -- Shotgun
tr3_entity_tbl[187] = {coll = 0x0000};           -- Desert Eagle
tr3_entity_tbl[188] = {coll = 0x0000};           -- Uzis
tr3_entity_tbl[189] = {coll = 0x0000};           -- Harpoon gun
tr3_entity_tbl[190] = {coll = 0x0000};           -- MP5
tr3_entity_tbl[191] = {coll = 0x0000};           -- Rocket launcher
tr3_entity_tbl[192] = {coll = 0x0000};           -- Grenade launcher
tr3_entity_tbl[193] = {coll = 0x0000};           -- Pistol ammo
tr3_entity_tbl[194] = {coll = 0x0000};           -- Shotgun ammo
tr3_entity_tbl[195] = {coll = 0x0000};           -- Desert Eagle ammo
tr3_entity_tbl[196] = {coll = 0x0000};           -- Uzi ammo
tr3_entity_tbl[197] = {coll = 0x0000};           -- Harpoons
tr3_entity_tbl[198] = {coll = 0x0000};           -- MP5 ammo
tr3_entity_tbl[199] = {coll = 0x0000};           -- Rockets
tr3_entity_tbl[200] = {coll = 0x0000};           -- Grenades
tr3_entity_tbl[201] = {coll = 0x0000};           -- Small medipack
tr3_entity_tbl[202] = {coll = 0x0000};           -- Large medipack
tr3_entity_tbl[203] = {coll = 0x0000};           -- Flares
tr3_entity_tbl[204] = {coll = 0x0000};           -- Savegame crystal

-- PUZZLE ITEMS & KEYS --

tr3_entity_tbl[205] = {coll = 0x0000};           -- Puzzle 1 (pick-up)
tr3_entity_tbl[206] = {coll = 0x0000};           -- Puzzle 2 (pick-up)
tr3_entity_tbl[207] = {coll = 0x0000};           -- Puzzle 3 (pick-up)
tr3_entity_tbl[208] = {coll = 0x0000};           -- Puzzle 4 (pick-up)
tr3_entity_tbl[209] = {coll = 0x0000};           -- Puzzle 1 (menu item)
tr3_entity_tbl[210] = {coll = 0x0000};           -- Puzzle 2 (menu item)
tr3_entity_tbl[211] = {coll = 0x0000};           -- Puzzle 3 (menu item)
tr3_entity_tbl[212] = {coll = 0x0000};           -- Puzzle 4 (menu item)
tr3_entity_tbl[213] = {coll = 0x0002};           -- Slot 1 empty
tr3_entity_tbl[214] = {coll = 0x0002};           -- Slot 2 empty
tr3_entity_tbl[215] = {coll = 0x0002};           -- Slot 3 empty
tr3_entity_tbl[216] = {coll = 0x0002};           -- Slot 4 empty
tr3_entity_tbl[217] = {coll = 0x0002};           -- Slot 1 full
tr3_entity_tbl[218] = {coll = 0x0002};           -- Slot 2 full
tr3_entity_tbl[219] = {coll = 0x0002};           -- Slot 3 full
tr3_entity_tbl[220] = {coll = 0x0002};           -- Slot 4 full
tr3_entity_tbl[224] = {coll = 0x0000};           -- Key 1 (pick-up)
tr3_entity_tbl[225] = {coll = 0x0000};           -- Key 2 (pick-up)
tr3_entity_tbl[226] = {coll = 0x0000};           -- Key 3 (pick-up)
tr3_entity_tbl[227] = {coll = 0x0000};           -- Key 4 (pick-up)
tr3_entity_tbl[228] = {coll = 0x0000};           -- Key 1 (menu item)
tr3_entity_tbl[229] = {coll = 0x0000};           -- Key 2 (menu item)
tr3_entity_tbl[230] = {coll = 0x0000};           -- Key 3 (menu item)
tr3_entity_tbl[231] = {coll = 0x0000};           -- Key 4 (menu item)
tr3_entity_tbl[232] = {coll = 0x0002};           -- Lock 1
tr3_entity_tbl[233] = {coll = 0x0002};           -- Lock 2
tr3_entity_tbl[234] = {coll = 0x0002};           -- Lock 3
tr3_entity_tbl[235] = {coll = 0x0002};           -- Lock 4
tr3_entity_tbl[236] = {coll = 0x0000};           -- Pickup 1 (pick-up)
tr3_entity_tbl[237] = {coll = 0x0000};           -- Pickup 2 .unused] (pick-up)
tr3_entity_tbl[238] = {coll = 0x0000};           -- Pickup 1 (menu item)
tr3_entity_tbl[239] = {coll = 0x0000};           -- Pickup 2 .unused] (menu item)
tr3_entity_tbl[240] = {coll = 0x0000};           -- Infada stone (pick-up)
tr3_entity_tbl[241] = {coll = 0x0000};           -- Element 115 (pick-up)
tr3_entity_tbl[242] = {coll = 0x0000};           -- Eye of Isis (pick-up)
tr3_entity_tbl[243] = {coll = 0x0000};           -- Ora dagger (pick-up)
tr3_entity_tbl[244] = {coll = 0x0000};           -- Infada stone (menu item)
tr3_entity_tbl[245] = {coll = 0x0000};           -- Element 115 (menu item)
tr3_entity_tbl[246] = {coll = 0x0000};           -- Eye of Isis (menu item)
tr3_entity_tbl[247] = {coll = 0x0000};           -- Ora dagger (menu item)
tr3_entity_tbl[272] = {coll = 0x0000};           -- Keys (sprite)
tr3_entity_tbl[273] = {coll = 0x0000};           -- Keys (sprite)
tr3_entity_tbl[276] = {coll = 0x0000};           -- Infada stone
tr3_entity_tbl[277] = {coll = 0x0000};           -- Element 115
tr3_entity_tbl[278] = {coll = 0x0000};           -- Eye of Isis
tr3_entity_tbl[279] = {coll = 0x0000};           -- Ora dagger

tr3_entity_tbl[282] = {coll = 0x0000};           -- Fire-breathing dragon statue
tr3_entity_tbl[285] = {coll = 0x0000};           -- Unknown visible #285

-- ENEMIES, cont. --

tr3_entity_tbl[287] = {coll = 0x0001};           -- Tyrannosaur
tr3_entity_tbl[288] = {coll = 0x0002};           -- Raptor

-- TRAPS, cont. --

tr3_entity_tbl[291] = {coll = 0x0001};           -- Laser sweeper
tr3_entity_tbl[292] = {coll = 0x0000, hide = 1}; -- Electrified Field
tr3_entity_tbl[295] = {coll = 0x0001};           -- Detonator switch box

-- SERVICE ITEMS --

tr3_entity_tbl[300] = {coll = 0x0000};           -- Gunflare / Gunflare (spiky)
tr3_entity_tbl[301] = {coll = 0x0000};           -- Spiky gunflare for MP5
tr3_entity_tbl[304] = {coll = 0x0000, hide = 1}; -- Look At item
tr3_entity_tbl[305] = {coll = 0x0000, hide = 1}; -- Smoke Edge
tr3_entity_tbl[306] = {coll = 0x0001};           -- Harpoon (single)
tr3_entity_tbl[309] = {coll = 0x0001};           -- Rocket (single)
tr3_entity_tbl[310] = {coll = 0x0001};           -- Harpoon (single)
tr3_entity_tbl[311] = {coll = 0x0001};           -- Grenade (single)
tr3_entity_tbl[312] = {coll = 0x0001};           -- Big missile
tr3_entity_tbl[313] = {coll = 0x0000, hide = 1}; -- Smoke
tr3_entity_tbl[314] = {coll = 0x0000, hide = 1}; -- Movable Boom
tr3_entity_tbl[315] = {coll = 0x0000, hide = 1}; -- Lara true appearance
tr3_entity_tbl[317] = {coll = 0x0000};           -- Unknown visible #317
tr3_entity_tbl[318] = {coll = 0x0000, hide = 1}; -- Red ceiling rotating(?) light
tr3_entity_tbl[319] = {coll = 0x0000, hide = 1}; -- Light
tr3_entity_tbl[321] = {coll = 0x0000, hide = 1}; -- Light #2
tr3_entity_tbl[322] = {coll = 0x0000, hide = 1}; -- Pulsating Light
tr3_entity_tbl[324] = {coll = 0x0000, hide = 1}; -- Red Light
tr3_entity_tbl[325] = {coll = 0x0000, hide = 1}; -- Green Light
tr3_entity_tbl[326] = {coll = 0x0000, hide = 1}; -- Blue Light
tr3_entity_tbl[327] = {coll = 0x0000, hide = 1}; -- Light #3
tr3_entity_tbl[328] = {coll = 0x0000, hide = 1}; -- Light #4
tr3_entity_tbl[330] = {coll = 0x0000, hide = 1}; -- Fire
tr3_entity_tbl[331] = {coll = 0x0000, hide = 1}; -- Alternate Fire
tr3_entity_tbl[332] = {coll = 0x0000, hide = 1}; -- Alternate Fire #2
tr3_entity_tbl[333] = {coll = 0x0000, hide = 1}; -- Fire #2
tr3_entity_tbl[334] = {coll = 0x0000, hide = 1}; -- Smoke #2
tr3_entity_tbl[335] = {coll = 0x0000, hide = 1}; -- Smoke #3
tr3_entity_tbl[336] = {coll = 0x0000, hide = 1}; -- Smoke #4
tr3_entity_tbl[337] = {coll = 0x0000, hide = 1}; -- Greenish Smoke
tr3_entity_tbl[338] = {coll = 0x0000, hide = 1}; -- Pirahnas
tr3_entity_tbl[339] = {coll = 0x0000, hide = 1}; -- Fish
tr3_entity_tbl[347] = {coll = 0x0000, hide = 1}; -- Bat swarm

-- ANIMATINGS --

tr3_entity_tbl[349] = {coll = 0x0001};           -- Misc item (Animating 1)
tr3_entity_tbl[350] = {coll = 0x0001};           -- Misc item (Animating 2)
tr3_entity_tbl[351] = {coll = 0x0001};           -- Misc item (Animating 3)
tr3_entity_tbl[352] = {coll = 0x0001};           -- Misc item (Animating 4)
tr3_entity_tbl[353] = {coll = 0x0001};           -- Footstool / Fish swimming in tank / Radar display
tr3_entity_tbl[354] = {coll = 0x0001};           -- Dead raptor / Alarm box / Mason-lodge dagger / Small version of big antenna


tr3_entity_tbl[355] = {coll = 0x0000};           -- Skybox
tr3_entity_tbl[357] = {coll = 0x0000, hide = 1}; -- Unknown id #357
tr3_entity_tbl[358] = {coll = 0x0000, hide = 1}; -- Unknown id #358


tr3_entity_tbl[360] = {coll = 0x0001};           -- The butler
tr3_entity_tbl[361] = {coll = 0x0001};           -- The butler in military outfit and target


tr3_entity_tbl[365] = {coll = 0x0000};           -- Earthquake
tr3_entity_tbl[366] = {coll = 0x0000};           -- Yellow shell casing
tr3_entity_tbl[367] = {coll = 0x0000};           -- Red shell casing
tr3_entity_tbl[370] = {coll = 0x0000};           -- Tinnos light shaft
tr3_entity_tbl[373] = {coll = 0x0002};           -- Electrical switch box

--------------------------------------------------------------------------------
--------------------------------- TR_IV, TR_IV_DEMO ----------------------------
--------------------------------------------------------------------------------
tr4_entity_tbl = {};

-- Remark: object IDs 0-30 are used for Lara model and speechheads, and never
-- show in game independently.

-- VEHICLES

tr4_entity_tbl[031] = {coll = 0x0001}; -- Bike
tr4_entity_tbl[032] = {coll = 0x0001}; -- Jeep
tr4_entity_tbl[034] = {coll = 0x0001}; -- Enemy jeep

-- ENEMIES

tr4_entity_tbl[035] = {coll = 0x0002}; -- Skeleton
tr4_entity_tbl[036] = {coll = 0x0002}; -- Skeleton MIP - UNUSED
tr4_entity_tbl[037] = {coll = 0x0002}; -- Guide
tr4_entity_tbl[038] = {coll = 0x0002}; -- Guide MIP - UNUSED
tr4_entity_tbl[039] = {coll = 0x0002}; -- Von Croy
tr4_entity_tbl[040] = {coll = 0x0002}; -- Guide MIP - UNUSED
tr4_entity_tbl[041] = {coll = 0x0002}; -- Baddy 1
tr4_entity_tbl[042] = {coll = 0x0002}; -- Baddy 1 MIP - UNUSED
tr4_entity_tbl[043] = {coll = 0x0002}; -- Baddy 2
tr4_entity_tbl[044] = {coll = 0x0002}; -- Baddy 2 MIP - UNUSED
tr4_entity_tbl[045] = {coll = 0x0002}; -- Setha
tr4_entity_tbl[046] = {coll = 0x0002}; -- Setha MIP - UNUSED
tr4_entity_tbl[047] = {coll = 0x0002}; -- Mummy
tr4_entity_tbl[048] = {coll = 0x0002}; -- Mummy MIP - UNUSED
tr4_entity_tbl[049] = {coll = 0x0002}; -- Sphinx / Bull
tr4_entity_tbl[050] = {coll = 0x0002}; -- Sphinx / Bull MIP - UNUSED
tr4_entity_tbl[051] = {coll = 0x0002}; -- Crocodile
tr4_entity_tbl[052] = {coll = 0x0002}; -- Crocodile MIP - UNUSED
tr4_entity_tbl[053] = {coll = 0x0002}; -- Horseman
tr4_entity_tbl[054] = {coll = 0x0002}; -- Horseman MIP - UNUSED
tr4_entity_tbl[055] = {coll = 0x0002}; -- Scorpion
tr4_entity_tbl[056] = {coll = 0x0002}; -- Scorpion MIP - UNUSED
tr4_entity_tbl[057] = {coll = 0x0002}; -- Jean-Yves
tr4_entity_tbl[058] = {coll = 0x0002}; -- Jean-Yves MIP - UNUSED
tr4_entity_tbl[059] = {coll = 0x0002}; -- Troops
tr4_entity_tbl[060] = {coll = 0x0002}; -- Troops MIP - UNUSED
tr4_entity_tbl[061] = {coll = 0x0002}; -- Knights Templar
tr4_entity_tbl[062] = {coll = 0x0002}; -- Knights Templar MIP - UNUSED
tr4_entity_tbl[063] = {coll = 0x0002}; -- Mutant
tr4_entity_tbl[064] = {coll = 0x0002}; -- Mutant MIP - UNUSED
tr4_entity_tbl[065] = {coll = 0x0002}; -- Horse
tr4_entity_tbl[066] = {coll = 0x0002}; -- Horse MIP - UNUSED
tr4_entity_tbl[067] = {coll = 0x0002}; -- Baboon normal
tr4_entity_tbl[068] = {coll = 0x0002}; -- Baboon normal MIP - UNUSED
tr4_entity_tbl[069] = {coll = 0x0002}; -- Baboon invisible
tr4_entity_tbl[070] = {coll = 0x0002}; -- Baboon invisible MIP - UNUSED
tr4_entity_tbl[071] = {coll = 0x0002}; -- Baboon silent
tr4_entity_tbl[072] = {coll = 0x0002}; -- Baboon silent MIP - UNUSED
tr4_entity_tbl[073] = {coll = 0x0002}; -- Wild boar
tr4_entity_tbl[074] = {coll = 0x0002}; -- Wild boar MIP - UNUSED
tr4_entity_tbl[075] = {coll = 0x0002}; -- Harpy
tr4_entity_tbl[076] = {coll = 0x0002}; -- Harpy MIP - UNUSED
tr4_entity_tbl[077] = {coll = 0x0002}; -- Demigod 1
tr4_entity_tbl[078] = {coll = 0x0002}; -- Demigod 1 MIP - UNUSED
tr4_entity_tbl[079] = {coll = 0x0002}; -- Demigod 2
tr4_entity_tbl[080] = {coll = 0x0002}; -- Demigod 2 MIP - UNUSED
tr4_entity_tbl[081] = {coll = 0x0002}; -- Demigod 3
tr4_entity_tbl[082] = {coll = 0x0002}; -- Demigod 3 MIP - UNUSED
tr4_entity_tbl[083] = {coll = 0x0000}; -- Little beetle - NO COLLISION, SWARM
tr4_entity_tbl[084] = {coll = 0x0002}; -- Big beetle
tr4_entity_tbl[085] = {coll = 0x0002}; -- Big beetle MIP - UNUSED
tr4_entity_tbl[086] = {coll = 0x0000}; -- Wraith 1 - SPIRIT, NO COLLISION
tr4_entity_tbl[087] = {coll = 0x0000}; -- Wraith 2 - SPIRIT, NO COLLISION
tr4_entity_tbl[088] = {coll = 0x0000}; -- Wraith 3 - SPIRIT, NO COLLISION
tr4_entity_tbl[089] = {coll = 0x0000}; -- Wraith 4 - SPIRIT, NO COLLISION
tr4_entity_tbl[090] = {coll = 0x0000}; -- Bat - TOO SMALL TO COLLIDE WITH
tr4_entity_tbl[091] = {coll = 0x0002}; -- Dog
tr4_entity_tbl[092] = {coll = 0x0002}; -- Dog MIP - UNUSED
tr4_entity_tbl[093] = {coll = 0x0002}; -- Hammerhead
tr4_entity_tbl[094] = {coll = 0x0002}; -- Hammerhead MIP - UNUSED
tr4_entity_tbl[095] = {coll = 0x0002}; -- SAS
tr4_entity_tbl[096] = {coll = 0x0002}; -- SAS MIP - UNUSED
tr4_entity_tbl[097] = {coll = 0x0002}; -- SAS dying
tr4_entity_tbl[098] = {coll = 0x0002}; -- SAS dying MIP - UNUSED
tr4_entity_tbl[099] = {coll = 0x0002}; -- SAS Captain
tr4_entity_tbl[100] = {coll = 0x0002}; -- SAS Captain MIP - UNUSED
tr4_entity_tbl[101] = {coll = 0x0001}; -- SAS Drag bloke
tr4_entity_tbl[102] = {coll = 0x0002}; -- Ahmet
tr4_entity_tbl[103] = {coll = 0x0002}; -- Ahmet MIP - UNUSED
tr4_entity_tbl[104] = {coll = 0x0001}; -- Lara double
tr4_entity_tbl[105] = {coll = 0x0001}; -- Lara double MIP - UNUSED
tr4_entity_tbl[106] = {coll = 0x0002}; -- Small scorpion
tr4_entity_tbl[107] = {coll = 0x0000}; -- Locust (ex-Fish) - NO COLLISION, SWARM

-- PUZZLE ACTION ITEMS

tr4_entity_tbl[108] = {coll = 0x0001}; -- Game piece 1
tr4_entity_tbl[109] = {coll = 0x0001}; -- Game piece 2
tr4_entity_tbl[110] = {coll = 0x0001}; -- Game piece 3
tr4_entity_tbl[111] = {coll = 0x0001}; -- Enemy piece
tr4_entity_tbl[112] = {coll = 0x0002}; -- Wheel of fortune
tr4_entity_tbl[113] = {coll = 0x0002}; -- Scales

-- DART EMITTER

tr4_entity_tbl[114] = {coll = 0x0000, hide = 1}; -- Darts - SPAWNED OBJECT
tr4_entity_tbl[115] = {coll = 0x0000, hide = 1}; -- Dart emitter
tr4_entity_tbl[116] = {coll = 0x0000, hide = 1}; -- Homing dart emitter  - UNUSED

-- DESTROYABLE / MOVABLE TERRAIN

tr4_entity_tbl[117] = {coll = 0x0001}; -- Falling ceiling
tr4_entity_tbl[118] = {coll = 0x0001}; -- Falling block
tr4_entity_tbl[119] = {coll = 0x0001}; -- Falling block2
tr4_entity_tbl[120] = {coll = 0x0001}; -- Smashable bike wall
tr4_entity_tbl[121] = {coll = 0x0001}; -- Smashable bike floor
tr4_entity_tbl[122] = {coll = 0x0001}; -- Trapdoor 1
tr4_entity_tbl[123] = {coll = 0x0001}; -- Trapdoor 2
tr4_entity_tbl[124] = {coll = 0x0001}; -- Trapdoor 3
tr4_entity_tbl[125] = {coll = 0x0001}; -- Floor trapdoor 1
tr4_entity_tbl[126] = {coll = 0x0001}; -- Floor trapdoor 2
tr4_entity_tbl[127] = {coll = 0x0001}; -- Ceiling trapdoor 1
tr4_entity_tbl[128] = {coll = 0x0001}; -- Ceiling trapdoor 2
tr4_entity_tbl[129] = {coll = 0x0001}; -- Scaling trapdoor

-- TRAPS & INTERACTION OBJECTS

tr4_entity_tbl[130] = {coll = 0x0001}; -- Rolling ball
tr4_entity_tbl[131] = {coll = 0x0000}; -- Spikey floor - UNUSED?
tr4_entity_tbl[132] = {coll = 0x0000}; -- Teeth spikes
tr4_entity_tbl[133] = {coll = 0x0000}; -- Joby spikes
tr4_entity_tbl[134] = {coll = 0x0000}; -- Slicer dicer
tr4_entity_tbl[135] = {coll = 0x0001}; -- Chain
tr4_entity_tbl[136] = {coll = 0x0001}; -- Plough
tr4_entity_tbl[137] = {coll = 0x0001}; -- Stargate
tr4_entity_tbl[138] = {coll = 0x0000}; -- Hammer
tr4_entity_tbl[139] = {coll = 0x0001}; -- Burning floor
tr4_entity_tbl[140] = {coll = 0x0000}; -- Cog
tr4_entity_tbl[141] = {coll = 0x0000}; -- Spike ball

tr4_entity_tbl[142] = {coll = 0x0000, hide = 1}; -- Flame - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[143] = {coll = 0x0000, hide = 1}; -- Flame emitter
tr4_entity_tbl[144] = {coll = 0x0000, hide = 1}; -- Flame emitter 2
tr4_entity_tbl[145] = {coll = 0x0000, hide = 1}; -- Flame emitter 3
tr4_entity_tbl[146] = {coll = 0x0000, hide = 1}; -- Rope

tr4_entity_tbl[147] = {coll = 0x0000}; -- Fire rope
tr4_entity_tbl[148] = {coll = 0x0000}; -- Pole rope
tr4_entity_tbl[149] = {coll = 0x0001}; -- One block platform  - UNUSED
tr4_entity_tbl[150] = {coll = 0x0001}; -- Two block platform
tr4_entity_tbl[151] = {coll = 0x0002}; -- Raising block 1 - RESIZABLE MESH!
tr4_entity_tbl[152] = {coll = 0x0002}; -- Raising block 2 - RESIZABLE MESH!
tr4_entity_tbl[153] = {coll = 0x0002}; -- Expanding platform - RESIZABLE MESH!
tr4_entity_tbl[154] = {coll = 0x0002}; -- Squishy block 1
tr4_entity_tbl[155] = {coll = 0x0002}; -- Squishy block 2
tr4_entity_tbl[156] = {coll = 0x0002}; -- Pushable object 1
tr4_entity_tbl[157] = {coll = 0x0002}; -- Pushable object 2
tr4_entity_tbl[158] = {coll = 0x0002}; -- Pushable object 3
tr4_entity_tbl[159] = {coll = 0x0002}; -- Pushable object 4
tr4_entity_tbl[160] = {coll = 0x0002}; -- Pushable object 5
tr4_entity_tbl[161] = {coll = 0x0000}; -- Trip wire  - UNUSED
tr4_entity_tbl[162] = {coll = 0x0002}; -- Sentry gun
tr4_entity_tbl[163] = {coll = 0x0002}; -- Mine
tr4_entity_tbl[164] = {coll = 0x0000}; -- Mapper
tr4_entity_tbl[165] = {coll = 0x0002}; -- Obelisk
tr4_entity_tbl[166] = {coll = 0x0001}; -- Floor 4 blade
tr4_entity_tbl[167] = {coll = 0x0001}; -- Roof 4 blade
tr4_entity_tbl[168] = {coll = 0x0001}; -- Bird blade
tr4_entity_tbl[169] = {coll = 0x0001}; -- Catwalk blade
tr4_entity_tbl[170] = {coll = 0x0001}; -- Moving blade
tr4_entity_tbl[171] = {coll = 0x0001}; -- Plinth blade
tr4_entity_tbl[172] = {coll = 0x0001}; -- Seth blade

tr4_entity_tbl[173] = {coll = 0x0000, hide = 1}; -- Lighting conductor

-- PICK-UP WALKTHROUGH ITEMS

tr4_entity_tbl[174] = {coll = 0x0000}; -- Element puzzle
tr4_entity_tbl[175] = {coll = 0x0000}; -- Puzzle item 1
tr4_entity_tbl[176] = {coll = 0x0000}; -- Puzzle item 2
tr4_entity_tbl[177] = {coll = 0x0000}; -- Puzzle item 3
tr4_entity_tbl[178] = {coll = 0x0000}; -- Puzzle item 4
tr4_entity_tbl[179] = {coll = 0x0000}; -- Puzzle item 5
tr4_entity_tbl[180] = {coll = 0x0000}; -- Puzzle item 6
tr4_entity_tbl[181] = {coll = 0x0000}; -- Puzzle item 7
tr4_entity_tbl[182] = {coll = 0x0000}; -- Puzzle item 8
tr4_entity_tbl[183] = {coll = 0x0000}; -- Puzzle item 9
tr4_entity_tbl[184] = {coll = 0x0000}; -- Puzzle item 10
tr4_entity_tbl[185] = {coll = 0x0000}; -- Puzzle item 11
tr4_entity_tbl[186] = {coll = 0x0000}; -- Puzzle item 12
tr4_entity_tbl[187] = {coll = 0x0000}; -- Puzzle item 1 combo 1
tr4_entity_tbl[188] = {coll = 0x0000}; -- Puzzle item 1 combo 2
tr4_entity_tbl[189] = {coll = 0x0000}; -- Puzzle item 2 combo 1
tr4_entity_tbl[190] = {coll = 0x0000}; -- Puzzle item 2 combo 2
tr4_entity_tbl[191] = {coll = 0x0000}; -- Puzzle item 3 combo 1
tr4_entity_tbl[192] = {coll = 0x0000}; -- Puzzle item 3 combo 2
tr4_entity_tbl[193] = {coll = 0x0000}; -- Puzzle item 4 combo 1
tr4_entity_tbl[194] = {coll = 0x0000}; -- Puzzle item 4 combo 2
tr4_entity_tbl[195] = {coll = 0x0000}; -- Puzzle item 5 combo 1
tr4_entity_tbl[196] = {coll = 0x0000}; -- Puzzle item 5 combo 2
tr4_entity_tbl[197] = {coll = 0x0000}; -- Puzzle item 6 combo 1
tr4_entity_tbl[198] = {coll = 0x0000}; -- Puzzle item 6 combo 2
tr4_entity_tbl[199] = {coll = 0x0000}; -- Puzzle item 7 combo 1
tr4_entity_tbl[200] = {coll = 0x0000}; -- Puzzle item 7 combo 2
tr4_entity_tbl[201] = {coll = 0x0000}; -- Puzzle item 8 combo 1
tr4_entity_tbl[202] = {coll = 0x0000}; -- Puzzle item 8 combo 2
tr4_entity_tbl[203] = {coll = 0x0000}; -- Key item 1
tr4_entity_tbl[204] = {coll = 0x0000}; -- Key item 2
tr4_entity_tbl[205] = {coll = 0x0000}; -- Key item 3
tr4_entity_tbl[206] = {coll = 0x0000}; -- Key item 4
tr4_entity_tbl[207] = {coll = 0x0000}; -- Key item 5
tr4_entity_tbl[208] = {coll = 0x0000}; -- Key item 6
tr4_entity_tbl[209] = {coll = 0x0000}; -- Key item 7
tr4_entity_tbl[210] = {coll = 0x0000}; -- Key item 8
tr4_entity_tbl[211] = {coll = 0x0000}; -- Key item 9
tr4_entity_tbl[212] = {coll = 0x0000}; -- Key item 10
tr4_entity_tbl[213] = {coll = 0x0000}; -- Key item 11
tr4_entity_tbl[214] = {coll = 0x0000}; -- Key item 12
tr4_entity_tbl[215] = {coll = 0x0000}; -- Key item 1 combo 1
tr4_entity_tbl[216] = {coll = 0x0000}; -- Key item 1 combo 2
tr4_entity_tbl[217] = {coll = 0x0000}; -- Key item 2 combo 1
tr4_entity_tbl[218] = {coll = 0x0000}; -- Key item 2 combo 2
tr4_entity_tbl[219] = {coll = 0x0000}; -- Key item 3 combo 1
tr4_entity_tbl[220] = {coll = 0x0000}; -- Key item 3 combo 2
tr4_entity_tbl[221] = {coll = 0x0000}; -- Key item 4 combo 1
tr4_entity_tbl[222] = {coll = 0x0000}; -- Key item 4 combo 2
tr4_entity_tbl[223] = {coll = 0x0000}; -- Key item 5 combo 1
tr4_entity_tbl[224] = {coll = 0x0000}; -- Key item 5 combo 2
tr4_entity_tbl[225] = {coll = 0x0000}; -- Key item 6 combo 1
tr4_entity_tbl[226] = {coll = 0x0000}; -- Key item 6 combo 2
tr4_entity_tbl[227] = {coll = 0x0000}; -- Key item 7 combo 1
tr4_entity_tbl[228] = {coll = 0x0000}; -- Key item 7 combo 2
tr4_entity_tbl[229] = {coll = 0x0000}; -- Key item 8 combo 1
tr4_entity_tbl[230] = {coll = 0x0000}; -- Key item 8 combo 2
tr4_entity_tbl[231] = {coll = 0x0000}; -- Pickup item 1
tr4_entity_tbl[232] = {coll = 0x0000}; -- Pickup item 2
tr4_entity_tbl[233] = {coll = 0x0000}; -- Pickup item 3
tr4_entity_tbl[234] = {coll = 0x0000}; -- Pickup item 4
tr4_entity_tbl[235] = {coll = 0x0000}; -- Pickup item 1 combo 1
tr4_entity_tbl[236] = {coll = 0x0000}; -- Pickup item 1 combo 2
tr4_entity_tbl[237] = {coll = 0x0000}; -- Pickup item 2 combo 1
tr4_entity_tbl[238] = {coll = 0x0000}; -- Pickup item 2 combo 2
tr4_entity_tbl[239] = {coll = 0x0000}; -- Pickup item 3 combo 1
tr4_entity_tbl[240] = {coll = 0x0000}; -- Pickup item 3 combo 2
tr4_entity_tbl[241] = {coll = 0x0000}; -- Pickup item 4 combo 1
tr4_entity_tbl[242] = {coll = 0x0000}; -- Pickup item 4 combo 2
tr4_entity_tbl[243] = {coll = 0x0000}; -- Examine 1
tr4_entity_tbl[244] = {coll = 0x0000}; -- Examine 2
tr4_entity_tbl[245] = {coll = 0x0000}; -- Examine 3
tr4_entity_tbl[246] = {coll = 0x0000}; -- Crowbar item
tr4_entity_tbl[247] = {coll = 0x0000}; -- Burning torch item
tr4_entity_tbl[248] = {coll = 0x0000}; -- Clock work beetle
tr4_entity_tbl[249] = {coll = 0x0000}; -- Clock work beetle combo 1
tr4_entity_tbl[250] = {coll = 0x0000}; -- Clock work beetle combo 2
tr4_entity_tbl[251] = {coll = 0x0000}; -- Mine detector
tr4_entity_tbl[252] = {coll = 0x0000}; -- Quest item 1
tr4_entity_tbl[253] = {coll = 0x0000}; -- Quest item 2
tr4_entity_tbl[254] = {coll = 0x0000}; -- Quest item 3
tr4_entity_tbl[255] = {coll = 0x0000}; -- Quest item 4
tr4_entity_tbl[256] = {coll = 0x0000}; -- Quest item 5
tr4_entity_tbl[257] = {coll = 0x0000}; -- Quest item 6
tr4_entity_tbl[258] = {coll = 0x0000}; -- Map - UNUSED
tr4_entity_tbl[259] = {coll = 0x0000}; -- Secret map - UNUSED

-- PUZZLE HOLES AND KEYHOLES

tr4_entity_tbl[260] = {coll = 0x0000}; -- Puzzle hole 1
tr4_entity_tbl[261] = {coll = 0x0000}; -- Puzzle hole 2
tr4_entity_tbl[262] = {coll = 0x0000}; -- Puzzle hole 3
tr4_entity_tbl[263] = {coll = 0x0000}; -- Puzzle hole 4
tr4_entity_tbl[264] = {coll = 0x0000}; -- Puzzle hole 5
tr4_entity_tbl[265] = {coll = 0x0000}; -- Puzzle hole 6
tr4_entity_tbl[266] = {coll = 0x0000}; -- Puzzle hole 7
tr4_entity_tbl[267] = {coll = 0x0000}; -- Puzzle hole 8
tr4_entity_tbl[268] = {coll = 0x0000}; -- Puzzle hole 9
tr4_entity_tbl[269] = {coll = 0x0000}; -- Puzzle hole 10
tr4_entity_tbl[270] = {coll = 0x0000}; -- Puzzle hole 11
tr4_entity_tbl[271] = {coll = 0x0000}; -- Puzzle hole 12
tr4_entity_tbl[272] = {coll = 0x0000}; -- Puzzle done 1
tr4_entity_tbl[273] = {coll = 0x0000}; -- Puzzle done 2
tr4_entity_tbl[274] = {coll = 0x0000}; -- Puzzle done 3
tr4_entity_tbl[275] = {coll = 0x0000}; -- Puzzle done 4
tr4_entity_tbl[276] = {coll = 0x0000}; -- Puzzle done 5
tr4_entity_tbl[277] = {coll = 0x0000}; -- Puzzle done 6
tr4_entity_tbl[278] = {coll = 0x0000}; -- Puzzle done 7
tr4_entity_tbl[279] = {coll = 0x0000}; -- Puzzle done 8
tr4_entity_tbl[280] = {coll = 0x0000}; -- Puzzle done 9
tr4_entity_tbl[281] = {coll = 0x0000}; -- Puzzle done 10
tr4_entity_tbl[282] = {coll = 0x0000}; -- Puzzle done 11
tr4_entity_tbl[283] = {coll = 0x0000}; -- Puzzle done 12
tr4_entity_tbl[284] = {coll = 0x0002}; -- Key hole 1 (ig keyhole hub.tr4)
tr4_entity_tbl[285] = {coll = 0x0000}; -- Key hole 2
tr4_entity_tbl[286] = {coll = 0x0000}; -- Key hole 3
tr4_entity_tbl[287] = {coll = 0x0000}; -- Key hole 4
tr4_entity_tbl[288] = {coll = 0x0000}; -- Key hole 5
tr4_entity_tbl[289] = {coll = 0x0000}; -- Key hole 6
tr4_entity_tbl[290] = {coll = 0x0000}; -- Key hole 7
tr4_entity_tbl[291] = {coll = 0x0000}; -- Key hole 8
tr4_entity_tbl[292] = {coll = 0x0000}; -- Key hole 9
tr4_entity_tbl[293] = {coll = 0x0000}; -- Key hole 10
tr4_entity_tbl[294] = {coll = 0x0000}; -- Key hole 11
tr4_entity_tbl[295] = {coll = 0x0000}; -- Key hole 12

-- WATERSKIN ITEMS

tr4_entity_tbl[296] = {coll = 0x0000}; -- Water skin 1 empty
tr4_entity_tbl[297] = {coll = 0x0000}; -- Water skin 1 1
tr4_entity_tbl[298] = {coll = 0x0000}; -- Water skin 1 2
tr4_entity_tbl[299] = {coll = 0x0000}; -- Water skin 1 3
tr4_entity_tbl[300] = {coll = 0x0000}; -- Water skin 2 empty
tr4_entity_tbl[301] = {coll = 0x0000}; -- Water skin 2 1
tr4_entity_tbl[302] = {coll = 0x0000}; -- Water skin 2 2
tr4_entity_tbl[303] = {coll = 0x0000}; -- Water skin 2 3
tr4_entity_tbl[304] = {coll = 0x0000}; -- Water skin 2 4
tr4_entity_tbl[305] = {coll = 0x0000}; -- Water skin 2 5

-- SWITCHES

tr4_entity_tbl[306] = {coll = 0x0002}; -- Switch type 1
tr4_entity_tbl[307] = {coll = 0x0002}; -- Switch type 2
tr4_entity_tbl[308] = {coll = 0x0002}; -- Switch type 3
tr4_entity_tbl[309] = {coll = 0x0002}; -- Switch type 4
tr4_entity_tbl[310] = {coll = 0x0002}; -- Switch type 5
tr4_entity_tbl[311] = {coll = 0x0002}; -- Switch type 6
tr4_entity_tbl[312] = {coll = 0x0002}; -- Switch type 7
tr4_entity_tbl[313] = {coll = 0x0002}; -- Switch type 8
tr4_entity_tbl[314] = {coll = 0x0002}; -- Underwater switch 1
tr4_entity_tbl[315] = {coll = 0x0002}; -- Underwater switch 2
tr4_entity_tbl[316] = {coll = 0x0002}; -- Turn switch
tr4_entity_tbl[317] = {coll = 0x0002}; -- Cog switch
tr4_entity_tbl[318] = {coll = 0x0002}; -- Lever switch
tr4_entity_tbl[319] = {coll = 0x0002}; -- Jump switch
tr4_entity_tbl[320] = {coll = 0x0002}; -- Crowbar switch
tr4_entity_tbl[321] = {coll = 0x0002}; -- Pulley

-- DOORS

tr4_entity_tbl[322] = {coll = 0x0001}; -- Door type 1
tr4_entity_tbl[323] = {coll = 0x0001}; -- Door type 2
tr4_entity_tbl[324] = {coll = 0x0001}; -- Door type 3
tr4_entity_tbl[325] = {coll = 0x0001}; -- Door type 4
tr4_entity_tbl[326] = {coll = 0x0001}; -- Door type 5
tr4_entity_tbl[327] = {coll = 0x0001}; -- Door type 6
tr4_entity_tbl[328] = {coll = 0x0001}; -- Door type 7
tr4_entity_tbl[329] = {coll = 0x0001}; -- Door type 8
tr4_entity_tbl[330] = {coll = 0x0001}; -- Push pull door 1
tr4_entity_tbl[331] = {coll = 0x0001}; -- Push pull door 2
tr4_entity_tbl[332] = {coll = 0x0001}; -- Kick door 1
tr4_entity_tbl[333] = {coll = 0x0001}; -- Kick door 2
tr4_entity_tbl[334] = {coll = 0x0001}; -- Underwater door
tr4_entity_tbl[335] = {coll = 0x0001}; -- Double doors

-- STATIC TERRAIN

tr4_entity_tbl[336] = {coll = 0x0001}; -- Bridge flat
tr4_entity_tbl[337] = {coll = 0x0001}; -- Bridge tilt 1
tr4_entity_tbl[338] = {coll = 0x0001}; -- Bridge tilt 2

-- MISC INTERACTION OBJECTS

tr4_entity_tbl[339] = {coll = 0x0001}; -- Sarcophagus
tr4_entity_tbl[340] = {coll = 0x0001}; -- Sequence door 1
tr4_entity_tbl[341] = {coll = 0x0002}; -- Sequence switch 1
tr4_entity_tbl[342] = {coll = 0x0002}; -- Sequence switch 2
tr4_entity_tbl[343] = {coll = 0x0002}; -- Sequence switch 3
tr4_entity_tbl[344] = {coll = 0x0002}; -- Sarcophagus cut
tr4_entity_tbl[345] = {coll = 0x0001}; -- Horus statue
tr4_entity_tbl[346] = {coll = 0x0000}; -- God head
tr4_entity_tbl[347] = {coll = 0x0001}; -- Seth door
tr4_entity_tbl[348] = {coll = 0x0001}; -- Statue plinth

 -- PICK-UP SUPPLY ITEMS

tr4_entity_tbl[349] = {coll = 0x0000}; -- Pistols item
tr4_entity_tbl[350] = {coll = 0x0000}; -- Pistols ammo item
tr4_entity_tbl[351] = {coll = 0x0000}; -- Uzi item
tr4_entity_tbl[352] = {coll = 0x0000}; -- Uzi ammo item
tr4_entity_tbl[353] = {coll = 0x0000}; -- Shotgun item
tr4_entity_tbl[354] = {coll = 0x0000}; -- Shotgun ammo 1 item
tr4_entity_tbl[355] = {coll = 0x0000}; -- Shotgun ammo 2 item
tr4_entity_tbl[356] = {coll = 0x0000}; -- Crossbow item
tr4_entity_tbl[357] = {coll = 0x0000}; -- Crossbow ammo 1 item
tr4_entity_tbl[358] = {coll = 0x0000}; -- Crossbow ammo 2 item
tr4_entity_tbl[359] = {coll = 0x0000}; -- Crossbow ammo 3 item
tr4_entity_tbl[360] = {coll = 0x0000}; -- Crossbow bolt
tr4_entity_tbl[361] = {coll = 0x0000}; -- Grenade gun item
tr4_entity_tbl[362] = {coll = 0x0000}; -- Grenade gun ammo 1 item
tr4_entity_tbl[363] = {coll = 0x0000}; -- Grenade gun ammo 2 item
tr4_entity_tbl[364] = {coll = 0x0000}; -- Grenade gun ammo 3 item
tr4_entity_tbl[365] = {coll = 0x0000}; -- Grenade
tr4_entity_tbl[366] = {coll = 0x0000}; -- Six shooter item
tr4_entity_tbl[367] = {coll = 0x0000}; -- Six shooter ammo item
tr4_entity_tbl[368] = {coll = 0x0000}; -- Big medipack item
tr4_entity_tbl[369] = {coll = 0x0000}; -- Small medipack item
tr4_entity_tbl[370] = {coll = 0x0000}; -- Laser sight item
tr4_entity_tbl[371] = {coll = 0x0000}; -- Binoculars item
tr4_entity_tbl[372] = {coll = 0x0000}; -- Flare item
tr4_entity_tbl[373] = {coll = 0x0000}; -- Flare inv item
tr4_entity_tbl[374] = {coll = 0x0000}; -- Diary item - UNUSED

-- INVENTORY ITEMS

tr4_entity_tbl[375] = {coll = 0x0000}; -- Compass item - NOT A PROPER PICK UP OBJECT (INVENTORY ONLY)
tr4_entity_tbl[376] = {coll = 0x0000}; -- Mem card load inv item - UNUSED
tr4_entity_tbl[377] = {coll = 0x0000}; -- Mem card save inv item - UNUSED
tr4_entity_tbl[378] = {coll = 0x0000}; -- PC load inv item - NOT A PROPER PICK UP OBJECT (INVENTORY ONLY)
tr4_entity_tbl[379] = {coll = 0x0000}; -- PC save inv item - NOT A PROPER PICK UP OBJECT (INVENTORY ONLY)

-- NULLMESHES, SERVICE OBJECTS AND EMITTERS

tr4_entity_tbl[380] = {coll = 0x0000, hide = 1}; -- Smoke emitter white - EMITTER
tr4_entity_tbl[381] = {coll = 0x0000, hide = 1}; -- Smoke emitter black - EMITTER
tr4_entity_tbl[382] = {coll = 0x0000, hide = 1}; -- Steam emitter - EMITTER
tr4_entity_tbl[383] = {coll = 0x0000, hide = 1}; -- Earth quake - SHAKES CAMERA
tr4_entity_tbl[384] = {coll = 0x0000, hide = 1}; -- Bubbles - EMITTER
tr4_entity_tbl[385] = {coll = 0x0000, hide = 1}; -- Waterfall mist - EMITTER

tr4_entity_tbl[386] = {coll = 0x0000}; -- Gun shell - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[387] = {coll = 0x0000}; -- Shotgun shell - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[388] = {coll = 0x0000}; -- Gun flash - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[389] = {coll = 0x0000}; -- Butterfly - UNUSED
tr4_entity_tbl[390] = {coll = 0x0000}; -- Sprinkler - EMITTER
tr4_entity_tbl[391] = {coll = 0x0000}; -- Red light - STATIC LIGHT

tr4_entity_tbl[392] = {coll = 0x0000, hide = 1}; -- Green light - STATIC LIGHT
tr4_entity_tbl[393] = {coll = 0x0000, hide = 1}; -- Blue light - STATIC LIGHT
tr4_entity_tbl[394] = {coll = 0x0000, hide = 1}; -- Amber light - DYNAMIC LIGHT
tr4_entity_tbl[395] = {coll = 0x0000, hide = 1}; -- White light - STATIC LIGHT
tr4_entity_tbl[396] = {coll = 0x0000, hide = 1}; -- Blinking light - DYNAMIC LIGHT
tr4_entity_tbl[397] = {coll = 0x0000, hide = 1}; -- Lens flare


-- Remark: objects ID 398-408 are AI / trigger nullmeshes, and never shows in game.

tr4_entity_tbl[398] = {hide = 1};
tr4_entity_tbl[399] = {hide = 1};
tr4_entity_tbl[400] = {hide = 1};
tr4_entity_tbl[401] = {hide = 1};
tr4_entity_tbl[402] = {hide = 1};
tr4_entity_tbl[403] = {hide = 1};
tr4_entity_tbl[404] = {hide = 1};
tr4_entity_tbl[405] = {hide = 1};
tr4_entity_tbl[406] = {hide = 1};
tr4_entity_tbl[407] = {hide = 1};
tr4_entity_tbl[408] = {hide = 1};

-- MISC. OBJECTS

tr4_entity_tbl[409] = {coll = 0x0001}; -- Smash object 1
tr4_entity_tbl[410] = {coll = 0x0001}; -- Smash object 2
tr4_entity_tbl[411] = {coll = 0x0001}; -- Smash object 3
tr4_entity_tbl[412] = {coll = 0x0001}; -- Smash object 4
tr4_entity_tbl[413] = {coll = 0x0001}; -- Smash object 5
tr4_entity_tbl[414] = {coll = 0x0001}; -- Smash object 6
tr4_entity_tbl[415] = {coll = 0x0001}; -- Smash object 7
tr4_entity_tbl[416] = {coll = 0x0001}; -- Smash object 8
tr4_entity_tbl[417] = {coll = 0x0000}; -- Mesh swap 1
tr4_entity_tbl[418] = {coll = 0x0000}; -- Mesh swap 2
tr4_entity_tbl[419] = {coll = 0x0000}; -- Mesh swap 3
tr4_entity_tbl[420] = {coll = 0x0000}; -- Death slide
tr4_entity_tbl[421] = {coll = 0x0000}; -- Body part - UNUSED? SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[422] = {coll = 0x0000, hide = 1}; -- Camera target - TARGET FOR CAMERA

-- WATERFALLS (MESHES WITH UV-SCROLL ANIMATED TEXTURES)

tr4_entity_tbl[423] = {coll = 0x0000}; -- Waterfall 1
tr4_entity_tbl[424] = {coll = 0x0000}; -- Waterfall 2
tr4_entity_tbl[425] = {coll = 0x0000}; -- Waterfall 3
tr4_entity_tbl[426] = {coll = 0x0000}; -- Planet effect

-- ANIMATINGS

tr4_entity_tbl[427] = {coll = 0x0001}; -- Animating 1
tr4_entity_tbl[428] = {coll = 0x0001}; -- Animating 1 MIP
tr4_entity_tbl[429] = {coll = 0x0001}; -- Animating 2
tr4_entity_tbl[430] = {coll = 0x0001}; -- Animating 2 MIP
tr4_entity_tbl[431] = {coll = 0x0001}; -- Animating 3
tr4_entity_tbl[432] = {coll = 0x0001}; -- Animating 3 MIP
tr4_entity_tbl[433] = {coll = 0x0001}; -- Animating 4
tr4_entity_tbl[434] = {coll = 0x0001}; -- Animating 4 MIP
tr4_entity_tbl[435] = {coll = 0x0001}; -- Animating 5
tr4_entity_tbl[436] = {coll = 0x0001}; -- Animating 5 MIP
tr4_entity_tbl[437] = {coll = 0x0001}; -- Animating 6
tr4_entity_tbl[438] = {coll = 0x0001}; -- Animating 6 MIP
tr4_entity_tbl[439] = {coll = 0x0001}; -- Animating 7
tr4_entity_tbl[440] = {coll = 0x0001}; -- Animating 7 MIP
tr4_entity_tbl[441] = {coll = 0x0001}; -- Animating 8
tr4_entity_tbl[442] = {coll = 0x0001}; -- Animating 8 MIP
tr4_entity_tbl[443] = {coll = 0x0001}; -- Animating 9
tr4_entity_tbl[444] = {coll = 0x0001}; -- Animating 9 MIP
tr4_entity_tbl[445] = {coll = 0x0001}; -- Animating 10
tr4_entity_tbl[446] = {coll = 0x0001}; -- Animating 10 MIP
tr4_entity_tbl[447] = {coll = 0x0001}; -- Animating 11
tr4_entity_tbl[448] = {coll = 0x0001}; -- Animating 11 MIP
tr4_entity_tbl[449] = {coll = 0x0001}; -- Animating 12
tr4_entity_tbl[450] = {coll = 0x0001}; -- Animating 12 MIP
tr4_entity_tbl[451] = {coll = 0x0001}; -- Animating 13
tr4_entity_tbl[452] = {coll = 0x0001}; -- Animating 13 MIP
tr4_entity_tbl[453] = {coll = 0x0001}; -- Animating 14 (sunlight)
tr4_entity_tbl[454] = {coll = 0x0001}; -- Animating 14 MIP
tr4_entity_tbl[455] = {coll = 0x0001}; -- Animating 15 (basket, hub.tr4, sunlight)
tr4_entity_tbl[456] = {coll = 0x0001}; -- Animating 15 MIP
tr4_entity_tbl[457] = {coll = 0x0001}; -- Animating 16
tr4_entity_tbl[458] = {coll = 0x0001}; -- Animating 16 MIP

--------------------------------------------------------------------------------
------------------------------------- TR_V -------------------------------------
--------------------------------------------------------------------------------
tr5_entity_tbl = {};

-- Remark: object IDs 0-30 are used for Lara model and speechheads, and never
-- show in game independently.

-- ENEMIES

tr5_entity_tbl[031] = {coll = 0x0002}; -- SAS - UNUSED
tr5_entity_tbl[032] = {coll = 0x0002}; -- SAS MIP - UNUSED
tr5_entity_tbl[033] = {coll = 0x0002}; -- SWAT  - UNUSED
tr5_entity_tbl[034] = {coll = 0x0002}; -- SWAT MIP - UNUSED
tr5_entity_tbl[035] = {coll = 0x0002}; -- VCI guard (SWAT_PLUS)
tr5_entity_tbl[036] = {coll = 0x0002}; -- VCI guard MIP (SWAT_PLUS MIP)
tr5_entity_tbl[037] = {coll = 0x0002}; -- Guard gun (BLUE_GUARD)
tr5_entity_tbl[038] = {coll = 0x0002}; -- Guard gun MIP (BLUE_GUARD MIP)
tr5_entity_tbl[039] = {coll = 0x0002}; -- Guard laser (TWOGUN)
tr5_entity_tbl[040] = {coll = 0x0002}; -- Guard laser MIP (TWOGUN MIP)
tr5_entity_tbl[041] = {coll = 0x0002}; -- Doberman (DOG)
tr5_entity_tbl[042] = {coll = 0x0002}; -- Doberman MIP (DOG MIP)
tr5_entity_tbl[043] = {coll = 0x0002}; -- Crow
tr5_entity_tbl[044] = {coll = 0x0002}; -- Crow MIP
tr5_entity_tbl[045] = {coll = 0x0002}; -- Larson
tr5_entity_tbl[046] = {coll = 0x0000}; -- Keycard 1 (Ex-LARSON MIP)
tr5_entity_tbl[047] = {coll = 0x0002}; -- Pierre
tr5_entity_tbl[048] = {coll = 0x0000}; -- Keycard 2  (Ex-PIERRE MIP)
tr5_entity_tbl[049] = {coll = 0x0002}; -- Armed baddy 1 (MAFIA)
tr5_entity_tbl[050] = {coll = 0x0002}; -- Armed baddy 1 MIP (MAFIA MIP)
tr5_entity_tbl[051] = {coll = 0x0002}; -- Armed baddy 2 (MAFIA2)
tr5_entity_tbl[052] = {coll = 0x0002}; -- Armed baddy 2 MIP (MAFIA2 MIP)
tr5_entity_tbl[053] = {coll = 0x0002}; -- (Ex-SAILOR) - UNUSED?
tr5_entity_tbl[054] = {coll = 0x0002}; -- (Ex-SAILOR MIP) - UNUSED?
tr5_entity_tbl[055] = {coll = 0x0002}; -- Guard robot control (CRANE_GUY)
tr5_entity_tbl[056] = {coll = 0x0002}; -- Guard robot control MIP (CRANE_GUY MIP)
tr5_entity_tbl[057] = {coll = 0x0002}; -- Lion
tr5_entity_tbl[058] = {coll = 0x0002}; -- Lion MIP
tr5_entity_tbl[059] = {coll = 0x0002}; -- Gladiator
tr5_entity_tbl[060] = {coll = 0x0002}; -- Gladiator MIP
tr5_entity_tbl[061] = {coll = 0x0002}; -- Roman statue
tr5_entity_tbl[062] = {coll = 0x0000}; -- Spear tip (Ex-ROMAN_GOD MIP)
tr5_entity_tbl[063] = {coll = 0x0002}; -- Hydra
tr5_entity_tbl[064] = {coll = 0x0002}; -- Flat floor (Ex-HYDRA MIP?)
tr5_entity_tbl[065] = {coll = 0x0002}; -- Laser head (GUARDIAN)
tr5_entity_tbl[066] = {coll = 0x0002}; -- Laser head MIP (GUARDIAN MIP)
tr5_entity_tbl[067] = {coll = 0x0002}; -- Cyborg
tr5_entity_tbl[068] = {coll = 0x0002}; -- Cyborg MIP
tr5_entity_tbl[069] = {coll = 0x0002}; -- VCI worker
tr5_entity_tbl[070] = {coll = 0x0002}; -- VCI worker MIP
tr5_entity_tbl[071] = {coll = 0x0002}; -- Willowisp Guide
tr5_entity_tbl[072] = {coll = 0x0002}; -- Willowisp Guide MIP
tr5_entity_tbl[073] = {coll = 0x0000}; -- Invisible ghost
tr5_entity_tbl[074] = {coll = 0x0000}; -- Invisible ghost MIP - UNUSED?
tr5_entity_tbl[075] = {coll = 0x0002}; -- Reaper - UNUSED?
tr5_entity_tbl[076] = {coll = 0x0002}; -- Reaper MIP- UNUSED?
tr5_entity_tbl[077] = {coll = 0x0002}; -- Maze Monster
tr5_entity_tbl[078] = {coll = 0x0002}; -- Maze Monster MIP - UNUSED?
tr5_entity_tbl[079] = {coll = 0x0002}; -- Lagoon Witch
tr5_entity_tbl[080] = {coll = 0x0002}; -- Lagoon Witch MIP - UNUSED?
tr5_entity_tbl[081] = {coll = 0x0002}; -- Submarine
tr5_entity_tbl[082] = {coll = 0x0002}; -- Submarine MIP
tr5_entity_tbl[083] = {coll = 0x0002}; -- M16 Guard
tr5_entity_tbl[084] = {coll = 0x0002}; -- Tree with huged man (Ex-M16 Guard MIP)
tr5_entity_tbl[085] = {coll = 0x0002}; -- Husky
tr5_entity_tbl[086] = {coll = 0x0002}; -- Husky MIP
tr5_entity_tbl[087] = {coll = 0x0002}; -- The chef
tr5_entity_tbl[088] = {coll = 0x0002}; -- Rich1 hammer door (Ex-CHEF MIP)
tr5_entity_tbl[089] = {coll = 0x0002}; -- Imp
tr5_entity_tbl[090] = {coll = 0x0002}; -- Padlock (Ex-IMP_MIP)
tr5_entity_tbl[091] = {coll = 0x0002}; -- Gunship
tr5_entity_tbl[092] = {coll = 0x0002}; -- Gunship MIP
tr5_entity_tbl[093] = {coll = 0x0000}; -- Bats
tr5_entity_tbl[094] = {coll = 0x0000}; -- Little rats
tr5_entity_tbl[095] = {coll = 0x0000}; -- Spiders
tr5_entity_tbl[096] = {coll = 0x0000}; -- Spider generator - UNUSED?
tr5_entity_tbl[097] = {coll = 0x0002}; -- Auto guns
tr5_entity_tbl[098] = {coll = 0x0001}; -- Electricity wires

tr5_entity_tbl[099] = {coll = 0x0000, hide = 1}; -- Darts - SPAWN OBJECT
tr5_entity_tbl[100] = {coll = 0x0000, hide = 1}; -- Dart emitter
tr5_entity_tbl[101] = {coll = 0x0000, hide = 1}; -- Homing (fast) dart emitter

-- DESTROYABLE / MOVABLE TERRAIN

tr5_entity_tbl[102] = {coll = 0x0001}; -- Falling Ceiling
tr5_entity_tbl[103] = {coll = 0x0001}; -- Falling Block 1
tr5_entity_tbl[104] = {coll = 0x0001}; -- Falling Block 2
tr5_entity_tbl[105] = {coll = 0x0001}; -- Crumbling Floor
tr5_entity_tbl[106] = {coll = 0x0001}; -- Trapdoor 1
tr5_entity_tbl[107] = {coll = 0x0001}; -- Trapdoor 2
tr5_entity_tbl[108] = {coll = 0x0001}; -- Trapdoor 3
tr5_entity_tbl[109] = {coll = 0x0001}; -- Floor trapdoor 1
tr5_entity_tbl[110] = {coll = 0x0001}; -- Floor trapdoor 2
tr5_entity_tbl[111] = {coll = 0x0001}; -- Ceiling trapdoor 1
tr5_entity_tbl[112] = {coll = 0x0001}; -- Ceiling trapdoor 2
tr5_entity_tbl[113] = {coll = 0x0001}; -- Scaling trapdoor
tr5_entity_tbl[114] = {coll = 0x0001}; -- Box 1 (ROLLINGBALL)
tr5_entity_tbl[115] = {coll = 0x0001}; -- Rolling barrel - UNUSED?

-- TRAPS & INTERACTION OBJECTS

tr5_entity_tbl[116] = {coll = 0x0002}; -- Spikey Floor - UNUSED?
tr5_entity_tbl[117] = {coll = 0x0002}; -- Teeth Spikes
tr5_entity_tbl[118] = {coll = 0x0002}; -- Rome Hammer
tr5_entity_tbl[119] = {coll = 0x0002}; -- Hammer 2 - UNUSED?

tr5_entity_tbl[120] = {coll = 0x0000, hide = 1}; -- Flame
tr5_entity_tbl[121] = {coll = 0x0000, hide = 1}; -- Flame emitter
tr5_entity_tbl[122] = {coll = 0x0000, hide = 1}; -- Flame emitter 2
tr5_entity_tbl[123] = {coll = 0x0000, hide = 1}; -- Flame emitter 3
tr5_entity_tbl[124] = {coll = 0x0000, hide = 1}; -- Cooker flame

tr5_entity_tbl[125] = {coll = 0x0000}; -- Burning roots

tr5_entity_tbl[126] = {coll = 0x0000, hide = 1}; -- Rope

tr5_entity_tbl[127] = {coll = 0x0000}; -- Fire rope
tr5_entity_tbl[128] = {coll = 0x0000}; -- Pole rope
tr5_entity_tbl[129] = {coll = 0x0001}; -- Ventilator HORIZONTAL
tr5_entity_tbl[130] = {coll = 0x0001}; -- Ventilator VERTICAL

tr5_entity_tbl[131] = {coll = 0x0000, hide = 1}; -- Grappling gun target

tr5_entity_tbl[132] = {coll = 0x0001}; -- One block platform - UNUSED?
tr5_entity_tbl[133] = {coll = 0x0001}; -- Two block platform - UNUSED?
tr5_entity_tbl[134] = {coll = 0x0001}; -- Box 2 (Ex-RAISING_BLOCK1?)

tr5_entity_tbl[135] = {coll = 0x0001, hide = 1}; -- Teleport (Ex-RAISING_BLOCK2?)
tr5_entity_tbl[136] = {coll = 0x0000, hide = 1}; -- Headset talk point (Ex-EXPANDING_PLATFORM?)

tr5_entity_tbl[137] = {coll = 0x0001}; -- Pushable 1
tr5_entity_tbl[138] = {coll = 0x0001}; -- Pushable 2
tr5_entity_tbl[139] = {coll = 0x0001}; -- Pushable 3 - UNUSED?
tr5_entity_tbl[140] = {coll = 0x0001}; -- Pushable 4 - UNUSED?
tr5_entity_tbl[141] = {coll = 0x0001}; -- Pushable 5 - UNUSED?
tr5_entity_tbl[142] = {coll = 0x0000}; -- Robot arm (Ex-WRECKING BALL?)
tr5_entity_tbl[142] = {coll = 0x0000}; -- Death slide - UNUSED?
tr5_entity_tbl[144] = {coll = 0x0000}; -- Rocket item - TORPEDO
tr5_entity_tbl[145] = {coll = 0x0000}; -- Chaff flare
tr5_entity_tbl[146] = {coll = 0x0000}; -- Satchel Bomb - UNUSED?
tr5_entity_tbl[147] = {coll = 0x0000}; -- Electric Fence - UNUSED?
tr5_entity_tbl[148] = {coll = 0x0000}; -- Lift - UNUSED

tr5_entity_tbl[149] = {coll = 0x0000, hide = 1}; -- Explosion
tr5_entity_tbl[150] = {coll = 0x0000, hide = 1}; -- Deadly Electric bolt (IRIS_LIGHTNING)

tr5_entity_tbl[151] = {coll = 0x0002}; -- Monitor screen
tr5_entity_tbl[152] = {coll = 0x0002}; -- Security camera board

tr5_entity_tbl[153] = {coll = 0x0000, hide = 1}; -- Motion sensor
tr5_entity_tbl[154] = {coll = 0x0000, hide = 1}; -- Tight rope

tr5_entity_tbl[155] = {coll = 0x0001}; -- Parallel bars

tr5_entity_tbl[156] = {coll = 0x0000, hide = 1}; -- X-Ray Controller (?)

tr5_entity_tbl[157] = {coll = 0x0000}; -- Cutscene rope
tr5_entity_tbl[158] = {coll = 0x0001}; -- Flat window
tr5_entity_tbl[159] = {coll = 0x0000}; -- GEN_SLOT1 - UNUSED?

tr5_entity_tbl[160] = {coll = 0x0000, hide = 1}; -- Gas emitter

tr5_entity_tbl[161] = {coll = 0x0000}; -- Sign
tr5_entity_tbl[162] = {coll = 0x0000}; -- Moving laser
tr5_entity_tbl[163] = {coll = 0x0000}; -- Imp Rock - SPAWN ITEM?
tr5_entity_tbl[164] = {coll = 0x0001}; -- Cupboard 1
tr5_entity_tbl[165] = {coll = 0x0001}; -- Cupboard 1 MIP
tr5_entity_tbl[166] = {coll = 0x0001}; -- Cupboard 2
tr5_entity_tbl[167] = {coll = 0x0001}; -- Cupboard 2 MIP
tr5_entity_tbl[168] = {coll = 0x0001}; -- Cupboard 3
tr5_entity_tbl[169] = {coll = 0x0001}; -- Cupboard 3 MIP
tr5_entity_tbl[170] = {coll = 0x0001}; -- Suitcase
tr5_entity_tbl[171] = {coll = 0x0001}; -- Suitcase MIP

-- PICK-UP WALKTHROUGH ITEMS

tr5_entity_tbl[172] = {coll = 0x0000}; -- Puzzle 1
tr5_entity_tbl[173] = {coll = 0x0000}; -- Puzzle 2
tr5_entity_tbl[174] = {coll = 0x0000}; -- Puzzle 3
tr5_entity_tbl[175] = {coll = 0x0000}; -- Puzzle 4
tr5_entity_tbl[176] = {coll = 0x0000}; -- Puzzle 5
tr5_entity_tbl[177] = {coll = 0x0000}; -- Puzzle 6
tr5_entity_tbl[178] = {coll = 0x0000}; -- Puzzle 7
tr5_entity_tbl[179] = {coll = 0x0000}; -- Puzzle 8
tr5_entity_tbl[180] = {coll = 0x0000}; -- Puzzle 1 Combo 1
tr5_entity_tbl[181] = {coll = 0x0000}; -- Puzzle 1 Combo 2
tr5_entity_tbl[182] = {coll = 0x0000}; -- Puzzle 2 Combo 1
tr5_entity_tbl[183] = {coll = 0x0000}; -- Puzzle 2 Combo 2
tr5_entity_tbl[184] = {coll = 0x0000}; -- Puzzle 3 Combo 1
tr5_entity_tbl[185] = {coll = 0x0000}; -- Puzzle 3 Combo 2
tr5_entity_tbl[186] = {coll = 0x0000}; -- Puzzle 4 Combo 1
tr5_entity_tbl[187] = {coll = 0x0000}; -- Puzzle 4 Combo 2
tr5_entity_tbl[188] = {coll = 0x0000}; -- Puzzle 5 Combo 1
tr5_entity_tbl[189] = {coll = 0x0000}; -- Puzzle 5 Combo 2
tr5_entity_tbl[190] = {coll = 0x0000}; -- Puzzle 6 Combo 1
tr5_entity_tbl[191] = {coll = 0x0000}; -- Puzzle 6 Combo 2
tr5_entity_tbl[192] = {coll = 0x0000}; -- Puzzle 7 Combo 1
tr5_entity_tbl[193] = {coll = 0x0000}; -- Puzzle 7 Combo 2
tr5_entity_tbl[194] = {coll = 0x0000}; -- Puzzle 8 Combo 1
tr5_entity_tbl[195] = {coll = 0x0000}; -- Puzzle 8 Combo 2
tr5_entity_tbl[196] = {coll = 0x0000}; -- Key 1
tr5_entity_tbl[197] = {coll = 0x0000}; -- Key 2
tr5_entity_tbl[198] = {coll = 0x0000}; -- Key 3 - UNUSED?
tr5_entity_tbl[199] = {coll = 0x0000}; -- Key 4 - UNUSED?
tr5_entity_tbl[200] = {coll = 0x0000}; -- Key 5 - UNUSED?
tr5_entity_tbl[201] = {coll = 0x0000}; -- Key 6
tr5_entity_tbl[202] = {coll = 0x0000}; -- Key 7
tr5_entity_tbl[203] = {coll = 0x0000}; -- Key 8

tr5_entity_tbl[204] = {coll = 0x0000}; -- Key item 1 combo 1
tr5_entity_tbl[205] = {coll = 0x0000}; -- Key item 1 combo 2
tr5_entity_tbl[206] = {coll = 0x0000}; -- Key item 2 combo 1
tr5_entity_tbl[207] = {coll = 0x0000}; -- Key item 2 combo 2
tr5_entity_tbl[208] = {coll = 0x0000}; -- Key item 3 combo 1
tr5_entity_tbl[209] = {coll = 0x0000}; -- Key item 3 combo 2
tr5_entity_tbl[210] = {coll = 0x0000}; -- Key item 4 combo 1
tr5_entity_tbl[211] = {coll = 0x0000}; -- Key item 4 combo 2
tr5_entity_tbl[212] = {coll = 0x0000}; -- Key item 5 combo 1
tr5_entity_tbl[213] = {coll = 0x0000}; -- Key item 5 combo 2
tr5_entity_tbl[214] = {coll = 0x0000}; -- Key item 6 combo 1
tr5_entity_tbl[215] = {coll = 0x0000}; -- Key item 6 combo 2
tr5_entity_tbl[216] = {coll = 0x0000}; -- Key item 7 combo 1
tr5_entity_tbl[217] = {coll = 0x0000}; -- Key item 7 combo 2
tr5_entity_tbl[218] = {coll = 0x0000}; -- Key item 8 combo 1
tr5_entity_tbl[219] = {coll = 0x0000}; -- Key item 8 combo 2

tr5_entity_tbl[220] = {coll = 0x0000}; -- Pickup item 1
tr5_entity_tbl[221] = {coll = 0x0000}; -- Pickup item 2
tr5_entity_tbl[222] = {coll = 0x0000}; -- Pickup item 3 - UNUSED?
tr5_entity_tbl[223] = {coll = 0x0000}; -- Gold rose

tr5_entity_tbl[224] = {coll = 0x0000}; -- Pickup item 1 combo 1 - UNUSED
tr5_entity_tbl[225] = {coll = 0x0000}; -- Pickup item 1 combo 2 - UNUSED
tr5_entity_tbl[226] = {coll = 0x0000}; -- Pickup item 2 combo 1 - UNUSED
tr5_entity_tbl[227] = {coll = 0x0000}; -- Pickup item 2 combo 2 - UNUSED
tr5_entity_tbl[228] = {coll = 0x0000}; -- Pickup item 3 combo 1 - UNUSED
tr5_entity_tbl[229] = {coll = 0x0000}; -- Pickup item 3 combo 2 - UNUSED
tr5_entity_tbl[230] = {coll = 0x0000}; -- Pickup item 4 combo 1 - UNUSED
tr5_entity_tbl[231] = {coll = 0x0000}; -- Pickup item 4 combo 2 - UNUSED

tr5_entity_tbl[232] = {coll = 0x0000}; -- Examine 1 - UNUSED
tr5_entity_tbl[233] = {coll = 0x0000}; -- Examine 2 - UNUSED
tr5_entity_tbl[234] = {coll = 0x0000}; -- Examine 3 - UNUSED

tr5_entity_tbl[235] = {coll = 0x0000}; -- Chloroform cloth
tr5_entity_tbl[236] = {coll = 0x0000}; -- Chloroform bottle
tr5_entity_tbl[237] = {coll = 0x0000}; -- Chloroform soaked cloth
tr5_entity_tbl[238] = {coll = 0x0000}; -- Cosh (?)
tr5_entity_tbl[239] = {coll = 0x0000}; -- Hammer item - UNUSED?
tr5_entity_tbl[240] = {coll = 0x0000}; -- Crowbar item
tr5_entity_tbl[241] = {coll = 0x0000}; -- Torch item

-- PUZZLEHOLES AND KEYHOLES

tr5_entity_tbl[242] = {coll = 0x0001}; -- Puzzle hole 1
tr5_entity_tbl[243] = {coll = 0x0001}; -- Puzzle hole 2
tr5_entity_tbl[244] = {coll = 0x0001}; -- Puzzle hole 3
tr5_entity_tbl[245] = {coll = 0x0001}; -- Puzzle hole 4
tr5_entity_tbl[246] = {coll = 0x0001}; -- Puzzle hole 5
tr5_entity_tbl[247] = {coll = 0x0001}; -- Puzzle hole 6
tr5_entity_tbl[248] = {coll = 0x0001}; -- Puzzle hole 7
tr5_entity_tbl[249] = {coll = 0x0001}; -- Puzzle hole 8
tr5_entity_tbl[250] = {coll = 0x0001}; -- Puzzle done 1
tr5_entity_tbl[251] = {coll = 0x0001}; -- Puzzle done 2
tr5_entity_tbl[252] = {coll = 0x0001}; -- Puzzle done 3
tr5_entity_tbl[253] = {coll = 0x0001}; -- Puzzle done 4
tr5_entity_tbl[254] = {coll = 0x0001}; -- Puzzle done 5
tr5_entity_tbl[255] = {coll = 0x0001}; -- Puzzle done 6
tr5_entity_tbl[256] = {coll = 0x0001}; -- Puzzle done 7
tr5_entity_tbl[257] = {coll = 0x0001}; -- Puzzle done 8
tr5_entity_tbl[258] = {coll = 0x0001}; -- Keyhole 1
tr5_entity_tbl[259] = {coll = 0x0001}; -- Keyhole 2
tr5_entity_tbl[260] = {coll = 0x0001}; -- Keyhole 3 - UNUSED?
tr5_entity_tbl[261] = {coll = 0x0001}; -- Keyhole 4 - UNUSED?
tr5_entity_tbl[262] = {coll = 0x0001}; -- Keyhole 5 - UNUSED?
tr5_entity_tbl[263] = {coll = 0x0001}; -- Keyhole 6
tr5_entity_tbl[264] = {coll = 0x0001}; -- Keyhole 7
tr5_entity_tbl[265] = {coll = 0x0001}; -- Keyhole 8

-- SWITCHES

tr5_entity_tbl[266] = {coll = 0x0001}; -- Switch type 1
tr5_entity_tbl[267] = {coll = 0x0001}; -- Switch type 2
tr5_entity_tbl[268] = {coll = 0x0001}; -- Switch type 3
tr5_entity_tbl[269] = {coll = 0x0001}; -- Switch type 4
tr5_entity_tbl[270] = {coll = 0x0001}; -- Switch type 5
tr5_entity_tbl[271] = {coll = 0x0001}; -- Switch type 6
tr5_entity_tbl[272] = {coll = 0x0001}; -- Shoot switch 1
tr5_entity_tbl[273] = {coll = 0x0001}; -- Shoot switch 2
tr5_entity_tbl[274] = {coll = 0x0001}; -- Airlock switch
tr5_entity_tbl[275] = {coll = 0x0001}; -- Underwater switch 1 - UNUSED?
tr5_entity_tbl[276] = {coll = 0x0001}; -- Underwater switch 2 - UNUSED?
tr5_entity_tbl[277] = {coll = 0x0001}; -- Turn switch - UNUSED?
tr5_entity_tbl[278] = {coll = 0x0001}; -- Cog switch
tr5_entity_tbl[279] = {coll = 0x0001}; -- Lever switch - UNUSED?
tr5_entity_tbl[280] = {coll = 0x0001}; -- Jump switch
tr5_entity_tbl[281] = {coll = 0x0001}; -- Crowbar switch - UNUSED?
tr5_entity_tbl[282] = {coll = 0x0001}; -- Pulley
tr5_entity_tbl[283] = {coll = 0x0001}; -- Crowdove switch

-- DOORS

tr5_entity_tbl[284] = {coll = 0x0001}; -- Door 1
tr5_entity_tbl[285] = {coll = 0x0001}; -- Door 1 MIP
tr5_entity_tbl[286] = {coll = 0x0001}; -- Door 2
tr5_entity_tbl[287] = {coll = 0x0001}; -- Door 2 MIP
tr5_entity_tbl[288] = {coll = 0x0001}; -- Door 3
tr5_entity_tbl[289] = {coll = 0x0001}; -- Door 3 MIP
tr5_entity_tbl[290] = {coll = 0x0001}; -- Door 4
tr5_entity_tbl[291] = {coll = 0x0001}; -- Door 4 MIP
tr5_entity_tbl[292] = {coll = 0x0001}; -- Door 5
tr5_entity_tbl[293] = {coll = 0x0001}; -- Door 5 MIP
tr5_entity_tbl[294] = {coll = 0x0001}; -- Door 6
tr5_entity_tbl[295] = {coll = 0x0001}; -- Door 6 MIP
tr5_entity_tbl[296] = {coll = 0x0001}; -- Door 7
tr5_entity_tbl[297] = {coll = 0x0001}; -- Door 7 MIP
tr5_entity_tbl[298] = {coll = 0x0001}; -- Door 8
tr5_entity_tbl[299] = {coll = 0x0001}; -- Door 8 MIP
tr5_entity_tbl[300] = {coll = 0x0001}; -- Closed Door 1
tr5_entity_tbl[301] = {coll = 0x0001}; -- Closed Door 1 MIP
tr5_entity_tbl[302] = {coll = 0x0001}; -- Closed Door 2
tr5_entity_tbl[303] = {coll = 0x0001}; -- Closed Door 2 MIP
tr5_entity_tbl[304] = {coll = 0x0001}; -- Closed Door 3
tr5_entity_tbl[305] = {coll = 0x0001}; -- Closed Door 3 MIP
tr5_entity_tbl[306] = {coll = 0x0001}; -- Closed Door 4
tr5_entity_tbl[307] = {coll = 0x0001}; -- Closed Door 4 MIP
tr5_entity_tbl[308] = {coll = 0x0001}; -- Closed Door 5
tr5_entity_tbl[309] = {coll = 0x0001}; -- Closed Door 5 MIP
tr5_entity_tbl[310] = {coll = 0x0001}; -- Closed Door 6
tr5_entity_tbl[311] = {coll = 0x0001}; -- Closed Door 6 MIP
tr5_entity_tbl[312] = {coll = 0x0001}; -- Lift doors 1
tr5_entity_tbl[313] = {coll = 0x0001}; -- Lift doors 1 MIP
tr5_entity_tbl[314] = {coll = 0x0001}; -- Lift doors 2
tr5_entity_tbl[315] = {coll = 0x0001}; -- Lift doors 2 MIP
tr5_entity_tbl[316] = {coll = 0x0001}; -- Push-pull door 1 - UNUSED?
tr5_entity_tbl[317] = {coll = 0x0001}; -- Push-pull door 1 MIP - UNUSED?
tr5_entity_tbl[318] = {coll = 0x0001}; -- Push-pull door 2 - UNUSED?
tr5_entity_tbl[319] = {coll = 0x0001}; -- Push-pull door 2 - UNUSED?
tr5_entity_tbl[320] = {coll = 0x0001}; -- Kick door 1
tr5_entity_tbl[321] = {coll = 0x0001}; -- Kick door 1 MIP - UNUSED?
tr5_entity_tbl[322] = {coll = 0x0001}; -- Kick door 2
tr5_entity_tbl[323] = {coll = 0x0001}; -- Kick door 2 MIP - UNUSED?
tr5_entity_tbl[324] = {coll = 0x0001}; -- Underwater door
tr5_entity_tbl[325] = {coll = 0x0001}; -- Underwater door MIP - UNUSED?
tr5_entity_tbl[326] = {coll = 0x0001}; -- Double doors
tr5_entity_tbl[327] = {coll = 0x0001}; -- Double doors MIP
tr5_entity_tbl[328] = {coll = 0x0001}; -- Sequence door
tr5_entity_tbl[329] = {coll = 0x0001}; -- Sequence switch (door) 1
tr5_entity_tbl[330] = {coll = 0x0001}; -- Sequence switch (door) 2
tr5_entity_tbl[331] = {coll = 0x0001}; -- Sequence switch (door) 3
tr5_entity_tbl[332] = {coll = 0x0001}; -- Steel door

tr5_entity_tbl[333] = {coll = 0x0000}; -- God head - UNUSED, TR4 LEFTOVER

-- PICK-UP SUPPLY ITEMS

tr5_entity_tbl[334] = {coll = 0x0000}; -- Pistols
tr5_entity_tbl[335] = {coll = 0x0000}; -- Pistols ammo
tr5_entity_tbl[336] = {coll = 0x0000}; -- Uzis
tr5_entity_tbl[337] = {coll = 0x0000}; -- Uzis ammo
tr5_entity_tbl[338] = {coll = 0x0000}; -- Shotgun
tr5_entity_tbl[339] = {coll = 0x0000}; -- Shotgun shells 1
tr5_entity_tbl[340] = {coll = 0x0000}; -- Shotgun shells 2
tr5_entity_tbl[341] = {coll = 0x0000}; -- Grappling gun
tr5_entity_tbl[342] = {coll = 0x0000}; -- Grappling ammo type 1
tr5_entity_tbl[343] = {coll = 0x0000}; -- Grappling ammo type 2
tr5_entity_tbl[344] = {coll = 0x0000}; -- Grappling ammo type 3
tr5_entity_tbl[345] = {coll = 0x0000}; -- HK Gun
tr5_entity_tbl[346] = {coll = 0x0000}; -- HK ammo
tr5_entity_tbl[347] = {coll = 0x0000}; -- Revolver
tr5_entity_tbl[348] = {coll = 0x0000}; -- Revolver bullets
tr5_entity_tbl[349] = {coll = 0x0000}; -- Big Medi-Pack
tr5_entity_tbl[350] = {coll = 0x0000}; -- Small Medi-Pack
tr5_entity_tbl[351] = {coll = 0x0000}; -- Laser sight
tr5_entity_tbl[352] = {coll = 0x0000}; -- Binoculars
tr5_entity_tbl[353] = {coll = 0x0000}; -- Silencer
tr5_entity_tbl[354] = {coll = 0x0000}; -- Burning flare
tr5_entity_tbl[355] = {coll = 0x0000}; -- Flares
tr5_entity_tbl[356] = {coll = 0x0000}; -- Timex-TMX (Compass)

-- INVENTORY ITEMS

tr5_entity_tbl[357] = {coll = 0x0000}; -- Load inventory
tr5_entity_tbl[358] = {coll = 0x0000}; -- Save inventory
tr5_entity_tbl[359] = {coll = 0x0000}; -- Disk load
tr5_entity_tbl[360] = {coll = 0x0000}; -- Disk save
tr5_entity_tbl[361] = {coll = 0x0000}; -- Memcard load
tr5_entity_tbl[362] = {coll = 0x0000}; -- Memcard save

-- NULLMESHES, SERVICE OBJECTS AND EMITTERS

tr5_entity_tbl[363] = {coll = 0x0000, hide = 1}; -- Smoke emitter white - EMITTER
tr5_entity_tbl[364] = {coll = 0x0000, hide = 1}; -- Smoke emitter black - EMITTER
tr5_entity_tbl[365] = {coll = 0x0000, hide = 1}; -- Steam emitter - EMITTER
tr5_entity_tbl[366] = {coll = 0x0000, hide = 1}; -- Earthquake - SHAKE CAMERA
tr5_entity_tbl[367] = {coll = 0x0000, hide = 1}; -- Bubbles - EMITTER
tr5_entity_tbl[368] = {coll = 0x0000, hide = 1}; -- Waterfall Mist - EMITTER

tr5_entity_tbl[369] = {coll = 0x0000}; -- Gun shell - SPAWNED OBJECT, NO DIRECT USE
tr5_entity_tbl[370] = {coll = 0x0000}; -- Shotgun shell - SPAWNED OBJECT, NO DIRECT USE
tr5_entity_tbl[371] = {coll = 0x0000}; -- Gun flash - SPAWNED OBJECT, NO DIRECT USE

tr5_entity_tbl[372] = {coll = 0x0000, hide = 1}; -- Color light - UNUSED?
tr5_entity_tbl[373] = {coll = 0x0000, hide = 1}; -- Blinking light - UNUSED?
tr5_entity_tbl[374] = {coll = 0x0000, hide = 1}; -- Pulse light - DYNAMIC LIGHT
tr5_entity_tbl[375] = {coll = 0x0000, hide = 1}; -- Alarm light - DYNAMIC LIGHT
tr5_entity_tbl[376] = {coll = 0x0000, hide = 1}; -- Electrical light - DYNAMIC LIGHT
tr5_entity_tbl[377] = {coll = 0x0000, hide = 1}; -- Lens flare - UNUSED?

-- Remark: objects 378-386 are AI nullmeshes, and never show in game.

tr5_entity_tbl[378] = {hide = 1};
tr5_entity_tbl[379] = {hide = 1};
tr5_entity_tbl[380] = {hide = 1};
tr5_entity_tbl[381] = {hide = 1};
tr5_entity_tbl[382] = {hide = 1};
tr5_entity_tbl[383] = {hide = 1};
tr5_entity_tbl[384] = {hide = 1};
tr5_entity_tbl[385] = {hide = 1};
tr5_entity_tbl[386] = {hide = 1};

tr5_entity_tbl[387] = {coll = 0x0000, hide = 1}; -- Teleporter
tr5_entity_tbl[388] = {coll = 0x0000, hide = 1}; -- Lift teleporter

tr5_entity_tbl[389] = {coll = 0x0000}; -- Raising cog

tr5_entity_tbl[390] = {coll = 0x0000, hide = 1}; -- Laser
tr5_entity_tbl[391] = {coll = 0x0000, hide = 1}; -- Steam laser
tr5_entity_tbl[392] = {coll = 0x0000, hide = 1}; -- Floor laser 3

tr5_entity_tbl[393] = {coll = 0x0000, hide = 1}; -- Kill all triggers (or Laser 4?)
tr5_entity_tbl[394] = {coll = 0x0000, hide = 1}; -- Trigger Triggerer

-- MISC. SOLID OBJECTS (?)

tr5_entity_tbl[395] = {coll = 0x0001}; -- High object 1 (Polerope puzzle)
tr5_entity_tbl[396] = {coll = 0x0000}; -- High object 2 (Flame Emiter with sparks)
tr5_entity_tbl[397] = {coll = 0x0001}; -- Smash object 1 (Breakeable glass Floor)
tr5_entity_tbl[398] = {coll = 0x0001}; -- Smash object 2 (Breakeable glass Door)
tr5_entity_tbl[399] = {coll = 0x0001}; -- Smash object 3 - UNUSED?
tr5_entity_tbl[400] = {coll = 0x0001}; -- Smash object 4 - UNUSED?
tr5_entity_tbl[401] = {coll = 0x0001}; -- Smash object 5 - UNUSED?
tr5_entity_tbl[402] = {coll = 0x0001}; -- Smash object 6 - UNUSED?
tr5_entity_tbl[403] = {coll = 0x0001}; -- Smash object 7 - UNUSED?
tr5_entity_tbl[404] = {coll = 0x0001}; -- Smash object 8 - UNUSED?
tr5_entity_tbl[405] = {coll = 0x0001}; -- Meshswap 1
tr5_entity_tbl[406] = {coll = 0x0001}; -- Meshswap 2
tr5_entity_tbl[407] = {coll = 0x0001}; -- Meshswap 3

-- NULLMESHES

tr5_entity_tbl[408] = {coll = 0x0000}; -- Body part - UNUSED?
tr5_entity_tbl[409] = {coll = 0x0000, hide = 1}; -- Camera target

-- WATERFALLS (MESHES WITH UV-SCROLL ANIMATED TEXTURES)

tr5_entity_tbl[410] = {coll = 0x0000}; -- Waterfall 1
tr5_entity_tbl[411] = {coll = 0x0000}; -- Waterfall 2
tr5_entity_tbl[412] = {coll = 0x0000}; -- Waterfall 3
tr5_entity_tbl[413] = {coll = 0x0000}; -- Fishtank waterfall
tr5_entity_tbl[414] = {coll = 0x0000}; -- Waterfalls 1
tr5_entity_tbl[415] = {coll = 0x0000}; -- Waterfalls 2

-- ANIMATINGS

tr5_entity_tbl[416] = {coll = 0x0001}; -- Animating 1
tr5_entity_tbl[417] = {coll = 0x0001}; -- Animating 1 MIP
tr5_entity_tbl[418] = {coll = 0x0001}; -- Animating 2
tr5_entity_tbl[419] = {coll = 0x0001}; -- Animating 2 MIP
tr5_entity_tbl[420] = {coll = 0x0001}; -- Animating 3
tr5_entity_tbl[421] = {coll = 0x0001}; -- Animating 3 MIP
tr5_entity_tbl[422] = {coll = 0x0001}; -- Animating 4
tr5_entity_tbl[423] = {coll = 0x0001}; -- Animating 4 MIP
tr5_entity_tbl[424] = {coll = 0x0001}; -- Animating 5
tr5_entity_tbl[425] = {coll = 0x0001}; -- Animating 5 MIP
tr5_entity_tbl[426] = {coll = 0x0001}; -- Animating 6
tr5_entity_tbl[427] = {coll = 0x0001}; -- Animating 6 MIP
tr5_entity_tbl[428] = {coll = 0x0001}; -- Animating 7
tr5_entity_tbl[429] = {coll = 0x0001}; -- Animating 7 MIP
tr5_entity_tbl[430] = {coll = 0x0001}; -- Animating 8
tr5_entity_tbl[431] = {coll = 0x0001}; -- Animating 8 MIP
tr5_entity_tbl[432] = {coll = 0x0001}; -- Animating 9
tr5_entity_tbl[433] = {coll = 0x0001}; -- Animating 9 MIP
tr5_entity_tbl[434] = {coll = 0x0001}; -- Animating 10
tr5_entity_tbl[435] = {coll = 0x0001}; -- Animating 10 MIP
tr5_entity_tbl[436] = {coll = 0x0001}; -- Animating 11
tr5_entity_tbl[437] = {coll = 0x0001}; -- Animating 11 MIP
tr5_entity_tbl[438] = {coll = 0x0001}; -- Animating 12
tr5_entity_tbl[439] = {coll = 0x0001}; -- Animating 12 MIP
tr5_entity_tbl[440] = {coll = 0x0001}; -- Animating 13
tr5_entity_tbl[441] = {coll = 0x0001}; -- Animating 13 MIP
tr5_entity_tbl[442] = {coll = 0x0001}; -- Animating 14
tr5_entity_tbl[443] = {coll = 0x0001}; -- Animating 14 MIP
tr5_entity_tbl[444] = {coll = 0x0001}; -- Animating 15
tr5_entity_tbl[445] = {coll = 0x0001}; -- Animating 15 MIP
tr5_entity_tbl[446] = {coll = 0x0001}; -- Animating 16
tr5_entity_tbl[447] = {coll = 0x0001}; -- Animating 16 MIP

-- STATIC TERRAIN

tr5_entity_tbl[448] = {coll = 0x0001}; -- Bridge flat
tr5_entity_tbl[449] = {coll = 0x0001}; -- Bridge flat MIP
tr5_entity_tbl[450] = {coll = 0x0001}; -- Bridge tilt 1
tr5_entity_tbl[451] = {coll = 0x0001}; -- Bridge tilt 1 MIP
tr5_entity_tbl[452] = {coll = 0x0001}; -- Bridge tilt 2
tr5_entity_tbl[453] = {coll = 0x0001}; -- Bridge tilt 2 MIP


function GetEntityFlags(ver, id)
    tbl = {};
    if(ver < 3) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        tbl = tr1_entity_tbl;
    elseif(ver < 5) then                -- TR_II, TR_II_DEMO
        tbl = tr2_entity_tbl;
    elseif(ver < 6) then                -- TR_III
        tbl = tr3_entity_tbl;
    elseif(ver < 8) then                -- TR_IV, TR_IV_DEMO
        tbl = tr4_entity_tbl;
    elseif(ver < 9) then                -- TR_V
        tbl = tr5_entity_tbl;
    else
        return 0, 0;
    end;

    if(tbl[id] == nil) then
        return nil, nil;
    else
        return tbl[id].coll, tbl[id].hide;
    end;
end;
