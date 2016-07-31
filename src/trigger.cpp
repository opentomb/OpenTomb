
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_rwops.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/system.h"
#include "core/console.h"
#include "core/vmath.h"
#include "room.h"
#include "trigger.h"
#include "script.h"
#include "gameflow.h"
#include "game.h"
#include "audio.h"
#include "skeletal_model.h"
#include "entity.h"
#include "character_controller.h"
#include "anim_state_control.h"
#include "world.h"


inline uint32_t Entity_GetSectorStatus(entity_p ent)
{
    return (ent) ? ((ent->trigger_layout & ENTITY_TLAYOUT_SSTATUS) >> 7) : (0);
}


inline void Entity_SetSectorStatus(entity_p ent, uint16_t status)
{
    if(ent)
    {
        ent->trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_SSTATUS);
        ent->trigger_layout ^=  ((uint8_t)status) << 7;   // sector_status  - 10000000
    }
}


inline uint32_t Entity_GetLock(entity_p ent)
{
    return (ent && (ent->trigger_layout & ENTITY_TLAYOUT_LOCK) >> 6);      // lock
}


inline void Entity_SetLock(entity_p ent, uint16_t status)
{
    if(ent)
    {
        ent->trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);
        ent->trigger_layout ^= ((uint8_t)status) << 6;   // lock  - 01000000
    }
}


inline uint32_t Entity_GetLayoutEvent(entity_p ent)
{
    return (ent && (ent->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5);
}


inline void Entity_SetLayoutEvent(entity_p ent, uint16_t status)
{
    if(ent)
    {
        ent->trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT);
        ent->trigger_layout ^= ((uint8_t)status) << 5;   // event  - 00100000
    }
}

/*
-- Does specified flipeffect.

function doEffect(effect_index, extra_parameter) -- extra parameter is usually the timer field
    if(flipeffects[effect_index] ~= nil) then
        return flipeffects[effect_index](parameter);
    else
        return nil; -- Add hardcoded flipeffect routine here
    end;
end


-- Clear dead enemies, if they have CLEAR BODY flag specified.
function clearBodies()
    print("CLEAR BODIES");
end


-- Plays specified cutscene. Only valid in retail TR4-5.
function playCutscene(cutscene_index)
    if(getLevelVersion() < TR_IV) then return 0 end;
    print("CUTSCENE: index = " .. cutscene_index);
end
 */

