-- OPENTOMB ENTITY PROPERTIES CONFIGURATION
-- By TeslaRus, Lwmte 2013-2014
print("entity_properties->loaded !");

--------------------------------------------------------------------------------
-- [ hide ] flag values:
--   0x00    - Object is visible.
--   0x01    - Object is invisible.

-- [ coll ] flag values:
-- COLLISION_NONE
-- COLLISION_GROUP_STATIC_OBLECT            -- static object - never moved
-- COLLISION_GROUP_KINEMATIC                -- doors and other moveable statics
-- COLLISION_GROUP_DYNAMICS                 -- hellow full physics interaction
-- COLLISION_GROUP_CHARACTERS               -- actor, enemies, NPC, animals
-- COLLISION_GROUP_VEHICLE                  -- car, moto, bike
-- COLLISION_GROUP_TRIGGERS                 -- no fix character position, but works in collision callbacks and interacts with dynamic objects

-- [ shape ] flag values:
-- COLLISION_SHAPE_BOX
-- COLLISION_SHAPE_BOX_BASE
-- COLLISION_SHAPE_SPHERE
-- COLLISION_SHAPE_TRIMESH
-- COLLISION_SHAPE_TRIMESH_CONVEX
-- COLLISION_SHAPE_SINGLE_BOX
-- COLLISION_SHAPE_SINGLE_SPHERE

-- [ func ] :
-- Function which spawns a scripted behaviour for ALL entities sharing the same
-- entity model ID.

--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
----------------------------- TR_I, TR_I_DEMO, TR_I_UB -------------------------
--------------------------------------------------------------------------------

tr1_entity_tbl = {};

-- NOTE: Objects before ID 06 are internal service objects and never appear in-game.

-- ACTORS --
tr1_entity_tbl[0]  = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "Lara"};                            -- Lara
tr1_entity_tbl[06] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_lara_mutant"};                 -- Doppelgagner (alias Lara Mutant)
tr1_entity_tbl[07] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_wolf"};                        -- Wolf
tr1_entity_tbl[08] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_bear"};                        -- Bear
tr1_entity_tbl[09] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_bat"};                         -- Bat
tr1_entity_tbl[10] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_crocodile"};                   -- Crocodile (land)
tr1_entity_tbl[11] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_crocodile"};                   -- Crocodile (water)
tr1_entity_tbl[12] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_lion"};                        -- Lion Male
tr1_entity_tbl[13] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_lion"};                        -- Lion Female
tr1_entity_tbl[14] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_puma"};                        -- Puma
tr1_entity_tbl[15] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_gorilla"};                     -- Gorilla
tr1_entity_tbl[16] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_rat"};                         -- Rat (land)
tr1_entity_tbl[17] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_rat"};                         -- Rat (water)
tr1_entity_tbl[18] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_trex"};                        -- T-Rex
tr1_entity_tbl[19] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_raptor"};                      -- Raptor
tr1_entity_tbl[20] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_winged_mutant"};               -- Winged mutant
tr1_entity_tbl[21] = {coll = COLLISION_NONE,                 shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_mutant_spawner", hide = 0x01}; -- mutant spawner
tr1_entity_tbl[22] = {coll = COLLISION_NONE,                 shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_mummy_spawner",  hide = 0x01}; -- mummy spawner
tr1_entity_tbl[23] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_centaur"};                     -- Centaur
tr1_entity_tbl[24] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX};                                           -- Mummy
tr1_entity_tbl[25] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX};                                           -- DinoWarrior (UNUSED!)
tr1_entity_tbl[26] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX};                                           -- Fish
tr1_entity_tbl[27] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_larson"};                      -- Larson
tr1_entity_tbl[28] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_pierre"};                      -- Pierre
tr1_entity_tbl[29] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX};                                           -- Skateboard
tr1_entity_tbl[30] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_skateboardist"};               -- Skateboard Kid
tr1_entity_tbl[31] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_cowboy"};                      -- Cowboy
tr1_entity_tbl[32] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_mrt"};                         -- Mr. T
tr1_entity_tbl[33] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_natla"};                       -- Natla (winged)
tr1_entity_tbl[34] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr1_torsoboss"};                   -- Torso Boss

-- ANIMATINGS --

tr1_entity_tbl[35] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_SINGLE_BOX, func = "fallblock"};  -- Falling floor
tr1_entity_tbl[36] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "swingblade"}; -- Swinging blade (Vilcabamba, etc.)
tr1_entity_tbl[37] = {coll = COLLISION_GROUP_TRIGGERS,  shape = COLLISION_SHAPE_BOX, func = "oldspike"};   -- Spikes
tr1_entity_tbl[38] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_SINGLE_SPHERE, func = "boulder_heavy"};         -- Boulder
tr1_entity_tbl[39] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                                       -- Dart
tr1_entity_tbl[40] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "dart_emitter"};                -- Dartgun
tr1_entity_tbl[41] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "anim_single"};                 -- Lifting door
tr1_entity_tbl[42] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "gen_trap"};                        -- Slamming sawtooth doors
tr1_entity_tbl[43] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "damocles"};                    -- Sword of Damocles
tr1_entity_tbl[44] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "Thor_hummer"};                 -- Thor's hammer (handle)
tr1_entity_tbl[45] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                                       -- Thor's hammer (block) - spawned by script
tr1_entity_tbl[46] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH };                                      -- Thor's lightning ball
tr1_entity_tbl[47] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "anim_single"};                 -- Barricade
tr1_entity_tbl[48] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"};         -- Pushable block
tr1_entity_tbl[49] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"};         -- Pushable block
tr1_entity_tbl[50] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"};         -- Pushable block
tr1_entity_tbl[51] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"};         -- Pushable block
tr1_entity_tbl[52] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "tallblock"};                    -- Moving block
tr1_entity_tbl[53] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "fallceiling"};                      -- Falling ceiling
tr1_entity_tbl[54] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                  -- Sword of Damocles (unused?)
tr1_entity_tbl[55] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"};     -- Wall switch (lever)
tr1_entity_tbl[56] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"};     -- Underwater switch (lever)

-- DOORS --

tr1_entity_tbl[57] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};       -- Door 1
tr1_entity_tbl[58] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};       -- Door 2
tr1_entity_tbl[59] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};       -- Door 3
tr1_entity_tbl[60] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX,     func = "door"};       -- Door 4
tr1_entity_tbl[61] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};       -- Door 5
tr1_entity_tbl[62] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};       -- Door 6
tr1_entity_tbl[63] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};       -- Door 7
tr1_entity_tbl[64] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};       -- Door 8

tr1_entity_tbl[65] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};       -- Floor trapdoor
tr1_entity_tbl[66] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};       -- Floor trapdoor

-- COLLISION OBJECTS --

tr1_entity_tbl[68] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                      -- Bridge flat
tr1_entity_tbl[69] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                      -- Bridge tilt 1
tr1_entity_tbl[70] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                      -- Bridge tilt 2

-- MENU ITEMS AND ANIMATINGS --

tr1_entity_tbl[71] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                      -- Menu: Passport
tr1_entity_tbl[72] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                      -- Menu: Compass
tr1_entity_tbl[73] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                      -- Menu: Lara's Home photo

tr1_entity_tbl[74] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "anim_single"};         -- Animating 1
tr1_entity_tbl[75] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "anim_single"};         -- Animating 2
tr1_entity_tbl[76] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "anim_single"};         -- Animating 3

tr1_entity_tbl[81] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                                -- Menu: Passport (closed)

tr1_entity_tbl[82] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                                -- Natla Logo
tr1_entity_tbl[83] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_TRIMESH, func = "save_crystal"};     -- Savegame crystal

tr1_entity_tbl[95] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                        -- Menu: Sunglasses
tr1_entity_tbl[96] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                        -- Menu: Cassette player
tr1_entity_tbl[97] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                        -- Menu: Arrow keys
tr1_entity_tbl[98] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                        -- Menu: Flashlight (UNUSED!)

-- GENERAL MENU ITEMS --

tr1_entity_tbl[99] =  {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Menu: Pistols
tr1_entity_tbl[100] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Menu: Shotgun
tr1_entity_tbl[101] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Menu: Magnums
tr1_entity_tbl[102] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Menu: Uzis
tr1_entity_tbl[103] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Menu: Pistol ammo
tr1_entity_tbl[104] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Menu: Shotgun ammo
tr1_entity_tbl[105] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Menu: Magnum ammo
tr1_entity_tbl[106] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Menu: Uzi ammo
tr1_entity_tbl[107] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Menu: Grenade ammo (UNUSED!)
tr1_entity_tbl[108] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Pick-up: Small medipack
tr1_entity_tbl[109] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Pick-up: Large medipack

-- PUZZLE PICKUPS --

tr1_entity_tbl[114] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Pick-up: Puzzle 1
tr1_entity_tbl[115] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Pick-up: Puzzle 2
tr1_entity_tbl[116] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Pick-up: Puzzle 3
tr1_entity_tbl[117] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Pick-up: Puzzle 4

-- PUZZLES, KEYS AND SLOTS FOR THEM --

tr1_entity_tbl[118] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};    -- Slot 1 empty
tr1_entity_tbl[119] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};    -- Slot 2 empty
tr1_entity_tbl[120] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};    -- Slot 3 empty
tr1_entity_tbl[121] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};    -- Slot 4 empty
tr1_entity_tbl[122] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                      -- Slot 1 full
tr1_entity_tbl[123] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                      -- Slot 2 full
tr1_entity_tbl[124] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                      -- Slot 3 full
tr1_entity_tbl[125] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                      -- Slot 4 full

tr1_entity_tbl[127] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};                      -- Puzzle item 1
tr1_entity_tbl[128] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX, hide = 0x01, func = "midastouch"};        -- Midas gold touch

tr1_entity_tbl[133] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Key 1
tr1_entity_tbl[134] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Key 2
tr1_entity_tbl[135] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Key 3
tr1_entity_tbl[136] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};    -- Key 4

tr1_entity_tbl[137] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};    -- Lock 1
tr1_entity_tbl[138] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};    -- Lock 2
tr1_entity_tbl[139] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};    -- Lock 3
tr1_entity_tbl[140] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};    -- Lock 4

tr1_entity_tbl[145] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};              -- Scion 1
tr1_entity_tbl[146] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Scion 2
tr1_entity_tbl[147] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_SINGLE_BOX, func = "ScionHolder"};  -- Scion holder
tr1_entity_tbl[150] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX, func = "scion"};               -- Scion piece

-- ANIMATINGS --

tr1_entity_tbl[161] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "centaur_statue"};  -- Centaur statue
--tr1_entity_tbl[162] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "natla_cabin"};     -- Natla's mines cabin
tr1_entity_tbl[163] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "tr1_mutant_egg"};      -- Mutant egg

-- SERVICE OBJECTS --

tr1_entity_tbl[166] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                     -- Gunflash
tr1_entity_tbl[169] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};        -- Camera target
tr1_entity_tbl[170] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};        -- Waterfall mist

tr1_entity_tbl[172] = {coll = COLLISION_GROUP_TRIGGERS, shape = COLLISION_SHAPE_BOX};                     -- Mutant bullet
tr1_entity_tbl[173] = {coll = COLLISION_GROUP_TRIGGERS, shape = COLLISION_SHAPE_BOX};                     -- Mutant grenade

tr1_entity_tbl[177] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};        -- Lava particle emitter

tr1_entity_tbl[179] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};        -- Flame emitter
tr1_entity_tbl[180] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "spikewall"};                     -- Moving lava mass
tr1_entity_tbl[181] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "tr1_mutant_boss_egg"};               -- Mutant egg (big)
tr1_entity_tbl[182] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                     -- Motorboat

tr1_entity_tbl[183] = {coll = COLLISION_NONE, shape = COLLISION_SHAPE_TRIMESH, hide = 0x01};        -- Earthquake

--------------------------------------------------------------------------------
--------------------------------- TR_II, TR_II_DEMO ----------------------------
--------------------------------------------------------------------------------
tr2_entity_tbl = {};

