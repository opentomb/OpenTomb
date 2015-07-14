-- OPENTOMB CONSTANTS SCRIPT
-- By TeslaRus, Lwmte 2014

--------------------------------------------------------------------------------
-- Here we place all system script constants and globals.
-- DO NOT place any function calls or functions here, as this script is loaded
-- first, when system notifications are not ready yet, consider it as equivalent
-- of header file in C/C++.
--------------------------------------------------------------------------------

-- Engine version constants

TR_I                = 0;
TR_I_DEMO           = 1;
TR_I_UB             = 2;
TR_II               = 3;
TR_II_DEMO          = 4;
TR_III              = 5;
TR_IV               = 6;
TR_IV_DEMO          = 7;
TR_V                = 8;
TR_UNKNOWN          = 127;

-- Entity constants

ENTITY_STATE_ENABLED                      = 0x0001;     -- Entity is enabled
ENTITY_STATE_ACTIVE                       = 0x0002;     -- Entity is animated - RENAME IT TO ENTITY_STATE_ANIMATING
ENTITY_STATE_VISIBLE                      = 0x0004;     -- Entity is visible

ENTITY_TYPE_GENERIC                       = 0x0000;
ENTITY_TYPE_INTERACTIVE                   = 0x0001;
ENTITY_TYPE_TRIGGER_ACTIVATOR             = 0x0002;
ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR        = 0x0004;
ENTITY_TYPE_PICKABLE                      = 0x0008;
ENTITY_TYPE_TRAVERSE                      = 0x0010;
ENTITY_TYPE_TRAVERSE_FLOOR                = 0x0020;
ENTITY_TYPE_DYNAMIC                       = 0x0040;
ENTITY_TYPE_ACTOR                         = 0x0080;

ENTITY_CALLBACK_NONE                      = 0x00000000;
ENTITY_CALLBACK_ACTIVATE                  = 0x00000001;
ENTITY_CALLBACK_DEACTIVATE                = 0x00000002;
ENTITY_CALLBACK_COLLISION                 = 0x00000004;
ENTITY_CALLBACK_STAND                     = 0x00000008;
ENTITY_CALLBACK_HIT                       = 0x00000010;

-- Animation mode constants

ANIM_NORMAL_CONTROL  = 0;
ANIM_LOOP_LAST_FRAME = 1;
ANIM_LOCK            = 2;

-- Move type constants

MOVE_STATIC_POS    = 0;
MOVE_KINEMATIC     = 1;
MOVE_ON_FLOOR      = 2;
MOVE_WADE          = 3;
MOVE_QUICKSAND     = 4;
MOVE_ON_WATER      = 5;
MOVE_UNDERWATER    = 6;
MOVE_FREE_FALLING  = 7;
MOVE_CLIMBING      = 8;
MOVE_MONKEYSWING   = 9;
MOVE_WALLS_CLIMB   = 10;
MOVE_DOZY          = 11;

-- Response constants

RESP_KILL           = 0;
RESP_VERT_COLLIDE   = 1;
RESP_HOR_COLLIDE    = 2;
RESP_SLIDE          = 3;

-- Entity timer constants

TICK_IDLE    = 0;
TICK_STOPPED = 1;
TICK_ACTIVE  = 2;

-- Trigger operation constants

TRIGGER_OP_OR  = 0;
TRIGGER_OP_XOR = 1;

-- Global frame time variable, in seconds

FRAME_TIME = 1.0 / 60.0;