///@TODO: move here TickEntity with Inversing entity state... see carefully heavy irregular cases
void Trigger_DoCommands(trigger_header_p trigger, struct entity_s *entity_activator)
{
    if(trigger && entity_activator)
    {
        bool has_non_continuos_triggers = false;

        for(trigger_command_p command = trigger->commands; command; command = command->next)
        {
            switch(command->function)
            {
                case TR_FD_TRIGFUNC_UWCURRENT:
                    if(entity_activator->move_type == MOVE_ON_WATER)
                    {
                        if(entity_activator->bf->animations.current_animation != TR_ANIMATION_LARA_ONWATER_DIVE_ALTERNATE)
                        {
                            Entity_SetAnimation(entity_activator, ANIM_TYPE_BASE, TR_ANIMATION_LARA_ONWATER_DIVE_ALTERNATE, 0);
                            entity_activator->move_type = MOVE_UNDERWATER;
                        }
                    }
                    else if(entity_activator->move_type == MOVE_UNDERWATER)
                    {
                        Entity_MoveToSink(entity_activator, command->operands);
                    }
                    break;

                default:
                    has_non_continuos_triggers = true;
                    break;
            };
        }

        if(has_non_continuos_triggers)
        {
            int activator           = TR_ACTIVATOR_NORMAL;      // Activator is normal by default.
            int action_type         = TR_ACTIONTYPE_NORMAL;     // Action type is normal by default.
            int mask_mode           = TRIGGER_OP_OR;            // Activation mask by default.
            int activator_sector_status = Entity_GetSectorStatus(entity_activator);
            bool header_condition   = true;

            // Activator type is LARA for all triggers except HEAVY ones, which are triggered by
            // some specific entity classes.
            // entity_activator_type  == TR_ACTIVATORTYPE_LARA and
            // trigger_activator_type == TR_ACTIVATORTYPE_MISC
            switch(trigger->sub_function)
            {
                case TR_FD_TRIGTYPE_HEAVY:
                case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
                case TR_FD_TRIGTYPE_HEAVYSWITCH:
                    if((entity_activator->type_flags & ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR) == 0)
                    {
                        return;
                    }
                    break;
            }

            switch(trigger->sub_function)
            {
                case TR_FD_TRIGTYPE_TRIGGER:
                case TR_FD_TRIGTYPE_HEAVY:
                    activator = TR_ACTIVATOR_NORMAL;
                    break;

                case TR_FD_TRIGTYPE_ANTIPAD:
                    action_type = TR_ACTIONTYPE_ANTI;
                case TR_FD_TRIGTYPE_PAD:
                    // Check move type for triggering entity.
                    header_condition = (entity_activator->move_type == MOVE_ON_FLOOR);
                    break;

                case TR_FD_TRIGTYPE_SWITCH:
                    // Set activator and action type for now; conditions are linked with first item in operand chain.
                    activator = TR_ACTIVATOR_SWITCH;
                    action_type = TR_ACTIONTYPE_SWITCH;
                    mask_mode = TRIGGER_OP_XOR;
                    break;

                case TR_FD_TRIGTYPE_HEAVYSWITCH:
                    // Action type remains normal, as HEAVYSWITCH acts as "heavy trigger" with activator mask filter.
                    activator = TR_ACTIVATOR_SWITCH;
                    mask_mode = TRIGGER_OP_XOR;
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
                    header_condition = header_condition && (entity_activator->character && (entity_activator->character->weapon_current_state > 0));
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
                    header_condition = header_condition && (entity_activator->move_type == MOVE_MONKEYSWING);
                    break;

                case TR_FD_TRIGTYPE_CLIMB:
                    header_condition = header_condition && (entity_activator->move_type == MOVE_CLIMBING);
                    break;

                case TR_FD_TRIGTYPE_TIGHTROPE:
                    // Check state range for triggering entity.
                    header_condition = header_condition && ((entity_activator->state_flags >= TR_STATE_LARA_TIGHTROPE_IDLE) && (entity_activator->state_flags <= TR_STATE_LARA_TIGHTROPE_EXIT));
                    break;

                case TR_FD_TRIGTYPE_CRAWLDUCK:
                    // Check state range for triggering entity.
                    header_condition = header_condition && ((entity_activator->state_flags >= TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_BEGIN) && (entity_activator->state_flags <= TR_ANIMATION_LARA_CRAWL_SMASH_LEFT));
                    break;
            }

            if(!header_condition)
            {
                return;
            }

            // Now execute operand chain for trigger function!
            bool first_command = true;
            int switch_sectorstatus = 0;
            int switch_event_state = -1;
            uint32_t switch_mask = 0;
            entity_p trig_entity = NULL;
            entity_p switch_entity = NULL;
            for(trigger_command_p command = trigger->commands; command; command = command->next)
            {
                switch(command->function)
                {
                    case TR_FD_TRIGFUNC_OBJECT:         // ACTIVATE / DEACTIVATE object
                        trig_entity = World_GetEntityByID(command->operands);
                        // If activator is specified, first item operand counts as activator index (except
                        // heavy switch case, which is ordinary heavy trigger case with certain differences).
                        if(!trig_entity)
                        {
                            break;
                        }

                        if(first_command && (activator != TR_ACTIVATOR_NORMAL))
                        {
                            first_command = false;
                            switch(activator)
                            {
                                case TR_ACTIVATOR_SWITCH:
                                    if(action_type == TR_ACTIONTYPE_SWITCH)
                                    {
                                        // Switch action type case.
                                        switch_entity = trig_entity;
                                        switch_event_state = (trig_entity->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5;
                                        switch_sectorstatus = (trig_entity->trigger_layout & ENTITY_TLAYOUT_SSTATUS) >> 7;
                                        switch_mask = (trig_entity->trigger_layout & ENTITY_TLAYOUT_MASK);
                                        // Trigger activation mask is here filtered through activator's own mask.
                                        switch_mask = (switch_mask == 0) ? (0x1F & trigger->mask) : (switch_mask & trigger->mask);

                                        if((switch_event_state == 0) && (switch_sectorstatus == 1))
                                        {
                                            Entity_SetSectorStatus(trig_entity, 0);
                                            trig_entity->timer = trigger->timer;
                                        }
                                        else if((switch_event_state == 1) && (switch_sectorstatus == 1))
                                        {
                                            // Create statement for antitriggering a switch.
                                            Entity_SetSectorStatus(trig_entity, 0);
                                            trig_entity->timer = 0.0f;
                                        }
                                        else
                                        {
                                            return;
                                        }
                                    }
                                    else    /// end if (action_type == TR_ACTIONTYPE_SWITCH)
                                    {
                                        // Ordinary type case (e.g. heavy switch).
                                        switch_sectorstatus = (trig_entity->trigger_layout & ENTITY_TLAYOUT_SSTATUS) >> 7;  ///@CHECK: vas entity_activator instead trig_entity
                                        switch_mask = (trig_entity->trigger_layout & ENTITY_TLAYOUT_MASK);                  ///@CHECK: vas entity_activator instead trig_entity
                                        // Trigger activation mask is here filtered through activator's own mask.
                                        switch_mask = (switch_mask == 0) ? (0x1F & trigger->mask) : (switch_mask & trigger->mask);

                                        if(switch_sectorstatus == 0)
                                        {
                                            int activation_state = Entity_Activate(trig_entity, entity_activator, switch_mask, mask_mode, trigger->once, trigger->timer);
                                            if(trigger->once && (activation_state != ENTITY_TRIGGERING_NOT_READY))
                                            {
                                                Entity_SetSectorStatus(entity_activator, 1);
                                                switch_sectorstatus = 1;
                                            }
                                        }
                                        else
                                        {
                                            return;
                                        }
                                    }
                                    break;

                                case TR_ACTIVATOR_KEY:
                                    if((Entity_GetLock(trig_entity) == 1) && (Entity_GetSectorStatus(trig_entity) == 0))
                                    {
                                        Entity_SetSectorStatus(trig_entity, 1);
                                    }
                                    else
                                    {
                                        return;
                                    }
                                    break;

                                case TR_ACTIVATOR_PICKUP:
                                    if(!(trig_entity->state_flags & ENTITY_STATE_ENABLED) && (Entity_GetSectorStatus(trig_entity) == 0))
                                    {
                                        Entity_SetSectorStatus(trig_entity, 1);
                                    }
                                    else
                                    {
                                        return;
                                    }
                                    break;
                            };
                        }
                        else
                        {
                            int activation_state = ENTITY_TRIGGERING_NOT_READY;
                            if(activator == TR_ACTIVATOR_SWITCH)
                            {
                                if(action_type == TR_ACTIONTYPE_ANTI)
                                {
                                    activation_state = Entity_Activate(trig_entity, entity_activator, switch_mask, mask_mode, trigger->once, 0.0f);
                                }
                                else// if(Entity_GetLayoutEvent(trig_entity) != switch_event_state)
                                {
                                    activation_state = Entity_Activate(trig_entity, entity_activator, switch_mask, mask_mode, trigger->once, trigger->timer);
                                }
                            }
                            else
                            {
                                if(action_type == TR_ACTIONTYPE_ANTI)
                                {
                                    activation_state = Entity_Deactivate(trig_entity, entity_activator);
                                }
                                else if((activator_sector_status == 0) || (trigger->timer > 0))
                                {
                                    activation_state = Entity_Activate(trig_entity, entity_activator, trigger->mask, mask_mode, trigger->once, trigger->timer);
                                }
                            }
                        }
                        break;

                    case TR_FD_TRIGFUNC_FLIPMAP:
                        // FLIPMAP trigger acts two-way for switch cases, so we add FLIPMAP off event to
                        // anti-events array.
                        if((activator_sector_status == 0) || (activator == TR_ACTIVATOR_SWITCH))
                        {
                            if(activator == TR_ACTIVATOR_SWITCH)
                            {
                                World_SetFlipMap(command->operands, switch_mask, TRIGGER_OP_XOR);
                                World_SetFlipState(command->operands, FLIP_STATE_BY_FLAG);
                            }
                            else
                            {
                                World_SetFlipMap(command->operands, trigger->mask, TRIGGER_OP_OR);
                                World_SetFlipState(command->operands, FLIP_STATE_BY_FLAG);
                            }
                        }
                        break;

                    case TR_FD_TRIGFUNC_FLIPON:
                        if((activator_sector_status == 0) || (activator == TR_ACTIVATOR_SWITCH))
                        {
                            // FLIP_ON trigger acts one-way even in switch cases, i.e. if you un-pull
                            // the switch with FLIP_ON trigger, room will remain flipped.
                            World_SetFlipState(command->operands, FLIP_STATE_ON);
                        }
                        break;

                    case TR_FD_TRIGFUNC_FLIPOFF:
                        if((activator_sector_status == 0) || (activator == TR_ACTIVATOR_SWITCH))
                        {
                            // FLIP_OFF trigger acts one-way even in switch cases, i.e. if you un-pull
                            // the switch with FLIP_OFF trigger, room will remain unflipped.
                            World_SetFlipState(command->operands, FLIP_STATE_OFF);
                        }
                        break;

                    case TR_FD_TRIGFUNC_SET_TARGET:
                        if((activator_sector_status == 0) || (activator == TR_ACTIVATOR_SWITCH))
                        {
                            Game_SetCameraTarget(command->operands, trigger->timer);
                        }
                        break;

                    case TR_FD_TRIGFUNC_SET_CAMERA:
                        Game_SetCamera(command->cam_index, command->once, command->cam_move, command->cam_timer);
                        break;

                    case TR_FD_TRIGFUNC_FLYBY:
                        if((activator_sector_status == 0) || (activator == TR_ACTIVATOR_SWITCH))
                        {
                            Game_PlayFlyBy(command->operands, command->once);
                        }
                        break;

                    case TR_FD_TRIGFUNC_CUTSCENE:
                        if((activator_sector_status == 0) || (activator == TR_ACTIVATOR_SWITCH))
                        {
                            ///snprintf(buf, 128, "   playCutscene(%d); \n", command->operands);
                        }
                        break;

                    case TR_FD_TRIGFUNC_ENDLEVEL:
                        Con_Notify("level was changed to %d", command->operands);
                        Game_LevelTransition(command->operands);
                        Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, command->operands);
                        break;

                    case TR_FD_TRIGFUNC_PLAYTRACK:
                        if((activator_sector_status == 0) || (activator == TR_ACTIVATOR_SWITCH))
                        {
                            Audio_StreamPlay(command->operands, (trigger->mask << 1) + trigger->once);
                        }
                        break;

                    case TR_FD_TRIGFUNC_FLIPEFFECT:
                        if((activator_sector_status == 0) || (activator == TR_ACTIVATOR_SWITCH))
                        {
                            //snprintf(buf, 128, "   doEffect(%d, %d); \n", command->operands, trigger->timer);
                        }
                        break;

                    case TR_FD_TRIGFUNC_SECRET:
                        if((command->operands < TR_GAMEFLOW_MAX_SECRETS) && (gameflow_manager.SecretsTriggerMap[command->operands] == 0))
                        {
                            gameflow_manager.SecretsTriggerMap[command->operands] = 1;
                            Audio_StreamPlay(Script_GetSecretTrackNumber(engine_lua));
                        }
                        break;

                    case TR_FD_TRIGFUNC_CLEARBODIES:
                        if(activator_sector_status == 0)
                        {
                            //snprintf(buf, 128, "   clearBodies(); \n");
                        }
                        break;

                    case TR_FD_TRIGFUNC_UWCURRENT:
                        // implemented in continuous section
                        break;

                    default:
                        if(activator_sector_status == 0)
                        {
                            Con_Printf("Unknown trigger function: 0x%X", command->function);
                        }
                        break;
                };
            }

            Entity_SetSectorStatus(entity_activator, 1);
        }
    }
}


