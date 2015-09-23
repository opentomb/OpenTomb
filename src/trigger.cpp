
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/console.h"
#include "core/vmath.h"
#include "room.h"
#include "trigger.h"
#include "script.h"
#include "character_controller.h"
#include "anim_state_control.h"



bool Trigger_IsEntityProcessed(int32_t *lookup_table, uint16_t entity_index)
{
    // Fool-proof check for entity existence. Fixes LOTS of stray non-existent
    // entity #256 occurences in original games (primarily TR4-5).
    /*if(!World_GetEntityByID(&engine_world, entity_index))
    {
        return true;
    }*/

    int32_t *curr_table_index = lookup_table;

    while(*curr_table_index != -1)
    {
        if(*curr_table_index == (int32_t)entity_index)
        {
            return true;
        }
        curr_table_index++;
    }

    *curr_table_index = (int32_t)entity_index;
    return false;
}


void Trigger_BuildScripts(trigger_header_p trigger, uint32_t trigger_index, struct lua_State *lua)
{
    if(trigger && lua)
    {
        char header[128];               header[0]            = 0;   // Header condition
        char once_condition[128];       once_condition[0]    = 0;   // One-shot condition
        char cont_events[4096];         cont_events[0]       = 0;   // Continous trigger events
        char single_events[4096];       single_events[0]     = 0;   // One-shot trigger events
        char item_events[4096];         item_events[0]       = 0;   // Item activation events
        char anti_events[4096];         anti_events[0]       = 0;   // Item deactivation events, if needed

        char script[8192];              script[0]            = 0;   // Final script compile

        char buf[512];                  buf[0]  = 0;    // Stream buffer
        char buf2[512];                 buf2[0] = 0;    // Conditional pre-buffer for SWITCH triggers

        int activator   = TR_ACTIVATOR_NORMAL;      // Activator is normal by default.
        int action_type = TR_ACTIONTYPE_NORMAL;     // Action type is normal by default.
        int condition   = 0;                        // No condition by default.
        int mask_mode   = AMASK_OP_OR;              // Activation mask by default.

        // Processed entities lookup array initialization.

        int32_t ent_lookup_table[64];
        memset(ent_lookup_table, 0xFF, sizeof(int32_t)*64);

        // Activator type is LARA for all triggers except HEAVY ones, which are triggered by
        // some specific entity classes.

        int activator_type = ( (trigger->sub_function == TR_FD_TRIGTYPE_HEAVY)            ||
                               (trigger->sub_function == TR_FD_TRIGTYPE_HEAVYANTITRIGGER) ||
                               (trigger->sub_function == TR_FD_TRIGTYPE_HEAVYSWITCH) )     ? TR_ACTIVATORTYPE_MISC : TR_ACTIVATORTYPE_LARA;

        // Table cell header.

        snprintf(buf, 256, "trigger_list[%d] = {activator_type = %d, func = function(entity_index) \n", trigger_index, activator_type);

        strcat(script, buf);
        buf[0] = 0;     // Zero out buffer to prevent further trashing.

        switch(trigger->sub_function)
        {
            case TR_FD_TRIGTYPE_TRIGGER:
            case TR_FD_TRIGTYPE_HEAVY:
                activator = TR_ACTIVATOR_NORMAL;
                break;

            case TR_FD_TRIGTYPE_PAD:
            case TR_FD_TRIGTYPE_ANTIPAD:
                // Check move type for triggering entity.
                snprintf(buf, 128, " if(getEntityMoveType(entity_index) == %d) then \n", MOVE_ON_FLOOR);
                if(trigger->sub_function == TR_FD_TRIGTYPE_ANTIPAD) action_type = TR_ACTIONTYPE_ANTI;
                condition = 1;  // Set additional condition.
                break;

            case TR_FD_TRIGTYPE_SWITCH:
                // Set activator and action type for now; conditions are linked with first item in operand chain.
                activator = TR_ACTIVATOR_SWITCH;
                action_type = TR_ACTIONTYPE_SWITCH;
                mask_mode = AMASK_OP_XOR;
                break;

            case TR_FD_TRIGTYPE_HEAVYSWITCH:
                // Action type remains normal, as HEAVYSWITCH acts as "heavy trigger" with activator mask filter.
                activator = TR_ACTIVATOR_SWITCH;
                mask_mode = AMASK_OP_XOR;
                break;

            case TR_FD_TRIGTYPE_KEY:
                // Action type remains normal, as key acts one-way (no need in switch routines).
                activator = TR_ACTIVATOR_KEY;
                break;

            case TR_FD_TRIGTYPE_PICKUP:
                // Action type remains normal, as pick-up acts one-way (no need in switch routines).
                activator = TR_ACTIVATOR_PICKUP;
                break;

            case TR_FD_TRIGTYPE_COMBAT:
                // Check weapon status for triggering entity.
                snprintf(buf, 128, " if(getCharacterCombatMode(entity_index) > 0) then \n");
                condition = 1;  // Set additional condition.
                break;

            case TR_FD_TRIGTYPE_DUMMY:
            case TR_FD_TRIGTYPE_SKELETON:   ///@FIXME: Find the meaning later!!!
                // These triggers are being parsed, but not added to trigger script!
                action_type = TR_ACTIONTYPE_BYPASS;
                break;

            case TR_FD_TRIGTYPE_ANTITRIGGER:
            case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
                action_type = TR_ACTIONTYPE_ANTI;
                break;

            case TR_FD_TRIGTYPE_MONKEY:
            case TR_FD_TRIGTYPE_CLIMB:
                // Check move type for triggering entity.
                snprintf(buf, 128, " if(getEntityMoveType(entity_index) == %d) then \n", (trigger->sub_function == TR_FD_TRIGTYPE_MONKEY)?MOVE_MONKEYSWING:MOVE_CLIMBING);
                condition = 1;  // Set additional condition.
                break;

            case TR_FD_TRIGTYPE_TIGHTROPE:
                // Check state range for triggering entity.
                snprintf(buf, 128, " local state = getEntityState(entity_index) \n if((state >= %d) and (state <= %d)) then \n", TR_STATE_LARA_TIGHTROPE_IDLE, TR_STATE_LARA_TIGHTROPE_EXIT);
                condition = 1;  // Set additional condition.
                break;
            case TR_FD_TRIGTYPE_CRAWLDUCK:
                // Check state range for triggering entity.
                snprintf(buf, 128, " local state = getEntityState(entity_index) \n if((state >= %d) and (state <= %d)) then \n", TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_BEGIN, TR_ANIMATION_LARA_CRAWL_SMASH_LEFT);
                condition = 1;  // Set additional condition.
                break;
        }

        strcat(header, buf);    // Add condition to header.

        // Now parse operand chain for trigger function!
        uint16_t argn = 0;
        for(trigger_command_p command = trigger->commands; command; command = command->next)
        {
            switch(command->function)
            {
                case TR_FD_TRIGFUNC_OBJECT:         // ACTIVATE / DEACTIVATE object
                    // If activator is specified, first item operand counts as activator index (except
                    // heavy switch case, which is ordinary heavy trigger case with certain differences).
                    if((argn == 0) && (activator))
                    {
                        switch(activator)
                        {
                            case TR_ACTIVATOR_SWITCH:
                                if(action_type == TR_ACTIONTYPE_SWITCH)
                                {
                                    // Switch action type case.
                                    snprintf(buf, 256, " local switch_state = getEntityState(%d); \n local switch_sectorstatus = getEntitySectorStatus(%d); \n local switch_mask = getEntityMask(%d); \n\n", command->operands, command->operands, command->operands);
                                }
                                else
                                {
                                    // Ordinary type case (e.g. heavy switch).
                                    snprintf(buf, 256, " local switch_sectorstatus = getEntitySectorStatus(entity_index); \n local switch_mask = getEntityMask(entity_index); \n\n");
                                }
                                strcat(script, buf);

                                // Trigger activation mask is here filtered through activator's own mask.
                                snprintf(buf, 256, " if(switch_mask == 0) then switch_mask = 0x1F end; \n switch_mask = bit32.band(switch_mask, 0x%02X); \n\n", trigger->mask);
                                strcat(script, buf);
                                if(action_type == TR_ACTIONTYPE_SWITCH)
                                {
                                    // Switch action type case.
                                    snprintf(buf, 256, " if((switch_state == 0) and (switch_sectorstatus == 1)) then \n   setEntitySectorStatus(%d, 0); \n   setEntityTimer(%d, %d); \n", command->operands, command->operands, trigger->timer);
                                    if(trigger->once)
                                    {
                                        // Just lock out activator, no anti-action needed.
                                        snprintf(buf2, 128, " setEntityLock(%d, 1) \n", command->operands);
                                    }
                                    else
                                    {
                                        // Create statement for antitriggering a switch.
                                        snprintf(buf2, 256, " elseif((switch_state == 1) and (switch_sectorstatus == 1)) then\n   setEntitySectorStatus(%d, 0); \n   setEntityTimer(%d, 0); \n", command->operands, command->operands);
                                    }
                                }
                                else
                                {
                                    // Ordinary type case (e.g. heavy switch).
                                    snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %d, %d); \n", command->operands, mask_mode, trigger->once, trigger->timer);
                                    strcat(item_events, buf);
                                    snprintf(buf, 128, " if(switch_sectorstatus == 0) then \n   setEntitySectorStatus(entity_index, 1) \n");
                                }
                                break;

                            case TR_ACTIVATOR_KEY:
                                snprintf(buf, 256, " if((getEntityLock(%d) == 1) and (getEntitySectorStatus(%d) == 0)) then \n   setEntitySectorStatus(%d, 1); \n", command->operands, command->operands, command->operands);
                                break;

                            case TR_ACTIVATOR_PICKUP:
                                snprintf(buf, 256, " if((getEntityEnability(%d)) and (getEntitySectorStatus(%d) == 0)) then \n   setEntitySectorStatus(%d, 1); \n", command->operands, command->operands, command->operands);
                                break;
                        }

                        strcat(script, buf);
                    }
                    else
                    {
                        // In many original Core Design levels, level designers left dublicated entity activation operands.
                        // This results in setting same activation mask twice, effectively blocking entity from activation.
                        // To prevent this, a lookup table was implemented to know if entity already had its activation
                        // command added.
                        if(!Trigger_IsEntityProcessed(ent_lookup_table, command->operands))
                        {
                            // Other item operands are simply parsed as activation functions. Switch case is special, because
                            // function is fed with activation mask argument derived from activator mask filter (switch_mask),
                            // and also we need to process deactivation in a same way as activation, excluding resetting timer
                            // field. This is needed for two-way switch combinations (e.g. Palace Midas).
                            if(activator == TR_ACTIVATOR_SWITCH)
                            {
                                snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %d, %d); \n", command->operands, mask_mode, trigger->once, trigger->timer);
                                strcat(item_events, buf);
                                snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %d, 0); \n", command->operands, mask_mode, trigger->once);
                                strcat(anti_events, buf);
                            }
                            else
                            {
                                snprintf(buf, 128, "   activateEntity(%d, entity_index, 0x%02X, %d, %d, %d); \n", command->operands, trigger->mask, mask_mode, trigger->once, trigger->timer);
                                strcat(item_events, buf);
                                snprintf(buf, 128, "   deactivateEntity(%d, entity_index); \n", command->operands);
                                strcat(anti_events, buf);
                            }
                        }
                    }
                    argn++;
                    break;

                case TR_FD_TRIGFUNC_CAMERATARGET:
                    {
                        snprintf(buf, 128, "   setCamera(%d, %d, %d, %d); \n", command->cam_index, command->cam_timer, command->cam_once, command->cam_zoom);
                        strcat(single_events, buf);
                    }
                    break;

                case TR_FD_TRIGFUNC_UWCURRENT:
                    snprintf(buf, 128, "   moveToSink(entity_index, %d); \n", command->operands);
                    strcat(cont_events, buf);
                    break;

                case TR_FD_TRIGFUNC_FLIPMAP:
                    // FLIPMAP trigger acts two-way for switch cases, so we add FLIPMAP off event to
                    // anti-events array.
                    if(activator == TR_ACTIVATOR_SWITCH)
                    {
                        snprintf(buf, 128, "   setFlipMap(%d, switch_mask, 1); \n   setFlipState(%d, 1); \n", command->operands, command->operands);
                        strcat(single_events, buf);
                    }
                    else
                    {
                        snprintf(buf, 128, "   setFlipMap(%d, 0x%02X, 0); \n   setFlipState(%d, 1); \n", command->operands, trigger->mask, command->operands);
                        strcat(single_events, buf);
                    }
                    break;

                case TR_FD_TRIGFUNC_FLIPON:
                    // FLIP_ON trigger acts one-way even in switch cases, i.e. if you un-pull
                    // the switch with FLIP_ON trigger, room will remain flipped.
                    snprintf(buf, 128, "   setFlipState(%d, 1); \n", command->operands);
                    strcat(single_events, buf);
                    break;

                case TR_FD_TRIGFUNC_FLIPOFF:
                    // FLIP_OFF trigger acts one-way even in switch cases, i.e. if you un-pull
                    // the switch with FLIP_OFF trigger, room will remain unflipped.
                    snprintf(buf, 128, "   setFlipState(%d, 0); \n", command->operands);
                    strcat(single_events, buf);
                    break;

                case TR_FD_TRIGFUNC_LOOKAT:
                    snprintf(buf, 128, "   setCamTarget(%d, %d); \n", command->operands, trigger->timer);
                    strcat(single_events, buf);
                    break;

                case TR_FD_TRIGFUNC_ENDLEVEL:
                    snprintf(buf, 128, "   setLevel(%d); \n", command->operands);
                    strcat(single_events, buf);
                    break;

                case TR_FD_TRIGFUNC_PLAYTRACK:
                    snprintf(buf, 128, "   playStream(%d, 0x%02X); \n", command->operands, ((uint16_t)trigger->mask << 1) + trigger->once);
                    strcat(single_events, buf);
                    break;

                case TR_FD_TRIGFUNC_FLIPEFFECT:
                    snprintf(buf, 128, "   doEffect(%d, %d); \n", command->operands, trigger->timer);
                    strcat(cont_events, buf);
                    break;

                case TR_FD_TRIGFUNC_SECRET:
                    snprintf(buf, 128, "   findSecret(%d); \n", command->operands);
                    strcat(single_events, buf);
                    break;

                case TR_FD_TRIGFUNC_CLEARBODIES:
                    snprintf(buf, 128, "   clearBodies(); \n");
                    strcat(single_events, buf);
                    break;

                case TR_FD_TRIGFUNC_FLYBY:
                    {
                        snprintf(buf, 128, "   playFlyby(%d, %d); \n", command->operands, command->once);
                        strcat(cont_events, buf);
                    }
                    break;

                case TR_FD_TRIGFUNC_CUTSCENE:
                    snprintf(buf, 128, "   playCutscene(%d); \n", command->operands);
                    strcat(single_events, buf);
                    break;

                default: // UNKNOWN!
                    break;
            };
        }

        if(script[0])
        {
            strcat(script, header);

            // Heavy trigger and antitrigger item events are engaged ONLY
            // once, when triggering item is approaching sector. Hence, we
            // copy item events to single events and nullify original item
            // events sequence to prevent it to be merged into continous
            // events.

            if((trigger->sub_function == TR_FD_TRIGTYPE_HEAVY) ||
               (trigger->sub_function == TR_FD_TRIGTYPE_HEAVYANTITRIGGER))
            {
                if(action_type == TR_ACTIONTYPE_ANTI)
                {
                    strcat(single_events, anti_events);
                }
                else
                {
                    strcat(single_events, item_events);
                }

                anti_events[0] = 0;
                item_events[0] = 0;
            }

            if(activator == TR_ACTIVATOR_NORMAL)    // Ordinary trigger cases.
            {
                if(single_events[0])
                {
                    if(condition) strcat(once_condition, " ");
                    strcat(once_condition, " if(getEntitySectorStatus(entity_index) == 0) then \n");
                    strcat(script, once_condition);
                    strcat(script, single_events);
                    strcat(script, "   setEntitySectorStatus(entity_index, 1); \n");

                    if(condition)
                    {
                        strcat(script, "  end;\n"); // First ENDIF is tabbed for extra condition.
                    }
                    else
                    {
                        strcat(script, " end;\n");
                    }
                }

                // Item commands kind depends on action type. If type is ANTI, then item
                // antitriggering is engaged. If type is normal, ordinary triggering happens
                // in cycle with other continous commands. It is needed to prevent timer dispatch
                // before activator leaves trigger sector.

                if(action_type == TR_ACTIONTYPE_ANTI)
                {
                    strcat(script, anti_events);
                }
                else
                {
                    strcat(script, item_events);
                }

                strcat(script, cont_events);
                if(condition) strcat(script, " end;\n"); // Additional ENDIF for extra condition.
            }
            else    // SWITCH, KEY and ITEM cases.
            {
                strcat(script, single_events);
                strcat(script, item_events);
                strcat(script, cont_events);
                if((action_type == TR_ACTIONTYPE_SWITCH) && (activator == TR_ACTIVATOR_SWITCH))
                {
                    strcat(script, buf2);
                    if(!trigger->once)
                    {
                        strcat(script, single_events);
                        strcat(script, anti_events);    // Single/continous events are engaged along with
                        strcat(script, cont_events);    // antitriggered items, as described above.
                    }
                }
                strcat(script, " end;\n");
            }

            strcat(script, "return 1;\nend }\n");  // Finalize the entry.
        }

        if(action_type != TR_ACTIONTYPE_BYPASS)
        {
            // Sys_DebugLog("triggers.lua", script);    // Debug!
            luaL_loadstring(engine_lua, script);
            lua_CallAndLog(engine_lua, 0, LUA_MULTRET, 0); // Execute compiled script.
        }
    }
}