-- Remark: object IDs 0-12 are used for Lara model and never show in game
-- independently.

tr2_entity_tbl[0] = {coll = COLLISION_GROUP_CHARACTERS,      shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "Lara"};                        -- Lara

-- VEHICLES --

tr2_entity_tbl[13] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Red snowmobile (can go fast)
tr2_entity_tbl[14] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Boat
tr2_entity_tbl[51] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Black snowmobile (with guns)

-- ACTORS --

tr2_entity_tbl[15] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Doberman
tr2_entity_tbl[16] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "MaskedGoon"};            -- Masked goon (white mask, jacket)
tr2_entity_tbl[17] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "MaskedGoon"};            -- Masked goon (white mask, vest)
tr2_entity_tbl[18] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "MaskedGoon"};            -- Masked goon (black mask)
tr2_entity_tbl[19] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Knifethrower
tr2_entity_tbl[20] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Shotgun goon
tr2_entity_tbl[21] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Rat
tr2_entity_tbl[22] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX};                -- Dragon (front)
tr2_entity_tbl[23] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX};                -- Dragon (back)
tr2_entity_tbl[24] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX};                -- Gondola (Venetian boat)
tr2_entity_tbl[25] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Shark
tr2_entity_tbl[26] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Yellow moray eel
tr2_entity_tbl[27] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Black moray eel
tr2_entity_tbl[28] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Barracuda / Whiskered Fish
tr2_entity_tbl[29] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Scuba diver
tr2_entity_tbl[30] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Gun-wielding rig worker (khaki pants)
tr2_entity_tbl[31] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Gun-wielding rig worker (blue jeans)
tr2_entity_tbl[32] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Stick-wielding goon
tr2_entity_tbl[33] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Stick-wielding goon (can't climb)
tr2_entity_tbl[34] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Flamethrower-wielding goon
tr2_entity_tbl[36] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Spider
tr2_entity_tbl[37] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Giant spider
tr2_entity_tbl[38] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Crow
tr2_entity_tbl[39] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "tr2_tiger"};            -- Tiger / Leopard
tr2_entity_tbl[40] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Marco Bartoli
tr2_entity_tbl[41] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Spear-wielding Xian Guard
tr2_entity_tbl[42] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX};                -- Spear-wielding Xian Guard statue
tr2_entity_tbl[43] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Sword-wielding Xian Guard
tr2_entity_tbl[44] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX};                -- Sword-wielding Xian Guard statue
tr2_entity_tbl[45] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Yeti
tr2_entity_tbl[46] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Bird monster (guards Talion)
tr2_entity_tbl[47] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Eagle
tr2_entity_tbl[48] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Mercenary
tr2_entity_tbl[49] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Mercenary (black ski mask, gray jacket)
tr2_entity_tbl[50] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Mercenary (black ski mask, brown jacket)
tr2_entity_tbl[52] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Mercenary snowmobile driver
tr2_entity_tbl[53] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Monk with long stick
tr2_entity_tbl[54] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};                -- Monk with knife-end stick

-- TRAPS --

tr2_entity_tbl[55] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_SINGLE_BOX, func = "fallblock"};    -- Collapsible floor
tr2_entity_tbl[57] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                           -- Loose boards
tr2_entity_tbl[58] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Swinging sandbag / spiky ball
tr2_entity_tbl[59] = {coll = COLLISION_GROUP_TRIGGERS,     shape = COLLISION_SHAPE_BOX, func = "oldspike"};         -- Spikes / Glass shards
tr2_entity_tbl[60] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "boulder"};  -- Boulder
tr2_entity_tbl[61] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Disk (like dart)
tr2_entity_tbl[62] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                                -- Wall-mounted disk shooter (like dartgun)
tr2_entity_tbl[63] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                           -- Drawbridge
tr2_entity_tbl[64] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "gen_trap"};            -- Slamming door
tr2_entity_tbl[65] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                           -- Elevator
tr2_entity_tbl[66] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Minisub
tr2_entity_tbl[67] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"};            -- Movable cubical block (pushable)
tr2_entity_tbl[68] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"};            -- Movable cubical block (pushable)
tr2_entity_tbl[69] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"};            -- Movable cubical block (pushable)
tr2_entity_tbl[70] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"};            -- Movable cubical block (pushable)
tr2_entity_tbl[71] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Big bowl (Ice Palace)
tr2_entity_tbl[72] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "breakable_window"};    -- Breakable window (can shoot out)
tr2_entity_tbl[73] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "breakable_window_jmp"};-- Breakable window (must jump through)
tr2_entity_tbl[76] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "propeller"};           -- Airplane propeller
tr2_entity_tbl[77] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Power saw
tr2_entity_tbl[78] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Overhead pulley hook
tr2_entity_tbl[79] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "fallceiling"};         -- Sandbag / Ceiling fragments
tr2_entity_tbl[80] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Rolling spindle
tr2_entity_tbl[81] = {coll = COLLISION_GROUP_TRIGGERS,     shape = COLLISION_SHAPE_BOX, func = "wallblade"};        -- Wall-mounted knife blade
tr2_entity_tbl[82] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Statue with knife blade
tr2_entity_tbl[83] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH_CONVEX};                    -- Multiple boulders / snowballs
tr2_entity_tbl[84] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                               -- Detachable icicles
tr2_entity_tbl[85] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "spikewall"};           -- Spiky movable wall
tr2_entity_tbl[86] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                           -- Bounce pad
tr2_entity_tbl[87] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "spikeceiling"};        -- Spiky ceiling segment
tr2_entity_tbl[88] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                       -- Tibetan bell
tr2_entity_tbl[91] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                        -- Lara and a snowmobile
tr2_entity_tbl[92] = {coll = COLLISION_GROUP_KINEMATIC,shape = COLLISION_SHAPE_BOX, func = "WheelKnob"};    -- Wheel knob
tr2_entity_tbl[93] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "switch"};       -- Switch
tr2_entity_tbl[94] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "propeller"};   -- Underwater propeller
tr2_entity_tbl[95] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                       -- Air fan
tr2_entity_tbl[96] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                       -- Swinging box / spiky ball
tr2_entity_tbl[101] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                      -- Rolling storage drums
tr2_entity_tbl[102] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX, func = "zipline"};    -- Zipline handle
tr2_entity_tbl[103] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX, func = "switch"};     -- Switch
tr2_entity_tbl[104] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX, func = "switch"};     -- Switch
tr2_entity_tbl[105] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX, func = "switch"};     -- Underwater switch

-- DOORS --

tr2_entity_tbl[106] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};     -- Door
tr2_entity_tbl[107] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};     -- Door
tr2_entity_tbl[108] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};     -- Door
tr2_entity_tbl[109] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};     -- Door
tr2_entity_tbl[110] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};     -- Door
tr2_entity_tbl[111] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "door"};     -- Door (pulled upward in Temple of Xian)
tr2_entity_tbl[112] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "door"};     -- Door (pulled upward in Temple of Xian)
tr2_entity_tbl[113] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "door"};     -- Door (pulled upward)
tr2_entity_tbl[114] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};     -- Trapdoor (opens downward)
tr2_entity_tbl[115] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};     -- Trapdoor (opens downward)
tr2_entity_tbl[116] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};     -- Trapdoor (opens downward)
tr2_entity_tbl[117] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                    -- Bridge (flat)
tr2_entity_tbl[118] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                    -- Bridge (slope = 1)
tr2_entity_tbl[119] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                    -- Bridge (slope = 2)

-- MISC ITEMS --

tr2_entity_tbl[120] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Secret #1
tr2_entity_tbl[121] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Secret #2
tr2_entity_tbl[122] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Lara and butler picture
tr2_entity_tbl[133] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Secret #3
tr2_entity_tbl[134] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Natla's logo
tr2_entity_tbl[152] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Flare
tr2_entity_tbl[153] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Sunglasses
tr2_entity_tbl[154] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Portable CD player
tr2_entity_tbl[155] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Direction keys
tr2_entity_tbl[157] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Pistol
tr2_entity_tbl[158] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Shotgun
tr2_entity_tbl[159] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Auto-pistol
tr2_entity_tbl[160] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Uzi
tr2_entity_tbl[161] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Harpoon gun
tr2_entity_tbl[162] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- M16
tr2_entity_tbl[163] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Grenade launcher
tr2_entity_tbl[164] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Pistol ammo(?)
tr2_entity_tbl[165] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Shotgun ammo
tr2_entity_tbl[166] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Auto-pistol ammo
tr2_entity_tbl[167] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Uzi ammo
tr2_entity_tbl[168] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Harpoons
tr2_entity_tbl[169] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- M16 ammo
tr2_entity_tbl[170] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Grenades
tr2_entity_tbl[171] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Small medipack
tr2_entity_tbl[172] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Large medipack
tr2_entity_tbl[173] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Flares (opening box)
tr2_entity_tbl[178] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Puzzle 1
tr2_entity_tbl[179] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Puzzle 2
tr2_entity_tbl[180] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Puzzle 3 ?
tr2_entity_tbl[181] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Puzzle 4
tr2_entity_tbl[182] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"};             -- Slot 1 empty
tr2_entity_tbl[183] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"};             -- Slot 2 empty
tr2_entity_tbl[184] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"};             -- Slot 3 empty ?
tr2_entity_tbl[185] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"};             -- Slot 4 empty
tr2_entity_tbl[186] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Slot 1 full
tr2_entity_tbl[187] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Slot 2 full
tr2_entity_tbl[188] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Slot 3 full ?
tr2_entity_tbl[189] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Slot 4 full
tr2_entity_tbl[197] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Key 1
tr2_entity_tbl[198] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Key 2
tr2_entity_tbl[199] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Key 3
tr2_entity_tbl[200] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Key 4
tr2_entity_tbl[201] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"};             -- Lock 1
tr2_entity_tbl[202] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"};             -- Lock 2
tr2_entity_tbl[203] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"};             -- Lock 3
tr2_entity_tbl[204] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"};             -- Lock 4
tr2_entity_tbl[207] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Pickup 5
tr2_entity_tbl[208] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Pickup 6
tr2_entity_tbl[209] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Dragon explosion effect (expanding netted bubble)
tr2_entity_tbl[210] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Dragon explosion effect (expanding netted bubble)
tr2_entity_tbl[211] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Dragon explosion effect (expanding solid bubble)
tr2_entity_tbl[212] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "alarm_TR2"};           -- Alarm
tr2_entity_tbl[213] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01, func = "drips"};  -- Dripping water
tr2_entity_tbl[214] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                              -- Tyrannosaur
tr2_entity_tbl[215] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01, func = "venicebird"};  -- Singing birds
tr2_entity_tbl[216] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Placeholder
tr2_entity_tbl[217] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Placeholder
tr2_entity_tbl[218] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};               -- Dragon bones (front)
tr2_entity_tbl[219] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};               -- Dragon bones (back)
tr2_entity_tbl[222] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};               -- Aquatic Mine (Venice)
tr2_entity_tbl[223] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};               -- Menu background
tr2_entity_tbl[225] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};               -- Gong-hammering animation
tr2_entity_tbl[226] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};               -- Gong (Ice Palace)
tr2_entity_tbl[227] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};               -- Detonator box
tr2_entity_tbl[228] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "heli_rig_TR2"};               -- Helicopter (Diving Area)
tr2_entity_tbl[235] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};               -- Flare burning?
tr2_entity_tbl[240] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};               -- Gunflare
tr2_entity_tbl[241] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};               -- Gunflare (M16)
tr2_entity_tbl[243] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Camera target
tr2_entity_tbl[244] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Camera target - 2 (?)
tr2_entity_tbl[245] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Harpoon (single)
tr2_entity_tbl[247] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Pointer?
tr2_entity_tbl[248] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Grenade (single)
tr2_entity_tbl[249] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Harpoon (flying)
tr2_entity_tbl[251] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Sparks
tr2_entity_tbl[253] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Fire
tr2_entity_tbl[254] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};               -- Skybox
tr2_entity_tbl[256] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Monk
tr2_entity_tbl[257] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01, func = "doorbell"}   -- Door bell
tr2_entity_tbl[258] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01, func = "alarmbell"}   -- Alarm bell
tr2_entity_tbl[259] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "heli_TR2"};               -- Helicopter
tr2_entity_tbl[260] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};               -- The butler
tr2_entity_tbl[262] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Lara cutscene placement?
tr2_entity_tbl[263] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};               -- Shotgun animation (Home Sweet Home)
tr2_entity_tbl[264] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Dragon transform wave


