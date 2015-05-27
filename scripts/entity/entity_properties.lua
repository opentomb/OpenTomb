-- OPENTOMB ENTITY PROPERTIES CONFIGURATION
-- By TeslaRus, Lwmte 2013-2014

--------------------------------------------------------------------------------
-- [ hide ] flag values:

--   0x00    - Object is visible.
--   0x01    - Object is invisible.

-- [ trav ] flag values:

--   0x00    - Object is not pushable.
--   0x10    - Object is pushable, but can not been floor for other pushable
--   0x18    - Object is pushable, and can be floor for other pushable

-- [ coll ] flag values:

--   0x0000  - Object has no collisions
--   0x0001  - Object uses real mesh data for collision.
--   0x0002  - Object uses bounding box for collision.
--   0x0003  - Object uses sphere of a specified radius for collision.

-- [ func ] :

-- Function which spawns a scripted behaviour for ALL entities sharing the same
-- entity model ID.

--------------------------------------------------------------------------------

print("Collide / visibility flags script loaded");

--------------------------------------------------------------------------------
----------------------------- TR_I, TR_I_DEMO, TR_I_UB -------------------------
--------------------------------------------------------------------------------

tr1_entity_tbl = {};

-- NOTE: Objects before ID 06 are internal service objects and never appear in-game.

-- ACTORS --

tr1_entity_tbl[06] = {coll = 0x02};                      -- Doppelgagner
tr1_entity_tbl[07] = {coll = 0x02, func = "baddie"};                      -- Wolf
tr1_entity_tbl[08] = {coll = 0x02, func = "baddie"};                      -- Bear
tr1_entity_tbl[09] = {coll = 0x02, func = "baddie"};                      -- Bat
tr1_entity_tbl[10] = {coll = 0x02, func = "baddie"};                      -- Crocodile (land)
tr1_entity_tbl[11] = {coll = 0x02, func = "baddie"};                      -- Crocodile (water)
tr1_entity_tbl[12] = {coll = 0x02, func = "baddie"};                      -- Lion Male
tr1_entity_tbl[13] = {coll = 0x02, func = "baddie"};                      -- Lion Female
tr1_entity_tbl[14] = {coll = 0x02, func = "baddie"};                      -- Puma
tr1_entity_tbl[15] = {coll = 0x02, func = "baddie"};                      -- Gorilla
tr1_entity_tbl[16] = {coll = 0x02, func = "baddie"};                      -- Rat (land)
tr1_entity_tbl[17] = {coll = 0x02, func = "baddie"};                      -- Rat (water)
tr1_entity_tbl[18] = {coll = 0x01, func = "baddie"};                      -- T-Rex
tr1_entity_tbl[19] = {coll = 0x02, func = "baddie"};                      -- Raptor
tr1_entity_tbl[20] = {coll = 0x01, func = "baddie"};                      -- Winged mutant
tr1_entity_tbl[21] = {coll = 0x00, hide = 0x01};         -- (RESPAWN POINT?)
tr1_entity_tbl[22] = {coll = 0x00, hide = 0x01};         -- (AI TARGET?)
tr1_entity_tbl[23] = {coll = 0x01};                      -- Centaur
tr1_entity_tbl[24] = {coll = 0x02};                      -- Mummy
tr1_entity_tbl[25] = {coll = 0x02};                      -- DinoWarrior (UNUSED!)
tr1_entity_tbl[26] = {coll = 0x02};                      -- Fish
tr1_entity_tbl[27] = {coll = 0x02, func = "baddie"};                      -- Larson
tr1_entity_tbl[28] = {coll = 0x02, func = "baddie"};                      -- Pierre
tr1_entity_tbl[29] = {coll = 0x02};                      -- Skateboard
tr1_entity_tbl[30] = {coll = 0x02, func = "baddie"};                      -- Skateboard Kid
tr1_entity_tbl[31] = {coll = 0x02, func = "baddie"};                      -- Cowboy
tr1_entity_tbl[32] = {coll = 0x02, func = "baddie"};                      -- Mr. T
tr1_entity_tbl[33] = {coll = 0x02, func = "baddie"};                      -- Natla (winged)
tr1_entity_tbl[34] = {coll = 0x01, func = "baddie"};                      -- Torso Boss

-- ANIMATINGS --

tr1_entity_tbl[35] = {coll = 0x02, func = "fallblock"};  -- Falling floor
tr1_entity_tbl[36] = {coll = 0x01, func = "swingblade"}; -- Swinging blade (Vilcabamba, etc.)
tr1_entity_tbl[37] = {coll = 0x02, func = "oldspike"};   -- Spikes
tr1_entity_tbl[38] = {coll = 0x01, func = "boulder"};                      -- Boulder
tr1_entity_tbl[39] = {coll = 0x01};                      -- Dart
tr1_entity_tbl[40] = {coll = 0x01};                      -- Dartgun
tr1_entity_tbl[41] = {coll = 0x01};                      -- Lifting door
tr1_entity_tbl[42] = {coll = 0x01, func = "slamdoor"};   -- Slamming sawtooth doors
tr1_entity_tbl[43] = {coll = 0x01};                      -- Sword of Damocles
tr1_entity_tbl[44] = {coll = 0x01};                      -- Thor's hammer (handle)
tr1_entity_tbl[45] = {coll = 0x01};                      -- Thor's hammer (block)
tr1_entity_tbl[46] = {coll = 0x01, hide = 0x01};                      -- Thor's lightning ball
tr1_entity_tbl[47] = {coll = 0x01};                      -- Barricade
tr1_entity_tbl[48] = {coll = 0x02, trav = 0x18, func = "pushable"};         -- Pushable block
tr1_entity_tbl[49] = {coll = 0x02, trav = 0x18, func = "pushable"};         -- Pushable block
tr1_entity_tbl[50] = {coll = 0x02, trav = 0x18, func = "pushable"};         -- Pushable block
tr1_entity_tbl[51] = {coll = 0x02, trav = 0x18, func = "pushable"};         -- Pushable block
tr1_entity_tbl[52] = {coll = 0x01, func = "tallblock"};                      -- Moving block
tr1_entity_tbl[53] = {coll = 0x02, func = "fallceiling"};                      -- Falling ceiling
tr1_entity_tbl[54] = {coll = 0x01};                      -- Sword of Damocles (unused?)
tr1_entity_tbl[55] = {coll = 0x02, func = "switch"};     -- Wall switch (lever)
tr1_entity_tbl[56] = {coll = 0x02, func = "switch"};     -- Underwater switch (lever)

-- DOORS --

tr1_entity_tbl[57] = {coll = 0x01, func = "door"};       -- Door
tr1_entity_tbl[58] = {coll = 0x01, func = "door"};       -- Door
tr1_entity_tbl[59] = {coll = 0x01, func = "door"};       -- Door
tr1_entity_tbl[60] = {coll = 0x01, func = "door"};       -- Door
tr1_entity_tbl[61] = {coll = 0x01, func = "door"};       -- Door
tr1_entity_tbl[62] = {coll = 0x01, func = "door"};       -- Door
tr1_entity_tbl[63] = {coll = 0x01, func = "door"};       -- Door
tr1_entity_tbl[64] = {coll = 0x01, func = "door"};       -- Door

tr1_entity_tbl[65] = {coll = 0x01, func = "door"};       -- Floor trapdoor
tr1_entity_tbl[66] = {coll = 0x01, func = "door"};       -- Floor trapdoor

-- COLLISION OBJECTS --

tr1_entity_tbl[68] = {coll = 0x01};                      -- Bridge flat
tr1_entity_tbl[69] = {coll = 0x01};                      -- Bridge tilt 1
tr1_entity_tbl[70] = {coll = 0x01};                      -- Bridge tilt 2

-- MENU ITEMS AND ANIMATINGS --

tr1_entity_tbl[71] = {coll = 0x00};                      -- Menu: Passport
tr1_entity_tbl[72] = {coll = 0x00};                      -- Menu: Compass
tr1_entity_tbl[73] = {coll = 0x00};                      -- Menu: Lara's Home photo

tr1_entity_tbl[74] = {coll = 0x02, func = "anim"};       -- Animating 1
tr1_entity_tbl[75] = {coll = 0x02, func = "anim"};       -- Animating 2
tr1_entity_tbl[76] = {coll = 0x02, func = "anim"};       -- Animating 3

tr1_entity_tbl[81] = {coll = 0x00};                      -- Menu: Passport (closed)

tr1_entity_tbl[82] = {coll = 0x00};                      -- Natla Logo
tr1_entity_tbl[83] = {coll = 0x01};                      -- Savegame crystal

tr1_entity_tbl[95] = {coll = 0x00};                      -- Menu: Sunglasses
tr1_entity_tbl[96] = {coll = 0x00};                      -- Menu: Cassette player
tr1_entity_tbl[97] = {coll = 0x00};                      -- Menu: Arrow keys
tr1_entity_tbl[98] = {coll = 0x00};                      -- Menu: Flashlight (UNUSED!)

-- GENERAL MENU ITEMS --

tr1_entity_tbl[99] = {coll = 0x00};                      -- Menu: Pistols
tr1_entity_tbl[100] = {coll = 0x00};                     -- Menu: Shotgun
tr1_entity_tbl[101] = {coll = 0x00};                     -- Menu: Magnums
tr1_entity_tbl[102] = {coll = 0x00};                     -- Menu: Uzis
tr1_entity_tbl[103] = {coll = 0x00};                     -- Menu: Pistol ammo
tr1_entity_tbl[104] = {coll = 0x00};                     -- Menu: Shotgun ammo
tr1_entity_tbl[105] = {coll = 0x00};                     -- Menu: Magnum ammo
tr1_entity_tbl[106] = {coll = 0x00};                     -- Menu: Uzi ammo
tr1_entity_tbl[107] = {coll = 0x00};                     -- Menu: Grenade ammo (UNUSED!)
tr1_entity_tbl[108] = {coll = 0x00};                     -- Pick-up: Small medipack
tr1_entity_tbl[109] = {coll = 0x00};                     -- Pick-up: Large medipack

-- PUZZLE PICKUPS --

tr1_entity_tbl[114] = {coll = 0x00};                     -- Pick-up: Puzzle 1
tr1_entity_tbl[115] = {coll = 0x00};                     -- Pick-up: Puzzle 2
tr1_entity_tbl[116] = {coll = 0x00};                     -- Pick-up: Puzzle 3
tr1_entity_tbl[117] = {coll = 0x00};                     -- Pick-up: Puzzle 4

-- PUZZLES, KEYS AND SLOTS FOR THEM --

tr1_entity_tbl[118] = {coll = 0x02, func = "keyhole"};   -- Slot 1 empty
tr1_entity_tbl[119] = {coll = 0x02, func = "keyhole"};   -- Slot 2 empty
tr1_entity_tbl[120] = {coll = 0x02, func = "keyhole"};   -- Slot 3 empty
tr1_entity_tbl[121] = {coll = 0x02, func = "keyhole"};   -- Slot 4 empty
tr1_entity_tbl[122] = {coll = 0x02};                     -- Slot 1 full
tr1_entity_tbl[123] = {coll = 0x02};                     -- Slot 2 full
tr1_entity_tbl[124] = {coll = 0x02};                     -- Slot 3 full
tr1_entity_tbl[125] = {coll = 0x02};                     -- Slot 4 full

tr1_entity_tbl[127] = {coll = 0x02};                     -- Puzzle item 1
tr1_entity_tbl[128] = {coll = 0x00, hide = 0x01, func = "midastouch"};        -- Midas gold touch

tr1_entity_tbl[133] = {coll = 0x00};                     -- Key 1
tr1_entity_tbl[134] = {coll = 0x00};                     -- Key 2
tr1_entity_tbl[135] = {coll = 0x00};                     -- Key 3
tr1_entity_tbl[136] = {coll = 0x00};                     -- Key 4

tr1_entity_tbl[137] = {coll = 0x02, func = "keyhole"};   -- Lock 1
tr1_entity_tbl[138] = {coll = 0x02, func = "keyhole"};   -- Lock 2
tr1_entity_tbl[139] = {coll = 0x02, func = "keyhole"};   -- Lock 3
tr1_entity_tbl[140] = {coll = 0x02, func = "keyhole"};   -- Lock 4

tr1_entity_tbl[145] = {coll = 0x02};                     -- Scion 1
tr1_entity_tbl[146] = {coll = 0x02};                     -- Scion 2
tr1_entity_tbl[147] = {coll = 0x00};                     -- Scion holder
tr1_entity_tbl[150] = {coll = 0x00};                     -- Scion piece

-- ANIMATINGS --

tr1_entity_tbl[161] = {coll = 0x01};                     -- Centaur statue
tr1_entity_tbl[162] = {coll = 0x01};                     -- Natla's mines cabin
tr1_entity_tbl[163] = {coll = 0x01};                     -- Mutant egg

-- SERVICE OBJECTS --

tr1_entity_tbl[166] = {coll = 0x00};                     -- Gunflash
tr1_entity_tbl[169] = {coll = 0x00, hide = 0x01};        -- Camera target
tr1_entity_tbl[170] = {coll = 0x00, hide = 0x01};        -- Waterfall mist

tr1_entity_tbl[172] = {coll = 0x01};                     -- Mutant bullet
tr1_entity_tbl[173] = {coll = 0x01};                     -- Mutant grenade

tr1_entity_tbl[177] = {coll = 0x00, hide = 0x01};        -- Lava particle emitter

tr1_entity_tbl[179] = {coll = 0x00, hide = 0x01};        -- Flame emitter
tr1_entity_tbl[180] = {coll = 0x00};                     -- Moving lava mass
tr1_entity_tbl[181] = {coll = 0x01};                     -- Mutant egg (big)
tr1_entity_tbl[182] = {coll = 0x01};                     -- Motorboat

tr1_entity_tbl[183] = {coll = 0x01, hide = 0x01};        -- [UNKNOWN YET]

--------------------------------------------------------------------------------
--------------------------------- TR_II, TR_II_DEMO ----------------------------
--------------------------------------------------------------------------------
tr2_entity_tbl = {};

-- Remark: object IDs 0-12 are used for Lara model and never show in game
-- independently.

-- VEHICLES --

tr2_entity_tbl[13] = {coll = 0x01};                -- Red snowmobile (can go fast)
tr2_entity_tbl[14] = {coll = 0x01};                -- Boat
tr2_entity_tbl[51] = {coll = 0x01};                -- Black snowmobile (with guns)

-- ACTORS --

