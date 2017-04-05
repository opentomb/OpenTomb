
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER///@GH0ST
#include <SDL.h>
#include <SDL_rwops.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_rwops.h>
#endif

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/system.h"
#include "core/console.h"
#include "core/vmath.h"
#include "script/script.h"
#include "render/camera.h"
#include "room.h"
#include "trigger.h"
#include "gameflow.h"
#include "game.h"
#include "audio.h"
#include "skeletal_model.h"
#include "entity.h"
#include "character_controller.h"
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
    if(entity_activator && entity_activator->character)
    {
        entity_activator->character->state.uw_current = 0x00;
    }
    if(trigger && entity_activator)
    {
        bool has_non_continuos_triggers = false;

        for(trigger_command_p command = trigger->commands; command; command = command->next)
        {
            switch(command->function)
            {
                case TR_FD_TRIGFUNC_UWCURRENT:
                    if(entity_activator->character)
                    {
                        static_camera_sink_p sink = World_GetstaticCameraSink(command->operands);
                        if(sink && (entity_activator->current_sector != Room_GetSectorRaw(entity_activator->self->room, sink->pos)))
                        {
                            if(entity_activator->move_type == MOVE_UNDERWATER)
                            {
                                Entity_MoveToSink(entity_activator, sink);
                            }
                            entity_activator->character->state.uw_current = 0x01;
                        }
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
            bool is_heavy           = false;
            // Activator type is LARA for all triggers except HEAVY ones, which are triggered by
            // some specific entity classes.
            // entity_activator_type  == TR_ACTIVATORTYPE_LARA and
            // trigger_activator_type == TR_ACTIVATORTYPE_MISC
            switch(trigger->sub_function)
            {
                case TR_FD_TRIGTYPE_HEAVY:
                case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
                case TR_FD_TRIGTYPE_HEAVYSWITCH:
                    is_heavy = true;
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
                    mask_mode = TRIGGER_OP_AND_INV;
                case TR_FD_TRIGTYPE_PAD:
                    // Check move type for triggering entity.
                    {
                        room_sector_p lowest_sector  = Sector_GetLowest(entity_activator->current_sector);
                        header_condition = (entity_activator->move_type == MOVE_ON_FLOOR) && lowest_sector &&
                                           (entity_activator->transform[12 + 2] <= lowest_sector->floor + 16);
                    }
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
                    header_condition = false;
                    break;

                case TR_FD_TRIGTYPE_ANTITRIGGER:
                case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
                    action_type = TR_ACTIONTYPE_ANTI;
                    mask_mode = TRIGGER_OP_AND_INV;
                    break;

                case TR_FD_TRIGTYPE_MONKEY:
                    header_condition = header_condition && (entity_activator->move_type == MOVE_MONKEYSWING);
                    break;

                case TR_FD_TRIGTYPE_CLIMB:
                    header_condition = header_condition && (entity_activator->move_type == MOVE_CLIMBING);
                    break;

                case TR_FD_TRIGTYPE_TIGHTROPE:
                    // Check state range for triggering entity.
                    header_condition = header_condition && entity_activator->character && entity_activator->character->state.tightrope;
                    break;

                case TR_FD_TRIGTYPE_CRAWLDUCK:
                    // Check state range for triggering entity.
                    header_condition = header_condition && entity_activator->character && entity_activator->character->state.crouch;
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
                                            trig_entity->timer = 0;
                                        }
                                        else if((switch_event_state == 1) && (switch_sectorstatus == 1))
                                        {
                                            // Create statement for antitriggering a switch.
                                            Entity_SetSectorStatus(trig_entity, 0);
                                            trig_entity->timer = trigger->timer;
                                        }
                                        else
                                        {
                                            return;
                                        }
                                    }
                                    else    /// end if (action_type == TR_ACTIONTYPE_SWITCH)
                                    {
                                        // Ordinary type case (e.g. heavy switch).
                                        switch_sectorstatus = (entity_activator->trigger_layout & ENTITY_TLAYOUT_SSTATUS) >> 7;
                                        switch_mask = (entity_activator->trigger_layout & ENTITY_TLAYOUT_MASK);
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
                                    activation_state = Entity_Activate(trig_entity, entity_activator, trigger->mask, mask_mode, trigger->once, 0.0f);
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
                                World_SetFlipMap(command->operands, switch_mask, mask_mode);
                                World_SetFlipState(command->operands, FLIP_STATE_BY_FLAG);
                            }
                            else
                            {
                                World_SetFlipMap(command->operands, trigger->mask, mask_mode);
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
                        if(!is_heavy || (activator_sector_status == 0))
                        {
                            Game_SetCameraTarget(command->operands, trigger->timer);
                        }
                        break;

                    case TR_FD_TRIGFUNC_SET_CAMERA:
                        if(!is_heavy || (activator_sector_status == 0))
                        {
                            Game_SetCamera(command->camera.index, command->once, command->camera.move, command->camera.timer);
                        }
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
                        if(!gameflow.Send(GF_OP_LEVELCOMPLETE, command->operands))
                        {
                            Con_Warning("TR_FD_TRIGFUNC_ENDLEVEL: Failed to add opcode to gameflow action list");
                        }
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
                            Script_DoFlipEffect(engine_lua, command->operands, entity_activator->id, trigger->timer);
                        }
                        break;

                    case TR_FD_TRIGFUNC_SECRET:
                        if((command->operands < GF_MAX_SECRETS) && (gameflow.getSecretStateAtIndex(command->operands) == 0))
                        {
                            gameflow.setSecretStateAtIndex(command->operands, 1);
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