--------------------------------------------------------------------------------
-------------------------------------- TR_III ----------------------------------
--------------------------------------------------------------------------------
tr3_entity_tbl = {};

-- NOTE: Objects before ID 12 are internal service objects and never appear in-game.

tr3_entity_tbl[0] = {coll = COLLISION_GROUP_CHARACTERS,      shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "Lara"};                        -- Lara

-- VEHICLES --

tr3_entity_tbl[12] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- UPV
tr3_entity_tbl[14] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Kayak
tr3_entity_tbl[15] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Inflatable boat
tr3_entity_tbl[16] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Quadbike
tr3_entity_tbl[17] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Mine cart
tr3_entity_tbl[18] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Big gun

-- ACTORS --

tr3_entity_tbl[19] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};               -- Hydro propeller (?)
tr3_entity_tbl[20] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Tribesman with spiked axe
tr3_entity_tbl[21] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Tribesman with poison-dart gun
tr3_entity_tbl[22] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Dog
tr3_entity_tbl[23] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Rat
tr3_entity_tbl[24] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};     -- Kill All Triggers
tr3_entity_tbl[25] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Killer whale
tr3_entity_tbl[26] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Scuba diver
tr3_entity_tbl[27] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Crow
tr3_entity_tbl[28] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Tiger
tr3_entity_tbl[29] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Vulture
tr3_entity_tbl[30] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Assault-course target
tr3_entity_tbl[31] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Crawler mutant in closet
tr3_entity_tbl[32] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Crocodile (in water)
tr3_entity_tbl[34] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Compsognathus
tr3_entity_tbl[35] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Lizard thing
tr3_entity_tbl[36] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Puna guy
tr3_entity_tbl[37] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Mercenary
tr3_entity_tbl[38] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Raptor hung by rope (fish bait)
tr3_entity_tbl[39] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- RX-Tech guy in red jacket
tr3_entity_tbl[40] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- RX-Tech guy with gun (dressed like flamethrower guy)
tr3_entity_tbl[41] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Dog (Antarctica)
tr3_entity_tbl[42] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Crawler mutant
tr3_entity_tbl[44] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Tinnos wasp
tr3_entity_tbl[45] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Tinnos monster
tr3_entity_tbl[46] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Brute mutant (with claw)
tr3_entity_tbl[47] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Tinnos wasp respawn point
tr3_entity_tbl[48] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Raptor respawn point
tr3_entity_tbl[49] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Willard spider
tr3_entity_tbl[50] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- RX-Tech flamethrower guy
tr3_entity_tbl[51] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- London goon
tr3_entity_tbl[53] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- 'Damned' stick-wielding goon
tr3_entity_tbl[56] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- London guard
tr3_entity_tbl[57] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Sophia Lee
tr3_entity_tbl[58] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "cleaner"};              -- Thames Wharf machine
tr3_entity_tbl[60] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- MP with stick
tr3_entity_tbl[61] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- MP with gun
tr3_entity_tbl[62] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Prisoner
tr3_entity_tbl[63] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- MP with sighted gun and night sight
tr3_entity_tbl[64] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};               -- Gun turret
tr3_entity_tbl[65] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Dam guard
tr3_entity_tbl[66] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Kind of tripwire
tr3_entity_tbl[67] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Electrified wire
tr3_entity_tbl[68] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Killer tripwire
tr3_entity_tbl[69] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Cobra / Rattlesnake
tr3_entity_tbl[70] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};               -- Temple statue
tr3_entity_tbl[71] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Monkey
tr3_entity_tbl[73] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "basic"};               -- Tony Firehands

-- AI OBJECTS --

tr3_entity_tbl[74] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- AI Guard
tr3_entity_tbl[75] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- AI Ambush
tr3_entity_tbl[76] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- AI Path
tr3_entity_tbl[77] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- AI Unknown #77
tr3_entity_tbl[78] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- AI Follow
tr3_entity_tbl[79] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- AI Patrol
tr3_entity_tbl[80] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Unknown Id #80
tr3_entity_tbl[81] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Unknown Id #81
tr3_entity_tbl[82] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};  -- Unknown Id #82

-- TRAPS & DOORS --

tr3_entity_tbl[83] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_SINGLE_BOX, func = "fallblock"};                 -- Collapsible floor
tr3_entity_tbl[86] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                 -- Swinging thing
tr3_entity_tbl[87] = {coll = COLLISION_GROUP_TRIGGERS,     shape = COLLISION_SHAPE_BOX, func = "oldspike"};                 -- Spikes / Barbed wire
tr3_entity_tbl[88] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "boulder"};                 -- Boulder / Barrel
tr3_entity_tbl[89] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "boulder"};                 -- Giant boulder (Temple of Puna)
tr3_entity_tbl[90] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                 -- Disk (like dart)
tr3_entity_tbl[91] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, hide = 0x01};                 -- Dart shooter
tr3_entity_tbl[94] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "gen_trap"};                 -- Spiked impaled skeleton / Slamming door
tr3_entity_tbl[97] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushable"};    -- Movable cubical block (pushable)
tr3_entity_tbl[98] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushable"};    -- Movable cubical block (pushable)
tr3_entity_tbl[101] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Destroyable boarded-up window
tr3_entity_tbl[102] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Destroyable boarded-up window / wall
tr3_entity_tbl[106] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Overhead pulley hook
tr3_entity_tbl[107] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "fallceiling"};                -- Falling fragments
tr3_entity_tbl[108] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Rolling spindle
tr3_entity_tbl[110] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Subway train
tr3_entity_tbl[111] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "wallblade"};                -- Wall-mounted knife blade / Knife disk
tr3_entity_tbl[113] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Detachable stalactites
tr3_entity_tbl[114] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "spikewall"};                -- Spiky movable wall
tr3_entity_tbl[116] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "spikeceiling"};                -- Spiky movable vertical wall / Tunnel borer
tr3_entity_tbl[117] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Valve wheel / Pulley
tr3_entity_tbl[118] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"};                -- Switch
tr3_entity_tbl[119] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "propeller"};                -- Underwater propeller / Diver sitting on block / Underwater rotating knives / Meteorite
tr3_entity_tbl[120] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "propeller"};                -- Fan
tr3_entity_tbl[121] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Heavy stamper / Grinding drum / Underwater rotating knives
tr3_entity_tbl[122] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Temple statue (original petrified state)
tr3_entity_tbl[123] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                -- Monkey with medipack
tr3_entity_tbl[124] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                -- Monkey with key
tr3_entity_tbl[127] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX,     func = "zipline"};               -- Zipline handle
tr3_entity_tbl[128] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"};                -- Switch
tr3_entity_tbl[129] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"};                -- Switch
tr3_entity_tbl[130] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"};                -- Underwater switch
tr3_entity_tbl[131] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Door
tr3_entity_tbl[132] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Door
tr3_entity_tbl[133] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Door
tr3_entity_tbl[134] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Door
tr3_entity_tbl[135] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Door
tr3_entity_tbl[136] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Door
tr3_entity_tbl[137] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Door
tr3_entity_tbl[138] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Door
tr3_entity_tbl[139] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Trapdoor
tr3_entity_tbl[140] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Trapdoor
tr3_entity_tbl[141] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"};                -- Trapdoor
tr3_entity_tbl[142] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Bridge (flat)
tr3_entity_tbl[143] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Bridge (slope = 1)
tr3_entity_tbl[144] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Bridge (slope = 2)

-- PICK-UPS --

tr3_entity_tbl[145] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Passport (opening up)
tr3_entity_tbl[146] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Stopwatch
tr3_entity_tbl[147] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Lara's Home photo
tr3_entity_tbl[158] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Passport (closed)
tr3_entity_tbl[159] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                               -- Natla logo
tr3_entity_tbl[160] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Pistols (pick-up)
tr3_entity_tbl[161] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Shotgun (pick-up)
tr3_entity_tbl[162] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Desert Eagle (pick-up)
tr3_entity_tbl[163] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Uzis (pick-up)
tr3_entity_tbl[164] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Harpoon gun (pick-up)
tr3_entity_tbl[165] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- MP5 (pick-up)
tr3_entity_tbl[166] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Rocket launcher (pick-up)
tr3_entity_tbl[167] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Grenade launcher (pick-up)
tr3_entity_tbl[168] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Pistol ammo (pick-up)
tr3_entity_tbl[169] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Shotgun ammo (pick-up)
tr3_entity_tbl[170] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Desert Eagle ammo (pick-up)
tr3_entity_tbl[171] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Uzi ammo (pick-up)
tr3_entity_tbl[172] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Harpoons (pick-up)
tr3_entity_tbl[173] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- MP5 ammo (pick-up)
tr3_entity_tbl[174] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Rockets (pick-up)
tr3_entity_tbl[175] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Grenades (pick-up)
tr3_entity_tbl[176] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Small medipack (pick-up)
tr3_entity_tbl[177] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Large medipack (pick-up)
tr3_entity_tbl[178] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Flares (pick-up)
tr3_entity_tbl[179] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};            -- Flare (pick-up)
tr3_entity_tbl[180] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "crystal_TR3"};         -- Savegame crystal (pick-up)

-- MENU ITEMS --

tr3_entity_tbl[181] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Sunglasses
tr3_entity_tbl[182] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Portable CD Player
tr3_entity_tbl[183] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Direction keys
tr3_entity_tbl[184] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Globe (for indicating destinations)
tr3_entity_tbl[185] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Pistols
tr3_entity_tbl[186] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Shotgun
tr3_entity_tbl[187] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Desert Eagle
tr3_entity_tbl[188] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Uzis
tr3_entity_tbl[189] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Harpoon gun
tr3_entity_tbl[190] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- MP5
tr3_entity_tbl[191] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Rocket launcher
tr3_entity_tbl[192] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Grenade launcher
tr3_entity_tbl[193] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Pistol ammo
tr3_entity_tbl[194] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Shotgun ammo
tr3_entity_tbl[195] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Desert Eagle ammo
tr3_entity_tbl[196] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Uzi ammo
tr3_entity_tbl[197] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Harpoons
tr3_entity_tbl[198] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- MP5 ammo
tr3_entity_tbl[199] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Rockets
tr3_entity_tbl[200] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Grenades
tr3_entity_tbl[201] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Small medipack
tr3_entity_tbl[202] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Large medipack
tr3_entity_tbl[203] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Flares
tr3_entity_tbl[204] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Savegame crystal

-- PUZZLE ITEMS & KEYS --