tr2_entity_tbl[15] = {coll = 0x02, func = "baddie"};                -- Doberman
tr2_entity_tbl[16] = {coll = 0x02, func = "baddie"};                -- Masked goon (white mask, jacket)
tr2_entity_tbl[17] = {coll = 0x02, func = "baddie"};                -- Masked goon (white mask, vest)
tr2_entity_tbl[18] = {coll = 0x02, func = "baddie"};                -- Masked goon (black mask)
tr2_entity_tbl[19] = {coll = 0x02, func = "baddie"};                -- Knifethrower
tr2_entity_tbl[20] = {coll = 0x02, func = "baddie"};                -- Shotgun goon
tr2_entity_tbl[21] = {coll = 0x02, func = "baddie"};                -- Rat
tr2_entity_tbl[22] = {coll = 0x02};                -- Dragon (front)
tr2_entity_tbl[23] = {coll = 0x02};                -- Dragon (back)
tr2_entity_tbl[24] = {coll = 0x02};                -- Gondola (Venetian boat)
tr2_entity_tbl[25] = {coll = 0x02, func = "baddie"};                -- Shark
tr2_entity_tbl[26] = {coll = 0x02, func = "baddie"};                -- Yellow moray eel
tr2_entity_tbl[27] = {coll = 0x02, func = "baddie"};                -- Black moray eel
tr2_entity_tbl[28] = {coll = 0x02, func = "baddie"};                -- Barracuda / Whiskered Fish
tr2_entity_tbl[29] = {coll = 0x02, func = "baddie"};                -- Scuba diver
tr2_entity_tbl[30] = {coll = 0x02, func = "baddie"};                -- Gun-wielding rig worker (khaki pants)
tr2_entity_tbl[31] = {coll = 0x02, func = "baddie"};                -- Gun-wielding rig worker (blue jeans)
tr2_entity_tbl[32] = {coll = 0x02, func = "baddie"};                -- Stick-wielding goon
tr2_entity_tbl[33] = {coll = 0x02, func = "baddie"};                -- Stick-wielding goon (can't climb)
tr2_entity_tbl[34] = {coll = 0x02, func = "baddie"};                -- Flamethrower-wielding goon
tr2_entity_tbl[36] = {coll = 0x02, func = "baddie"};                -- Spider
tr2_entity_tbl[37] = {coll = 0x02, func = "baddie"};                -- Giant spider
tr2_entity_tbl[38] = {coll = 0x02, func = "baddie"};                -- Crow
tr2_entity_tbl[39] = {coll = 0x02, func = "baddie"};                -- Tiger / Leopard
tr2_entity_tbl[40] = {coll = 0x02, func = "baddie"};                -- Marco Bartoli
tr2_entity_tbl[41] = {coll = 0x02, func = "baddie"};                -- Spear-wielding Xian Guard
tr2_entity_tbl[42] = {coll = 0x02};                -- Spear-wielding Xian Guard statue
tr2_entity_tbl[43] = {coll = 0x02, func = "baddie"};                -- Sword-wielding Xian Guard
tr2_entity_tbl[44] = {coll = 0x02};                -- Sword-wielding Xian Guard statue
tr2_entity_tbl[45] = {coll = 0x02, func = "baddie"};                -- Yeti
tr2_entity_tbl[46] = {coll = 0x02, func = "baddie"};                -- Bird monster (guards Talion)
tr2_entity_tbl[47] = {coll = 0x02, func = "baddie"};                -- Eagle
tr2_entity_tbl[48] = {coll = 0x02, func = "baddie"};                -- Mercenary
tr2_entity_tbl[49] = {coll = 0x02, func = "baddie"};                -- Mercenary (black ski mask, gray jacket)
tr2_entity_tbl[50] = {coll = 0x02, func = "baddie"};                -- Mercenary (black ski mask, brown jacket)
tr2_entity_tbl[52] = {coll = 0x02, func = "baddie"};                -- Mercenary snowmobile driver
tr2_entity_tbl[53] = {coll = 0x02, func = "baddie"};                -- Monk with long stick
tr2_entity_tbl[54] = {coll = 0x02, func = "baddie"};                -- Monk with knife-end stick

-- TRAPS --

tr2_entity_tbl[55] = {coll = 0x01, func = "fallblock"}; -- Collapsible floor
tr2_entity_tbl[57] = {coll = 0x01};                     -- Loose boards
tr2_entity_tbl[58] = {coll = 0x02};                     -- Swinging sandbag / spiky ball
tr2_entity_tbl[59] = {coll = 0x02, func = "oldspike"};  -- Spikes / Glass shards
tr2_entity_tbl[60] = {coll = 0x01, func = "boulder"};                     -- Boulder
tr2_entity_tbl[61] = {coll = 0x02};                     -- Disk (like dart)
tr2_entity_tbl[62] = {coll = 0x00};                     -- Wall-mounted disk shooter (like dartgun)
tr2_entity_tbl[63] = {coll = 0x01};                     -- Drawbridge
tr2_entity_tbl[64] = {coll = 0x02, func = "slamdoor"};                     -- Slamming door
tr2_entity_tbl[65] = {coll = 0x01};                     -- Elevator
tr2_entity_tbl[66] = {coll = 0x02};                     -- Minisub
tr2_entity_tbl[67] = {coll = 0x02, trav = 0x18, func = "pushable"};        -- Movable cubical block (pushable)
tr2_entity_tbl[68] = {coll = 0x02, trav = 0x18, func = "pushable"};        -- Movable cubical block (pushable)
tr2_entity_tbl[69] = {coll = 0x02, trav = 0x18, func = "pushable"};        -- Movable cubical block (pushable)
tr2_entity_tbl[70] = {coll = 0x02, trav = 0x18, func = "pushable"};        -- Movable cubical block (pushable)
tr2_entity_tbl[71] = {coll = 0x02};                     -- Big bowl (Ice Palace)
tr2_entity_tbl[72] = {coll = 0x02};                     -- Breakable window (can shoot out)
tr2_entity_tbl[73] = {coll = 0x02};                     -- Breakable window (must jump through)
tr2_entity_tbl[76] = {coll = 0x01};                     -- Airplane propeller
tr2_entity_tbl[77] = {coll = 0x02};                     -- Power saw
tr2_entity_tbl[78] = {coll = 0x02};                     -- Overhead pulley hook
tr2_entity_tbl[79] = {coll = 0x02, func = "fallceiling"};                     -- Sandbag / Ceiling fragments
tr2_entity_tbl[80] = {coll = 0x02};                     -- Rolling spindle
tr2_entity_tbl[81] = {coll = 0x02, func = "wallblade"};                     -- Wall-mounted knife blade
tr2_entity_tbl[82] = {coll = 0x02};                     -- Statue with knife blade
tr2_entity_tbl[83] = {coll = 0x01};                     -- Multiple boulders / snowballs
tr2_entity_tbl[84] = {coll = 0x02};                     -- Detachable icicles
tr2_entity_tbl[85] = {coll = 0x02};                     -- Spiky movable wall
tr2_entity_tbl[86] = {coll = 0x01};                     -- Bounce pad
tr2_entity_tbl[87] = {coll = 0x02};                     -- Spiky ceiling segment
tr2_entity_tbl[88] = {coll = 0x02};                     -- Tibetan bell
tr2_entity_tbl[91] = {coll = 0x00};                     -- Lara and a snowmobile
tr2_entity_tbl[92] = {coll = 0x00};                     -- Wheel knob
tr2_entity_tbl[93] = {coll = 0x00, func = "switch"};    -- Switch
tr2_entity_tbl[94] = {coll = 0x02};                     -- Underwater propeller
tr2_entity_tbl[95] = {coll = 0x02};                     -- Air fan
tr2_entity_tbl[96] = {coll = 0x02};                     -- Swinging box / spiky ball
tr2_entity_tbl[101] = {coll = 0x02};                    -- Rolling storage drums
tr2_entity_tbl[102] = {coll = 0x02};                    -- Zipline handle
tr2_entity_tbl[103] = {coll = 0x00, func = "switch"};   -- Switch
tr2_entity_tbl[104] = {coll = 0x00, func = "switch"};   -- Switch
tr2_entity_tbl[105] = {coll = 0x00, func = "switch"};   -- Underwater switch

-- DOORS --

tr2_entity_tbl[106] = {coll = 0x01, func = "door"};     -- Door
tr2_entity_tbl[107] = {coll = 0x01, func = "door"};     -- Door
tr2_entity_tbl[108] = {coll = 0x01, func = "door"};     -- Door
tr2_entity_tbl[109] = {coll = 0x01, func = "door"};     -- Door
tr2_entity_tbl[110] = {coll = 0x01, func = "door"};     -- Door
tr2_entity_tbl[111] = {coll = 0x02, func = "door"};     -- Door (pulled upward in Temple of Xian)
tr2_entity_tbl[112] = {coll = 0x02, func = "door"};     -- Door (pulled upward in Temple of Xian)
tr2_entity_tbl[113] = {coll = 0x02, func = "door"};     -- Door (pulled upward)
tr2_entity_tbl[114] = {coll = 0x01, func = "door"};     -- Trapdoor (opens downward)
tr2_entity_tbl[115] = {coll = 0x01, func = "door"};     -- Trapdoor (opens downward)
tr2_entity_tbl[116] = {coll = 0x01, func = "door"};     -- Trapdoor (opens downward)
tr2_entity_tbl[117] = {coll = 0x01};                    -- Bridge (flat)
tr2_entity_tbl[118] = {coll = 0x01};                    -- Bridge (slope = 1)
tr2_entity_tbl[119] = {coll = 0x01};                    -- Bridge (slope = 2)

-- MISC ITEMS --

tr2_entity_tbl[120] = {coll = 0x00};               -- Secret #1
tr2_entity_tbl[121] = {coll = 0x00};               -- Secret #2
tr2_entity_tbl[122] = {coll = 0x00};               -- Lara and butler picture
tr2_entity_tbl[133] = {coll = 0x00};               -- Secret #3
tr2_entity_tbl[134] = {coll = 0x00};               -- Natla's logo
tr2_entity_tbl[152] = {coll = 0x00};               -- Flare
tr2_entity_tbl[153] = {coll = 0x00};               -- Sunglasses
tr2_entity_tbl[154] = {coll = 0x00};               -- Portable CD player
tr2_entity_tbl[155] = {coll = 0x00};               -- Direction keys
tr2_entity_tbl[157] = {coll = 0x00};               -- Pistol
tr2_entity_tbl[158] = {coll = 0x00};               -- Shotgun
tr2_entity_tbl[159] = {coll = 0x00};               -- Auto-pistol
tr2_entity_tbl[160] = {coll = 0x00};               -- Uzi
tr2_entity_tbl[161] = {coll = 0x00};               -- Harpoon gun
tr2_entity_tbl[162] = {coll = 0x00};               -- M16
tr2_entity_tbl[163] = {coll = 0x00};               -- Grenade launcher
tr2_entity_tbl[164] = {coll = 0x00};               -- Pistol ammo(?)
tr2_entity_tbl[165] = {coll = 0x00};               -- Shotgun ammo
tr2_entity_tbl[166] = {coll = 0x00};               -- Auto-pistol ammo
tr2_entity_tbl[167] = {coll = 0x00};               -- Uzi ammo
tr2_entity_tbl[168] = {coll = 0x00};               -- Harpoons
tr2_entity_tbl[169] = {coll = 0x00};               -- M16 ammo
tr2_entity_tbl[170] = {coll = 0x00};               -- Grenades
tr2_entity_tbl[171] = {coll = 0x00};               -- Small medipack
tr2_entity_tbl[172] = {coll = 0x00};               -- Large medipack
tr2_entity_tbl[173] = {coll = 0x00};               -- Flares (opening box)
tr2_entity_tbl[178] = {coll = 0x00};               -- Puzzle 1
tr2_entity_tbl[179] = {coll = 0x00};               -- Puzzle 2
tr2_entity_tbl[180] = {coll = 0x00};               -- Puzzle 3 ?
tr2_entity_tbl[181] = {coll = 0x00};               -- Puzzle 4
tr2_entity_tbl[182] = {coll = 0x00, func = "keyhole"};               -- Slot 1 empty
tr2_entity_tbl[183] = {coll = 0x00, func = "keyhole"};               -- Slot 2 empty
tr2_entity_tbl[184] = {coll = 0x00, func = "keyhole"};               -- Slot 3 empty ?
tr2_entity_tbl[185] = {coll = 0x00, func = "keyhole"};               -- Slot 4 empty
tr2_entity_tbl[186] = {coll = 0x00};               -- Slot 1 full
tr2_entity_tbl[187] = {coll = 0x00};               -- Slot 2 full
tr2_entity_tbl[188] = {coll = 0x00};               -- Slot 3 full ?
tr2_entity_tbl[189] = {coll = 0x00};               -- Slot 4 full
tr2_entity_tbl[197] = {coll = 0x00};               -- Key 1
tr2_entity_tbl[198] = {coll = 0x00};               -- Key 2
tr2_entity_tbl[199] = {coll = 0x00};               -- Key 3
tr2_entity_tbl[200] = {coll = 0x00};               -- Key 4
tr2_entity_tbl[201] = {coll = 0x00, func = "keyhole"};               -- Lock 1
tr2_entity_tbl[202] = {coll = 0x00, func = "keyhole"};               -- Lock 2
tr2_entity_tbl[203] = {coll = 0x00, func = "keyhole"};               -- Lock 3
tr2_entity_tbl[204] = {coll = 0x00, func = "keyhole"};               -- Lock 4
tr2_entity_tbl[207] = {coll = 0x00};               -- Pickup 5
tr2_entity_tbl[208] = {coll = 0x00};               -- Pickup 6
tr2_entity_tbl[209] = {coll = 0x00};               -- Dragon explosion effect (expanding netted bubble)
tr2_entity_tbl[210] = {coll = 0x00};               -- Dragon explosion effect (expanding netted bubble)
tr2_entity_tbl[211] = {coll = 0x00};               -- Dragon explosion effect (expanding solid bubble)
tr2_entity_tbl[212] = {coll = 0x00, func = "alarm_TR2"};               -- Alarm
tr2_entity_tbl[213] = {coll = 0x00, hide = 0x01, func = "drips"};  -- Dripping water
tr2_entity_tbl[214] = {coll = 0x02};               -- Tyrannosaur
tr2_entity_tbl[215] = {coll = 0x00, hide = 0x01, func = "venicebird"};  -- Singing birds
tr2_entity_tbl[216] = {coll = 0x00, hide = 0x01};  -- Placeholder
tr2_entity_tbl[217] = {coll = 0x00, hide = 0x01};  -- Placeholder
tr2_entity_tbl[218] = {coll = 0x02};               -- Dragon bones (front)
tr2_entity_tbl[219] = {coll = 0x02};               -- Dragon bones (back)
tr2_entity_tbl[222] = {coll = 0x02};               -- Aquatic Mine (Venice)
tr2_entity_tbl[223] = {coll = 0x00};               -- Menu background
tr2_entity_tbl[225] = {coll = 0x00};               -- Gong-hammering animation
tr2_entity_tbl[226] = {coll = 0x00};               -- Gong (Ice Palace)
tr2_entity_tbl[227] = {coll = 0x00};               -- Detonator box
tr2_entity_tbl[228] = {coll = 0x00, func = "heli_rig_TR2"};               -- Helicopter (Diving Area)
tr2_entity_tbl[235] = {coll = 0x00};               -- Flare burning?
tr2_entity_tbl[240] = {coll = 0x00};               -- Gunflare
tr2_entity_tbl[241] = {coll = 0x00};               -- Gunflare (M16)
tr2_entity_tbl[243] = {coll = 0x00, hide = 0x01};  -- Camera target
tr2_entity_tbl[244] = {coll = 0x00, hide = 0x01};  -- Camera target - 2 (?)
tr2_entity_tbl[245] = {coll = 0x01};               -- Harpoon (single)
tr2_entity_tbl[247] = {coll = 0x01};               -- Pointer?
tr2_entity_tbl[248] = {coll = 0x01};               -- Grenade (single)
tr2_entity_tbl[249] = {coll = 0x01};               -- Harpoon (flying)
tr2_entity_tbl[251] = {coll = 0x00, hide = 0x01};  -- Sparks
tr2_entity_tbl[253] = {coll = 0x00, hide = 0x01};  -- Fire
tr2_entity_tbl[254] = {coll = 0x00};               -- Skybox
tr2_entity_tbl[256] = {coll = 0x01};               -- Monk
tr2_entity_tbl[257] = {coll = 0x00, hide = 0x01, func = "doorbell"}   -- Door bell
tr2_entity_tbl[258] = {coll = 0x00, hide = 0x01, func = "alarmbell"}   -- Alarm bell
tr2_entity_tbl[259] = {coll = 0x01, func = "heli_TR2"};               -- Helicopter
tr2_entity_tbl[260] = {coll = 0x02};               -- The butler
tr2_entity_tbl[262] = {coll = 0x00, hide = 0x01};  -- Lara cutscene placement?
tr2_entity_tbl[263] = {coll = 0x00};               -- Shotgun animation (Home Sweet Home)
tr2_entity_tbl[264] = {coll = 0x00, hide = 0x01};  -- Dragon transform wave


--------------------------------------------------------------------------------
-------------------------------------- TR_III ----------------------------------
--------------------------------------------------------------------------------
tr3_entity_tbl = {};

-- NOTE: Objects before ID 12 are internal service objects and never appear in-game.

-- VEHICLES --

tr3_entity_tbl[12] = {coll = 0x01};               -- UPV
tr3_entity_tbl[14] = {coll = 0x01};               -- Kayak
tr3_entity_tbl[15] = {coll = 0x01};               -- Inflatable boat
tr3_entity_tbl[16] = {coll = 0x01};               -- Quadbike
tr3_entity_tbl[17] = {coll = 0x01};               -- Mine cart
tr3_entity_tbl[18] = {coll = 0x01};               -- Big gun

-- ACTORS --

tr3_entity_tbl[19] = {coll = 0x02};               -- Hydro propeller (?)
tr3_entity_tbl[20] = {coll = 0x02, func = "baddie"};               -- Tribesman with spiked axe
tr3_entity_tbl[21] = {coll = 0x02, func = "baddie"};               -- Tribesman with poison-dart gun
tr3_entity_tbl[22] = {coll = 0x02, func = "baddie"};               -- Dog
tr3_entity_tbl[23] = {coll = 0x02, func = "baddie"};               -- Rat
tr3_entity_tbl[24] = {coll = 0x00, hide = 0x01};     -- Kill All Triggers
tr3_entity_tbl[25] = {coll = 0x02, func = "baddie"};               -- Killer whale
tr3_entity_tbl[26] = {coll = 0x02, func = "baddie"};               -- Scuba diver
tr3_entity_tbl[27] = {coll = 0x02, func = "baddie"};               -- Crow
tr3_entity_tbl[28] = {coll = 0x02, func = "baddie"};               -- Tiger
tr3_entity_tbl[29] = {coll = 0x02, func = "baddie"};               -- Vulture
tr3_entity_tbl[30] = {coll = 0x01, func = "baddie"};               -- Assault-course target
tr3_entity_tbl[31] = {coll = 0x02, func = "baddie"};               -- Crawler mutant in closet
tr3_entity_tbl[32] = {coll = 0x02, func = "baddie"};               -- Crocodile (in water)
tr3_entity_tbl[34] = {coll = 0x02, func = "baddie"};               -- Compsognathus
tr3_entity_tbl[35] = {coll = 0x02, func = "baddie"};               -- Lizard thing
tr3_entity_tbl[36] = {coll = 0x02, func = "baddie"};               -- Puna guy
tr3_entity_tbl[37] = {coll = 0x02, func = "baddie"};               -- Mercenary
tr3_entity_tbl[38] = {coll = 0x01, func = "baddie"};               -- Raptor hung by rope (fish bait)
tr3_entity_tbl[39] = {coll = 0x02, func = "baddie"};               -- RX-Tech guy in red jacket
tr3_entity_tbl[40] = {coll = 0x02, func = "baddie"};               -- RX-Tech guy with gun (dressed like flamethrower guy)
tr3_entity_tbl[41] = {coll = 0x02, func = "baddie"};               -- Dog (Antarctica)
tr3_entity_tbl[42] = {coll = 0x02, func = "baddie"};               -- Crawler mutant
tr3_entity_tbl[44] = {coll = 0x02, func = "baddie"};               -- Tinnos wasp
tr3_entity_tbl[45] = {coll = 0x02, func = "baddie"};               -- Tinnos monster
tr3_entity_tbl[46] = {coll = 0x02, func = "baddie"};               -- Brute mutant (with claw)
tr3_entity_tbl[47] = {coll = 0x02, func = "baddie"};               -- Tinnos wasp respawn point
tr3_entity_tbl[48] = {coll = 0x02, func = "baddie"};               -- Raptor respawn point
tr3_entity_tbl[49] = {coll = 0x02, func = "baddie"};               -- Willard spider
tr3_entity_tbl[50] = {coll = 0x02, func = "baddie"};               -- RX-Tech flamethrower guy
tr3_entity_tbl[51] = {coll = 0x02, func = "baddie"};               -- London goon
tr3_entity_tbl[53] = {coll = 0x02, func = "baddie"};               -- 'Damned' stick-wielding goon
tr3_entity_tbl[56] = {coll = 0x02, func = "baddie"};               -- London guard
tr3_entity_tbl[57] = {coll = 0x02, func = "baddie"};               -- Sophia Lee
tr3_entity_tbl[58] = {coll = 0x01, func = "baddie"};               -- Thames Wharf machine
tr3_entity_tbl[60] = {coll = 0x02, func = "baddie"};               -- MP with stick
tr3_entity_tbl[61] = {coll = 0x02, func = "baddie"};               -- MP with gun
tr3_entity_tbl[62] = {coll = 0x02, func = "baddie"};               -- Prisoner
tr3_entity_tbl[63] = {coll = 0x02, func = "baddie"};               -- MP with sighted gun and night sight
tr3_entity_tbl[64] = {coll = 0x02};               -- Gun turret
tr3_entity_tbl[65] = {coll = 0x02, func = "baddie"};               -- Dam guard
tr3_entity_tbl[66] = {coll = 0x00, hide = 0x01};  -- Kind of tripwire
tr3_entity_tbl[67] = {coll = 0x00, hide = 0x01};  -- Electrified wire
tr3_entity_tbl[68] = {coll = 0x00, hide = 0x01};  -- Killer tripwire
tr3_entity_tbl[69] = {coll = 0x02, func = "baddie"};               -- Cobra / Rattlesnake
tr3_entity_tbl[70] = {coll = 0x01};               -- Temple statue
tr3_entity_tbl[71] = {coll = 0x02, func = "baddie"};               -- Monkey
tr3_entity_tbl[73] = {coll = 0x02, func = "baddie"};               -- Tony Firehands

-- AI OBJECTS --

tr3_entity_tbl[74] = {coll = 0x00, hide = 0x01};  -- AI Guard
tr3_entity_tbl[75] = {coll = 0x00, hide = 0x01};  -- AI Ambush
tr3_entity_tbl[76] = {coll = 0x00, hide = 0x01};  -- AI Path
tr3_entity_tbl[77] = {coll = 0x00, hide = 0x01};  -- AI Unknown #77
tr3_entity_tbl[78] = {coll = 0x00, hide = 0x01};  -- AI Follow
tr3_entity_tbl[79] = {coll = 0x00, hide = 0x01};  -- AI Patrol
tr3_entity_tbl[80] = {coll = 0x00, hide = 0x01};  -- Unknown Id #80
tr3_entity_tbl[81] = {coll = 0x00, hide = 0x01};  -- Unknown Id #81
tr3_entity_tbl[82] = {coll = 0x00, hide = 0x01};  -- Unknown Id #82

-- TRAPS & DOORS --

tr3_entity_tbl[83] = {coll = 0x01, func = "fallblock"};                 -- Collapsible floor
tr3_entity_tbl[86] = {coll = 0x01};                 -- Swinging thing
tr3_entity_tbl[87] = {coll = 0x01, func = "oldspike"};                 -- Spikes / Barbed wire
tr3_entity_tbl[88] = {coll = 0x01, func = "boulder"};                 -- Boulder / Barrel
tr3_entity_tbl[89] = {coll = 0x01};                 -- Giant boulder (Temple of Puna)
tr3_entity_tbl[90] = {coll = 0x01};                 -- Disk (like dart)
tr3_entity_tbl[91] = {coll = 0x01, hide = 0x01};                 -- Dart shooter
tr3_entity_tbl[94] = {coll = 0x01, func = "slamdoor"};                 -- Spiked impaled skeleton / Slamming door
tr3_entity_tbl[97] = {coll = 0x01, trav = 0x18, func = "pushable"};    -- Movable cubical block (pushable)
tr3_entity_tbl[98] = {coll = 0x01, trav = 0x18, func = "pushable"};    -- Movable cubical block (pushable)
tr3_entity_tbl[101] = {coll = 0x01};                -- Destroyable boarded-up window
tr3_entity_tbl[102] = {coll = 0x01};                -- Destroyable boarded-up window / wall
tr3_entity_tbl[106] = {coll = 0x01};                -- Overhead pulley hook
tr3_entity_tbl[107] = {coll = 0x01, func = "fallceiling"};                -- Falling fragments
tr3_entity_tbl[108] = {coll = 0x01};                -- Rolling spindle
tr3_entity_tbl[110] = {coll = 0x01};                -- Subway train
tr3_entity_tbl[111] = {coll = 0x01, func = "wallblade"};                -- Wall-mounted knife blade / Knife disk
tr3_entity_tbl[113] = {coll = 0x01};                -- Detachable stalactites
tr3_entity_tbl[114] = {coll = 0x01};                -- Spiky movable wall
tr3_entity_tbl[116] = {coll = 0x01};                -- Spiky movable vertical wall / Tunnel borer
tr3_entity_tbl[117] = {coll = 0x01};                -- Valve wheel / Pulley
tr3_entity_tbl[118] = {coll = 0x01, func = "switch"};                -- Switch
tr3_entity_tbl[119] = {coll = 0x01};                -- Underwater propeller / Diver sitting on block / Underwater rotating knives / Meteorite
tr3_entity_tbl[120] = {coll = 0x01};                -- Fan
tr3_entity_tbl[121] = {coll = 0x01};                -- Heavy stamper / Grinding drum / Underwater rotating knives
tr3_entity_tbl[122] = {coll = 0x01};                -- Temple statue (original petrified state)
tr3_entity_tbl[123] = {coll = 0x02};                -- Monkey with medipack
tr3_entity_tbl[124] = {coll = 0x02};                -- Monkey with key
tr3_entity_tbl[127] = {coll = 0x02};                -- Zipline handle
tr3_entity_tbl[128] = {coll = 0x01, func = "switch"};                -- Switch
tr3_entity_tbl[129] = {coll = 0x01, func = "switch"};                -- Switch
tr3_entity_tbl[130] = {coll = 0x01, func = "switch"};                -- Underwater switch
tr3_entity_tbl[131] = {coll = 0x01, func = "door"};                -- Door
tr3_entity_tbl[132] = {coll = 0x01, func = "door"};                -- Door
tr3_entity_tbl[133] = {coll = 0x01, func = "door"};                -- Door
tr3_entity_tbl[134] = {coll = 0x01, func = "door"};                -- Door
tr3_entity_tbl[135] = {coll = 0x01, func = "door"};                -- Door
tr3_entity_tbl[136] = {coll = 0x01, func = "door"};                -- Door
tr3_entity_tbl[137] = {coll = 0x01, func = "door"};                -- Door
tr3_entity_tbl[138] = {coll = 0x01, func = "door"};                -- Door
tr3_entity_tbl[139] = {coll = 0x01, func = "door"};                -- Trapdoor
tr3_entity_tbl[140] = {coll = 0x01, func = "door"};                -- Trapdoor
tr3_entity_tbl[141] = {coll = 0x01, func = "door"};                -- Trapdoor
tr3_entity_tbl[142] = {coll = 0x01};                -- Bridge (flat)
tr3_entity_tbl[143] = {coll = 0x01};                -- Bridge (slope = 1)
tr3_entity_tbl[144] = {coll = 0x01};                -- Bridge (slope = 2)

-- PICK-UPS --

tr3_entity_tbl[145] = {coll = 0x00};           -- Passport (opening up)
tr3_entity_tbl[146] = {coll = 0x00};           -- Stopwatch
tr3_entity_tbl[147] = {coll = 0x00};           -- Lara's Home photo
tr3_entity_tbl[158] = {coll = 0x00};           -- Passport (closed)
tr3_entity_tbl[159] = {coll = 0x00};           -- Natla logo
tr3_entity_tbl[160] = {coll = 0x00};           -- Pistols (pick-up)
tr3_entity_tbl[161] = {coll = 0x00};           -- Shotgun (pick-up)
tr3_entity_tbl[162] = {coll = 0x00};           -- Desert Eagle (pick-up)
tr3_entity_tbl[163] = {coll = 0x00};           -- Uzis (pick-up)
tr3_entity_tbl[164] = {coll = 0x00};           -- Harpoon gun (pick-up)
tr3_entity_tbl[165] = {coll = 0x00};           -- MP5 (pick-up)
tr3_entity_tbl[166] = {coll = 0x00};           -- Rocket launcher (pick-up)
tr3_entity_tbl[167] = {coll = 0x00};           -- Grenade launcher (pick-up)
tr3_entity_tbl[168] = {coll = 0x00};           -- Pistol ammo (pick-up)
tr3_entity_tbl[169] = {coll = 0x00};           -- Shotgun ammo (pick-up)
tr3_entity_tbl[170] = {coll = 0x00};           -- Desert Eagle ammo (pick-up)
tr3_entity_tbl[171] = {coll = 0x00};           -- Uzi ammo (pick-up)
tr3_entity_tbl[172] = {coll = 0x00};           -- Harpoons (pick-up)
tr3_entity_tbl[173] = {coll = 0x00};           -- MP5 ammo (pick-up)
tr3_entity_tbl[174] = {coll = 0x00};           -- Rockets (pick-up)
tr3_entity_tbl[175] = {coll = 0x00};           -- Grenades (pick-up)
tr3_entity_tbl[176] = {coll = 0x00};           -- Small medipack (pick-up)
tr3_entity_tbl[177] = {coll = 0x00};           -- Large medipack (pick-up)
tr3_entity_tbl[178] = {coll = 0x00};           -- Flares (pick-up)
tr3_entity_tbl[179] = {coll = 0x00};           -- Flare (pick-up)
tr3_entity_tbl[180] = {coll = 0x00, func = "crystal_TR3"};           -- Savegame crystal (pick-up)

-- MENU ITEMS --

tr3_entity_tbl[181] = {coll = 0x00};           -- Sunglasses
tr3_entity_tbl[182] = {coll = 0x00};           -- Portable CD Player
tr3_entity_tbl[183] = {coll = 0x00};           -- Direction keys
tr3_entity_tbl[184] = {coll = 0x00};           -- Globe (for indicating destinations)
tr3_entity_tbl[185] = {coll = 0x00};           -- Pistols
tr3_entity_tbl[186] = {coll = 0x00};           -- Shotgun
tr3_entity_tbl[187] = {coll = 0x00};           -- Desert Eagle
tr3_entity_tbl[188] = {coll = 0x00};           -- Uzis
tr3_entity_tbl[189] = {coll = 0x00};           -- Harpoon gun
tr3_entity_tbl[190] = {coll = 0x00};           -- MP5
tr3_entity_tbl[191] = {coll = 0x00};           -- Rocket launcher
tr3_entity_tbl[192] = {coll = 0x00};           -- Grenade launcher
tr3_entity_tbl[193] = {coll = 0x00};           -- Pistol ammo
tr3_entity_tbl[194] = {coll = 0x00};           -- Shotgun ammo
tr3_entity_tbl[195] = {coll = 0x00};           -- Desert Eagle ammo
tr3_entity_tbl[196] = {coll = 0x00};           -- Uzi ammo
tr3_entity_tbl[197] = {coll = 0x00};           -- Harpoons
tr3_entity_tbl[198] = {coll = 0x00};           -- MP5 ammo
tr3_entity_tbl[199] = {coll = 0x00};           -- Rockets
tr3_entity_tbl[200] = {coll = 0x00};           -- Grenades
tr3_entity_tbl[201] = {coll = 0x00};           -- Small medipack
tr3_entity_tbl[202] = {coll = 0x00};           -- Large medipack
tr3_entity_tbl[203] = {coll = 0x00};           -- Flares
tr3_entity_tbl[204] = {coll = 0x00};           -- Savegame crystal

-- PUZZLE ITEMS & KEYS --

tr3_entity_tbl[205] = {coll = 0x00};           -- Puzzle 1 (pick-up)
tr3_entity_tbl[206] = {coll = 0x00};           -- Puzzle 2 (pick-up)
tr3_entity_tbl[207] = {coll = 0x00};           -- Puzzle 3 (pick-up)
tr3_entity_tbl[208] = {coll = 0x00};           -- Puzzle 4 (pick-up)
tr3_entity_tbl[209] = {coll = 0x00};           -- Puzzle 1 (menu item)
tr3_entity_tbl[210] = {coll = 0x00};           -- Puzzle 2 (menu item)
tr3_entity_tbl[211] = {coll = 0x00};           -- Puzzle 3 (menu item)
tr3_entity_tbl[212] = {coll = 0x00};           -- Puzzle 4 (menu item)
tr3_entity_tbl[213] = {coll = 0x02, func = "keyhole"};           -- Slot 1 empty
tr3_entity_tbl[214] = {coll = 0x02, func = "keyhole"};           -- Slot 2 empty
tr3_entity_tbl[215] = {coll = 0x02, func = "keyhole"};           -- Slot 3 empty
tr3_entity_tbl[216] = {coll = 0x02, func = "keyhole"};           -- Slot 4 empty
tr3_entity_tbl[217] = {coll = 0x02};           -- Slot 1 full
tr3_entity_tbl[218] = {coll = 0x02};           -- Slot 2 full
tr3_entity_tbl[219] = {coll = 0x02};           -- Slot 3 full
tr3_entity_tbl[220] = {coll = 0x02};           -- Slot 4 full
tr3_entity_tbl[224] = {coll = 0x00};           -- Key 1 (pick-up)
tr3_entity_tbl[225] = {coll = 0x00};           -- Key 2 (pick-up)
tr3_entity_tbl[226] = {coll = 0x00};           -- Key 3 (pick-up)
tr3_entity_tbl[227] = {coll = 0x00};           -- Key 4 (pick-up)
tr3_entity_tbl[228] = {coll = 0x00};           -- Key 1 (menu item)
tr3_entity_tbl[229] = {coll = 0x00};           -- Key 2 (menu item)
tr3_entity_tbl[230] = {coll = 0x00};           -- Key 3 (menu item)
tr3_entity_tbl[231] = {coll = 0x00};           -- Key 4 (menu item)
tr3_entity_tbl[232] = {coll = 0x02, func = "keyhole"};           -- Lock 1
tr3_entity_tbl[233] = {coll = 0x02, func = "keyhole"};           -- Lock 2
tr3_entity_tbl[234] = {coll = 0x02, func = "keyhole"};           -- Lock 3
tr3_entity_tbl[235] = {coll = 0x02, func = "keyhole"};           -- Lock 4
tr3_entity_tbl[236] = {coll = 0x00};           -- Pickup 1 (pick-up)
tr3_entity_tbl[237] = {coll = 0x00};           -- Pickup 2 .unused] (pick-up)
tr3_entity_tbl[238] = {coll = 0x00};           -- Pickup 1 (menu item)
tr3_entity_tbl[239] = {coll = 0x00};           -- Pickup 2 .unused] (menu item)
tr3_entity_tbl[240] = {coll = 0x00};           -- Infada stone (pick-up)
tr3_entity_tbl[241] = {coll = 0x00};           -- Element 115 (pick-up)
tr3_entity_tbl[242] = {coll = 0x00};           -- Eye of Isis (pick-up)
tr3_entity_tbl[243] = {coll = 0x00};           -- Ora dagger (pick-up)
tr3_entity_tbl[244] = {coll = 0x00};           -- Infada stone (menu item)
tr3_entity_tbl[245] = {coll = 0x00};           -- Element 115 (menu item)
tr3_entity_tbl[246] = {coll = 0x00};           -- Eye of Isis (menu item)
tr3_entity_tbl[247] = {coll = 0x00};           -- Ora dagger (menu item)
tr3_entity_tbl[272] = {coll = 0x00};           -- Keys (sprite)
tr3_entity_tbl[273] = {coll = 0x00};           -- Keys (sprite)
tr3_entity_tbl[276] = {coll = 0x00};           -- Infada stone
tr3_entity_tbl[277] = {coll = 0x00};           -- Element 115
tr3_entity_tbl[278] = {coll = 0x00};           -- Eye of Isis
tr3_entity_tbl[279] = {coll = 0x00};           -- Ora dagger

