-- OPENTOMB CHARACTER DEFINE FILE
-- by Lwmte, Oct 2014

--------------------------------------------------------------------------------
-- Here we define character options to ease usage of getCharacterOption and
-- setCharacterOption functions.
--------------------------------------------------------------------------------

PARAM_HEALTH   = 0;
PARAM_AIR      = 1;
PARAM_STAMINA  = 2;
PARAM_WARMTH   = 3;
PARAM_EXTRA1   = 4;
PARAM_EXTRA2   = 5;
PARAM_EXTRA3   = 6;
PARAM_EXTRA4   = 7;

-------- Lara's model-------
--           .=.
--          | 14|
--           \ / \
--       / |     | \
--  11  / |   7   | \  8
--     /   |     |   \
--     |    =====    |
--  12 |    =====    | 9
--     |   /  0  \   |
--  13 0  /_______\  0 10
--        |  | |  |
--        |1 | |4 |
--        |  | |  |
--        |__| |__|
--        |  | |  |
--        |2 | |5 |
--        |  | |  |
--        |__| |__|
--     3  |__| |__|  6
----------------------------

BODY_PART_BODY_LOW     = 0x00000001;           -- 0
BODY_PART_BODY_UPPER   = 0x00000002;           -- 7
BODY_PART_HEAD         = 0x00000004;           -- 14

BODY_PART_LEFT_HAND_1  = 0x00000008;           -- 11
BODY_PART_LEFT_HAND_2  = 0x00000010;           -- 12
BODY_PART_LEFT_HAND_3  = 0x00000020;           -- 13
BODY_PART_RIGHT_HAND_1 = 0x00000040;           -- 8
BODY_PART_RIGHT_HAND_2 = 0x00000080;           -- 9
BODY_PART_RIGHT_HAND_3 = 0x00000100;           -- 10

BODY_PART_LEFT_LEG_1   = 0x00000200;           -- 1
BODY_PART_LEFT_LEG_2   = 0x00000400;           -- 2
BODY_PART_LEFT_LEG_3   = 0x00000800;           -- 3
BODY_PART_RIGHT_LEG_1  = 0x00001000;           -- 4
BODY_PART_RIGHT_LEG_2  = 0x00002000;           -- 5
BODY_PART_RIGHT_LEG_3  = 0x00004000;           -- 6