tr3_entity_tbl[205] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Puzzle 1 (pick-up)
tr3_entity_tbl[206] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Puzzle 2 (pick-up)
tr3_entity_tbl[207] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Puzzle 3 (pick-up)
tr3_entity_tbl[208] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Puzzle 4 (pick-up)
tr3_entity_tbl[209] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Puzzle 1 (menu item)
tr3_entity_tbl[210] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Puzzle 2 (menu item)
tr3_entity_tbl[211] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Puzzle 3 (menu item)
tr3_entity_tbl[212] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Puzzle 4 (menu item)
tr3_entity_tbl[213] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};        -- Slot 1 empty
tr3_entity_tbl[214] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};        -- Slot 2 empty
tr3_entity_tbl[215] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};        -- Slot 3 empty
tr3_entity_tbl[216] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};        -- Slot 4 empty
tr3_entity_tbl[217] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                          -- Slot 1 full
tr3_entity_tbl[218] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                          -- Slot 2 full
tr3_entity_tbl[219] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                          -- Slot 3 full
tr3_entity_tbl[220] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                          -- Slot 4 full
tr3_entity_tbl[224] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Key 1 (pick-up)
tr3_entity_tbl[225] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Key 2 (pick-up)
tr3_entity_tbl[226] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Key 3 (pick-up)
tr3_entity_tbl[227] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Key 4 (pick-up)
tr3_entity_tbl[228] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Key 1 (menu item)
tr3_entity_tbl[229] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Key 2 (menu item)
tr3_entity_tbl[230] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Key 3 (menu item)
tr3_entity_tbl[231] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Key 4 (menu item)
tr3_entity_tbl[232] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};        -- Lock 1
tr3_entity_tbl[233] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};        -- Lock 2
tr3_entity_tbl[234] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};        -- Lock 3
tr3_entity_tbl[235] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"};        -- Lock 4
tr3_entity_tbl[236] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Pickup 1 (pick-up)
tr3_entity_tbl[237] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Pickup 2 .unused  (pick-up)
tr3_entity_tbl[238] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Pickup 1 (menu item)
tr3_entity_tbl[239] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Pickup 2 .unused  (menu item)
tr3_entity_tbl[240] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Infada stone (pick-up)
tr3_entity_tbl[241] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Element 115 (pick-up)
tr3_entity_tbl[242] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Eye of Isis (pick-up)
tr3_entity_tbl[243] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"};        -- Ora dagger (pick-up)
tr3_entity_tbl[244] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Infada stone (menu item)
tr3_entity_tbl[245] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Element 115 (menu item)
tr3_entity_tbl[246] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Eye of Isis (menu item)
tr3_entity_tbl[247] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Ora dagger (menu item)
tr3_entity_tbl[272] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Keys (sprite)
tr3_entity_tbl[273] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Keys (sprite)
tr3_entity_tbl[276] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Infada stone
tr3_entity_tbl[277] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Element 115
tr3_entity_tbl[278] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Eye of Isis
tr3_entity_tbl[279] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Ora dagger

tr3_entity_tbl[282] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Fire-breathing dragon statue
tr3_entity_tbl[285] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                           -- Unknown visible #285

-- ENEMIES, cont. --

tr3_entity_tbl[287] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};              -- Tyrannosaur
tr3_entity_tbl[288] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};                  -- Raptor

-- TRAPS, cont. --

tr3_entity_tbl[291] = {coll = COLLISION_GROUP_TRIGGERS, shape = COLLISION_SHAPE_BOX, func = "lasersweep"};           -- Laser sweeper
tr3_entity_tbl[292] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Electrified Field
tr3_entity_tbl[295] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};           -- Detonator switch box

-- SERVICE ITEMS --

tr3_entity_tbl[300] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                -- Gunflare / Gunflare (spiky)
tr3_entity_tbl[301] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                -- Spiky gunflare for MP5
tr3_entity_tbl[304] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Look At item
tr3_entity_tbl[305] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Smoke Edge
tr3_entity_tbl[306] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Harpoon (single)
tr3_entity_tbl[309] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Rocket (single)
tr3_entity_tbl[310] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Harpoon (single)
tr3_entity_tbl[311] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Grenade (single)
tr3_entity_tbl[312] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};                -- Big missile
tr3_entity_tbl[313] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Smoke
tr3_entity_tbl[314] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Movable Boom
tr3_entity_tbl[315] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Lara 0x01 appearance
tr3_entity_tbl[317] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                -- Unknown visible #317
tr3_entity_tbl[318] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Red ceiling rotating(?) light
tr3_entity_tbl[319] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Light
tr3_entity_tbl[321] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Light #2
tr3_entity_tbl[322] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Pulsating Light
tr3_entity_tbl[324] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Red Light
tr3_entity_tbl[325] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Green Light
tr3_entity_tbl[326] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Blue Light
tr3_entity_tbl[327] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Light #3
tr3_entity_tbl[328] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Light #4
tr3_entity_tbl[330] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Fire
tr3_entity_tbl[331] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Alternate Fire
tr3_entity_tbl[332] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Alternate Fire #2
tr3_entity_tbl[333] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Fire #2
tr3_entity_tbl[334] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Smoke #2
tr3_entity_tbl[335] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Smoke #3
tr3_entity_tbl[336] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Smoke #4
tr3_entity_tbl[337] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Greenish Smoke
tr3_entity_tbl[338] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Pirahnas
tr3_entity_tbl[339] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Fish
tr3_entity_tbl[347] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Bat swarm

-- ANIMATINGS --

tr3_entity_tbl[349] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "anim"};           -- Misc item (Animating 1)
tr3_entity_tbl[350] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "anim"};           -- Misc item (Animating 2)
tr3_entity_tbl[351] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "anim"};           -- Misc item (Animating 3)
tr3_entity_tbl[352] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "anim"};           -- Misc item (Animating 4)
tr3_entity_tbl[353] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "anim"};           -- Footstool / Fish swimming in tank / Radar display
tr3_entity_tbl[354] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "anim"};           -- Dead raptor / Alarm box / Mason-lodge dagger / Small version of big antenna


tr3_entity_tbl[355] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};                -- Skybox
tr3_entity_tbl[357] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Unknown id #357
tr3_entity_tbl[358] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};   -- Unknown id #358


tr3_entity_tbl[360] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};           -- The butler
tr3_entity_tbl[361] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH};           -- The butler in military outfit and target


tr3_entity_tbl[365] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};           -- Earthquake
tr3_entity_tbl[366] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Yellow shell casing
tr3_entity_tbl[367] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Red shell casing
tr3_entity_tbl[370] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};           -- Tinnos light shaft
tr3_entity_tbl[373] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX};           -- Electrical switch box

--------------------------------------------------------------------------------
--------------------------------- TR_IV, TR_IV_DEMO ----------------------------
--------------------------------------------------------------------------------
tr4_entity_tbl = {};

tr4_entity_tbl[0] = {coll = COLLISION_GROUP_CHARACTERS,      shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "Lara"};                        -- Lara

-- Remark: object IDs 0-30 are used for Lara model and speechheads, and never
-- show in game independently.

-- VEHICLES

tr4_entity_tbl[031] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bike
tr4_entity_tbl[032] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Jeep
tr4_entity_tbl[034] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Enemy jeep

-- ENEMIES

tr4_entity_tbl[035] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Skeleton
tr4_entity_tbl[036] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Skeleton MIP - UNUSED
tr4_entity_tbl[037] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Guide
tr4_entity_tbl[038] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Guide MIP - UNUSED
tr4_entity_tbl[039] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Von Croy
tr4_entity_tbl[040] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Guide MIP - UNUSED
tr4_entity_tbl[041] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baddy 1
tr4_entity_tbl[042] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baddy 1 MIP - UNUSED
tr4_entity_tbl[043] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baddy 2
tr4_entity_tbl[044] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baddy 2 MIP - UNUSED
tr4_entity_tbl[045] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Setha
tr4_entity_tbl[046] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Setha MIP - UNUSED
tr4_entity_tbl[047] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Mummy
tr4_entity_tbl[048] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Mummy MIP - UNUSED
tr4_entity_tbl[049] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Sphinx / Bull
tr4_entity_tbl[050] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Sphinx / Bull MIP - UNUSED
tr4_entity_tbl[051] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Crocodile
tr4_entity_tbl[052] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Crocodile MIP - UNUSED
tr4_entity_tbl[053] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Horseman
tr4_entity_tbl[054] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Horseman MIP - UNUSED
tr4_entity_tbl[055] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Scorpion
tr4_entity_tbl[056] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Scorpion MIP - UNUSED
tr4_entity_tbl[057] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Jean-Yves
tr4_entity_tbl[058] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Jean-Yves MIP - UNUSED
tr4_entity_tbl[059] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Troops
tr4_entity_tbl[060] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Troops MIP - UNUSED
tr4_entity_tbl[061] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Knights Templar
tr4_entity_tbl[062] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Knights Templar MIP - UNUSED
tr4_entity_tbl[063] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Mutant
tr4_entity_tbl[064] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Mutant MIP - UNUSED
tr4_entity_tbl[065] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Horse
tr4_entity_tbl[066] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Horse MIP - UNUSED
tr4_entity_tbl[067] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baboon normal
tr4_entity_tbl[068] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baboon normal MIP - UNUSED
tr4_entity_tbl[069] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baboon invisible
tr4_entity_tbl[070] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baboon invisible MIP - UNUSED
tr4_entity_tbl[071] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baboon silent
tr4_entity_tbl[072] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Baboon silent MIP - UNUSED
tr4_entity_tbl[073] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Wild boar
tr4_entity_tbl[074] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Wild boar MIP - UNUSED
tr4_entity_tbl[075] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Harpy
tr4_entity_tbl[076] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Harpy MIP - UNUSED
tr4_entity_tbl[077] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Demigod 1
tr4_entity_tbl[078] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Demigod 1 MIP - UNUSED
tr4_entity_tbl[079] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Demigod 2
tr4_entity_tbl[080] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Demigod 2 MIP - UNUSED
tr4_entity_tbl[081] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Demigod 3
tr4_entity_tbl[082] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Demigod 3 MIP - UNUSED
tr4_entity_tbl[083] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Little beetle - NO COLLISION, SWARM
tr4_entity_tbl[084] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Big beetle
tr4_entity_tbl[085] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Big beetle MIP - UNUSED
tr4_entity_tbl[086] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Wraith 1 - SPIRIT, NO COLLISION
tr4_entity_tbl[087] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Wraith 2 - SPIRIT, NO COLLISION
tr4_entity_tbl[088] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Wraith 3 - SPIRIT, NO COLLISION
tr4_entity_tbl[089] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Wraith 4 - SPIRIT, NO COLLISION
tr4_entity_tbl[090] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Bat - TOO SMALL TO COLLIDE WITH
tr4_entity_tbl[091] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Dog
tr4_entity_tbl[092] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Dog MIP - UNUSED
tr4_entity_tbl[093] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Hammerhead
tr4_entity_tbl[094] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Hammerhead MIP - UNUSED
tr4_entity_tbl[095] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SAS
tr4_entity_tbl[096] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SAS MIP - UNUSED
tr4_entity_tbl[097] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SAS dying
tr4_entity_tbl[098] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SAS dying MIP - UNUSED
tr4_entity_tbl[099] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SAS Captain
tr4_entity_tbl[100] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SAS Captain MIP - UNUSED
tr4_entity_tbl[101] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- SAS Drag bloke
tr4_entity_tbl[102] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Ahmet
tr4_entity_tbl[103] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Ahmet MIP - UNUSED
tr4_entity_tbl[104] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Lara double
tr4_entity_tbl[105] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Lara double MIP - UNUSED
tr4_entity_tbl[106] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Small scorpion
tr4_entity_tbl[107] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Locust (ex-Fish) - NO COLLISION, SWARM

-- PUZZLE ACTION ITEMS

tr4_entity_tbl[108] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Game piece 1
tr4_entity_tbl[109] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Game piece 2
tr4_entity_tbl[110] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Game piece 3
tr4_entity_tbl[111] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Enemy piece
tr4_entity_tbl[112] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Wheel of fortune
tr4_entity_tbl[113] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Scales

-- DART EMITTER

tr4_entity_tbl[114] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Darts - SPAWNED OBJECT
tr4_entity_tbl[115] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Dart emitter
tr4_entity_tbl[116] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Homing dart emitter  - UNUSED

-- DESTROYABLE / MOVABLE TERRAIN

tr4_entity_tbl[117] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Falling ceiling
tr4_entity_tbl[118] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_SINGLE_BOX, func = "fallblock"}; -- Falling block
tr4_entity_tbl[119] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_SINGLE_BOX, func = "fallblock"}; -- Falling block2
tr4_entity_tbl[120] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smashable bike wall
tr4_entity_tbl[121] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smashable bike floor
tr4_entity_tbl[122] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Trapdoor 1
tr4_entity_tbl[123] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Trapdoor 2
tr4_entity_tbl[124] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Trapdoor 3
tr4_entity_tbl[125] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Floor trapdoor 1
tr4_entity_tbl[126] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Floor trapdoor 2
tr4_entity_tbl[127] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Ceiling trapdoor 1
tr4_entity_tbl[128] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Ceiling trapdoor 2
tr4_entity_tbl[129] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Scaling trapdoor