tr3_entity_tbl[282] = {coll = 0x00};           -- Fire-breathing dragon statue
tr3_entity_tbl[285] = {coll = 0x00};           -- Unknown visible #285

-- ENEMIES, cont. --

tr3_entity_tbl[287] = {coll = 0x01};           -- Tyrannosaur
tr3_entity_tbl[288] = {coll = 0x02};           -- Raptor

-- TRAPS, cont. --

tr3_entity_tbl[291] = {coll = 0x01};           -- Laser sweeper
tr3_entity_tbl[292] = {coll = 0x00, hide = 0x01}; -- Electrified Field
tr3_entity_tbl[295] = {coll = 0x01};           -- Detonator switch box

-- SERVICE ITEMS --

tr3_entity_tbl[300] = {coll = 0x00};                -- Gunflare / Gunflare (spiky)
tr3_entity_tbl[301] = {coll = 0x00};                -- Spiky gunflare for MP5
tr3_entity_tbl[304] = {coll = 0x00, hide = 0x01};   -- Look At item
tr3_entity_tbl[305] = {coll = 0x00, hide = 0x01};   -- Smoke Edge
tr3_entity_tbl[306] = {coll = 0x01};                -- Harpoon (single)
tr3_entity_tbl[309] = {coll = 0x01};                -- Rocket (single)
tr3_entity_tbl[310] = {coll = 0x01};                -- Harpoon (single)
tr3_entity_tbl[311] = {coll = 0x01};                -- Grenade (single)
tr3_entity_tbl[312] = {coll = 0x01};                -- Big missile
tr3_entity_tbl[313] = {coll = 0x00, hide = 0x01};   -- Smoke
tr3_entity_tbl[314] = {coll = 0x00, hide = 0x01};   -- Movable Boom
tr3_entity_tbl[315] = {coll = 0x00, hide = 0x01};   -- Lara true appearance
tr3_entity_tbl[317] = {coll = 0x00};                -- Unknown visible #317
tr3_entity_tbl[318] = {coll = 0x00, hide = 0x01};   -- Red ceiling rotating(?) light
tr3_entity_tbl[319] = {coll = 0x00, hide = 0x01};   -- Light
tr3_entity_tbl[321] = {coll = 0x00, hide = 0x01};   -- Light #2
tr3_entity_tbl[322] = {coll = 0x00, hide = 0x01};   -- Pulsating Light
tr3_entity_tbl[324] = {coll = 0x00, hide = 0x01};   -- Red Light
tr3_entity_tbl[325] = {coll = 0x00, hide = 0x01};   -- Green Light
tr3_entity_tbl[326] = {coll = 0x00, hide = 0x01};   -- Blue Light
tr3_entity_tbl[327] = {coll = 0x00, hide = 0x01};   -- Light #3
tr3_entity_tbl[328] = {coll = 0x00, hide = 0x01};   -- Light #4
tr3_entity_tbl[330] = {coll = 0x00, hide = 0x01};   -- Fire
tr3_entity_tbl[331] = {coll = 0x00, hide = 0x01};   -- Alternate Fire
tr3_entity_tbl[332] = {coll = 0x00, hide = 0x01};   -- Alternate Fire #2
tr3_entity_tbl[333] = {coll = 0x00, hide = 0x01};   -- Fire #2
tr3_entity_tbl[334] = {coll = 0x00, hide = 0x01};   -- Smoke #2
tr3_entity_tbl[335] = {coll = 0x00, hide = 0x01};   -- Smoke #3
tr3_entity_tbl[336] = {coll = 0x00, hide = 0x01};   -- Smoke #4
tr3_entity_tbl[337] = {coll = 0x00, hide = 0x01};   -- Greenish Smoke
tr3_entity_tbl[338] = {coll = 0x00, hide = 0x01};   -- Pirahnas
tr3_entity_tbl[339] = {coll = 0x00, hide = 0x01};   -- Fish
tr3_entity_tbl[347] = {coll = 0x00, hide = 0x01};   -- Bat swarm