void Trigger_BuildScripts(trigger_header_p trigger, uint32_t trigger_index, const char *file_name)
{
    if(trigger && file_name)
    {
        SDL_RWops *file_dump = SDL_RWFromFile(file_name, "a");
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
        int mask_mode   = TRIGGER_OP_XOR;           // Activation mask by default.

        // Activator type is LARA for all triggers except HEAVY ones, which are triggered by
        // some specific entity classes.

        int activator_type = ( (trigger->sub_function == TR_FD_TRIGTYPE_HEAVY)            ||
                               (trigger->sub_function == TR_FD_TRIGTYPE_HEAVYANTITRIGGER) ||
                               (trigger->sub_function == TR_FD_TRIGTYPE_HEAVYSWITCH) ) ? TR_ACTIVATORTYPE_MISC : TR_ACTIVATORTYPE_LARA;

        // Table cell header.

        snprintf(buf, 256, "trigger_list[%d] = {activator_type = %d, func = function(entity_index) \n", trigger_index, activator_type);

        strncat(script, buf, 8192);
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
                mask_mode = TRIGGER_OP_XOR;
                break;

            case TR_FD_TRIGTYPE_HEAVYSWITCH:
                // Action type remains normal, as HEAVYSWITCH acts as "heavy trigger" with activator mask filter.
                activator = TR_ACTIVATOR_SWITCH;
                mask_mode = TRIGGER_OP_XOR;
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
                snprintf(buf, 128, " if(getEntityMoveType(entity_index) == %d) then \n", (trigger->sub_function == TR_FD_TRIGTYPE_MONKEY) ? MOVE_MONKEYSWING : MOVE_CLIMBING);
                condition = 1;  // Set additional condition.
                break;

            case TR_FD_TRIGTYPE_TIGHTROPE:
                // Check state range for triggering entity.
                snprintf(buf, 128, " local state = getEntityAnimState(entity_index, ANIM_TYPE_BASE) \n if((state >= %d) and (state <= %d)) then \n", TR_STATE_LARA_TIGHTROPE_IDLE, TR_STATE_LARA_TIGHTROPE_EXIT);
                condition = 1;  // Set additional condition.
                break;
            case TR_FD_TRIGTYPE_CRAWLDUCK:
                // Check state range for triggering entity.
                snprintf(buf, 128, " local state = getEntityAnimState(entity_index, ANIM_TYPE_BASE) \n if((state >= %d) and (state <= %d)) then \n", TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_BEGIN, TR_ANIMATION_LARA_CRAWL_SMASH_LEFT);
                condition = 1;  // Set additional condition.
                break;
        }

        strncat(header, buf, 128);    // Add condition to header.

        // Now parse operand chain for trigger function!
        int argn = 0;
        trigger_command_p prev_command = NULL;
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
                                    snprintf(buf, 256, " local switch_state = getEntityAnimState(%d, ANIM_TYPE_BASE); \n local switch_sectorstatus = getEntitySectorStatus(%d); \n local switch_mask = getEntityMask(%d); \n\n", command->operands, command->operands, command->operands);
                                }
                                else
                                {
                                    // Ordinary type case (e.g. heavy switch).
                                    snprintf(buf, 256, " local switch_sectorstatus = getEntitySectorStatus(entity_index); \n local switch_mask = getEntityMask(entity_index); \n\n");
                                }
                                strncat(script, buf, 8192);

                                // Trigger activation mask is here filtered through activator's own mask.
                                snprintf(buf, 256, " if(switch_mask == 0) then switch_mask = 0x1F end; \n switch_mask = bit32.band(switch_mask, 0x%02X); \n\n", trigger->mask);
                                strncat(script, buf, 8192);
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
                                    strncat(item_events, buf, 4096);
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

                        strncat(script, buf, 8192);
                    }
                    else if(!prev_command || (prev_command->operands != command->operands))
                    {
                        // In many original Core Design levels, level designers left dublicated entity activation operands.
                        // This results in setting same activation mask twice, effectively blocking entity from activation.
                        // To prevent this, we compare previous item_id and current item_id.
                        // Other item operands are simply parsed as activation functions. Switch case is special, because
                        // function is fed with activation mask argument derived from activator mask filter (switch_mask),
                        // and also we need to process deactivation in a same way as activation, excluding resetting timer
                        // field. This is needed for two-way switch combinations (e.g. Palace Midas).
                        if(activator == TR_ACTIVATOR_SWITCH)
                        {
                            snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %d, %d); \n", command->operands, mask_mode, trigger->once, trigger->timer);
                            strncat(item_events, buf, 4096);
                            snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %d, 0); \n", command->operands, mask_mode, trigger->once);
                            strncat(anti_events, buf, 4096);
                        }
                        else
                        {
                            snprintf(buf, 128, "   activateEntity(%d, entity_index, 0x%02X, %d, %d, %d); \n", command->operands, trigger->mask, mask_mode, trigger->once, trigger->timer);
                            strncat(item_events, buf, 4096);
                            snprintf(buf, 128, "   deactivateEntity(%d, entity_index); \n", command->operands);
                            strncat(anti_events, buf, 4096);
                        }
                    }
                    argn++;
                    break;

                case TR_FD_TRIGFUNC_SET_CAMERA:
                    {
                        snprintf(buf, 128, "   setCamera(%d, %d, %d, %d); \n", command->cam_index, command->cam_timer, command->once, command->cam_move);
                        strncat(single_events, buf, 4096);
                    }
                    break;

                case TR_FD_TRIGFUNC_UWCURRENT:
                    snprintf(buf, 128, "   moveToSink(entity_index, %d); \n", command->operands);
                    strncat(cont_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_FLIPMAP:
                    // FLIPMAP trigger acts two-way for switch cases, so we add FLIPMAP off event to
                    // anti-events array.
                    if(activator == TR_ACTIVATOR_SWITCH)
                    {
                        snprintf(buf, 128, "   setFlipMap(%d, switch_mask, 1); \n   setFlipState(%d, 1); \n", command->operands, command->operands);
                        strncat(single_events, buf, 4096);
                    }
                    else
                    {
                        snprintf(buf, 128, "   setFlipMap(%d, 0x%02X, 0); \n   setFlipState(%d, 1); \n", command->operands, trigger->mask, command->operands);
                        strncat(single_events, buf, 4096);
                    }
                    break;

                case TR_FD_TRIGFUNC_FLIPON:
                    // FLIP_ON trigger acts one-way even in switch cases, i.e. if you un-pull
                    // the switch with FLIP_ON trigger, room will remain flipped.
                    snprintf(buf, 128, "   setFlipState(%d, 1); \n", command->operands);
                    strncat(single_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_FLIPOFF:
                    // FLIP_OFF trigger acts one-way even in switch cases, i.e. if you un-pull
                    // the switch with FLIP_OFF trigger, room will remain unflipped.
                    snprintf(buf, 128, "   setFlipState(%d, 0); \n", command->operands);
                    strncat(single_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_SET_TARGET:
                    snprintf(buf, 128, "   setCamTarget(%d, %d); \n", command->operands, trigger->timer);
                    strncat(single_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_ENDLEVEL:
                    snprintf(buf, 128, "   setLevel(%d); \n", command->operands);
                    strncat(single_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_PLAYTRACK:
                    snprintf(buf, 128, "   playStream(%d, 0x%02X); \n", command->operands, ((uint16_t)trigger->mask << 1) + trigger->once);
                    strncat(single_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_FLIPEFFECT:
                    snprintf(buf, 128, "   doEffect(%d, %d); \n", command->operands, trigger->timer);
                    strncat(cont_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_SECRET:
                    snprintf(buf, 128, "   findSecret(%d); \n", command->operands);
                    strncat(single_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_CLEARBODIES:
                    snprintf(buf, 128, "   clearBodies(); \n");
                    strncat(single_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_FLYBY:
                    snprintf(buf, 128, "   playFlyby(%d, %d); \n", command->operands, command->once);
                    strncat(cont_events, buf, 4096);
                    break;

                case TR_FD_TRIGFUNC_CUTSCENE:
                    snprintf(buf, 128, "   playCutscene(%d); \n", command->operands);
                    strncat(single_events, buf, 4096);
                    break;

                default: // UNKNOWN!
                    break;
            };
            prev_command = command;
        }

        if(script[0])
        {
            strncat(script, header, 8192);

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
                    strncat(single_events, anti_events, 4096);
                }
                else
                {
                    strncat(single_events, item_events, 4096);
                }

                anti_events[0] = 0;
                item_events[0] = 0;
            }

            if(activator == TR_ACTIVATOR_NORMAL)    // Ordinary trigger cases.
            {
                if(single_events[0])
                {
                    if(condition) strncat(once_condition, " ", 128);
                    strncat(once_condition, " if(getEntitySectorStatus(entity_index) == 0) then \n", 128);
                    strncat(script, once_condition, 8192);
                    strncat(script, single_events, 8192);
                    strncat(script, "   setEntitySectorStatus(entity_index, 1); \n", 8192);

                    if(condition)
                    {
                        strncat(script, "  end;\n", 8192); // First ENDIF is tabbed for extra condition.
                    }
                    else
                    {
                        strncat(script, " end;\n", 8192);
                    }
                }

                // Item commands kind depends on action type. If type is ANTI, then item
                // antitriggering is engaged. If type is normal, ordinary triggering happens
                // in cycle with other continous commands. It is needed to prevent timer dispatch
                // before activator leaves trigger sector.

                if(action_type == TR_ACTIONTYPE_ANTI)
                {
                    strncat(script, anti_events, 8192);
                }
                else
                {
                    strncat(script, item_events, 8192);
                }

                strncat(script, cont_events, 8192);
                if(condition) strncat(script, " end;\n", 8192); // Additional ENDIF for extra condition.
            }
            else    // SWITCH, KEY and ITEM cases.
            {
                strncat(script, single_events, 8192);
                strncat(script, item_events, 8192);
                strncat(script, cont_events, 8192);
                if((action_type == TR_ACTIONTYPE_SWITCH) && (activator == TR_ACTIVATOR_SWITCH))
                {
                    strncat(script, buf2, 8192);
                    if(!trigger->once)
                    {
                        strncat(script, single_events, 8192);
                        strncat(script, anti_events, 8192);    // Single/continous events are engaged along with
                        strncat(script, cont_events, 8192);    // antitriggered items, as described above.
                    }
                }
                strncat(script, " end;\n", 8192);
            }

            strncat(script, "return 1;\nend }\n\n", 8192);  // Finalize the entry.
        }

        if(file_dump)
        {
            if(action_type != TR_ACTIONTYPE_BYPASS)
            {
                SDL_RWwrite(file_dump, script, 1, strlen(script));
            }
            SDL_RWclose(file_dump);
        }
    }
}


void Trigger_TrigMaskToStr(char buf[9], uint8_t flag)
{
    for(int i = 7; i >= 0; --i)
    {
        buf[i] = ((flag & 0x01) != 0) ? ('1') : ('0');
        flag >>= 1;
    }
    buf[8] = 0;
}


void Trigger_TrigTypeToStr(char *buf, uint32_t size, uint32_t func)
{
    buf[0] = 0;
    switch(func)
    {
        case TR_FD_TRIGTYPE_TRIGGER:
            strncpy(buf, "TRIGGER", size);
            break;

        case TR_FD_TRIGTYPE_PAD:
            strncpy(buf, "PAD", size);
            break;

        case TR_FD_TRIGTYPE_SWITCH:
            strncpy(buf, "SWITCH", size);
            break;

        case TR_FD_TRIGTYPE_KEY:
            strncpy(buf, "KEY", size);
            break;

        case TR_FD_TRIGTYPE_PICKUP:
            strncpy(buf, "PICKUP", size);
            break;

        case TR_FD_TRIGTYPE_HEAVY:
            strncpy(buf, "HEAVY", size);
            break;

        case TR_FD_TRIGTYPE_ANTIPAD:
            strncpy(buf, "ANTIPAD", size);
            break;

        case TR_FD_TRIGTYPE_COMBAT:
            strncpy(buf, "COMBAT", size);
            break;

        case TR_FD_TRIGTYPE_DUMMY:
            strncpy(buf, "DUMMY", size);
            break;

        case TR_FD_TRIGTYPE_ANTITRIGGER:
            strncpy(buf, "ANTITRIGGER", size);
            break;

        case TR_FD_TRIGTYPE_HEAVYSWITCH:
            strncpy(buf, "HEAVYSWITCH", size);
            break;

        case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
            strncpy(buf, "HEAVYANTITRIGGER", size);
            break;

        case TR_FD_TRIGTYPE_MONKEY:
            strncpy(buf, "MONKEY", size);
            break;

        case TR_FD_TRIGTYPE_SKELETON:
            strncpy(buf, "SKELETON", size);
            break;

        case TR_FD_TRIGTYPE_TIGHTROPE:
            strncpy(buf, "TIGHTROPE", size);
            break;

        case TR_FD_TRIGTYPE_CRAWLDUCK:
            strncpy(buf, "CRAWLDUCK", size);
            break;

        case TR_FD_TRIGTYPE_CLIMB:
            strncpy(buf, "CLIMB", size);
            break;
    };
}


void Trigger_TrigCmdToStr(char *buf, uint32_t size, uint32_t func)
{
    buf[0] = 0;
    switch(func)
    {
        case TR_FD_TRIGFUNC_OBJECT:
            strncpy(buf, "OBJECT", size);
            break;

        case TR_FD_TRIGFUNC_SET_CAMERA:
            strncpy(buf, "SET_CAMERA", size);
            break;

        case TR_FD_TRIGFUNC_UWCURRENT:
            strncpy(buf, "UWCURRENT", size);
            break;

        case TR_FD_TRIGFUNC_FLIPMAP:
            strncpy(buf, "FLIPMAP", size);
            break;

        case TR_FD_TRIGFUNC_FLIPON:
            strncpy(buf, "FLIPON", size);
            break;

        case TR_FD_TRIGFUNC_FLIPOFF:
            strncpy(buf, "FLIPOFF", size);
            break;

        case TR_FD_TRIGFUNC_SET_TARGET:
            strncpy(buf, "SET_TARGET", size);
            break;

        case TR_FD_TRIGFUNC_ENDLEVEL:
            strncpy(buf, "ENDLEVEL", size);
            break;

        case TR_FD_TRIGFUNC_PLAYTRACK:
            strncpy(buf, "PLAYTRACK", size);
            break;

        case TR_FD_TRIGFUNC_FLIPEFFECT:
            strncpy(buf, "FLIPEFFECT", size);
            break;

        case TR_FD_TRIGFUNC_SECRET:
            strncpy(buf, "SECRET", size);
            break;

        case TR_FD_TRIGFUNC_CLEARBODIES:
            strncpy(buf, "CLEARBODIES", size);
            break;

        case TR_FD_TRIGFUNC_FLYBY:
            strncpy(buf, "FLYBY", size);
            break;

        case TR_FD_TRIGFUNC_CUTSCENE:
            strncpy(buf, "CUTSCENE", size);
            break;

        default:
            snprintf(buf, size, "0x%X", func);
            break;
    };
}