-- TRAPS & INTERACTION OBJECTS

tr4_entity_tbl[130] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "boulder"}; -- Rolling ball
tr4_entity_tbl[131] = {coll = COLLISION_GROUP_TRIGGERS,     shape = COLLISION_SHAPE_BOX, func = "oldspike"}; -- Spikey floor - UNUSED?
tr4_entity_tbl[132] = {coll = COLLISION_GROUP_TRIGGERS,     shape = COLLISION_SHAPE_BOX, func = "newspike"}; -- Teeth spikes
tr4_entity_tbl[133] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Joby spikes
tr4_entity_tbl[134] = {coll = COLLISION_GROUP_KINEMATIC,      shape = COLLISION_SHAPE_BOX, func = "slicerdicer"}; -- Slicer dicer
tr4_entity_tbl[135] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Chain
tr4_entity_tbl[136] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "plough"}; -- Plough
tr4_entity_tbl[137] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Stargate
tr4_entity_tbl[138] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Hammer
tr4_entity_tbl[139] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Burning floor
tr4_entity_tbl[140] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Cog
tr4_entity_tbl[141] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Spike ball

tr4_entity_tbl[142] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Flame - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[143] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Flame emitter
tr4_entity_tbl[144] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Flame emitter 2
tr4_entity_tbl[145] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Flame emitter 3
tr4_entity_tbl[146] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Rope

tr4_entity_tbl[147] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Fire rope
tr4_entity_tbl[148] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pole rope
tr4_entity_tbl[149] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "twobp"}; -- One block platform  - UNUSED
tr4_entity_tbl[150] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "twobp"}; -- Two block platform
tr4_entity_tbl[151] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "rblock"}; -- Raising block 1 - RESIZABLE MESH!
tr4_entity_tbl[152] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "rblock2"}; -- Raising block 2 - RESIZABLE MESH!
tr4_entity_tbl[153] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "expplatform"}; -- Expanding platform - RESIZABLE MESH!
tr4_entity_tbl[154] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Squishy block 1
tr4_entity_tbl[155] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Squishy block 2

tr4_entity_tbl[156] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"}; -- Pushable object 1
tr4_entity_tbl[157] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"}; -- Pushable object 2
tr4_entity_tbl[158] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"}; -- Pushable object 3
tr4_entity_tbl[159] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"}; -- Pushable object 4
tr4_entity_tbl[160] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "pushable"}; -- Pushable object 5

tr4_entity_tbl[161] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Tripwire  - UNUSED
tr4_entity_tbl[162] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Sentry gun
tr4_entity_tbl[163] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Mine
tr4_entity_tbl[164] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Mapper
tr4_entity_tbl[165] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Obelisk
tr4_entity_tbl[166] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Floor 4 blade
tr4_entity_tbl[167] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Roof 4 blade
tr4_entity_tbl[168] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bird blade
tr4_entity_tbl[169] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Catwalk blade
tr4_entity_tbl[170] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Moving blade
tr4_entity_tbl[171] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Plinth blade
tr4_entity_tbl[172] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "sethblade"}; -- Seth blade

tr4_entity_tbl[173] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Lightning conductor

-- PICK-UP WALKTHROUGH ITEMS

tr4_entity_tbl[174] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Element puzzle
tr4_entity_tbl[175] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 1
tr4_entity_tbl[176] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 2
tr4_entity_tbl[177] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 3
tr4_entity_tbl[178] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 4
tr4_entity_tbl[179] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 5
tr4_entity_tbl[180] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 6
tr4_entity_tbl[181] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 7
tr4_entity_tbl[182] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 8
tr4_entity_tbl[183] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 9
tr4_entity_tbl[184] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 10
tr4_entity_tbl[185] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 11
tr4_entity_tbl[186] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 12
tr4_entity_tbl[187] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 1 combo 1
tr4_entity_tbl[188] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 1 combo 2
tr4_entity_tbl[189] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 2 combo 1
tr4_entity_tbl[190] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 2 combo 2
tr4_entity_tbl[191] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 3 combo 1
tr4_entity_tbl[192] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 3 combo 2
tr4_entity_tbl[193] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 4 combo 1
tr4_entity_tbl[194] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 4 combo 2
tr4_entity_tbl[195] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 5 combo 1
tr4_entity_tbl[196] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 5 combo 2
tr4_entity_tbl[197] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 6 combo 1
tr4_entity_tbl[198] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 6 combo 2
tr4_entity_tbl[199] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 7 combo 1
tr4_entity_tbl[200] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 7 combo 2
tr4_entity_tbl[201] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 8 combo 1
tr4_entity_tbl[202] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle item 8 combo 2
tr4_entity_tbl[203] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 1
tr4_entity_tbl[204] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 2
tr4_entity_tbl[205] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 3
tr4_entity_tbl[206] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 4
tr4_entity_tbl[207] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 5
tr4_entity_tbl[208] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 6
tr4_entity_tbl[209] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 7
tr4_entity_tbl[210] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 8
tr4_entity_tbl[211] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 9
tr4_entity_tbl[212] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 10
tr4_entity_tbl[213] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 11
tr4_entity_tbl[214] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 12
tr4_entity_tbl[215] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 1 combo 1
tr4_entity_tbl[216] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 1 combo 2
tr4_entity_tbl[217] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 2 combo 1
tr4_entity_tbl[218] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 2 combo 2
tr4_entity_tbl[219] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 3 combo 1
tr4_entity_tbl[220] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 3 combo 2
tr4_entity_tbl[221] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 4 combo 1
tr4_entity_tbl[222] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 4 combo 2
tr4_entity_tbl[223] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 5 combo 1
tr4_entity_tbl[224] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 5 combo 2
tr4_entity_tbl[225] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 6 combo 1
tr4_entity_tbl[226] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 6 combo 2
tr4_entity_tbl[227] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 7 combo 1
tr4_entity_tbl[228] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 7 combo 2
tr4_entity_tbl[229] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 8 combo 1
tr4_entity_tbl[230] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 8 combo 2
tr4_entity_tbl[231] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 1
tr4_entity_tbl[232] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 2
tr4_entity_tbl[233] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 3
tr4_entity_tbl[234] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 4
tr4_entity_tbl[235] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 1 combo 1
tr4_entity_tbl[236] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 1 combo 2
tr4_entity_tbl[237] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 2 combo 1
tr4_entity_tbl[238] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 2 combo 2
tr4_entity_tbl[239] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 3 combo 1
tr4_entity_tbl[240] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 3 combo 2
tr4_entity_tbl[241] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 4 combo 1
tr4_entity_tbl[242] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 4 combo 2
tr4_entity_tbl[243] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Examine 1
tr4_entity_tbl[244] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Examine 2
tr4_entity_tbl[245] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Examine 3
tr4_entity_tbl[246] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Crowbar item
tr4_entity_tbl[247] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Burning torch item
tr4_entity_tbl[248] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Clock work beetle
tr4_entity_tbl[249] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Clock work beetle combo 1
tr4_entity_tbl[250] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Clock work beetle combo 2
tr4_entity_tbl[251] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Mine detector
tr4_entity_tbl[252] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Quest item 1
tr4_entity_tbl[253] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Quest item 2
tr4_entity_tbl[254] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Quest item 3
tr4_entity_tbl[255] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Quest item 4
tr4_entity_tbl[256] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Quest item 5
tr4_entity_tbl[257] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Quest item 6
tr4_entity_tbl[258] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Map - UNUSED
tr4_entity_tbl[259] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Secret map - UNUSED

-- PUZZLE HOLES AND KEYHOLES

tr4_entity_tbl[260] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 1
tr4_entity_tbl[261] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 2
tr4_entity_tbl[262] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 3
tr4_entity_tbl[263] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 4
tr4_entity_tbl[264] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 5
tr4_entity_tbl[265] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 6
tr4_entity_tbl[266] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 7
tr4_entity_tbl[267] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 8
tr4_entity_tbl[268] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 9
tr4_entity_tbl[269] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 10
tr4_entity_tbl[270] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 11
tr4_entity_tbl[271] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Puzzle hole 12
tr4_entity_tbl[272] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 1
tr4_entity_tbl[273] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 2
tr4_entity_tbl[274] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 3
tr4_entity_tbl[275] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 4
tr4_entity_tbl[276] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 5
tr4_entity_tbl[277] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 6
tr4_entity_tbl[278] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 7
tr4_entity_tbl[279] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 8
tr4_entity_tbl[280] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 9
tr4_entity_tbl[281] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 10
tr4_entity_tbl[282] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 11
tr4_entity_tbl[283] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle done 12
tr4_entity_tbl[284] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 1 (ig keyhole hub.tr4)
tr4_entity_tbl[285] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 2
tr4_entity_tbl[286] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 3
tr4_entity_tbl[287] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 4
tr4_entity_tbl[288] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 5
tr4_entity_tbl[289] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 6
tr4_entity_tbl[290] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 7
tr4_entity_tbl[291] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 8
tr4_entity_tbl[292] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 9
tr4_entity_tbl[293] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 10
tr4_entity_tbl[294] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 11
tr4_entity_tbl[295] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "keyhole"}; -- Key hole 12

-- WATERSKIN ITEMS

tr4_entity_tbl[296] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 1 empty
tr4_entity_tbl[297] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 1 1
tr4_entity_tbl[298] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 1 2
tr4_entity_tbl[299] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 1 3
tr4_entity_tbl[300] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 2 empty
tr4_entity_tbl[301] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 2 1
tr4_entity_tbl[302] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 2 2
tr4_entity_tbl[303] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 2 3
tr4_entity_tbl[304] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 2 4
tr4_entity_tbl[305] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Water skin 2 5

-- SWITCHES

tr4_entity_tbl[306] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Switch type 1
tr4_entity_tbl[307] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Switch type 2
tr4_entity_tbl[308] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Switch type 3
tr4_entity_tbl[309] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Switch type 4
tr4_entity_tbl[310] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Switch type 5
tr4_entity_tbl[311] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Switch type 6
tr4_entity_tbl[312] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Switch type 7
tr4_entity_tbl[313] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Switch type 8
tr4_entity_tbl[314] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Underwater switch 1
tr4_entity_tbl[315] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Underwater switch 2
tr4_entity_tbl[316] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Turn switch
tr4_entity_tbl[317] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Cog switch
tr4_entity_tbl[318] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "lever_switch"}; -- Lever switch
tr4_entity_tbl[319] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Jump switch
tr4_entity_tbl[320] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Crowbar switch
tr4_entity_tbl[321] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "switch"}; -- Pulley

-- DOORS

tr4_entity_tbl[322] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "door"}; -- Door type 1
tr4_entity_tbl[323] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door type 2
tr4_entity_tbl[324] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door type 3
tr4_entity_tbl[325] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door type 4
tr4_entity_tbl[326] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door type 5
tr4_entity_tbl[327] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door type 6
tr4_entity_tbl[328] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door type 7
tr4_entity_tbl[329] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door type 8
tr4_entity_tbl[330] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Push pull door 1
tr4_entity_tbl[331] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Push pull door 2
tr4_entity_tbl[332] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Kick door 1
tr4_entity_tbl[333] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Kick door 2
tr4_entity_tbl[334] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Underwater door
tr4_entity_tbl[335] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushdoor"}; -- Double doors

-- STATIC TERRAIN

tr4_entity_tbl[336] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bridge flat
tr4_entity_tbl[337] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bridge tilt 1
tr4_entity_tbl[338] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bridge tilt 2

-- MISC INTERACTION OBJECTS