-- ANIMATINGS --

tr3_entity_tbl[349] = {coll = 0x01};           -- Misc item (Animating 1)
tr3_entity_tbl[350] = {coll = 0x01};           -- Misc item (Animating 2)
tr3_entity_tbl[351] = {coll = 0x01};           -- Misc item (Animating 3)
tr3_entity_tbl[352] = {coll = 0x01};           -- Misc item (Animating 4)
tr3_entity_tbl[353] = {coll = 0x01};           -- Footstool / Fish swimming in tank / Radar display
tr3_entity_tbl[354] = {coll = 0x01};           -- Dead raptor / Alarm box / Mason-lodge dagger / Small version of big antenna


tr3_entity_tbl[355] = {coll = 0x00};                -- Skybox
tr3_entity_tbl[357] = {coll = 0x00, hide = 0x01};   -- Unknown id #357
tr3_entity_tbl[358] = {coll = 0x00, hide = 0x01};   -- Unknown id #358


tr3_entity_tbl[360] = {coll = 0x01};           -- The butler
tr3_entity_tbl[361] = {coll = 0x01};           -- The butler in military outfit and target


tr3_entity_tbl[365] = {coll = 0x00};           -- Earthquake
tr3_entity_tbl[366] = {coll = 0x00};           -- Yellow shell casing
tr3_entity_tbl[367] = {coll = 0x00};           -- Red shell casing
tr3_entity_tbl[370] = {coll = 0x00};           -- Tinnos light shaft
tr3_entity_tbl[373] = {coll = 0x02};           -- Electrical switch box

--------------------------------------------------------------------------------
--------------------------------- TR_IV, TR_IV_DEMO ----------------------------
--------------------------------------------------------------------------------
tr4_entity_tbl = {};

-- Remark: object IDs 0-30 are used for Lara model and speechheads, and never
-- show in game independently.

-- VEHICLES

tr4_entity_tbl[031] = {coll = 0x01}; -- Bike
tr4_entity_tbl[032] = {coll = 0x01}; -- Jeep
tr4_entity_tbl[034] = {coll = 0x01}; -- Enemy jeep

-- ENEMIES

tr4_entity_tbl[035] = {coll = 0x02}; -- Skeleton
tr4_entity_tbl[036] = {coll = 0x02}; -- Skeleton MIP - UNUSED
tr4_entity_tbl[037] = {coll = 0x02}; -- Guide
tr4_entity_tbl[038] = {coll = 0x02}; -- Guide MIP - UNUSED
tr4_entity_tbl[039] = {coll = 0x02}; -- Von Croy
tr4_entity_tbl[040] = {coll = 0x02}; -- Guide MIP - UNUSED
tr4_entity_tbl[041] = {coll = 0x02}; -- Baddy 1
tr4_entity_tbl[042] = {coll = 0x02}; -- Baddy 1 MIP - UNUSED
tr4_entity_tbl[043] = {coll = 0x02}; -- Baddy 2
tr4_entity_tbl[044] = {coll = 0x02}; -- Baddy 2 MIP - UNUSED
tr4_entity_tbl[045] = {coll = 0x02}; -- Setha
tr4_entity_tbl[046] = {coll = 0x02}; -- Setha MIP - UNUSED
tr4_entity_tbl[047] = {coll = 0x02}; -- Mummy
tr4_entity_tbl[048] = {coll = 0x02}; -- Mummy MIP - UNUSED
tr4_entity_tbl[049] = {coll = 0x02}; -- Sphinx / Bull
tr4_entity_tbl[050] = {coll = 0x02}; -- Sphinx / Bull MIP - UNUSED
tr4_entity_tbl[051] = {coll = 0x02}; -- Crocodile
tr4_entity_tbl[052] = {coll = 0x02}; -- Crocodile MIP - UNUSED
tr4_entity_tbl[053] = {coll = 0x02}; -- Horseman
tr4_entity_tbl[054] = {coll = 0x02}; -- Horseman MIP - UNUSED
tr4_entity_tbl[055] = {coll = 0x02}; -- Scorpion
tr4_entity_tbl[056] = {coll = 0x02}; -- Scorpion MIP - UNUSED
tr4_entity_tbl[057] = {coll = 0x02}; -- Jean-Yves
tr4_entity_tbl[058] = {coll = 0x02}; -- Jean-Yves MIP - UNUSED
tr4_entity_tbl[059] = {coll = 0x02}; -- Troops
tr4_entity_tbl[060] = {coll = 0x02}; -- Troops MIP - UNUSED
tr4_entity_tbl[061] = {coll = 0x02}; -- Knights Templar
tr4_entity_tbl[062] = {coll = 0x02}; -- Knights Templar MIP - UNUSED
tr4_entity_tbl[063] = {coll = 0x02}; -- Mutant
tr4_entity_tbl[064] = {coll = 0x02}; -- Mutant MIP - UNUSED
tr4_entity_tbl[065] = {coll = 0x02}; -- Horse
tr4_entity_tbl[066] = {coll = 0x02}; -- Horse MIP - UNUSED
tr4_entity_tbl[067] = {coll = 0x02}; -- Baboon normal
tr4_entity_tbl[068] = {coll = 0x02}; -- Baboon normal MIP - UNUSED
tr4_entity_tbl[069] = {coll = 0x02}; -- Baboon invisible
tr4_entity_tbl[070] = {coll = 0x02}; -- Baboon invisible MIP - UNUSED
tr4_entity_tbl[071] = {coll = 0x02}; -- Baboon silent
tr4_entity_tbl[072] = {coll = 0x02}; -- Baboon silent MIP - UNUSED
tr4_entity_tbl[073] = {coll = 0x02}; -- Wild boar
tr4_entity_tbl[074] = {coll = 0x02}; -- Wild boar MIP - UNUSED
tr4_entity_tbl[075] = {coll = 0x02}; -- Harpy
tr4_entity_tbl[076] = {coll = 0x02}; -- Harpy MIP - UNUSED
tr4_entity_tbl[077] = {coll = 0x02}; -- Demigod 1
tr4_entity_tbl[078] = {coll = 0x02}; -- Demigod 1 MIP - UNUSED
tr4_entity_tbl[079] = {coll = 0x02}; -- Demigod 2
tr4_entity_tbl[080] = {coll = 0x02}; -- Demigod 2 MIP - UNUSED
tr4_entity_tbl[081] = {coll = 0x02}; -- Demigod 3
tr4_entity_tbl[082] = {coll = 0x02}; -- Demigod 3 MIP - UNUSED
tr4_entity_tbl[083] = {coll = 0x00}; -- Little beetle - NO COLLISION, SWARM
tr4_entity_tbl[084] = {coll = 0x02}; -- Big beetle
tr4_entity_tbl[085] = {coll = 0x02}; -- Big beetle MIP - UNUSED
tr4_entity_tbl[086] = {coll = 0x00}; -- Wraith 1 - SPIRIT, NO COLLISION
tr4_entity_tbl[087] = {coll = 0x00}; -- Wraith 2 - SPIRIT, NO COLLISION
tr4_entity_tbl[088] = {coll = 0x00}; -- Wraith 3 - SPIRIT, NO COLLISION
tr4_entity_tbl[089] = {coll = 0x00}; -- Wraith 4 - SPIRIT, NO COLLISION
tr4_entity_tbl[090] = {coll = 0x00}; -- Bat - TOO SMALL TO COLLIDE WITH
tr4_entity_tbl[091] = {coll = 0x02}; -- Dog
tr4_entity_tbl[092] = {coll = 0x02}; -- Dog MIP - UNUSED
tr4_entity_tbl[093] = {coll = 0x02}; -- Hammerhead
tr4_entity_tbl[094] = {coll = 0x02}; -- Hammerhead MIP - UNUSED
tr4_entity_tbl[095] = {coll = 0x02}; -- SAS
tr4_entity_tbl[096] = {coll = 0x02}; -- SAS MIP - UNUSED
tr4_entity_tbl[097] = {coll = 0x02}; -- SAS dying
tr4_entity_tbl[098] = {coll = 0x02}; -- SAS dying MIP - UNUSED
tr4_entity_tbl[099] = {coll = 0x02}; -- SAS Captain
tr4_entity_tbl[100] = {coll = 0x02}; -- SAS Captain MIP - UNUSED
tr4_entity_tbl[101] = {coll = 0x01}; -- SAS Drag bloke
tr4_entity_tbl[102] = {coll = 0x02}; -- Ahmet
tr4_entity_tbl[103] = {coll = 0x02}; -- Ahmet MIP - UNUSED
tr4_entity_tbl[104] = {coll = 0x01}; -- Lara double
tr4_entity_tbl[105] = {coll = 0x01}; -- Lara double MIP - UNUSED
tr4_entity_tbl[106] = {coll = 0x02}; -- Small scorpion
tr4_entity_tbl[107] = {coll = 0x00, hide = 0x01}; -- Locust (ex-Fish) - NO COLLISION, SWARM

-- PUZZLE ACTION ITEMS

tr4_entity_tbl[108] = {coll = 0x01}; -- Game piece 1
tr4_entity_tbl[109] = {coll = 0x01}; -- Game piece 2
tr4_entity_tbl[110] = {coll = 0x01}; -- Game piece 3
tr4_entity_tbl[111] = {coll = 0x01}; -- Enemy piece
tr4_entity_tbl[112] = {coll = 0x02}; -- Wheel of fortune
tr4_entity_tbl[113] = {coll = 0x02}; -- Scales

-- DART EMITTER

tr4_entity_tbl[114] = {coll = 0x00, hide = 0x01}; -- Darts - SPAWNED OBJECT
tr4_entity_tbl[115] = {coll = 0x00, hide = 0x01}; -- Dart emitter
tr4_entity_tbl[116] = {coll = 0x00, hide = 0x01}; -- Homing dart emitter  - UNUSED

-- DESTROYABLE / MOVABLE TERRAIN

tr4_entity_tbl[117] = {coll = 0x01}; -- Falling ceiling
tr4_entity_tbl[118] = {coll = 0x01, func = "fallblock"}; -- Falling block
tr4_entity_tbl[119] = {coll = 0x01, func = "fallblock"}; -- Falling block2
tr4_entity_tbl[120] = {coll = 0x01}; -- Smashable bike wall
tr4_entity_tbl[121] = {coll = 0x01}; -- Smashable bike floor
tr4_entity_tbl[122] = {coll = 0x01, func = "door"}; -- Trapdoor 1
tr4_entity_tbl[123] = {coll = 0x01, func = "door"}; -- Trapdoor 2
tr4_entity_tbl[124] = {coll = 0x01, func = "door"}; -- Trapdoor 3
tr4_entity_tbl[125] = {coll = 0x01, func = "door"}; -- Floor trapdoor 1
tr4_entity_tbl[126] = {coll = 0x01, func = "door"}; -- Floor trapdoor 2
tr4_entity_tbl[127] = {coll = 0x01, func = "door"}; -- Ceiling trapdoor 1
tr4_entity_tbl[128] = {coll = 0x01, func = "door"}; -- Ceiling trapdoor 2
tr4_entity_tbl[129] = {coll = 0x01}; -- Scaling trapdoor

-- TRAPS & INTERACTION OBJECTS

tr4_entity_tbl[130] = {coll = 0x01, func = "boulder"}; -- Rolling ball
tr4_entity_tbl[131] = {coll = 0x00, func = "oldspike"}; -- Spikey floor - UNUSED?
tr4_entity_tbl[132] = {coll = 0x00, func = "oldspike"}; -- Teeth spikes
tr4_entity_tbl[133] = {coll = 0x00}; -- Joby spikes
tr4_entity_tbl[134] = {coll = 0x00}; -- Slicer dicer
tr4_entity_tbl[135] = {coll = 0x01}; -- Chain
tr4_entity_tbl[136] = {coll = 0x01}; -- Plough
tr4_entity_tbl[137] = {coll = 0x01}; -- Stargate
tr4_entity_tbl[138] = {coll = 0x00}; -- Hammer
tr4_entity_tbl[139] = {coll = 0x01}; -- Burning floor
tr4_entity_tbl[140] = {coll = 0x00}; -- Cog
tr4_entity_tbl[141] = {coll = 0x00}; -- Spike ball

tr4_entity_tbl[142] = {coll = 0x00, hide = 0x01}; -- Flame - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[143] = {coll = 0x00, hide = 0x01}; -- Flame emitter
tr4_entity_tbl[144] = {coll = 0x00, hide = 0x01}; -- Flame emitter 2
tr4_entity_tbl[145] = {coll = 0x00, hide = 0x01}; -- Flame emitter 3
tr4_entity_tbl[146] = {coll = 0x00, hide = 0x01}; -- Rope

