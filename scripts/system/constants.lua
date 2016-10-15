-- OPENTOMB CONSTANTS SCRIPT
-- By TeslaRus, Lwmte 2014

--------------------------------------------------------------------------------
-- Here we place all system script constants and globals.
-- DO NOT place any function calls or functions here, as this script is loaded
-- first, when system notifications are not ready yet, consider it as equivalent
-- of header file in C/C++.
--------------------------------------------------------------------------------

-- Response constants

RESP_KILL           = 0;
RESP_VERT_COLLIDE   = 1;
RESP_HOR_COLLIDE    = 2;
RESP_SLIDE          = 3;
RESP_LEAN           = 4;

-- Entity timer constants

TICK_IDLE    = 0;
TICK_STOPPED = 1;
TICK_ACTIVE  = 2;

-- Trigger operation constants

TRIGGER_OP_OR  = 0;
TRIGGER_OP_XOR = 1;

-- Sector material types

SECTOR_MATERIAL_MUD      = 0
SECTOR_MATERIAL_SNOW     = 1
SECTOR_MATERIAL_SAND     = 2
SECTOR_MATERIAL_GRAVEL   = 3
SECTOR_MATERIAL_ICE      = 4
SECTOR_MATERIAL_WATER    = 5
SECTOR_MATERIAL_STONE    = 6
SECTOR_MATERIAL_WOOD     = 7
SECTOR_MATERIAL_METAL    = 8
SECTOR_MATERIAL_MARBLE   = 9
SECTOR_MATERIAL_GRASS    = 10
SECTOR_MATERIAL_CONCRETE = 11
SECTOR_MATERIAL_OLDWOOD  = 12
SECTOR_MATERIAL_OLDMETAL = 13

-- Global frame time variable, in seconds

FRAME_TIME = 1.0 / 60.0;