tr4_entity_tbl[339] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Sarcophagus
tr4_entity_tbl[340] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Sequence door 1
tr4_entity_tbl[341] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Sequence switch 1
tr4_entity_tbl[342] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Sequence switch 2
tr4_entity_tbl[343] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Sequence switch 3
tr4_entity_tbl[344] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Sarcophagus cut
tr4_entity_tbl[345] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Horus statue
tr4_entity_tbl[346] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- God head
tr4_entity_tbl[347] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Seth door
tr4_entity_tbl[348] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Statue plinth

 -- PICK-UP SUPPLY ITEMS

tr4_entity_tbl[349] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pistols item
tr4_entity_tbl[350] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pistols ammo item
tr4_entity_tbl[351] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Uzi item
tr4_entity_tbl[352] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Uzi ammo item
tr4_entity_tbl[353] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Shotgun item
tr4_entity_tbl[354] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Shotgun ammo 1 item
tr4_entity_tbl[355] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Shotgun ammo 2 item
tr4_entity_tbl[356] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Crossbow item
tr4_entity_tbl[357] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Crossbow ammo 1 item
tr4_entity_tbl[358] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Crossbow ammo 2 item
tr4_entity_tbl[359] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Crossbow ammo 3 item
tr4_entity_tbl[360] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Crossbow bolt
tr4_entity_tbl[361] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Grenade gun item
tr4_entity_tbl[362] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Grenade gun ammo 1 item
tr4_entity_tbl[363] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Grenade gun ammo 2 item
tr4_entity_tbl[364] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Grenade gun ammo 3 item
tr4_entity_tbl[365] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Grenade
tr4_entity_tbl[366] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Six shooter item
tr4_entity_tbl[367] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Six shooter ammo item
tr4_entity_tbl[368] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Big medipack item
tr4_entity_tbl[369] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Small medipack item
tr4_entity_tbl[370] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Laser sight item
tr4_entity_tbl[371] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Binoculars item
tr4_entity_tbl[372] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Flare item
tr4_entity_tbl[373] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Flare inv item
tr4_entity_tbl[374] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Diary item - UNUSED

-- INVENTORY ITEMS

tr4_entity_tbl[375] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Compass item - NOT A PROPER PICK UP OBJECT (INVENTORY ONLY)
tr4_entity_tbl[376] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Mem card load inv item - UNUSED
tr4_entity_tbl[377] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Mem card save inv item - UNUSED
tr4_entity_tbl[378] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- PC load inv item - NOT A PROPER PICK UP OBJECT (INVENTORY ONLY)
tr4_entity_tbl[379] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- PC save inv item - NOT A PROPER PICK UP OBJECT (INVENTORY ONLY)

-- NULLMESHES, SERVICE OBJECTS AND EMITTERS

tr4_entity_tbl[380] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Smoke emitter white - EMITTER
tr4_entity_tbl[381] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Smoke emitter black - EMITTER
tr4_entity_tbl[382] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Steam emitter - EMITTER
tr4_entity_tbl[383] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Earth quake - SHAKES CAMERA
tr4_entity_tbl[384] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Bubbles - EMITTER
tr4_entity_tbl[385] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Waterfall mist - EMITTER

tr4_entity_tbl[386] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Gun shell - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[387] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Shotgun shell - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[388] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Gun flash - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[389] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Butterfly - UNUSED
tr4_entity_tbl[390] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Sprinkler - EMITTER
tr4_entity_tbl[391] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Red light - STATIC LIGHT

tr4_entity_tbl[392] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Green light - STATIC LIGHT
tr4_entity_tbl[393] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Blue light - STATIC LIGHT
tr4_entity_tbl[394] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Amber light - DYNAMIC LIGHT
tr4_entity_tbl[395] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- White light - STATIC LIGHT
tr4_entity_tbl[396] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Blinking light - DYNAMIC LIGHT
tr4_entity_tbl[397] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Lens flare


-- Remark: objects ID 398-408 are AI / trigger nullmeshes, and never shows in game.

tr4_entity_tbl[398] = {hide = 0x01};
tr4_entity_tbl[399] = {hide = 0x01};
tr4_entity_tbl[400] = {hide = 0x01};
tr4_entity_tbl[401] = {hide = 0x01};
tr4_entity_tbl[402] = {hide = 0x01};
tr4_entity_tbl[403] = {hide = 0x01};
tr4_entity_tbl[404] = {hide = 0x01};
tr4_entity_tbl[405] = {hide = 0x01};
tr4_entity_tbl[406] = {hide = 0x01};
tr4_entity_tbl[407] = {hide = 0x01};
tr4_entity_tbl[408] = {hide = 0x01};

-- MISC. OBJECTS

tr4_entity_tbl[409] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 1
tr4_entity_tbl[410] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 2
tr4_entity_tbl[411] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 3
tr4_entity_tbl[412] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 4
tr4_entity_tbl[413] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 5
tr4_entity_tbl[414] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 6
tr4_entity_tbl[415] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 7
tr4_entity_tbl[416] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 8
tr4_entity_tbl[417] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Mesh swap 1
tr4_entity_tbl[418] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Mesh swap 2
tr4_entity_tbl[419] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Mesh swap 3
tr4_entity_tbl[420] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Death slide
tr4_entity_tbl[421] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Body part - UNUSED? SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[422] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Camera target - TARGET FOR CAMERA

-- WATERFALLS (MESHES WITH UV-SCROLL ANIMATED TEXTURES)

tr4_entity_tbl[423] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Waterfall 1
tr4_entity_tbl[424] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Waterfall 2
tr4_entity_tbl[425] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Waterfall 3
tr4_entity_tbl[426] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Planet effect

-- ANIMATINGS

tr4_entity_tbl[427] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 1
tr4_entity_tbl[428] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 1 MIP
tr4_entity_tbl[429] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 2
tr4_entity_tbl[430] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 2 MIP
tr4_entity_tbl[431] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 3
tr4_entity_tbl[432] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 3 MIP
tr4_entity_tbl[433] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 4
tr4_entity_tbl[434] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 4 MIP
tr4_entity_tbl[435] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 5
tr4_entity_tbl[436] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 5 MIP
tr4_entity_tbl[437] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 6
tr4_entity_tbl[438] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 6 MIP
tr4_entity_tbl[439] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 7
tr4_entity_tbl[440] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 7 MIP
tr4_entity_tbl[441] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 8
tr4_entity_tbl[442] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 8 MIP
tr4_entity_tbl[443] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 9
tr4_entity_tbl[444] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 9 MIP
tr4_entity_tbl[445] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 10
tr4_entity_tbl[446] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 10 MIP
tr4_entity_tbl[447] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 11
tr4_entity_tbl[448] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 11 MIP
tr4_entity_tbl[449] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 12
tr4_entity_tbl[450] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 12 MIP
tr4_entity_tbl[451] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 13
tr4_entity_tbl[452] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 13 MIP
tr4_entity_tbl[453] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 14 (sunlight)
tr4_entity_tbl[454] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 14 MIP
tr4_entity_tbl[455] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 15 (basket, hub.tr4, sunlight)
tr4_entity_tbl[456] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 15 MIP
tr4_entity_tbl[457] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 16
tr4_entity_tbl[458] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 16 MIP

--------------------------------------------------------------------------------
------------------------------------- TR_V -------------------------------------
--------------------------------------------------------------------------------
tr5_entity_tbl = {};

tr5_entity_tbl[0] = {coll = COLLISION_GROUP_CHARACTERS,      shape = COLLISION_SHAPE_TRIMESH_CONVEX, func = "Lara"};                        -- Lara

-- Remark: object IDs 0-30 are used for Lara model and speechheads, and never
-- show in game independently.

-- ENEMIES

tr5_entity_tbl[031] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SAS - UNUSED
tr5_entity_tbl[032] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SAS MIP - UNUSED
tr5_entity_tbl[033] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SWAT  - UNUSED
tr5_entity_tbl[034] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- SWAT MIP - UNUSED
tr5_entity_tbl[035] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- VCI guard (SWAT_PLUS)
tr5_entity_tbl[036] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- VCI guard MIP (SWAT_PLUS MIP)
tr5_entity_tbl[037] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Guard gun (BLUE_GUARD)
tr5_entity_tbl[038] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Guard gun MIP (BLUE_GUARD MIP)
tr5_entity_tbl[039] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Guard laser (TWOGUN)
tr5_entity_tbl[040] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Guard laser MIP (TWOGUN MIP)
tr5_entity_tbl[041] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Doberman (DOG)
tr5_entity_tbl[042] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Doberman MIP (DOG MIP)
tr5_entity_tbl[043] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Crow
tr5_entity_tbl[044] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Crow MIP
tr5_entity_tbl[045] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Larson
tr5_entity_tbl[046] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Keycard 1 (Ex-LARSON MIP)
tr5_entity_tbl[047] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Pierre
tr5_entity_tbl[048] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Keycard 2  (Ex-PIERRE MIP)
tr5_entity_tbl[049] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Armed baddy 1 (MAFIA)
tr5_entity_tbl[050] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Armed baddy 1 MIP (MAFIA MIP)
tr5_entity_tbl[051] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Armed baddy 2 (MAFIA2)
tr5_entity_tbl[052] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Armed baddy 2 MIP (MAFIA2 MIP)
tr5_entity_tbl[053] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- (Ex-SAILOR) - UNUSED?
tr5_entity_tbl[054] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- (Ex-SAILOR MIP) - UNUSED?
tr5_entity_tbl[055] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Guard robot control (CRANE_GUY)
tr5_entity_tbl[056] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Guard robot control MIP (CRANE_GUY MIP)
tr5_entity_tbl[057] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Lion
tr5_entity_tbl[058] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Lion MIP
tr5_entity_tbl[059] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Gladiator
tr5_entity_tbl[060] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Gladiator MIP
tr5_entity_tbl[061] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Roman statue
tr5_entity_tbl[062] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Spear tip (Ex-ROMAN_GOD MIP)
tr5_entity_tbl[063] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Hydra
tr5_entity_tbl[064] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Flat floor (Ex-HYDRA MIP?)
tr5_entity_tbl[065] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Laser head (GUARDIAN)
tr5_entity_tbl[066] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Laser head MIP (GUARDIAN MIP)
tr5_entity_tbl[067] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Cyborg
tr5_entity_tbl[068] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Cyborg MIP
tr5_entity_tbl[069] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- VCI worker
tr5_entity_tbl[070] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- VCI worker MIP
tr5_entity_tbl[071] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Willowisp Guide
tr5_entity_tbl[072] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Willowisp Guide MIP
tr5_entity_tbl[073] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Invisible ghost
tr5_entity_tbl[074] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Invisible ghost MIP - UNUSED?
tr5_entity_tbl[075] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Reaper - UNUSED?
tr5_entity_tbl[076] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Reaper MIP- UNUSED?
tr5_entity_tbl[077] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Maze Monster
tr5_entity_tbl[078] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Maze Monster MIP - UNUSED?
tr5_entity_tbl[079] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Lagoon Witch
tr5_entity_tbl[080] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Lagoon Witch MIP - UNUSED?
tr5_entity_tbl[081] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Submarine
tr5_entity_tbl[082] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Submarine MIP
tr5_entity_tbl[083] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- M16 Guard
tr5_entity_tbl[084] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Tree with huged man (Ex-M16 Guard MIP)
tr5_entity_tbl[085] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Husky
tr5_entity_tbl[086] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Husky MIP
tr5_entity_tbl[087] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- The chef
tr5_entity_tbl[088] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Rich1 hammer door (Ex-CHEF MIP)
tr5_entity_tbl[089] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Imp
tr5_entity_tbl[090] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Padlock (Ex-IMP_MIP)
tr5_entity_tbl[091] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Gunship
tr5_entity_tbl[092] = {coll = COLLISION_GROUP_CHARACTERS,     shape = COLLISION_SHAPE_TRIMESH_CONVEX}; -- Gunship MIP
tr5_entity_tbl[093] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Bats
tr5_entity_tbl[094] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Little rats
tr5_entity_tbl[095] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Spiders
tr5_entity_tbl[096] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Spider generator - UNUSED?
tr5_entity_tbl[097] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Auto guns
tr5_entity_tbl[098] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Electricity wires