tr4_entity_tbl[147] = {coll = 0x00}; -- Fire rope
tr4_entity_tbl[148] = {coll = 0x00}; -- Pole rope
tr4_entity_tbl[149] = {coll = 0x01}; -- One block platform  - UNUSED
tr4_entity_tbl[150] = {coll = 0x01}; -- Two block platform
tr4_entity_tbl[151] = {coll = 0x02}; -- Raising block 1 - RESIZABLE MESH!
tr4_entity_tbl[152] = {coll = 0x02}; -- Raising block 2 - RESIZABLE MESH!
tr4_entity_tbl[153] = {coll = 0x02}; -- Expanding platform - RESIZABLE MESH!
tr4_entity_tbl[154] = {coll = 0x02}; -- Squishy block 1
tr4_entity_tbl[155] = {coll = 0x02}; -- Squishy block 2

tr4_entity_tbl[156] = {coll = 0x02, trav = 0x10, func = "pushable"}; -- Pushable object 1
tr4_entity_tbl[157] = {coll = 0x02, trav = 0x10, func = "pushable"}; -- Pushable object 2
tr4_entity_tbl[158] = {coll = 0x02, trav = 0x10, func = "pushable"}; -- Pushable object 3
tr4_entity_tbl[159] = {coll = 0x02, trav = 0x10, func = "pushable"}; -- Pushable object 4
tr4_entity_tbl[160] = {coll = 0x02, trav = 0x10, func = "pushable"}; -- Pushable object 5

tr4_entity_tbl[161] = {coll = 0x00}; -- Tripwire  - UNUSED
tr4_entity_tbl[162] = {coll = 0x02}; -- Sentry gun
tr4_entity_tbl[163] = {coll = 0x02}; -- Mine
tr4_entity_tbl[164] = {coll = 0x00}; -- Mapper
tr4_entity_tbl[165] = {coll = 0x02}; -- Obelisk
tr4_entity_tbl[166] = {coll = 0x01}; -- Floor 4 blade
tr4_entity_tbl[167] = {coll = 0x01}; -- Roof 4 blade
tr4_entity_tbl[168] = {coll = 0x01}; -- Bird blade
tr4_entity_tbl[169] = {coll = 0x01}; -- Catwalk blade
tr4_entity_tbl[170] = {coll = 0x01}; -- Moving blade
tr4_entity_tbl[171] = {coll = 0x01}; -- Plinth blade
tr4_entity_tbl[172] = {coll = 0x01}; -- Seth blade

tr4_entity_tbl[173] = {coll = 0x00, hide = 0x01}; -- Lightning conductor

-- PICK-UP WALKTHROUGH ITEMS

tr4_entity_tbl[174] = {coll = 0x00}; -- Element puzzle
tr4_entity_tbl[175] = {coll = 0x00}; -- Puzzle item 1
tr4_entity_tbl[176] = {coll = 0x00}; -- Puzzle item 2
tr4_entity_tbl[177] = {coll = 0x00}; -- Puzzle item 3
tr4_entity_tbl[178] = {coll = 0x00}; -- Puzzle item 4
tr4_entity_tbl[179] = {coll = 0x00}; -- Puzzle item 5
tr4_entity_tbl[180] = {coll = 0x00}; -- Puzzle item 6
tr4_entity_tbl[181] = {coll = 0x00}; -- Puzzle item 7
tr4_entity_tbl[182] = {coll = 0x00}; -- Puzzle item 8
tr4_entity_tbl[183] = {coll = 0x00}; -- Puzzle item 9
tr4_entity_tbl[184] = {coll = 0x00}; -- Puzzle item 10
tr4_entity_tbl[185] = {coll = 0x00}; -- Puzzle item 11
tr4_entity_tbl[186] = {coll = 0x00}; -- Puzzle item 12
tr4_entity_tbl[187] = {coll = 0x00}; -- Puzzle item 1 combo 1
tr4_entity_tbl[188] = {coll = 0x00}; -- Puzzle item 1 combo 2
tr4_entity_tbl[189] = {coll = 0x00}; -- Puzzle item 2 combo 1
tr4_entity_tbl[190] = {coll = 0x00}; -- Puzzle item 2 combo 2
tr4_entity_tbl[191] = {coll = 0x00}; -- Puzzle item 3 combo 1
tr4_entity_tbl[192] = {coll = 0x00}; -- Puzzle item 3 combo 2
tr4_entity_tbl[193] = {coll = 0x00}; -- Puzzle item 4 combo 1
tr4_entity_tbl[194] = {coll = 0x00}; -- Puzzle item 4 combo 2
tr4_entity_tbl[195] = {coll = 0x00}; -- Puzzle item 5 combo 1
tr4_entity_tbl[196] = {coll = 0x00}; -- Puzzle item 5 combo 2
tr4_entity_tbl[197] = {coll = 0x00}; -- Puzzle item 6 combo 1
tr4_entity_tbl[198] = {coll = 0x00}; -- Puzzle item 6 combo 2
tr4_entity_tbl[199] = {coll = 0x00}; -- Puzzle item 7 combo 1
tr4_entity_tbl[200] = {coll = 0x00}; -- Puzzle item 7 combo 2
tr4_entity_tbl[201] = {coll = 0x00}; -- Puzzle item 8 combo 1
tr4_entity_tbl[202] = {coll = 0x00}; -- Puzzle item 8 combo 2
tr4_entity_tbl[203] = {coll = 0x00}; -- Key item 1
tr4_entity_tbl[204] = {coll = 0x00}; -- Key item 2
tr4_entity_tbl[205] = {coll = 0x00}; -- Key item 3
tr4_entity_tbl[206] = {coll = 0x00}; -- Key item 4
tr4_entity_tbl[207] = {coll = 0x00}; -- Key item 5
tr4_entity_tbl[208] = {coll = 0x00}; -- Key item 6
tr4_entity_tbl[209] = {coll = 0x00}; -- Key item 7
tr4_entity_tbl[210] = {coll = 0x00}; -- Key item 8
tr4_entity_tbl[211] = {coll = 0x00}; -- Key item 9
tr4_entity_tbl[212] = {coll = 0x00}; -- Key item 10
tr4_entity_tbl[213] = {coll = 0x00}; -- Key item 11
tr4_entity_tbl[214] = {coll = 0x00}; -- Key item 12
tr4_entity_tbl[215] = {coll = 0x00}; -- Key item 1 combo 1
tr4_entity_tbl[216] = {coll = 0x00}; -- Key item 1 combo 2
tr4_entity_tbl[217] = {coll = 0x00}; -- Key item 2 combo 1
tr4_entity_tbl[218] = {coll = 0x00}; -- Key item 2 combo 2
tr4_entity_tbl[219] = {coll = 0x00}; -- Key item 3 combo 1
tr4_entity_tbl[220] = {coll = 0x00}; -- Key item 3 combo 2
tr4_entity_tbl[221] = {coll = 0x00}; -- Key item 4 combo 1
tr4_entity_tbl[222] = {coll = 0x00}; -- Key item 4 combo 2
tr4_entity_tbl[223] = {coll = 0x00}; -- Key item 5 combo 1
tr4_entity_tbl[224] = {coll = 0x00}; -- Key item 5 combo 2
tr4_entity_tbl[225] = {coll = 0x00}; -- Key item 6 combo 1
tr4_entity_tbl[226] = {coll = 0x00}; -- Key item 6 combo 2
tr4_entity_tbl[227] = {coll = 0x00}; -- Key item 7 combo 1
tr4_entity_tbl[228] = {coll = 0x00}; -- Key item 7 combo 2
tr4_entity_tbl[229] = {coll = 0x00}; -- Key item 8 combo 1
tr4_entity_tbl[230] = {coll = 0x00}; -- Key item 8 combo 2
tr4_entity_tbl[231] = {coll = 0x00}; -- Pickup item 1
tr4_entity_tbl[232] = {coll = 0x00}; -- Pickup item 2
tr4_entity_tbl[233] = {coll = 0x00}; -- Pickup item 3
tr4_entity_tbl[234] = {coll = 0x00}; -- Pickup item 4
tr4_entity_tbl[235] = {coll = 0x00}; -- Pickup item 1 combo 1
tr4_entity_tbl[236] = {coll = 0x00}; -- Pickup item 1 combo 2
tr4_entity_tbl[237] = {coll = 0x00}; -- Pickup item 2 combo 1
tr4_entity_tbl[238] = {coll = 0x00}; -- Pickup item 2 combo 2
tr4_entity_tbl[239] = {coll = 0x00}; -- Pickup item 3 combo 1
tr4_entity_tbl[240] = {coll = 0x00}; -- Pickup item 3 combo 2
tr4_entity_tbl[241] = {coll = 0x00}; -- Pickup item 4 combo 1
tr4_entity_tbl[242] = {coll = 0x00}; -- Pickup item 4 combo 2
tr4_entity_tbl[243] = {coll = 0x00}; -- Examine 1
tr4_entity_tbl[244] = {coll = 0x00}; -- Examine 2
tr4_entity_tbl[245] = {coll = 0x00}; -- Examine 3
tr4_entity_tbl[246] = {coll = 0x00}; -- Crowbar item
tr4_entity_tbl[247] = {coll = 0x00}; -- Burning torch item
tr4_entity_tbl[248] = {coll = 0x00}; -- Clock work beetle
tr4_entity_tbl[249] = {coll = 0x00}; -- Clock work beetle combo 1
tr4_entity_tbl[250] = {coll = 0x00}; -- Clock work beetle combo 2
tr4_entity_tbl[251] = {coll = 0x00}; -- Mine detector
tr4_entity_tbl[252] = {coll = 0x00}; -- Quest item 1
tr4_entity_tbl[253] = {coll = 0x00}; -- Quest item 2
tr4_entity_tbl[254] = {coll = 0x00}; -- Quest item 3
tr4_entity_tbl[255] = {coll = 0x00}; -- Quest item 4
tr4_entity_tbl[256] = {coll = 0x00}; -- Quest item 5
tr4_entity_tbl[257] = {coll = 0x00}; -- Quest item 6
tr4_entity_tbl[258] = {coll = 0x00}; -- Map - UNUSED
tr4_entity_tbl[259] = {coll = 0x00}; -- Secret map - UNUSED

-- PUZZLE HOLES AND KEYHOLES

tr4_entity_tbl[260] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 1
tr4_entity_tbl[261] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 2
tr4_entity_tbl[262] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 3
tr4_entity_tbl[263] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 4
tr4_entity_tbl[264] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 5
tr4_entity_tbl[265] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 6
tr4_entity_tbl[266] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 7
tr4_entity_tbl[267] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 8
tr4_entity_tbl[268] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 9
tr4_entity_tbl[269] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 10
tr4_entity_tbl[270] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 11
tr4_entity_tbl[271] = {coll = 0x00, func = "keyhole"}; -- Puzzle hole 12
tr4_entity_tbl[272] = {coll = 0x00}; -- Puzzle done 1
tr4_entity_tbl[273] = {coll = 0x00}; -- Puzzle done 2
tr4_entity_tbl[274] = {coll = 0x00}; -- Puzzle done 3
tr4_entity_tbl[275] = {coll = 0x00}; -- Puzzle done 4
tr4_entity_tbl[276] = {coll = 0x00}; -- Puzzle done 5
tr4_entity_tbl[277] = {coll = 0x00}; -- Puzzle done 6
tr4_entity_tbl[278] = {coll = 0x00}; -- Puzzle done 7
tr4_entity_tbl[279] = {coll = 0x00}; -- Puzzle done 8
tr4_entity_tbl[280] = {coll = 0x00}; -- Puzzle done 9
tr4_entity_tbl[281] = {coll = 0x00}; -- Puzzle done 10
tr4_entity_tbl[282] = {coll = 0x00}; -- Puzzle done 11
tr4_entity_tbl[283] = {coll = 0x00}; -- Puzzle done 12
tr4_entity_tbl[284] = {coll = 0x02, func = "keyhole"}; -- Key hole 1 (ig keyhole hub.tr4)
tr4_entity_tbl[285] = {coll = 0x00, func = "keyhole"}; -- Key hole 2
tr4_entity_tbl[286] = {coll = 0x00, func = "keyhole"}; -- Key hole 3
tr4_entity_tbl[287] = {coll = 0x00, func = "keyhole"}; -- Key hole 4
tr4_entity_tbl[288] = {coll = 0x00, func = "keyhole"}; -- Key hole 5
tr4_entity_tbl[289] = {coll = 0x00, func = "keyhole"}; -- Key hole 6
tr4_entity_tbl[290] = {coll = 0x00, func = "keyhole"}; -- Key hole 7
tr4_entity_tbl[291] = {coll = 0x00, func = "keyhole"}; -- Key hole 8
tr4_entity_tbl[292] = {coll = 0x00, func = "keyhole"}; -- Key hole 9
tr4_entity_tbl[293] = {coll = 0x00, func = "keyhole"}; -- Key hole 10
tr4_entity_tbl[294] = {coll = 0x00, func = "keyhole"}; -- Key hole 11
tr4_entity_tbl[295] = {coll = 0x00, func = "keyhole"}; -- Key hole 12

-- WATERSKIN ITEMS

tr4_entity_tbl[296] = {coll = 0x00}; -- Water skin 1 empty
tr4_entity_tbl[297] = {coll = 0x00}; -- Water skin 1 1
tr4_entity_tbl[298] = {coll = 0x00}; -- Water skin 1 2
tr4_entity_tbl[299] = {coll = 0x00}; -- Water skin 1 3
tr4_entity_tbl[300] = {coll = 0x00}; -- Water skin 2 empty
tr4_entity_tbl[301] = {coll = 0x00}; -- Water skin 2 1
tr4_entity_tbl[302] = {coll = 0x00}; -- Water skin 2 2
tr4_entity_tbl[303] = {coll = 0x00}; -- Water skin 2 3
tr4_entity_tbl[304] = {coll = 0x00}; -- Water skin 2 4
tr4_entity_tbl[305] = {coll = 0x00}; -- Water skin 2 5

-- SWITCHES

tr4_entity_tbl[306] = {coll = 0x02, func = "switch"}; -- Switch type 1
tr4_entity_tbl[307] = {coll = 0x02, func = "switch"}; -- Switch type 2
tr4_entity_tbl[308] = {coll = 0x02, func = "switch"}; -- Switch type 3
tr4_entity_tbl[309] = {coll = 0x02, func = "switch"}; -- Switch type 4
tr4_entity_tbl[310] = {coll = 0x02, func = "switch"}; -- Switch type 5
tr4_entity_tbl[311] = {coll = 0x02, func = "switch"}; -- Switch type 6
tr4_entity_tbl[312] = {coll = 0x02, func = "switch"}; -- Switch type 7
tr4_entity_tbl[313] = {coll = 0x02, func = "switch"}; -- Switch type 8
tr4_entity_tbl[314] = {coll = 0x02, func = "switch"}; -- Underwater switch 1
tr4_entity_tbl[315] = {coll = 0x02, func = "switch"}; -- Underwater switch 2
tr4_entity_tbl[316] = {coll = 0x02, func = "switch"}; -- Turn switch
tr4_entity_tbl[317] = {coll = 0x02, func = "switch"}; -- Cog switch
tr4_entity_tbl[318] = {coll = 0x02, func = "switch"}; -- Lever switch
tr4_entity_tbl[319] = {coll = 0x02, func = "switch"}; -- Jump switch
tr4_entity_tbl[320] = {coll = 0x02, func = "switch"}; -- Crowbar switch
tr4_entity_tbl[321] = {coll = 0x02, func = "switch"}; -- Pulley

-- DOORS

tr4_entity_tbl[322] = {coll = 0x01, func = "door"}; -- Door type 1
tr4_entity_tbl[323] = {coll = 0x01, func = "door"}; -- Door type 2
tr4_entity_tbl[324] = {coll = 0x01, func = "door"}; -- Door type 3
tr4_entity_tbl[325] = {coll = 0x01, func = "door"}; -- Door type 4
tr4_entity_tbl[326] = {coll = 0x01, func = "door"}; -- Door type 5
tr4_entity_tbl[327] = {coll = 0x01, func = "door"}; -- Door type 6
tr4_entity_tbl[328] = {coll = 0x01, func = "door"}; -- Door type 7
tr4_entity_tbl[329] = {coll = 0x01, func = "door"}; -- Door type 8
tr4_entity_tbl[330] = {coll = 0x01}; -- Push pull door 1
tr4_entity_tbl[331] = {coll = 0x01}; -- Push pull door 2
tr4_entity_tbl[332] = {coll = 0x01}; -- Kick door 1
tr4_entity_tbl[333] = {coll = 0x01}; -- Kick door 2
tr4_entity_tbl[334] = {coll = 0x01}; -- Underwater door
tr4_entity_tbl[335] = {coll = 0x01, func = "pushdoor"}; -- Double doors

-- STATIC TERRAIN

tr4_entity_tbl[336] = {coll = 0x01}; -- Bridge flat
tr4_entity_tbl[337] = {coll = 0x01}; -- Bridge tilt 1
tr4_entity_tbl[338] = {coll = 0x01}; -- Bridge tilt 2

-- MISC INTERACTION OBJECTS

