
#ifndef TRIGGER_H
#define TRIGGER_H

#include <stdint.h>

// Native TR floor data functions

#define TR_FD_FUNC_PORTALSECTOR                 0x01
#define TR_FD_FUNC_FLOORSLANT                   0x02
#define TR_FD_FUNC_CEILINGSLANT                 0x03
#define TR_FD_FUNC_TRIGGER                      0x04
#define TR_FD_FUNC_DEATH                        0x05
#define TR_FD_FUNC_CLIMB                        0x06
#define TR_FD_FUNC_FLOORTRIANGLE_NW             0x07    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NE             0x08    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW           0x09    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE           0x0A    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW   0x0B    //  [P\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE   0x0C    //  [_\P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE   0x0D    //  [_/P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW   0x0E    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW 0x0F    //  [P\_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE 0x10    //  [_\P]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW 0x11    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE 0x12    //  [_/P]
#define TR_FD_FUNC_MONKEY                       0x13
#define TR_FD_FUNC_MINECART_LEFT                0x14    // In TR3 only. Function changed in TR4+.
#define TR_FD_FUNC_MINECART_RIGHT               0x15    // In TR3 only. Function changed in TR4+.

// Native TR trigger (TR_FD_FUNC_TRIGGER) types.

#define TR_FD_TRIGTYPE_TRIGGER          0x00    // If Lara is in sector, run (any case).
#define TR_FD_TRIGTYPE_PAD              0x01    // If Lara is in sector, run (land case).
#define TR_FD_TRIGTYPE_SWITCH           0x02    // If item is activated, run, else stop.
#define TR_FD_TRIGTYPE_KEY              0x03    // If item is activated, run.
#define TR_FD_TRIGTYPE_PICKUP           0x04    // If item is picked up, run.
#define TR_FD_TRIGTYPE_HEAVY            0x05    // If item is in sector, run, else stop.
#define TR_FD_TRIGTYPE_ANTIPAD          0x06    // If Lara is in sector, stop (land case).
#define TR_FD_TRIGTYPE_COMBAT           0x07    // If Lara is in combat state, run (any case).
#define TR_FD_TRIGTYPE_DUMMY            0x08    // If Lara is in sector, run (air case).
#define TR_FD_TRIGTYPE_ANTITRIGGER      0x09    // TR2-5 only: If Lara is in sector, stop (any case).
#define TR_FD_TRIGTYPE_HEAVYSWITCH      0x0A    // TR3-5 only: If item is activated by item, run.
#define TR_FD_TRIGTYPE_HEAVYANTITRIGGER 0x0B    // TR3-5 only: If item is activated by item, stop.
#define TR_FD_TRIGTYPE_MONKEY           0x0C    // TR3-5 only: If Lara is monkey-swinging, run.
#define TR_FD_TRIGTYPE_SKELETON         0x0D    // TR5 only: Activated by skeleton only?
#define TR_FD_TRIGTYPE_TIGHTROPE        0x0E    // TR5 only: If Lara is on tightrope, run.
#define TR_FD_TRIGTYPE_CRAWLDUCK        0x0F    // TR5 only: If Lara is crawling, run.
#define TR_FD_TRIGTYPE_CLIMB            0x10    // TR5 only: If Lara is climbing, run.

// Native trigger function types.

#define TR_FD_TRIGFUNC_OBJECT           0x00
#define TR_FD_TRIGFUNC_SET_CAMERA       0x01
#define TR_FD_TRIGFUNC_UWCURRENT        0x02
#define TR_FD_TRIGFUNC_FLIPMAP          0x03
#define TR_FD_TRIGFUNC_FLIPON           0x04
#define TR_FD_TRIGFUNC_FLIPOFF          0x05
#define TR_FD_TRIGFUNC_SET_TARGET       0x06
#define TR_FD_TRIGFUNC_ENDLEVEL         0x07
#define TR_FD_TRIGFUNC_PLAYTRACK        0x08
#define TR_FD_TRIGFUNC_FLIPEFFECT       0x09
#define TR_FD_TRIGFUNC_SECRET           0x0A
#define TR_FD_TRIGFUNC_CLEARBODIES      0x0B    // Unused in TR4
#define TR_FD_TRIGFUNC_FLYBY            0x0C
#define TR_FD_TRIGFUNC_CUTSCENE         0x0D

// Action type specifies a kind of action which trigger performs. Mostly
// it's only related to item activation, as any other trigger operations
// are not affected by action type in original engines.

#define TR_ACTIONTYPE_NORMAL  0
#define TR_ACTIONTYPE_ANTI    1
#define TR_ACTIONTYPE_SWITCH  2
#define TR_ACTIONTYPE_BYPASS -1 // Used for "dummy" triggers from originals.

// Activator specifies a kind of triggering event (NOT to be confused
// with activator type mentioned below) to occur, like ordinary trigger,
// triggering by inserting a key, turning a switch or picking up item.

#define TR_ACTIVATOR_NORMAL 0
#define TR_ACTIVATOR_SWITCH 1
#define TR_ACTIVATOR_KEY    2
#define TR_ACTIVATOR_PICKUP 3

// Activator type is used to identify activator kind for specific
// trigger types (so-called HEAVY triggers). HEAVY means that trigger
// is activated by some other item, rather than Lara herself.

#define TR_ACTIVATORTYPE_LARA 0
#define TR_ACTIVATORTYPE_MISC 1


// Maximum number of flipmaps specifies how many flipmap indices to store. Usually,
// TR1-3 doesn't contain flipmaps above 10, while in TR4-5 number of flipmaps could
// be as much as 14-16. To make sure flipmap array will be suitable for all game
// versions, it is set to 32.

#define FLIPMAP_MAX_NUMBER          32

// Activation mask operation can be either XOR (for switch triggers) or OR (for any
// other types of triggers).

#define TRIGGER_OP_OR  0
#define TRIGGER_OP_XOR 1

struct lua_State;

typedef struct trigger_command_s
{
    uint16_t                        function;
    uint16_t                        operands;
    uint8_t                         once;
    uint8_t                         cam_index;
    uint8_t                         cam_timer;
    uint8_t                         cam_move;
    struct trigger_command_s       *next;
}trigger_command_t, *trigger_command_p;


typedef struct trigger_header_s
{
    uint16_t    function_value;
    uint16_t    sub_function : 14;
    uint16_t    once : 2;
    uint16_t    timer;
    uint16_t    mask;
    struct trigger_command_s       *commands;
}trigger_header_t, *trigger_header_p;


void Trigger_BuildScripts(trigger_header_p trigger, uint32_t trigger_index, const char *file_name);
void Trigger_DoCommands(trigger_header_p trigger, struct entity_s *ent);

void Trigger_TrigTypeToStr(char *buf, uint32_t size, uint32_t func);
void Trigger_TrigCmdToStr(char *buf, uint32_t size, uint32_t func);

#endif //TRIGGER_H