tr5_entity_tbl[099] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Darts - SPAWN OBJECT
tr5_entity_tbl[100] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Dart emitter
tr5_entity_tbl[101] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Homing (fast) dart emitter

-- DESTROYABLE / MOVABLE TERRAIN

tr5_entity_tbl[102] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Falling Ceiling
tr5_entity_tbl[103] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_SINGLE_BOX, func = "fallblock"}; -- Falling Block 1
tr5_entity_tbl[104] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_SINGLE_BOX, func = "fallblock"}; -- Falling Block 2
tr5_entity_tbl[105] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Crumbling Floor
tr5_entity_tbl[106] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Trapdoor 1
tr5_entity_tbl[107] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Trapdoor 2
tr5_entity_tbl[108] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Trapdoor 3
tr5_entity_tbl[109] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Floor trapdoor 1
tr5_entity_tbl[110] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Floor trapdoor 2
tr5_entity_tbl[111] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Ceiling trapdoor 1
tr5_entity_tbl[112] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Ceiling trapdoor 2
tr5_entity_tbl[113] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Scaling trapdoor
tr5_entity_tbl[114] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Box 1 (ROLLINGBALL)
tr5_entity_tbl[115] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Rolling barrel - UNUSED?

-- TRAPS & INTERACTION OBJECTS

tr5_entity_tbl[116] = {coll = COLLISION_GROUP_TRIGGERS,     shape = COLLISION_SHAPE_BOX, func = "oldspike"}; -- Spikey Floor - UNUSED?
tr5_entity_tbl[117] = {coll = COLLISION_GROUP_TRIGGERS,     shape = COLLISION_SHAPE_BOX, func = "newspike"}; -- Teeth Spikes
tr5_entity_tbl[118] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Rome Hammer
tr5_entity_tbl[119] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Hammer 2 - UNUSED?

tr5_entity_tbl[120] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Flame
tr5_entity_tbl[121] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Flame emitter
tr5_entity_tbl[122] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Flame emitter 2
tr5_entity_tbl[123] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Flame emitter 3
tr5_entity_tbl[124] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Cooker flame

tr5_entity_tbl[125] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Burning roots

tr5_entity_tbl[126] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Rope

tr5_entity_tbl[127] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Fire rope
tr5_entity_tbl[128] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pole rope
tr5_entity_tbl[129] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Ventilator HORIZONTAL
tr5_entity_tbl[130] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Ventilator VERTICAL

tr5_entity_tbl[131] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Grappling gun target

tr5_entity_tbl[132] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "twobp"}; -- One block platform - UNUSED?
tr5_entity_tbl[133] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "twobp"}; -- Two block platform - UNUSED?
tr5_entity_tbl[134] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX, func = "rblock"};     -- Raising block

tr5_entity_tbl[135] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Teleport (Ex-RAISING_BLOCK2?)
tr5_entity_tbl[136] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Headset talk point (Ex-EXPANDING_PLATFORM?)

tr5_entity_tbl[137] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushable"}; -- Pushable 1
tr5_entity_tbl[138] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushable"}; -- Pushable 2
tr5_entity_tbl[139] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushable"}; -- Pushable 3 - UNUSED?
tr5_entity_tbl[140] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushable"}; -- Pushable 4 - UNUSED?
tr5_entity_tbl[141] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushable"}; -- Pushable 5 - UNUSED?

tr5_entity_tbl[142] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Robot arm (Ex-WRECKING BALL?)
tr5_entity_tbl[142] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Death slide - UNUSED?
tr5_entity_tbl[144] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Rocket item - TORPEDO
tr5_entity_tbl[145] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Chaff flare
tr5_entity_tbl[146] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Satchel Bomb - UNUSED?
tr5_entity_tbl[147] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Electric Fence - UNUSED?
tr5_entity_tbl[148] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Lift - UNUSED

tr5_entity_tbl[149] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Explosion
tr5_entity_tbl[150] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Deadly Electric bolt (IRIS_LIGHTNING)

tr5_entity_tbl[151] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Monitor screen
tr5_entity_tbl[152] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_BOX}; -- Security camera board

tr5_entity_tbl[153] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Motion sensor
tr5_entity_tbl[154] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Tight rope

tr5_entity_tbl[155] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Parallel bars

tr5_entity_tbl[156] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- X-Ray Controller (?)

tr5_entity_tbl[157] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Cutscene rope
tr5_entity_tbl[158] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Flat window
tr5_entity_tbl[159] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- GEN_SLOT1 - UNUSED?

tr5_entity_tbl[160] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Gas emitter

tr5_entity_tbl[161] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Sign
tr5_entity_tbl[162] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Moving laser
tr5_entity_tbl[163] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Imp Rock - SPAWN ITEM?
tr5_entity_tbl[164] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Cupboard 1
tr5_entity_tbl[165] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Cupboard 1 MIP
tr5_entity_tbl[166] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Cupboard 2
tr5_entity_tbl[167] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Cupboard 2 MIP
tr5_entity_tbl[168] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Cupboard 3
tr5_entity_tbl[169] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Cupboard 3 MIP
tr5_entity_tbl[170] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Suitcase
tr5_entity_tbl[171] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Suitcase MIP

-- PICK-UP WALKTHROUGH ITEMS

tr5_entity_tbl[172] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 1
tr5_entity_tbl[173] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 2
tr5_entity_tbl[174] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 3
tr5_entity_tbl[175] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 4
tr5_entity_tbl[176] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 5
tr5_entity_tbl[177] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 6
tr5_entity_tbl[178] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 7
tr5_entity_tbl[179] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 8
tr5_entity_tbl[180] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 1 Combo 1
tr5_entity_tbl[181] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 1 Combo 2
tr5_entity_tbl[182] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 2 Combo 1
tr5_entity_tbl[183] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 2 Combo 2
tr5_entity_tbl[184] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 3 Combo 1
tr5_entity_tbl[185] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 3 Combo 2
tr5_entity_tbl[186] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 4 Combo 1
tr5_entity_tbl[187] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 4 Combo 2
tr5_entity_tbl[188] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 5 Combo 1
tr5_entity_tbl[189] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 5 Combo 2
tr5_entity_tbl[190] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 6 Combo 1
tr5_entity_tbl[191] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 6 Combo 2
tr5_entity_tbl[192] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 7 Combo 1
tr5_entity_tbl[193] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 7 Combo 2
tr5_entity_tbl[194] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 8 Combo 1
tr5_entity_tbl[195] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Puzzle 8 Combo 2
tr5_entity_tbl[196] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key 1
tr5_entity_tbl[197] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key 2
tr5_entity_tbl[198] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key 3 - UNUSED?
tr5_entity_tbl[199] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key 4 - UNUSED?
tr5_entity_tbl[200] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key 5 - UNUSED?
tr5_entity_tbl[201] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key 6
tr5_entity_tbl[202] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key 7
tr5_entity_tbl[203] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key 8

tr5_entity_tbl[204] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 1 combo 1
tr5_entity_tbl[205] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 1 combo 2
tr5_entity_tbl[206] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 2 combo 1
tr5_entity_tbl[207] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 2 combo 2
tr5_entity_tbl[208] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 3 combo 1
tr5_entity_tbl[209] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 3 combo 2
tr5_entity_tbl[210] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 4 combo 1
tr5_entity_tbl[211] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 4 combo 2
tr5_entity_tbl[212] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 5 combo 1
tr5_entity_tbl[213] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 5 combo 2
tr5_entity_tbl[214] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 6 combo 1
tr5_entity_tbl[215] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 6 combo 2
tr5_entity_tbl[216] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 7 combo 1
tr5_entity_tbl[217] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 7 combo 2
tr5_entity_tbl[218] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 8 combo 1
tr5_entity_tbl[219] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Key item 8 combo 2

tr5_entity_tbl[220] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 1
tr5_entity_tbl[221] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 2
tr5_entity_tbl[222] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pickup item 3 - UNUSED?
tr5_entity_tbl[223] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Gold rose

tr5_entity_tbl[224] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pickup item 1 combo 1 - UNUSED
tr5_entity_tbl[225] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pickup item 1 combo 2 - UNUSED
tr5_entity_tbl[226] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pickup item 2 combo 1 - UNUSED
tr5_entity_tbl[227] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pickup item 2 combo 2 - UNUSED
tr5_entity_tbl[228] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pickup item 3 combo 1 - UNUSED
tr5_entity_tbl[229] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pickup item 3 combo 2 - UNUSED
tr5_entity_tbl[230] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pickup item 4 combo 1 - UNUSED
tr5_entity_tbl[231] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Pickup item 4 combo 2 - UNUSED

tr5_entity_tbl[232] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Examine 1 - UNUSED
tr5_entity_tbl[233] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Examine 2 - UNUSED
tr5_entity_tbl[234] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Examine 3 - UNUSED

tr5_entity_tbl[235] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Chloroform cloth
tr5_entity_tbl[236] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Chloroform bottle
tr5_entity_tbl[237] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Chloroform soaked cloth
tr5_entity_tbl[238] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Cosh (?)
tr5_entity_tbl[239] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Hammer item - UNUSED?
tr5_entity_tbl[240] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Crowbar item
tr5_entity_tbl[241] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Torch item

-- PUZZLEHOLES AND KEYHOLES

tr5_entity_tbl[242] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Puzzle hole 1
tr5_entity_tbl[243] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Puzzle hole 2
tr5_entity_tbl[244] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Puzzle hole 3
tr5_entity_tbl[245] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Puzzle hole 4
tr5_entity_tbl[246] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Puzzle hole 5
tr5_entity_tbl[247] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Puzzle hole 6
tr5_entity_tbl[248] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Puzzle hole 7
tr5_entity_tbl[249] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Puzzle hole 8
tr5_entity_tbl[250] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Puzzle done 1
tr5_entity_tbl[251] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Puzzle done 2
tr5_entity_tbl[252] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Puzzle done 3
tr5_entity_tbl[253] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Puzzle done 4
tr5_entity_tbl[254] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Puzzle done 5
tr5_entity_tbl[255] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Puzzle done 6
tr5_entity_tbl[256] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Puzzle done 7
tr5_entity_tbl[257] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Puzzle done 8
tr5_entity_tbl[258] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Keyhole 1
tr5_entity_tbl[259] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Keyhole 2
tr5_entity_tbl[260] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Keyhole 3 - UNUSED?
tr5_entity_tbl[261] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Keyhole 4 - UNUSED?
tr5_entity_tbl[262] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Keyhole 5 - UNUSED?
tr5_entity_tbl[263] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Keyhole 6
tr5_entity_tbl[264] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Keyhole 7
tr5_entity_tbl[265] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "keyhole"}; -- Keyhole 8

-- SWITCHES

tr5_entity_tbl[266] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Switch type 1
tr5_entity_tbl[267] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Switch type 2
tr5_entity_tbl[268] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Switch type 3
tr5_entity_tbl[269] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Switch type 4
tr5_entity_tbl[270] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Switch type 5
tr5_entity_tbl[271] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Switch type 6
tr5_entity_tbl[272] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Shoot switch 1
tr5_entity_tbl[273] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Shoot switch 2
tr5_entity_tbl[274] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Airlock switch
tr5_entity_tbl[275] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Underwater switch 1 - UNUSED?
tr5_entity_tbl[276] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Underwater switch 2 - UNUSED?
tr5_entity_tbl[277] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Turn switch - UNUSED?
tr5_entity_tbl[278] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Cog switch
tr5_entity_tbl[279] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Lever switch - UNUSED?
tr5_entity_tbl[280] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Jump switch
tr5_entity_tbl[281] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Crowbar switch - UNUSED?
tr5_entity_tbl[282] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "switch"}; -- Pulley
tr5_entity_tbl[283] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Crowdove switch

-- DOORS