tr4_entity_tbl[339] = {coll = 0x01}; -- Sarcophagus
tr4_entity_tbl[340] = {coll = 0x01}; -- Sequence door 1
tr4_entity_tbl[341] = {coll = 0x02}; -- Sequence switch 1
tr4_entity_tbl[342] = {coll = 0x02}; -- Sequence switch 2
tr4_entity_tbl[343] = {coll = 0x02}; -- Sequence switch 3
tr4_entity_tbl[344] = {coll = 0x02}; -- Sarcophagus cut
tr4_entity_tbl[345] = {coll = 0x01}; -- Horus statue
tr4_entity_tbl[346] = {coll = 0x00}; -- God head
tr4_entity_tbl[347] = {coll = 0x01}; -- Seth door
tr4_entity_tbl[348] = {coll = 0x01}; -- Statue plinth

 -- PICK-UP SUPPLY ITEMS

tr4_entity_tbl[349] = {coll = 0x00}; -- Pistols item
tr4_entity_tbl[350] = {coll = 0x00}; -- Pistols ammo item
tr4_entity_tbl[351] = {coll = 0x00}; -- Uzi item
tr4_entity_tbl[352] = {coll = 0x00}; -- Uzi ammo item
tr4_entity_tbl[353] = {coll = 0x00}; -- Shotgun item
tr4_entity_tbl[354] = {coll = 0x00}; -- Shotgun ammo 1 item
tr4_entity_tbl[355] = {coll = 0x00}; -- Shotgun ammo 2 item
tr4_entity_tbl[356] = {coll = 0x00}; -- Crossbow item
tr4_entity_tbl[357] = {coll = 0x00}; -- Crossbow ammo 1 item
tr4_entity_tbl[358] = {coll = 0x00}; -- Crossbow ammo 2 item
tr4_entity_tbl[359] = {coll = 0x00}; -- Crossbow ammo 3 item
tr4_entity_tbl[360] = {coll = 0x00}; -- Crossbow bolt
tr4_entity_tbl[361] = {coll = 0x00}; -- Grenade gun item
tr4_entity_tbl[362] = {coll = 0x00}; -- Grenade gun ammo 1 item
tr4_entity_tbl[363] = {coll = 0x00}; -- Grenade gun ammo 2 item
tr4_entity_tbl[364] = {coll = 0x00}; -- Grenade gun ammo 3 item
tr4_entity_tbl[365] = {coll = 0x00}; -- Grenade
tr4_entity_tbl[366] = {coll = 0x00}; -- Six shooter item
tr4_entity_tbl[367] = {coll = 0x00}; -- Six shooter ammo item
tr4_entity_tbl[368] = {coll = 0x00}; -- Big medipack item
tr4_entity_tbl[369] = {coll = 0x00}; -- Small medipack item
tr4_entity_tbl[370] = {coll = 0x00}; -- Laser sight item
tr4_entity_tbl[371] = {coll = 0x00}; -- Binoculars item
tr4_entity_tbl[372] = {coll = 0x00}; -- Flare item
tr4_entity_tbl[373] = {coll = 0x00}; -- Flare inv item
tr4_entity_tbl[374] = {coll = 0x00}; -- Diary item - UNUSED

-- INVENTORY ITEMS

tr4_entity_tbl[375] = {coll = 0x00}; -- Compass item - NOT A PROPER PICK UP OBJECT (INVENTORY ONLY)
tr4_entity_tbl[376] = {coll = 0x00}; -- Mem card load inv item - UNUSED
tr4_entity_tbl[377] = {coll = 0x00}; -- Mem card save inv item - UNUSED
tr4_entity_tbl[378] = {coll = 0x00}; -- PC load inv item - NOT A PROPER PICK UP OBJECT (INVENTORY ONLY)
tr4_entity_tbl[379] = {coll = 0x00}; -- PC save inv item - NOT A PROPER PICK UP OBJECT (INVENTORY ONLY)

-- NULLMESHES, SERVICE OBJECTS AND EMITTERS

tr4_entity_tbl[380] = {coll = 0x00, hide = 0x01}; -- Smoke emitter white - EMITTER
tr4_entity_tbl[381] = {coll = 0x00, hide = 0x01}; -- Smoke emitter black - EMITTER
tr4_entity_tbl[382] = {coll = 0x00, hide = 0x01}; -- Steam emitter - EMITTER
tr4_entity_tbl[383] = {coll = 0x00, hide = 0x01}; -- Earth quake - SHAKES CAMERA
tr4_entity_tbl[384] = {coll = 0x00, hide = 0x01}; -- Bubbles - EMITTER
tr4_entity_tbl[385] = {coll = 0x00, hide = 0x01}; -- Waterfall mist - EMITTER

tr4_entity_tbl[386] = {coll = 0x00}; -- Gun shell - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[387] = {coll = 0x00}; -- Shotgun shell - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[388] = {coll = 0x00}; -- Gun flash - SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[389] = {coll = 0x00}; -- Butterfly - UNUSED
tr4_entity_tbl[390] = {coll = 0x00}; -- Sprinkler - EMITTER
tr4_entity_tbl[391] = {coll = 0x00}; -- Red light - STATIC LIGHT

tr4_entity_tbl[392] = {coll = 0x00, hide = 0x01}; -- Green light - STATIC LIGHT
tr4_entity_tbl[393] = {coll = 0x00, hide = 0x01}; -- Blue light - STATIC LIGHT
tr4_entity_tbl[394] = {coll = 0x00, hide = 0x01}; -- Amber light - DYNAMIC LIGHT
tr4_entity_tbl[395] = {coll = 0x00, hide = 0x01}; -- White light - STATIC LIGHT
tr4_entity_tbl[396] = {coll = 0x00, hide = 0x01}; -- Blinking light - DYNAMIC LIGHT
tr4_entity_tbl[397] = {coll = 0x00, hide = 0x01}; -- Lens flare


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

tr4_entity_tbl[409] = {coll = 0x01}; -- Smash object 1
tr4_entity_tbl[410] = {coll = 0x01}; -- Smash object 2
tr4_entity_tbl[411] = {coll = 0x01}; -- Smash object 3
tr4_entity_tbl[412] = {coll = 0x01}; -- Smash object 4
tr4_entity_tbl[413] = {coll = 0x01}; -- Smash object 5
tr4_entity_tbl[414] = {coll = 0x01}; -- Smash object 6
tr4_entity_tbl[415] = {coll = 0x01}; -- Smash object 7
tr4_entity_tbl[416] = {coll = 0x01}; -- Smash object 8
tr4_entity_tbl[417] = {coll = 0x00}; -- Mesh swap 1
tr4_entity_tbl[418] = {coll = 0x00}; -- Mesh swap 2
tr4_entity_tbl[419] = {coll = 0x00}; -- Mesh swap 3
tr4_entity_tbl[420] = {coll = 0x00}; -- Death slide
tr4_entity_tbl[421] = {coll = 0x00}; -- Body part - UNUSED? SPAWNED OBJECT, NO DIRECT USE
tr4_entity_tbl[422] = {coll = 0x00, hide = 0x01}; -- Camera target - TARGET FOR CAMERA

-- WATERFALLS (MESHES WITH UV-SCROLL ANIMATED TEXTURES)

tr4_entity_tbl[423] = {coll = 0x00}; -- Waterfall 1
tr4_entity_tbl[424] = {coll = 0x00}; -- Waterfall 2
tr4_entity_tbl[425] = {coll = 0x00}; -- Waterfall 3
tr4_entity_tbl[426] = {coll = 0x00}; -- Planet effect

-- ANIMATINGS

tr4_entity_tbl[427] = {coll = 0x01}; -- Animating 1
tr4_entity_tbl[428] = {coll = 0x01}; -- Animating 1 MIP
tr4_entity_tbl[429] = {coll = 0x01}; -- Animating 2
tr4_entity_tbl[430] = {coll = 0x01}; -- Animating 2 MIP
tr4_entity_tbl[431] = {coll = 0x01}; -- Animating 3
tr4_entity_tbl[432] = {coll = 0x01}; -- Animating 3 MIP
tr4_entity_tbl[433] = {coll = 0x01}; -- Animating 4
tr4_entity_tbl[434] = {coll = 0x01}; -- Animating 4 MIP
tr4_entity_tbl[435] = {coll = 0x01}; -- Animating 5
tr4_entity_tbl[436] = {coll = 0x01}; -- Animating 5 MIP
tr4_entity_tbl[437] = {coll = 0x01}; -- Animating 6
tr4_entity_tbl[438] = {coll = 0x01}; -- Animating 6 MIP
tr4_entity_tbl[439] = {coll = 0x01}; -- Animating 7
tr4_entity_tbl[440] = {coll = 0x01}; -- Animating 7 MIP
tr4_entity_tbl[441] = {coll = 0x01}; -- Animating 8
tr4_entity_tbl[442] = {coll = 0x01}; -- Animating 8 MIP
tr4_entity_tbl[443] = {coll = 0x01}; -- Animating 9
tr4_entity_tbl[444] = {coll = 0x01}; -- Animating 9 MIP
tr4_entity_tbl[445] = {coll = 0x01}; -- Animating 10
tr4_entity_tbl[446] = {coll = 0x01}; -- Animating 10 MIP
tr4_entity_tbl[447] = {coll = 0x01}; -- Animating 11
tr4_entity_tbl[448] = {coll = 0x01}; -- Animating 11 MIP
tr4_entity_tbl[449] = {coll = 0x01}; -- Animating 12
tr4_entity_tbl[450] = {coll = 0x01}; -- Animating 12 MIP
tr4_entity_tbl[451] = {coll = 0x01}; -- Animating 13
tr4_entity_tbl[452] = {coll = 0x01}; -- Animating 13 MIP
tr4_entity_tbl[453] = {coll = 0x01}; -- Animating 14 (sunlight)
tr4_entity_tbl[454] = {coll = 0x01}; -- Animating 14 MIP
tr4_entity_tbl[455] = {coll = 0x01}; -- Animating 15 (basket, hub.tr4, sunlight)
tr4_entity_tbl[456] = {coll = 0x01}; -- Animating 15 MIP
tr4_entity_tbl[457] = {coll = 0x01}; -- Animating 16 (map 9 - coll = 0x00)
tr4_entity_tbl[458] = {coll = 0x01}; -- Animating 16 MIP

--------------------------------------------------------------------------------
------------------------------------- TR_V -------------------------------------
--------------------------------------------------------------------------------
tr5_entity_tbl = {};

-- Remark: object IDs 0-30 are used for Lara model and speechheads, and never
-- show in game independently.

-- ENEMIES

tr5_entity_tbl[031] = {coll = 0x02}; -- SAS - UNUSED
tr5_entity_tbl[032] = {coll = 0x02}; -- SAS MIP - UNUSED
tr5_entity_tbl[033] = {coll = 0x02}; -- SWAT  - UNUSED
tr5_entity_tbl[034] = {coll = 0x02}; -- SWAT MIP - UNUSED
tr5_entity_tbl[035] = {coll = 0x02}; -- VCI guard (SWAT_PLUS)
tr5_entity_tbl[036] = {coll = 0x02}; -- VCI guard MIP (SWAT_PLUS MIP)
tr5_entity_tbl[037] = {coll = 0x02}; -- Guard gun (BLUE_GUARD)
tr5_entity_tbl[038] = {coll = 0x02}; -- Guard gun MIP (BLUE_GUARD MIP)
tr5_entity_tbl[039] = {coll = 0x02}; -- Guard laser (TWOGUN)
tr5_entity_tbl[040] = {coll = 0x02}; -- Guard laser MIP (TWOGUN MIP)
tr5_entity_tbl[041] = {coll = 0x02}; -- Doberman (DOG)
tr5_entity_tbl[042] = {coll = 0x02}; -- Doberman MIP (DOG MIP)
tr5_entity_tbl[043] = {coll = 0x02}; -- Crow
tr5_entity_tbl[044] = {coll = 0x02}; -- Crow MIP
tr5_entity_tbl[045] = {coll = 0x02}; -- Larson
tr5_entity_tbl[046] = {coll = 0x00}; -- Keycard 1 (Ex-LARSON MIP)
tr5_entity_tbl[047] = {coll = 0x02}; -- Pierre
tr5_entity_tbl[048] = {coll = 0x00}; -- Keycard 2  (Ex-PIERRE MIP)
tr5_entity_tbl[049] = {coll = 0x02}; -- Armed baddy 1 (MAFIA)
tr5_entity_tbl[050] = {coll = 0x02}; -- Armed baddy 1 MIP (MAFIA MIP)
tr5_entity_tbl[051] = {coll = 0x02}; -- Armed baddy 2 (MAFIA2)
tr5_entity_tbl[052] = {coll = 0x02}; -- Armed baddy 2 MIP (MAFIA2 MIP)
tr5_entity_tbl[053] = {coll = 0x02}; -- (Ex-SAILOR) - UNUSED?
tr5_entity_tbl[054] = {coll = 0x02}; -- (Ex-SAILOR MIP) - UNUSED?
tr5_entity_tbl[055] = {coll = 0x02}; -- Guard robot control (CRANE_GUY)
tr5_entity_tbl[056] = {coll = 0x02}; -- Guard robot control MIP (CRANE_GUY MIP)
tr5_entity_tbl[057] = {coll = 0x02}; -- Lion
tr5_entity_tbl[058] = {coll = 0x02}; -- Lion MIP
tr5_entity_tbl[059] = {coll = 0x02}; -- Gladiator
tr5_entity_tbl[060] = {coll = 0x02}; -- Gladiator MIP
tr5_entity_tbl[061] = {coll = 0x02}; -- Roman statue
tr5_entity_tbl[062] = {coll = 0x00}; -- Spear tip (Ex-ROMAN_GOD MIP)
tr5_entity_tbl[063] = {coll = 0x02}; -- Hydra
tr5_entity_tbl[064] = {coll = 0x02}; -- Flat floor (Ex-HYDRA MIP?)
tr5_entity_tbl[065] = {coll = 0x02}; -- Laser head (GUARDIAN)
tr5_entity_tbl[066] = {coll = 0x02}; -- Laser head MIP (GUARDIAN MIP)
tr5_entity_tbl[067] = {coll = 0x02}; -- Cyborg
tr5_entity_tbl[068] = {coll = 0x02}; -- Cyborg MIP
tr5_entity_tbl[069] = {coll = 0x02}; -- VCI worker
tr5_entity_tbl[070] = {coll = 0x02}; -- VCI worker MIP
tr5_entity_tbl[071] = {coll = 0x02}; -- Willowisp Guide
tr5_entity_tbl[072] = {coll = 0x02}; -- Willowisp Guide MIP
tr5_entity_tbl[073] = {coll = 0x00}; -- Invisible ghost
tr5_entity_tbl[074] = {coll = 0x00}; -- Invisible ghost MIP - UNUSED?
tr5_entity_tbl[075] = {coll = 0x02}; -- Reaper - UNUSED?
tr5_entity_tbl[076] = {coll = 0x02}; -- Reaper MIP- UNUSED?
tr5_entity_tbl[077] = {coll = 0x02}; -- Maze Monster
tr5_entity_tbl[078] = {coll = 0x02}; -- Maze Monster MIP - UNUSED?
tr5_entity_tbl[079] = {coll = 0x02}; -- Lagoon Witch
tr5_entity_tbl[080] = {coll = 0x02}; -- Lagoon Witch MIP - UNUSED?
tr5_entity_tbl[081] = {coll = 0x02}; -- Submarine
tr5_entity_tbl[082] = {coll = 0x02}; -- Submarine MIP
tr5_entity_tbl[083] = {coll = 0x02}; -- M16 Guard
tr5_entity_tbl[084] = {coll = 0x02}; -- Tree with huged man (Ex-M16 Guard MIP)
tr5_entity_tbl[085] = {coll = 0x02}; -- Husky
tr5_entity_tbl[086] = {coll = 0x02}; -- Husky MIP
tr5_entity_tbl[087] = {coll = 0x02}; -- The chef
tr5_entity_tbl[088] = {coll = 0x02}; -- Rich1 hammer door (Ex-CHEF MIP)
tr5_entity_tbl[089] = {coll = 0x02}; -- Imp
tr5_entity_tbl[090] = {coll = 0x02}; -- Padlock (Ex-IMP_MIP)
tr5_entity_tbl[091] = {coll = 0x02}; -- Gunship
tr5_entity_tbl[092] = {coll = 0x02}; -- Gunship MIP
tr5_entity_tbl[093] = {coll = 0x00}; -- Bats
tr5_entity_tbl[094] = {coll = 0x00}; -- Little rats
tr5_entity_tbl[095] = {coll = 0x00}; -- Spiders
tr5_entity_tbl[096] = {coll = 0x00}; -- Spider generator - UNUSED?
tr5_entity_tbl[097] = {coll = 0x02}; -- Auto guns
tr5_entity_tbl[098] = {coll = 0x01}; -- Electricity wires

tr5_entity_tbl[099] = {coll = 0x00, hide = 0x01}; -- Darts - SPAWN OBJECT
tr5_entity_tbl[100] = {coll = 0x00, hide = 0x01}; -- Dart emitter
tr5_entity_tbl[101] = {coll = 0x00, hide = 0x01}; -- Homing (fast) dart emitter

-- DESTROYABLE / MOVABLE TERRAIN

tr5_entity_tbl[102] = {coll = 0x01}; -- Falling Ceiling
tr5_entity_tbl[103] = {coll = 0x01, func = "fallblock"}; -- Falling Block 1
tr5_entity_tbl[104] = {coll = 0x01, func = "fallblock"}; -- Falling Block 2
tr5_entity_tbl[105] = {coll = 0x01}; -- Crumbling Floor
tr5_entity_tbl[106] = {coll = 0x01, func = "door"}; -- Trapdoor 1
tr5_entity_tbl[107] = {coll = 0x01, func = "door"}; -- Trapdoor 2
tr5_entity_tbl[108] = {coll = 0x01, func = "door"}; -- Trapdoor 3
tr5_entity_tbl[109] = {coll = 0x01, func = "door"}; -- Floor trapdoor 1
tr5_entity_tbl[110] = {coll = 0x01, func = "door"}; -- Floor trapdoor 2
tr5_entity_tbl[111] = {coll = 0x01, func = "door"}; -- Ceiling trapdoor 1
tr5_entity_tbl[112] = {coll = 0x01, func = "door"}; -- Ceiling trapdoor 2
tr5_entity_tbl[113] = {coll = 0x01, func = "door"}; -- Scaling trapdoor
tr5_entity_tbl[114] = {coll = 0x01}; -- Box 1 (ROLLINGBALL)
tr5_entity_tbl[115] = {coll = 0x01}; -- Rolling barrel - UNUSED?

-- TRAPS & INTERACTION OBJECTS

tr5_entity_tbl[116] = {coll = 0x02, func = "oldspike"}; -- Spikey Floor - UNUSED?
tr5_entity_tbl[117] = {coll = 0x02, func = "oldspike"}; -- Teeth Spikes
tr5_entity_tbl[118] = {coll = 0x02}; -- Rome Hammer
tr5_entity_tbl[119] = {coll = 0x02}; -- Hammer 2 - UNUSED?

tr5_entity_tbl[120] = {coll = 0x00, hide = 0x01}; -- Flame
tr5_entity_tbl[121] = {coll = 0x00, hide = 0x01}; -- Flame emitter
tr5_entity_tbl[122] = {coll = 0x00, hide = 0x01}; -- Flame emitter 2
tr5_entity_tbl[123] = {coll = 0x00, hide = 0x01}; -- Flame emitter 3
tr5_entity_tbl[124] = {coll = 0x00, hide = 0x01}; -- Cooker flame

tr5_entity_tbl[125] = {coll = 0x00}; -- Burning roots

tr5_entity_tbl[126] = {coll = 0x00, hide = 0x01}; -- Rope

tr5_entity_tbl[127] = {coll = 0x00}; -- Fire rope
tr5_entity_tbl[128] = {coll = 0x00}; -- Pole rope
tr5_entity_tbl[129] = {coll = 0x01}; -- Ventilator HORIZONTAL
tr5_entity_tbl[130] = {coll = 0x01}; -- Ventilator VERTICAL

tr5_entity_tbl[131] = {coll = 0x00, hide = 0x01}; -- Grappling gun target

tr5_entity_tbl[132] = {coll = 0x01}; -- One block platform - UNUSED?
tr5_entity_tbl[133] = {coll = 0x01}; -- Two block platform - UNUSED?
tr5_entity_tbl[134] = {coll = 0x01}; -- Box 2 (Ex-RAISING_BLOCK1?)

tr5_entity_tbl[135] = {coll = 0x00, hide = 0x01}; -- Teleport (Ex-RAISING_BLOCK2?)
tr5_entity_tbl[136] = {coll = 0x00, hide = 0x01}; -- Headset talk point (Ex-EXPANDING_PLATFORM?)

tr5_entity_tbl[137] = {coll = 0x01, trav = 0x10, func = "pushable"}; -- Pushable 1
tr5_entity_tbl[138] = {coll = 0x01, trav = 0x10, func = "pushable"}; -- Pushable 2
tr5_entity_tbl[139] = {coll = 0x01, trav = 0x10, func = "pushable"}; -- Pushable 3 - UNUSED?
tr5_entity_tbl[140] = {coll = 0x01, trav = 0x10, func = "pushable"}; -- Pushable 4 - UNUSED?
tr5_entity_tbl[141] = {coll = 0x01, trav = 0x10, func = "pushable"}; -- Pushable 5 - UNUSED?

tr5_entity_tbl[142] = {coll = 0x00}; -- Robot arm (Ex-WRECKING BALL?)
tr5_entity_tbl[142] = {coll = 0x00}; -- Death slide - UNUSED?
tr5_entity_tbl[144] = {coll = 0x00}; -- Rocket item - TORPEDO
tr5_entity_tbl[145] = {coll = 0x00}; -- Chaff flare
tr5_entity_tbl[146] = {coll = 0x00}; -- Satchel Bomb - UNUSED?
tr5_entity_tbl[147] = {coll = 0x00}; -- Electric Fence - UNUSED?
tr5_entity_tbl[148] = {coll = 0x00}; -- Lift - UNUSED

tr5_entity_tbl[149] = {coll = 0x00, hide = 0x01}; -- Explosion
tr5_entity_tbl[150] = {coll = 0x00, hide = 0x01}; -- Deadly Electric bolt (IRIS_LIGHTNING)

tr5_entity_tbl[151] = {coll = 0x02}; -- Monitor screen
tr5_entity_tbl[152] = {coll = 0x02}; -- Security camera board

tr5_entity_tbl[153] = {coll = 0x00, hide = 0x01}; -- Motion sensor
tr5_entity_tbl[154] = {coll = 0x00, hide = 0x01}; -- Tight rope

tr5_entity_tbl[155] = {coll = 0x01}; -- Parallel bars

tr5_entity_tbl[156] = {coll = 0x00, hide = 0x01}; -- X-Ray Controller (?)

tr5_entity_tbl[157] = {coll = 0x00}; -- Cutscene rope
tr5_entity_tbl[158] = {coll = 0x01}; -- Flat window
tr5_entity_tbl[159] = {coll = 0x00}; -- GEN_SLOT1 - UNUSED?

tr5_entity_tbl[160] = {coll = 0x00, hide = 0x01}; -- Gas emitter

tr5_entity_tbl[161] = {coll = 0x00}; -- Sign
tr5_entity_tbl[162] = {coll = 0x00}; -- Moving laser
tr5_entity_tbl[163] = {coll = 0x00}; -- Imp Rock - SPAWN ITEM?
tr5_entity_tbl[164] = {coll = 0x01}; -- Cupboard 1
tr5_entity_tbl[165] = {coll = 0x01}; -- Cupboard 1 MIP
tr5_entity_tbl[166] = {coll = 0x01}; -- Cupboard 2
tr5_entity_tbl[167] = {coll = 0x01}; -- Cupboard 2 MIP
tr5_entity_tbl[168] = {coll = 0x01}; -- Cupboard 3
tr5_entity_tbl[169] = {coll = 0x01}; -- Cupboard 3 MIP
tr5_entity_tbl[170] = {coll = 0x01}; -- Suitcase
tr5_entity_tbl[171] = {coll = 0x01}; -- Suitcase MIP

-- PICK-UP WALKTHROUGH ITEMS

tr5_entity_tbl[172] = {coll = 0x00}; -- Puzzle 1
tr5_entity_tbl[173] = {coll = 0x00}; -- Puzzle 2
tr5_entity_tbl[174] = {coll = 0x00}; -- Puzzle 3
tr5_entity_tbl[175] = {coll = 0x00}; -- Puzzle 4
tr5_entity_tbl[176] = {coll = 0x00}; -- Puzzle 5
tr5_entity_tbl[177] = {coll = 0x00}; -- Puzzle 6
tr5_entity_tbl[178] = {coll = 0x00}; -- Puzzle 7
tr5_entity_tbl[179] = {coll = 0x00}; -- Puzzle 8
tr5_entity_tbl[180] = {coll = 0x00}; -- Puzzle 1 Combo 1
tr5_entity_tbl[181] = {coll = 0x00}; -- Puzzle 1 Combo 2
tr5_entity_tbl[182] = {coll = 0x00}; -- Puzzle 2 Combo 1
tr5_entity_tbl[183] = {coll = 0x00}; -- Puzzle 2 Combo 2
tr5_entity_tbl[184] = {coll = 0x00}; -- Puzzle 3 Combo 1
tr5_entity_tbl[185] = {coll = 0x00}; -- Puzzle 3 Combo 2
tr5_entity_tbl[186] = {coll = 0x00}; -- Puzzle 4 Combo 1
tr5_entity_tbl[187] = {coll = 0x00}; -- Puzzle 4 Combo 2
tr5_entity_tbl[188] = {coll = 0x00}; -- Puzzle 5 Combo 1
tr5_entity_tbl[189] = {coll = 0x00}; -- Puzzle 5 Combo 2
tr5_entity_tbl[190] = {coll = 0x00}; -- Puzzle 6 Combo 1
tr5_entity_tbl[191] = {coll = 0x00}; -- Puzzle 6 Combo 2
tr5_entity_tbl[192] = {coll = 0x00}; -- Puzzle 7 Combo 1
tr5_entity_tbl[193] = {coll = 0x00}; -- Puzzle 7 Combo 2
tr5_entity_tbl[194] = {coll = 0x00}; -- Puzzle 8 Combo 1
tr5_entity_tbl[195] = {coll = 0x00}; -- Puzzle 8 Combo 2
tr5_entity_tbl[196] = {coll = 0x00}; -- Key 1
tr5_entity_tbl[197] = {coll = 0x00}; -- Key 2
tr5_entity_tbl[198] = {coll = 0x00}; -- Key 3 - UNUSED?
tr5_entity_tbl[199] = {coll = 0x00}; -- Key 4 - UNUSED?
tr5_entity_tbl[200] = {coll = 0x00}; -- Key 5 - UNUSED?
tr5_entity_tbl[201] = {coll = 0x00}; -- Key 6
tr5_entity_tbl[202] = {coll = 0x00}; -- Key 7
tr5_entity_tbl[203] = {coll = 0x00}; -- Key 8

tr5_entity_tbl[204] = {coll = 0x00}; -- Key item 1 combo 1
tr5_entity_tbl[205] = {coll = 0x00}; -- Key item 1 combo 2
tr5_entity_tbl[206] = {coll = 0x00}; -- Key item 2 combo 1
tr5_entity_tbl[207] = {coll = 0x00}; -- Key item 2 combo 2
tr5_entity_tbl[208] = {coll = 0x00}; -- Key item 3 combo 1
tr5_entity_tbl[209] = {coll = 0x00}; -- Key item 3 combo 2
tr5_entity_tbl[210] = {coll = 0x00}; -- Key item 4 combo 1
tr5_entity_tbl[211] = {coll = 0x00}; -- Key item 4 combo 2
tr5_entity_tbl[212] = {coll = 0x00}; -- Key item 5 combo 1
tr5_entity_tbl[213] = {coll = 0x00}; -- Key item 5 combo 2
tr5_entity_tbl[214] = {coll = 0x00}; -- Key item 6 combo 1
tr5_entity_tbl[215] = {coll = 0x00}; -- Key item 6 combo 2
tr5_entity_tbl[216] = {coll = 0x00}; -- Key item 7 combo 1
tr5_entity_tbl[217] = {coll = 0x00}; -- Key item 7 combo 2
tr5_entity_tbl[218] = {coll = 0x00}; -- Key item 8 combo 1
tr5_entity_tbl[219] = {coll = 0x00}; -- Key item 8 combo 2

tr5_entity_tbl[220] = {coll = 0x00}; -- Pickup item 1
tr5_entity_tbl[221] = {coll = 0x00}; -- Pickup item 2
tr5_entity_tbl[222] = {coll = 0x00}; -- Pickup item 3 - UNUSED?
tr5_entity_tbl[223] = {coll = 0x00}; -- Gold rose

tr5_entity_tbl[224] = {coll = 0x00}; -- Pickup item 1 combo 1 - UNUSED
tr5_entity_tbl[225] = {coll = 0x00}; -- Pickup item 1 combo 2 - UNUSED
tr5_entity_tbl[226] = {coll = 0x00}; -- Pickup item 2 combo 1 - UNUSED
tr5_entity_tbl[227] = {coll = 0x00}; -- Pickup item 2 combo 2 - UNUSED
tr5_entity_tbl[228] = {coll = 0x00}; -- Pickup item 3 combo 1 - UNUSED
tr5_entity_tbl[229] = {coll = 0x00}; -- Pickup item 3 combo 2 - UNUSED
tr5_entity_tbl[230] = {coll = 0x00}; -- Pickup item 4 combo 1 - UNUSED
tr5_entity_tbl[231] = {coll = 0x00}; -- Pickup item 4 combo 2 - UNUSED

tr5_entity_tbl[232] = {coll = 0x00}; -- Examine 1 - UNUSED
tr5_entity_tbl[233] = {coll = 0x00}; -- Examine 2 - UNUSED
tr5_entity_tbl[234] = {coll = 0x00}; -- Examine 3 - UNUSED

tr5_entity_tbl[235] = {coll = 0x00}; -- Chloroform cloth
tr5_entity_tbl[236] = {coll = 0x00}; -- Chloroform bottle
tr5_entity_tbl[237] = {coll = 0x00}; -- Chloroform soaked cloth
tr5_entity_tbl[238] = {coll = 0x00}; -- Cosh (?)
tr5_entity_tbl[239] = {coll = 0x00}; -- Hammer item - UNUSED?
tr5_entity_tbl[240] = {coll = 0x00}; -- Crowbar item
tr5_entity_tbl[241] = {coll = 0x00}; -- Torch item

-- PUZZLEHOLES AND KEYHOLES

tr5_entity_tbl[242] = {coll = 0x01, func = "keyhole"}; -- Puzzle hole 1
tr5_entity_tbl[243] = {coll = 0x01, func = "keyhole"}; -- Puzzle hole 2
tr5_entity_tbl[244] = {coll = 0x01, func = "keyhole"}; -- Puzzle hole 3
tr5_entity_tbl[245] = {coll = 0x01, func = "keyhole"}; -- Puzzle hole 4
tr5_entity_tbl[246] = {coll = 0x01, func = "keyhole"}; -- Puzzle hole 5
tr5_entity_tbl[247] = {coll = 0x01, func = "keyhole"}; -- Puzzle hole 6
tr5_entity_tbl[248] = {coll = 0x01, func = "keyhole"}; -- Puzzle hole 7
tr5_entity_tbl[249] = {coll = 0x01, func = "keyhole"}; -- Puzzle hole 8
tr5_entity_tbl[250] = {coll = 0x01}; -- Puzzle done 1
tr5_entity_tbl[251] = {coll = 0x01}; -- Puzzle done 2
tr5_entity_tbl[252] = {coll = 0x01}; -- Puzzle done 3
tr5_entity_tbl[253] = {coll = 0x01}; -- Puzzle done 4
tr5_entity_tbl[254] = {coll = 0x01}; -- Puzzle done 5
tr5_entity_tbl[255] = {coll = 0x01}; -- Puzzle done 6
tr5_entity_tbl[256] = {coll = 0x01}; -- Puzzle done 7
tr5_entity_tbl[257] = {coll = 0x01}; -- Puzzle done 8
tr5_entity_tbl[258] = {coll = 0x01, func = "keyhole"}; -- Keyhole 1
tr5_entity_tbl[259] = {coll = 0x01, func = "keyhole"}; -- Keyhole 2
tr5_entity_tbl[260] = {coll = 0x01, func = "keyhole"}; -- Keyhole 3 - UNUSED?
tr5_entity_tbl[261] = {coll = 0x01, func = "keyhole"}; -- Keyhole 4 - UNUSED?
tr5_entity_tbl[262] = {coll = 0x01, func = "keyhole"}; -- Keyhole 5 - UNUSED?
tr5_entity_tbl[263] = {coll = 0x01, func = "keyhole"}; -- Keyhole 6
tr5_entity_tbl[264] = {coll = 0x01, func = "keyhole"}; -- Keyhole 7
tr5_entity_tbl[265] = {coll = 0x01, func = "keyhole"}; -- Keyhole 8

-- SWITCHES

tr5_entity_tbl[266] = {coll = 0x01, func = "switch"}; -- Switch type 1
tr5_entity_tbl[267] = {coll = 0x01, func = "switch"}; -- Switch type 2
tr5_entity_tbl[268] = {coll = 0x01, func = "switch"}; -- Switch type 3
tr5_entity_tbl[269] = {coll = 0x01, func = "switch"}; -- Switch type 4
tr5_entity_tbl[270] = {coll = 0x01, func = "switch"}; -- Switch type 5
tr5_entity_tbl[271] = {coll = 0x01, func = "switch"}; -- Switch type 6
tr5_entity_tbl[272] = {coll = 0x01}; -- Shoot switch 1
tr5_entity_tbl[273] = {coll = 0x01}; -- Shoot switch 2
tr5_entity_tbl[274] = {coll = 0x01}; -- Airlock switch
tr5_entity_tbl[275] = {coll = 0x01, func = "switch"}; -- Underwater switch 1 - UNUSED?
tr5_entity_tbl[276] = {coll = 0x01, func = "switch"}; -- Underwater switch 2 - UNUSED?
tr5_entity_tbl[277] = {coll = 0x01}; -- Turn switch - UNUSED?
tr5_entity_tbl[278] = {coll = 0x01, func = "switch"}; -- Cog switch
tr5_entity_tbl[279] = {coll = 0x01}; -- Lever switch - UNUSED?
tr5_entity_tbl[280] = {coll = 0x01}; -- Jump switch
tr5_entity_tbl[281] = {coll = 0x01}; -- Crowbar switch - UNUSED?
tr5_entity_tbl[282] = {coll = 0x01, func = "switch"}; -- Pulley
tr5_entity_tbl[283] = {coll = 0x01}; -- Crowdove switch