tr5_entity_tbl[284] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 1
tr5_entity_tbl[285] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 1 MIP
tr5_entity_tbl[286] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 2
tr5_entity_tbl[287] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 2 MIP
tr5_entity_tbl[288] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 3
tr5_entity_tbl[289] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 3 MIP
tr5_entity_tbl[290] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 4
tr5_entity_tbl[291] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 4 MIP
tr5_entity_tbl[292] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 5
tr5_entity_tbl[293] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 5 MIP
tr5_entity_tbl[294] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 6
tr5_entity_tbl[295] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 6 MIP
tr5_entity_tbl[296] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 7
tr5_entity_tbl[297] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 7 MIP
tr5_entity_tbl[298] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 8
tr5_entity_tbl[299] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "door"}; -- Door 8 MIP
tr5_entity_tbl[300] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 1
tr5_entity_tbl[301] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 1 MIP
tr5_entity_tbl[302] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 2
tr5_entity_tbl[303] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 2 MIP
tr5_entity_tbl[304] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 3
tr5_entity_tbl[305] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 3 MIP
tr5_entity_tbl[306] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 4
tr5_entity_tbl[307] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 4 MIP
tr5_entity_tbl[308] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 5
tr5_entity_tbl[309] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 5 MIP
tr5_entity_tbl[310] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 6
tr5_entity_tbl[311] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Closed Door 6 MIP
tr5_entity_tbl[312] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Lift doors 1
tr5_entity_tbl[313] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Lift doors 1 MIP
tr5_entity_tbl[314] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Lift doors 2
tr5_entity_tbl[315] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Lift doors 2 MIP
tr5_entity_tbl[316] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Push-pull door 1 - UNUSED?
tr5_entity_tbl[317] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Push-pull door 1 MIP - UNUSED?
tr5_entity_tbl[318] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Push-pull door 2 - UNUSED?
tr5_entity_tbl[319] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Push-pull door 2 - UNUSED?
tr5_entity_tbl[320] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Kick door 1
tr5_entity_tbl[321] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Kick door 1 MIP - UNUSED?
tr5_entity_tbl[322] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Kick door 2
tr5_entity_tbl[323] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Kick door 2 MIP - UNUSED?
tr5_entity_tbl[324] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Underwater door
tr5_entity_tbl[325] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Underwater door MIP - UNUSED?
tr5_entity_tbl[326] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushdoor"}; -- Double doors
tr5_entity_tbl[327] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH, func = "pushdoor"}; -- Double doors MIP
tr5_entity_tbl[328] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Sequence door
tr5_entity_tbl[329] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Sequence switch (door) 1
tr5_entity_tbl[330] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Sequence switch (door) 2
tr5_entity_tbl[331] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Sequence switch (door) 3
tr5_entity_tbl[332] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Steel door

tr5_entity_tbl[333] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- God head - UNUSED, TR4 LEFTOVER

-- PICK-UP SUPPLY ITEMS

tr5_entity_tbl[334] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pistols
tr5_entity_tbl[335] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Pistols ammo
tr5_entity_tbl[336] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Uzis
tr5_entity_tbl[337] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Uzis ammo
tr5_entity_tbl[338] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Shotgun
tr5_entity_tbl[339] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Shotgun shells 1
tr5_entity_tbl[340] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Shotgun shells 2
tr5_entity_tbl[341] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Grappling gun
tr5_entity_tbl[342] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Grappling ammo type 1
tr5_entity_tbl[343] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Grappling ammo type 2
tr5_entity_tbl[344] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Grappling ammo type 3
tr5_entity_tbl[345] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- HK Gun
tr5_entity_tbl[346] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- HK ammo
tr5_entity_tbl[347] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Revolver
tr5_entity_tbl[348] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Revolver bullets
tr5_entity_tbl[349] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Big Medi-Pack
tr5_entity_tbl[350] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Small Medi-Pack
tr5_entity_tbl[351] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Laser sight
tr5_entity_tbl[352] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Binoculars
tr5_entity_tbl[353] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Silencer
tr5_entity_tbl[354] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Burning flare
tr5_entity_tbl[355] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Flares
tr5_entity_tbl[356] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, func = "pickable"}; -- Timex-TMX (Compass)

-- INVENTORY ITEMS

tr5_entity_tbl[357] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Load inventory
tr5_entity_tbl[358] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Save inventory
tr5_entity_tbl[359] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Disk load
tr5_entity_tbl[360] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Disk save
tr5_entity_tbl[361] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Memcard load
tr5_entity_tbl[362] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Memcard save

-- NULLMESHES, SERVICE OBJECTS AND EMITTERS

tr5_entity_tbl[363] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Smoke emitter white - EMITTER
tr5_entity_tbl[364] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Smoke emitter black - EMITTER
tr5_entity_tbl[365] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Steam emitter - EMITTER
tr5_entity_tbl[366] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Earthquake - SHAKE CAMERA
tr5_entity_tbl[367] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Bubbles - EMITTER
tr5_entity_tbl[368] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Waterfall Mist - EMITTER

tr5_entity_tbl[369] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Gun shell - SPAWNED OBJECT, NO DIRECT USE
tr5_entity_tbl[370] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Shotgun shell - SPAWNED OBJECT, NO DIRECT USE
tr5_entity_tbl[371] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Gun flash - SPAWNED OBJECT, NO DIRECT USE

tr5_entity_tbl[372] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Color light - UNUSED?
tr5_entity_tbl[373] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Blinking light - UNUSED?
tr5_entity_tbl[374] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Pulse light - DYNAMIC LIGHT
tr5_entity_tbl[375] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Alarm light - DYNAMIC LIGHT
tr5_entity_tbl[376] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Electrical light - DYNAMIC LIGHT
tr5_entity_tbl[377] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Lens flare - UNUSED?

-- Remark: objects 378-386 are AI nullmeshes, and never show in game.

tr5_entity_tbl[378] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};
tr5_entity_tbl[379] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};
tr5_entity_tbl[380] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};
tr5_entity_tbl[381] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};
tr5_entity_tbl[382] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};
tr5_entity_tbl[383] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};
tr5_entity_tbl[384] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};
tr5_entity_tbl[385] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};
tr5_entity_tbl[386] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01};

tr5_entity_tbl[387] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Teleporter
tr5_entity_tbl[388] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Lift teleporter

tr5_entity_tbl[389] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Raising cog

tr5_entity_tbl[390] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Laser
tr5_entity_tbl[391] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Steam laser
tr5_entity_tbl[392] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Floor laser 3

tr5_entity_tbl[393] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Kill all triggers (or Laser 4?)
tr5_entity_tbl[394] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Trigger Triggerer

-- MISC. SOLID OBJECTS (?)

tr5_entity_tbl[395] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- High object 1 (Polerope puzzle)
tr5_entity_tbl[396] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX};     -- High object 2 (Flame Emiter with sparks)
tr5_entity_tbl[397] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 1 (Breakeable glass Floor)
tr5_entity_tbl[398] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 2 (Breakeable glass Door)
tr5_entity_tbl[399] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 3 - UNUSED?
tr5_entity_tbl[400] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 4 - UNUSED?
tr5_entity_tbl[401] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 5 - UNUSED?
tr5_entity_tbl[402] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 6 - UNUSED?
tr5_entity_tbl[403] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 7 - UNUSED?
tr5_entity_tbl[404] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Smash object 8 - UNUSED?
tr5_entity_tbl[405] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Meshswap 1
tr5_entity_tbl[406] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Meshswap 2
tr5_entity_tbl[407] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Meshswap 3

-- NULLMESHES

tr5_entity_tbl[408] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX};              -- Body part - UNUSED?
tr5_entity_tbl[409] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX, hide = 0x01}; -- Camera target

-- WATERFALLS (MESHES WITH UV-SCROLL ANIMATED TEXTURES)

tr5_entity_tbl[410] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Waterfall 1
tr5_entity_tbl[411] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Waterfall 2
tr5_entity_tbl[412] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Waterfall 3
tr5_entity_tbl[413] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Fishtank waterfall
tr5_entity_tbl[414] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Waterfalls 1
tr5_entity_tbl[415] = {coll = COLLISION_NONE,           shape = COLLISION_SHAPE_BOX}; -- Waterfalls 2

-- ANIMATINGS

tr5_entity_tbl[416] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 1
tr5_entity_tbl[417] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 1 MIP
tr5_entity_tbl[418] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 2
tr5_entity_tbl[419] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 2 MIP
tr5_entity_tbl[420] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 3
tr5_entity_tbl[421] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 3 MIP
tr5_entity_tbl[422] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 4
tr5_entity_tbl[423] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 4 MIP
tr5_entity_tbl[424] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 5
tr5_entity_tbl[425] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 5 MIP
tr5_entity_tbl[426] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 6
tr5_entity_tbl[427] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 6 MIP
tr5_entity_tbl[428] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 7
tr5_entity_tbl[429] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 7 MIP
tr5_entity_tbl[430] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 8
tr5_entity_tbl[431] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 8 MIP
tr5_entity_tbl[432] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 9
tr5_entity_tbl[433] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 9 MIP
tr5_entity_tbl[434] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 10
tr5_entity_tbl[435] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 10 MIP
tr5_entity_tbl[436] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 11
tr5_entity_tbl[437] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 11 MIP
tr5_entity_tbl[438] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 12
tr5_entity_tbl[439] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 12 MIP
tr5_entity_tbl[440] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX}; -- Animating 13 / map 3 - light
tr5_entity_tbl[441] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 13 MIP
tr5_entity_tbl[442] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 14
tr5_entity_tbl[443] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 14 MIP
tr5_entity_tbl[444] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX}; -- Animating 15 / map 3 - light
tr5_entity_tbl[445] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 15 MIP
tr5_entity_tbl[446] = {coll = COLLISION_NONE,            shape = COLLISION_SHAPE_BOX}; -- Animating 16 / map 2 - light
tr5_entity_tbl[447] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Animating 16 MIP

-- STATIC TERRAIN

tr5_entity_tbl[448] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bridge flat
tr5_entity_tbl[449] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bridge flat MIP
tr5_entity_tbl[450] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bridge tilt 1
tr5_entity_tbl[451] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bridge tilt 1 MIP
tr5_entity_tbl[452] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bridge tilt 2
tr5_entity_tbl[453] = {coll = COLLISION_GROUP_KINEMATIC, shape = COLLISION_SHAPE_TRIMESH}; -- Bridge tilt 2 MIP


function getEntityModelProperties(ver, id)
    tbl = {};
    if(ver < TR_II) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        tbl = tr1_entity_tbl;
    elseif(ver < TR_III) then               -- TR_II, TR_II_DEMO
        tbl = tr2_entity_tbl;
    elseif(ver < TR_IV) then                -- TR_III
        tbl = tr3_entity_tbl;
    elseif(ver < TR_V) then                 -- TR_IV, TR_IV_DEMO
        tbl = tr4_entity_tbl;
    elseif(ver == TR_V) then                -- TR_V
        tbl = tr5_entity_tbl;
    else
        return nil, nil, nil;
    end;

    if(tbl[id] == nil) then
        return COLLISION_GROUP_STATIC_OBLECT, COLLISION_SHAPE_BOX, nil, nil;
    else
        return tbl[id].coll, tbl[id].shape, tbl[id].hide;
    end;
end;


function getEntityFunction(ver, id)
    tbl = {};
    if(ver < TR_II) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        tbl = tr1_entity_tbl;
    elseif(ver < TR_III) then               -- TR_II, TR_II_DEMO
        tbl = tr2_entity_tbl;
    elseif(ver < TR_IV) then                -- TR_III
        tbl = tr3_entity_tbl;
    elseif(ver < TR_V) then                 -- TR_IV, TR_IV_DEMO
        tbl = tr4_entity_tbl;
    elseif(ver == TR_V) then                -- TR_V
        tbl = tr5_entity_tbl;
    else
        return nil;
    end;

    if(tbl[id] == nil) then
        return nil;
    else
        return tbl[id].func;
    end;
end;

print("entity_properties->function / collide / visibility flag loaded !");