-- DOORS

tr5_entity_tbl[284] = {coll = 0x01, func = "door"}; -- Door 1
tr5_entity_tbl[285] = {coll = 0x01, func = "door"}; -- Door 1 MIP
tr5_entity_tbl[286] = {coll = 0x01, func = "door"}; -- Door 2
tr5_entity_tbl[287] = {coll = 0x01, func = "door"}; -- Door 2 MIP
tr5_entity_tbl[288] = {coll = 0x01, func = "door"}; -- Door 3
tr5_entity_tbl[289] = {coll = 0x01, func = "door"}; -- Door 3 MIP
tr5_entity_tbl[290] = {coll = 0x01, func = "door"}; -- Door 4
tr5_entity_tbl[291] = {coll = 0x01, func = "door"}; -- Door 4 MIP
tr5_entity_tbl[292] = {coll = 0x01, func = "door"}; -- Door 5
tr5_entity_tbl[293] = {coll = 0x01, func = "door"}; -- Door 5 MIP
tr5_entity_tbl[294] = {coll = 0x01, func = "door"}; -- Door 6
tr5_entity_tbl[295] = {coll = 0x01, func = "door"}; -- Door 6 MIP
tr5_entity_tbl[296] = {coll = 0x01, func = "door"}; -- Door 7
tr5_entity_tbl[297] = {coll = 0x01, func = "door"}; -- Door 7 MIP
tr5_entity_tbl[298] = {coll = 0x01, func = "door"}; -- Door 8
tr5_entity_tbl[299] = {coll = 0x01, func = "door"}; -- Door 8 MIP
tr5_entity_tbl[300] = {coll = 0x01}; -- Closed Door 1
tr5_entity_tbl[301] = {coll = 0x01}; -- Closed Door 1 MIP
tr5_entity_tbl[302] = {coll = 0x01}; -- Closed Door 2
tr5_entity_tbl[303] = {coll = 0x01}; -- Closed Door 2 MIP
tr5_entity_tbl[304] = {coll = 0x01}; -- Closed Door 3
tr5_entity_tbl[305] = {coll = 0x01}; -- Closed Door 3 MIP
tr5_entity_tbl[306] = {coll = 0x01}; -- Closed Door 4
tr5_entity_tbl[307] = {coll = 0x01}; -- Closed Door 4 MIP
tr5_entity_tbl[308] = {coll = 0x01}; -- Closed Door 5
tr5_entity_tbl[309] = {coll = 0x01}; -- Closed Door 5 MIP
tr5_entity_tbl[310] = {coll = 0x01}; -- Closed Door 6
tr5_entity_tbl[311] = {coll = 0x01}; -- Closed Door 6 MIP
tr5_entity_tbl[312] = {coll = 0x01}; -- Lift doors 1
tr5_entity_tbl[313] = {coll = 0x01}; -- Lift doors 1 MIP
tr5_entity_tbl[314] = {coll = 0x01}; -- Lift doors 2
tr5_entity_tbl[315] = {coll = 0x01}; -- Lift doors 2 MIP
tr5_entity_tbl[316] = {coll = 0x01}; -- Push-pull door 1 - UNUSED?
tr5_entity_tbl[317] = {coll = 0x01}; -- Push-pull door 1 MIP - UNUSED?
tr5_entity_tbl[318] = {coll = 0x01}; -- Push-pull door 2 - UNUSED?
tr5_entity_tbl[319] = {coll = 0x01}; -- Push-pull door 2 - UNUSED?
tr5_entity_tbl[320] = {coll = 0x01}; -- Kick door 1
tr5_entity_tbl[321] = {coll = 0x01}; -- Kick door 1 MIP - UNUSED?
tr5_entity_tbl[322] = {coll = 0x01}; -- Kick door 2
tr5_entity_tbl[323] = {coll = 0x01}; -- Kick door 2 MIP - UNUSED?
tr5_entity_tbl[324] = {coll = 0x01}; -- Underwater door
tr5_entity_tbl[325] = {coll = 0x01}; -- Underwater door MIP - UNUSED?
tr5_entity_tbl[326] = {coll = 0x01, func = "pushdoor"}; -- Double doors
tr5_entity_tbl[327] = {coll = 0x01, func = "pushdoor"}; -- Double doors MIP
tr5_entity_tbl[328] = {coll = 0x01}; -- Sequence door
tr5_entity_tbl[329] = {coll = 0x01}; -- Sequence switch (door) 1
tr5_entity_tbl[330] = {coll = 0x01}; -- Sequence switch (door) 2
tr5_entity_tbl[331] = {coll = 0x01}; -- Sequence switch (door) 3
tr5_entity_tbl[332] = {coll = 0x01}; -- Steel door

tr5_entity_tbl[333] = {coll = 0x00}; -- God head - UNUSED, TR4 LEFTOVER

-- PICK-UP SUPPLY ITEMS

tr5_entity_tbl[334] = {coll = 0x00}; -- Pistols
tr5_entity_tbl[335] = {coll = 0x00}; -- Pistols ammo
tr5_entity_tbl[336] = {coll = 0x00}; -- Uzis
tr5_entity_tbl[337] = {coll = 0x00}; -- Uzis ammo
tr5_entity_tbl[338] = {coll = 0x00}; -- Shotgun
tr5_entity_tbl[339] = {coll = 0x00}; -- Shotgun shells 1
tr5_entity_tbl[340] = {coll = 0x00}; -- Shotgun shells 2
tr5_entity_tbl[341] = {coll = 0x00}; -- Grappling gun
tr5_entity_tbl[342] = {coll = 0x00}; -- Grappling ammo type 1
tr5_entity_tbl[343] = {coll = 0x00}; -- Grappling ammo type 2
tr5_entity_tbl[344] = {coll = 0x00}; -- Grappling ammo type 3
tr5_entity_tbl[345] = {coll = 0x00}; -- HK Gun
tr5_entity_tbl[346] = {coll = 0x00}; -- HK ammo
tr5_entity_tbl[347] = {coll = 0x00}; -- Revolver
tr5_entity_tbl[348] = {coll = 0x00}; -- Revolver bullets
tr5_entity_tbl[349] = {coll = 0x00}; -- Big Medi-Pack
tr5_entity_tbl[350] = {coll = 0x00}; -- Small Medi-Pack
tr5_entity_tbl[351] = {coll = 0x00}; -- Laser sight
tr5_entity_tbl[352] = {coll = 0x00}; -- Binoculars
tr5_entity_tbl[353] = {coll = 0x00}; -- Silencer
tr5_entity_tbl[354] = {coll = 0x00}; -- Burning flare
tr5_entity_tbl[355] = {coll = 0x00}; -- Flares
tr5_entity_tbl[356] = {coll = 0x00}; -- Timex-TMX (Compass)

-- INVENTORY ITEMS

tr5_entity_tbl[357] = {coll = 0x00}; -- Load inventory
tr5_entity_tbl[358] = {coll = 0x00}; -- Save inventory
tr5_entity_tbl[359] = {coll = 0x00}; -- Disk load
tr5_entity_tbl[360] = {coll = 0x00}; -- Disk save
tr5_entity_tbl[361] = {coll = 0x00}; -- Memcard load
tr5_entity_tbl[362] = {coll = 0x00}; -- Memcard save

-- NULLMESHES, SERVICE OBJECTS AND EMITTERS

tr5_entity_tbl[363] = {coll = 0x00, hide = 0x01}; -- Smoke emitter white - EMITTER
tr5_entity_tbl[364] = {coll = 0x00, hide = 0x01}; -- Smoke emitter black - EMITTER
tr5_entity_tbl[365] = {coll = 0x00, hide = 0x01}; -- Steam emitter - EMITTER
tr5_entity_tbl[366] = {coll = 0x00, hide = 0x01}; -- Earthquake - SHAKE CAMERA
tr5_entity_tbl[367] = {coll = 0x00, hide = 0x01}; -- Bubbles - EMITTER
tr5_entity_tbl[368] = {coll = 0x00, hide = 0x01}; -- Waterfall Mist - EMITTER

tr5_entity_tbl[369] = {coll = 0x00}; -- Gun shell - SPAWNED OBJECT, NO DIRECT USE
tr5_entity_tbl[370] = {coll = 0x00}; -- Shotgun shell - SPAWNED OBJECT, NO DIRECT USE
tr5_entity_tbl[371] = {coll = 0x00}; -- Gun flash - SPAWNED OBJECT, NO DIRECT USE

tr5_entity_tbl[372] = {coll = 0x00, hide = 0x01}; -- Color light - UNUSED?
tr5_entity_tbl[373] = {coll = 0x00, hide = 0x01}; -- Blinking light - UNUSED?
tr5_entity_tbl[374] = {coll = 0x00, hide = 0x01}; -- Pulse light - DYNAMIC LIGHT
tr5_entity_tbl[375] = {coll = 0x00, hide = 0x01}; -- Alarm light - DYNAMIC LIGHT
tr5_entity_tbl[376] = {coll = 0x00, hide = 0x01}; -- Electrical light - DYNAMIC LIGHT
tr5_entity_tbl[377] = {coll = 0x00, hide = 0x01}; -- Lens flare - UNUSED?

-- Remark: objects 378-386 are AI nullmeshes, and never show in game.

tr5_entity_tbl[378] = {coll = 0x00, hide = 0x01};
tr5_entity_tbl[379] = {coll = 0x00, hide = 0x01};
tr5_entity_tbl[380] = {coll = 0x00, hide = 0x01};
tr5_entity_tbl[381] = {coll = 0x00, hide = 0x01};
tr5_entity_tbl[382] = {coll = 0x00, hide = 0x01};
tr5_entity_tbl[383] = {coll = 0x00, hide = 0x01};
tr5_entity_tbl[384] = {coll = 0x00, hide = 0x01};
tr5_entity_tbl[385] = {coll = 0x00, hide = 0x01};
tr5_entity_tbl[386] = {coll = 0x00, hide = 0x01};

tr5_entity_tbl[387] = {coll = 0x00, hide = 0x01}; -- Teleporter
tr5_entity_tbl[388] = {coll = 0x00, hide = 0x01}; -- Lift teleporter

tr5_entity_tbl[389] = {coll = 0x00}; -- Raising cog

tr5_entity_tbl[390] = {coll = 0x00, hide = 0x01}; -- Laser
tr5_entity_tbl[391] = {coll = 0x00, hide = 0x01}; -- Steam laser
tr5_entity_tbl[392] = {coll = 0x00, hide = 0x01}; -- Floor laser 3

tr5_entity_tbl[393] = {coll = 0x00, hide = 0x01}; -- Kill all triggers (or Laser 4?)
tr5_entity_tbl[394] = {coll = 0x00, hide = 0x01}; -- Trigger Triggerer

-- MISC. SOLID OBJECTS (?)

tr5_entity_tbl[395] = {coll = 0x01}; -- High object 1 (Polerope puzzle)
tr5_entity_tbl[396] = {coll = 0x00}; -- High object 2 (Flame Emiter with sparks)
tr5_entity_tbl[397] = {coll = 0x01}; -- Smash object 1 (Breakeable glass Floor)
tr5_entity_tbl[398] = {coll = 0x01}; -- Smash object 2 (Breakeable glass Door)
tr5_entity_tbl[399] = {coll = 0x01}; -- Smash object 3 - UNUSED?
tr5_entity_tbl[400] = {coll = 0x01}; -- Smash object 4 - UNUSED?
tr5_entity_tbl[401] = {coll = 0x01}; -- Smash object 5 - UNUSED?
tr5_entity_tbl[402] = {coll = 0x01}; -- Smash object 6 - UNUSED?
tr5_entity_tbl[403] = {coll = 0x01}; -- Smash object 7 - UNUSED?
tr5_entity_tbl[404] = {coll = 0x01}; -- Smash object 8 - UNUSED?
tr5_entity_tbl[405] = {coll = 0x01}; -- Meshswap 1
tr5_entity_tbl[406] = {coll = 0x01}; -- Meshswap 2
tr5_entity_tbl[407] = {coll = 0x01}; -- Meshswap 3

-- NULLMESHES

tr5_entity_tbl[408] = {coll = 0x00}; -- Body part - UNUSED?
tr5_entity_tbl[409] = {coll = 0x00, hide = 0x01}; -- Camera target

-- WATERFALLS (MESHES WITH UV-SCROLL ANIMATED TEXTURES)

tr5_entity_tbl[410] = {coll = 0x00}; -- Waterfall 1
tr5_entity_tbl[411] = {coll = 0x00}; -- Waterfall 2
tr5_entity_tbl[412] = {coll = 0x00}; -- Waterfall 3
tr5_entity_tbl[413] = {coll = 0x00}; -- Fishtank waterfall
tr5_entity_tbl[414] = {coll = 0x00}; -- Waterfalls 1
tr5_entity_tbl[415] = {coll = 0x00}; -- Waterfalls 2

-- ANIMATINGS

tr5_entity_tbl[416] = {coll = 0x01}; -- Animating 1
tr5_entity_tbl[417] = {coll = 0x01}; -- Animating 1 MIP
tr5_entity_tbl[418] = {coll = 0x01}; -- Animating 2
tr5_entity_tbl[419] = {coll = 0x01}; -- Animating 2 MIP
tr5_entity_tbl[420] = {coll = 0x01}; -- Animating 3
tr5_entity_tbl[421] = {coll = 0x01}; -- Animating 3 MIP
tr5_entity_tbl[422] = {coll = 0x01}; -- Animating 4
tr5_entity_tbl[423] = {coll = 0x01}; -- Animating 4 MIP
tr5_entity_tbl[424] = {coll = 0x01}; -- Animating 5
tr5_entity_tbl[425] = {coll = 0x01}; -- Animating 5 MIP
tr5_entity_tbl[426] = {coll = 0x01}; -- Animating 6
tr5_entity_tbl[427] = {coll = 0x01}; -- Animating 6 MIP
tr5_entity_tbl[428] = {coll = 0x01}; -- Animating 7
tr5_entity_tbl[429] = {coll = 0x01}; -- Animating 7 MIP
tr5_entity_tbl[430] = {coll = 0x01}; -- Animating 8
tr5_entity_tbl[431] = {coll = 0x01}; -- Animating 8 MIP
tr5_entity_tbl[432] = {coll = 0x01}; -- Animating 9
tr5_entity_tbl[433] = {coll = 0x01}; -- Animating 9 MIP
tr5_entity_tbl[434] = {coll = 0x01}; -- Animating 10
tr5_entity_tbl[435] = {coll = 0x01}; -- Animating 10 MIP
tr5_entity_tbl[436] = {coll = 0x01}; -- Animating 11
tr5_entity_tbl[437] = {coll = 0x01}; -- Animating 11 MIP
tr5_entity_tbl[438] = {coll = 0x01}; -- Animating 12
tr5_entity_tbl[439] = {coll = 0x01}; -- Animating 12 MIP
tr5_entity_tbl[440] = {coll = 0x00}; -- Animating 13 / map 3 - light
tr5_entity_tbl[441] = {coll = 0x01}; -- Animating 13 MIP
tr5_entity_tbl[442] = {coll = 0x01}; -- Animating 14
tr5_entity_tbl[443] = {coll = 0x01}; -- Animating 14 MIP
tr5_entity_tbl[444] = {coll = 0x00}; -- Animating 15 / map 3 - light
tr5_entity_tbl[445] = {coll = 0x01}; -- Animating 15 MIP
tr5_entity_tbl[446] = {coll = 0x00}; -- Animating 16 / map 2 - light
tr5_entity_tbl[447] = {coll = 0x01}; -- Animating 16 MIP

-- STATIC TERRAIN

tr5_entity_tbl[448] = {coll = 0x01}; -- Bridge flat
tr5_entity_tbl[449] = {coll = 0x01}; -- Bridge flat MIP
tr5_entity_tbl[450] = {coll = 0x01}; -- Bridge tilt 1
tr5_entity_tbl[451] = {coll = 0x01}; -- Bridge tilt 1 MIP
tr5_entity_tbl[452] = {coll = 0x01}; -- Bridge tilt 2
tr5_entity_tbl[453] = {coll = 0x01}; -- Bridge tilt 2 MIP


function getEntityModelProperties(ver, id)
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
        return nil, nil, nil, nil;
    end;

    if(tbl[id] == nil) then
        return nil, nil, nil, nil;
    else
        return tbl[id].coll, tbl[id].hide, tbl[id].trav, tbl[id].func;
    end;
end;
