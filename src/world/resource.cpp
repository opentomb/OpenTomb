#include "resource.h"

#include "LuaState.h"
#include "loader/level.h"

#include "animation/animids.h"
#include "bordered_texture_atlas.h"
#include "engine/bullet.h"
#include "engine/engine.h"
#include "engine/gameflow.h"
#include "gui/console.h"
#include "gui/gui.h"
#include "render/render.h"
#include "render/shader_description.h"
#include "script/script.h"
#include "strings.h"
#include "util/helpers.h"
#include "util/vmath.h"
#include "world/animation/animcommands.h"
#include "world/character.h"
#include "world/core/basemesh.h"
#include "world/core/orientedboundingbox.h"
#include "world/core/polygon.h"
#include "world/core/sprite.h"
#include "world/entity.h"
#include "world/portal.h"
#include "world/room.h"
#include "world/skeletalmodel.h"
#include "world/staticmesh.h"
#include "world/world.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <numeric>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptors.hpp>

// #define LOG_ANIM_DISPATCHES

using gui::Console;

namespace world
{
void Res_SetEntityProperties(std::shared_ptr<Entity> ent)
{
    if(ent->m_skeleton.getModel() == nullptr || !engine_lua["getEntityModelProperties"].is<lua::Callable>())
        return;

    int collisionType, collisionShape, flg;
    lua::tie(collisionType, collisionShape, ent->m_visible, flg) = engine_lua.call("getEntityModelProperties", static_cast<int>(engine::Engine::instance.m_world.m_engineVersion), ent->m_skeleton.getModel()->getId());
    ent->setCollisionType(static_cast<CollisionType>(collisionType));
    ent->setCollisionShape(static_cast<CollisionShape>(collisionShape));

    ent->m_visible = !ent->m_visible;
    ent->m_typeFlags |= flg;
}

void Res_SetEntityFunction(std::shared_ptr<Entity> ent)
{
    if(!ent->m_skeleton.getModel())
        return;

    auto funcName = engine_lua.call("getEntityFunction", static_cast<int>(engine::Engine::instance.m_world.m_engineVersion), ent->m_skeleton.getModel()->getId()).toString();
    if(!funcName.empty())
        Res_CreateEntityFunc(engine_lua, funcName, ent->getId());
}

void Res_CreateEntityFunc(script::ScriptEngine& state, const std::string& func_name, ObjectId entity_id)
{
    if(state["entity_funcs"][entity_id].is<lua::Nil>())
        state["entity_funcs"].set(entity_id, lua::Table());
    state[(func_name + "_init").c_str()](entity_id);
}

void Res_GenEntityFunctions(std::map<uint32_t, std::shared_ptr<Entity> > &entities)
{
    if(entities.empty())
        return;

    for(const std::shared_ptr<Entity>& entity : entities | boost::adaptors::map_values)
        Res_SetEntityFunction(entity);
}

void Res_SetStaticMeshProperties(std::shared_ptr<StaticMesh> r_static)
{
    int _collision_type, _collision_shape;
    bool _hide;
    lua::tie(_collision_type, _collision_shape, _hide) = engine_lua.call("getStaticMeshProperties", r_static->getId());

    if(_collision_type <= 0)
        return;

    r_static->setCollisionType(static_cast<CollisionType>(_collision_type));
    r_static->setCollisionShape(static_cast<CollisionShape>(_collision_shape));
    r_static->hide = _hide;
}

/*
 * BASIC SECTOR COLLISION LAYOUT
 *
 *
 *  OY                            OZ
 *  ^   0 ___________ 1            ^  1  ___________  2
 *  |    |           |             |    |           |
 *  |    |           |             |    |   tween   |
 *  |    |   SECTOR  |             |    |  corners  |
 *  |    |           |             |    |           |
 *  |   3|___________|2            |  0 |___________| 3
 *  |-------------------> OX       |--------------------> OXY
 */

bool Res_Sector_IsWall(const RoomSector* ws, const RoomSector* ns)
{
    BOOST_ASSERT(ws != nullptr);
    BOOST_ASSERT(ns != nullptr);

    if(!ws->portal_to_room && !ns->portal_to_room && ws->floor_penetration_config == PenetrationConfig::Wall)
    {
        return true;
    }

    if(!ns->portal_to_room && ns->floor_penetration_config != PenetrationConfig::Wall && ws->portal_to_room)
    {
        ws = ws->checkPortalPointer();
        if(ws->floor_penetration_config == PenetrationConfig::Wall || !ns->is2SidePortals(ws))
        {
            return true;
        }
    }

    return false;
}

// Check if entity index was already processed (needed to remove dublicated activation calls).
// If entity is not processed, add its index into lookup table.
bool Res_IsEntityProcessed(std::set<ObjectId>& lookup_table, ObjectId entity_index)
{
    // Fool-proof check for entity existence. Fixes LOTS of stray non-existent
    // entity #256 occurences in original games (primarily TR4-5).

    if(!engine::Engine::instance.m_world.getEntityByID(entity_index))
        return true;

    return !lookup_table.insert(entity_index).second;
}

int TR_Sector_TranslateFloorData(RoomSector& sector, const loader::FloorData& floorData, loader::Engine engine)
{
    if(sector.trig_index <= 0 || sector.trig_index >= floorData.size())
    {
        return 0;
    }

    sector.flags = 0;  // Clear sector flags before parsing.

    /*
     * PARSE FUNCTIONS
     */

    const uint16_t *end_p = floorData.data() + floorData.size() - 1;
    const uint16_t *entry = floorData.data() + sector.trig_index;

    int ret = 0;
    uint16_t end_bit;

    do
    {
        // TR1 - TR2
        //function = (*entry) & 0x00FF;                   // 0b00000000 11111111
        //sub_function = ((*entry) & 0x7F00) >> 8;        // 0b01111111 00000000

        //TR3+, but works with TR1 - TR2
        uint16_t function = *entry & 0x001F;             // 0b00000000 00011111
        // uint16_t function_value = ((*entry) & 0x00E0) >> 5;        // 0b00000000 11100000  TR3+
        uint16_t sub_function = (*entry & 0x7F00) >> 8;        // 0b01111111 00000000

        end_bit = (*entry & 0x8000) >> 15;       // 0b10000000 00000000

        entry++;

        switch(function)
        {
            case TR_FD_FUNC_PORTALSECTOR:          // PORTAL DATA
                if(sub_function == 0x00)
                {
                    if(*entry < engine::Engine::instance.m_world.m_rooms.size())
                    {
                        sector.portal_to_room = *entry;
                        sector.floor_penetration_config = PenetrationConfig::Ghost;
                        sector.ceiling_penetration_config = PenetrationConfig::Ghost;
                    }
                    entry++;
                }
                break;

            case TR_FD_FUNC_FLOORSLANT:          // FLOOR SLANT
                if(sub_function == 0x00)
                {
                    int8_t raw_y_slant = *entry & 0x00FF;
                    int8_t raw_x_slant = (*entry & 0xFF00) >> 8;

                    sector.floor_diagonal_type = DiagonalType::None;
                    sector.floor_penetration_config = PenetrationConfig::Solid;

                    if(raw_x_slant > 0)
                    {
                        sector.floor_corners[2][2] -= static_cast<glm::float_t>(raw_x_slant) * MeteringStep;
                        sector.floor_corners[3][2] -= static_cast<glm::float_t>(raw_x_slant) * MeteringStep;
                    }
                    else if(raw_x_slant < 0)
                    {
                        sector.floor_corners[0][2] -= glm::abs(static_cast<glm::float_t>(raw_x_slant)) * MeteringStep;
                        sector.floor_corners[1][2] -= glm::abs(static_cast<glm::float_t>(raw_x_slant)) * MeteringStep;
                    }

                    if(raw_y_slant > 0)
                    {
                        sector.floor_corners[0][2] -= static_cast<glm::float_t>(raw_y_slant) * MeteringStep;
                        sector.floor_corners[3][2] -= static_cast<glm::float_t>(raw_y_slant) * MeteringStep;
                    }
                    else if(raw_y_slant < 0)
                    {
                        sector.floor_corners[1][2] -= glm::abs(static_cast<glm::float_t>(raw_y_slant)) * MeteringStep;
                        sector.floor_corners[2][2] -= glm::abs(static_cast<glm::float_t>(raw_y_slant)) * MeteringStep;
                    }

                    entry++;
                }
                break;

            case TR_FD_FUNC_CEILINGSLANT:          // CEILING SLANT
                if(sub_function == 0x00)
                {
                    int8_t raw_y_slant = *entry & 0x00FF;
                    int8_t raw_x_slant = (*entry & 0xFF00) >> 8;

                    sector.ceiling_diagonal_type = DiagonalType::None;
                    sector.ceiling_penetration_config = PenetrationConfig::Solid;

                    if(raw_x_slant > 0)
                    {
                        sector.ceiling_corners[3][2] += static_cast<glm::float_t>(raw_x_slant) * MeteringStep;
                        sector.ceiling_corners[2][2] += static_cast<glm::float_t>(raw_x_slant) * MeteringStep;
                    }
                    else if(raw_x_slant < 0)
                    {
                        sector.ceiling_corners[1][2] += glm::abs(static_cast<glm::float_t>(raw_x_slant)) * MeteringStep;
                        sector.ceiling_corners[0][2] += glm::abs(static_cast<glm::float_t>(raw_x_slant)) * MeteringStep;
                    }

                    if(raw_y_slant > 0)
                    {
                        sector.ceiling_corners[1][2] += static_cast<glm::float_t>(raw_y_slant) * MeteringStep;
                        sector.ceiling_corners[2][2] += static_cast<glm::float_t>(raw_y_slant) * MeteringStep;
                    }
                    else if(raw_y_slant < 0)
                    {
                        sector.ceiling_corners[0][2] += glm::abs(static_cast<glm::float_t>(raw_y_slant)) * MeteringStep;
                        sector.ceiling_corners[3][2] += glm::abs(static_cast<glm::float_t>(raw_y_slant)) * MeteringStep;
                    }

                    entry++;
                }
                break;

            case TR_FD_FUNC_TRIGGER:          // TRIGGERS
            {
                std::string header;         // Header condition
                std::string once_condition; // One-shot condition
                std::string cont_events;    // Continous trigger events
                std::string single_events;  // One-shot trigger events
                std::string item_events;    // Item activation events
                std::string anti_events;    // Item deactivation events, if needed

                std::string script;         // Final script compile

                char buf[512];                  buf[0] = 0;    // Stream buffer
                char buf2[512];                 buf2[0] = 0;    // Conditional pre-buffer for SWITCH triggers

                ActivatorType activator = ActivatorType::Normal;      // Activator is normal by default.
                ActionType action_type = ActionType::Normal;     // Action type is normal by default.
                bool condition = false;                        // No condition by default.
                int mask_mode = AMASK_OP_OR;              // Activation mask by default.

                int8_t  timer_field = *entry & 0x00FF;          // Used as common parameter for some commands.
                uint8_t trigger_mask = (*entry & 0x3E00) >> 9;
                uint8_t only_once = (*entry & 0x0100) >> 8;    // Lock out triggered items after activation.

                // Processed entities lookup array initialization.

                std::set<ObjectId> ent_lookup_table;

                // Activator type is LARA for all triggers except HEAVY ones, which are triggered by
                // some specific entity classes.

                int activator_type = (sub_function == TR_FD_TRIGTYPE_HEAVY) ||
                    (sub_function == TR_FD_TRIGTYPE_HEAVYANTITRIGGER) ||
                    (sub_function == TR_FD_TRIGTYPE_HEAVYSWITCH) ? TR_ACTIVATORTYPE_MISC : TR_ACTIVATORTYPE_LARA;

                // Table cell header.

                snprintf(buf, 256, "trigger_list[%d] = {activator_type = %d, func = function(entity_index) \n",
                         sector.trig_index, activator_type);

                script += buf;
                buf[0] = 0;     // Zero out buffer to prevent further trashing.

                switch(sub_function)
                {
                    case TR_FD_TRIGTYPE_TRIGGER:
                    case TR_FD_TRIGTYPE_HEAVY:
                        activator = ActivatorType::Normal;
                        break;

                    case TR_FD_TRIGTYPE_PAD:
                    case TR_FD_TRIGTYPE_ANTIPAD:
                        // Check move type for triggering entity.
                        snprintf(buf, 128, " if(getEntityMoveType(entity_index) == MOVE_ON_FLOOR) then \n");
                        if(sub_function == TR_FD_TRIGTYPE_ANTIPAD)
                            action_type = ActionType::Anti;
                        condition = true;  // Set additional condition.
                        break;

                    case TR_FD_TRIGTYPE_SWITCH:
                        // Set activator and action type for now; conditions are linked with first item in operand chain.
                        activator = ActivatorType::Switch;
                        action_type = ActionType::Switch;
                        mask_mode = AMASK_OP_XOR;
                        break;

                    case TR_FD_TRIGTYPE_HEAVYSWITCH:
                        // Action type remains normal, as HEAVYSWITCH acts as "heavy trigger" with activator mask filter.
                        activator = ActivatorType::Switch;
                        mask_mode = AMASK_OP_XOR;
                        break;

                    case TR_FD_TRIGTYPE_KEY:
                        // Action type remains normal, as key acts one-way (no need in switch routines).
                        activator = ActivatorType::Key;
                        break;

                    case TR_FD_TRIGTYPE_PICKUP:
                        // Action type remains normal, as pick-up acts one-way (no need in switch routines).
                        activator = ActivatorType::Pickup;
                        break;

                    case TR_FD_TRIGTYPE_COMBAT:
                        // Check weapon status for triggering entity.
                        snprintf(buf, 128, " if(getCharacterCombatMode(entity_index) > 0) then \n");
                        condition = true;  // Set additional condition.
                        break;

                    case TR_FD_TRIGTYPE_DUMMY:
                    case TR_FD_TRIGTYPE_SKELETON:   ///@FIXME: Find the meaning later!!!
                        // These triggers are being parsed, but not added to trigger script!
                        action_type = ActionType::Bypass;
                        break;

                    case TR_FD_TRIGTYPE_ANTITRIGGER:
                    case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
                        action_type = ActionType::Anti;
                        break;

                    case TR_FD_TRIGTYPE_MONKEY:
                    case TR_FD_TRIGTYPE_CLIMB:
                        // Check move type for triggering entity.
                        snprintf(buf, 128, " if(getEntityMoveType(entity_index) == %d) then \n", (sub_function == TR_FD_TRIGTYPE_MONKEY) ? MoveType::Monkeyswing : MoveType::Climbing);
                        condition = true;  // Set additional condition.
                        break;

                    case TR_FD_TRIGTYPE_TIGHTROPE:
                        // Check state range for triggering entity.
                        snprintf(buf, 128, " local state = getEntityState(entity_index) \n if((state >= %d) and (state <= %d)) then \n", static_cast<int>(LaraState::TightropeIdle), static_cast<int>(LaraState::TightropeExit));
                        condition = true;  // Set additional condition.
                        break;
                    case TR_FD_TRIGTYPE_CRAWLDUCK:
                        // Check state range for triggering entity.
                        snprintf(buf, 128, " local state = getEntityState(entity_index) \n if((state >= %d) and (state <= %d)) then \n", animation::TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_BEGIN, animation::TR_ANIMATION_LARA_CRAWL_SMASH_LEFT);
                        condition = true;  // Set additional condition.
                        break;
                }

                header += buf;    // Add condition to header.

                uint16_t cont_bit;
                uint16_t argn = 0;

                // Now parse operand chain for trigger function!

                do
                {
                    entry++;

                    uint16_t trigger_function = (*entry & 0x7C00) >> 10;    // 0b01111100 00000000
                    uint16_t operands = *entry & 0x03FF;                      // 0b00000011 11111111
                    cont_bit = (*entry & 0x8000) >> 15;              // 0b10000000 00000000

                    switch(trigger_function)
                    {
                        case TR_FD_TRIGFUNC_OBJECT:         // ACTIVATE / DEACTIVATE object
                            // If activator is specified, first item operand counts as activator index (except
                            // heavy switch case, which is ordinary heavy trigger case with certain differences).
                            if(argn == 0 && activator != ActivatorType::Normal)
                            {
                                switch(activator)
                                {
                                    case ActivatorType::Normal:
                                        BOOST_ASSERT(false);
                                        break;

                                    case ActivatorType::Switch:
                                        if(action_type == ActionType::Switch)
                                        {
                                            // Switch action type case.
                                            snprintf(buf, 256, " local switch_state = getEntityState(%d); \n local switch_sectorstatus = getEntitySectorStatus(%d); \n local switch_mask = getEntityMask(%d); \n\n", operands, operands, operands);
                                        }
                                        else
                                        {
                                            // Ordinary type case (e.g. heavy switch).
                                            snprintf(buf, 256, " local switch_sectorstatus = getEntitySectorStatus(entity_index); \n local switch_mask = getEntityMask(entity_index); \n\n");
                                        }
                                        script += buf;

                                        // Trigger activation mask is here filtered through activator's own mask.
                                        snprintf(buf, 256, " if(switch_mask == 0) then switch_mask = 0x1F end; \n switch_mask = bit32.band(switch_mask, 0x%02X); \n\n", trigger_mask);
                                        script += buf;
                                        if(action_type == ActionType::Switch)
                                        {
                                            // Switch action type case.
                                            snprintf(buf, 256, " if((switch_state == 0) and switch_sectorstatus) then \n   setEntitySectorStatus(%d, false); \n   setEntityTimer(%d, %d); \n", operands, operands, timer_field);
                                            if(engine::Engine::instance.m_world.m_engineVersion >= loader::Engine::TR3 && only_once)
                                            {
                                                // Just lock out activator, no anti-action needed.
                                                snprintf(buf2, 128, " setEntityLock(%d, true) \n", operands);
                                            }
                                            else
                                            {
                                                // Create statement for antitriggering a switch.
                                                snprintf(buf2, 256, " elseif((switch_state == 1) and switch_sectorstatus) then\n   setEntitySectorStatus(%d, false); \n   setEntityTimer(%d, 0); \n", operands, operands);
                                            }
                                        }
                                        else
                                        {
                                            // Ordinary type case (e.g. heavy switch).
                                            snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %s, %d); \n", operands, mask_mode, only_once ? "true" : "false", timer_field);
                                            item_events += buf;
                                            snprintf(buf, 128, " if(not switch_sectorstatus) then \n   setEntitySectorStatus(entity_index, true) \n");
                                        }
                                        break;

                                    case ActivatorType::Key:
                                        snprintf(buf, 256, " if((getEntityLock(%d)) and (not getEntitySectorStatus(%d))) then \n   setEntitySectorStatus(%d, true); \n", operands, operands, operands);
                                        break;

                                    case ActivatorType::Pickup:
                                        snprintf(buf, 256, " if((not getEntityEnability(%d)) and (not getEntitySectorStatus(%d))) then \n   setEntitySectorStatus(%d, true); \n", operands, operands, operands);
                                        break;
                                }

                                script += buf;
                            }
                            else
                            {
                                // In many original Core Design levels, level designers left dublicated entity activation operands.
                                // This results in setting same activation mask twice, effectively blocking entity from activation.
                                // To prevent this, a lookup table was implemented to know if entity already had its activation
                                // command added.
                                if(!Res_IsEntityProcessed(ent_lookup_table, operands))
                                {
                                    // Other item operands are simply parsed as activation functions. Switch case is special, because
                                    // function is fed with activation mask argument derived from activator mask filter (switch_mask),
                                    // and also we need to process deactivation in a same way as activation, excluding resetting timer
                                    // field. This is needed for two-way switch combinations (e.g. Palace Midas).
                                    if(activator == ActivatorType::Switch)
                                    {
                                        snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %s, %d); \n", operands, mask_mode, only_once ? "true" : "false", timer_field);
                                        item_events += buf;
                                        snprintf(buf, 128, "   activateEntity(%d, entity_index, switch_mask, %d, %s, 0); \n", operands, mask_mode, only_once ? "true" : "false");
                                        anti_events += buf;
                                    }
                                    else
                                    {
                                        snprintf(buf, 128, "   activateEntity(%d, entity_index, 0x%02X, %d, %s, %d); \n", operands, trigger_mask, mask_mode, only_once ? "true" : "false", timer_field);
                                        item_events += buf;
                                        snprintf(buf, 128, "   deactivateEntity(%d, entity_index, %s); \n", operands, only_once ? "true" : "false");
                                        anti_events += buf;
                                    }
                                }
                            }
                            argn++;
                            break;

                        case TR_FD_TRIGFUNC_CAMERATARGET:
                        {
                            uint8_t cam_index = *entry & 0x007F;
                            entry++;
                            uint8_t cam_timer = *entry & 0x00FF;
                            uint8_t cam_once = (*entry & 0x0100) >> 8;
                            uint8_t cam_zoom = engine::Engine::instance.m_world.m_engineVersion < loader::Engine::TR2
                                ? (*entry & 0x0400) >> 10
                                : (*entry & 0x1000) >> 12;
                            cont_bit = (*entry & 0x8000) >> 15;                       // 0b10000000 00000000

                            snprintf(buf, 128, "   setCamera(%d, %d, %d, %d); \n", cam_index, cam_timer, cam_once, cam_zoom);
                            single_events += buf;
                        }
                        break;

                        case TR_FD_TRIGFUNC_UWCURRENT:
                            snprintf(buf, 128, "   moveToSink(entity_index, %d); \n", operands);
                            cont_events += buf;
                            break;

                        case TR_FD_TRIGFUNC_FLIPMAP:
                            // FLIPMAP trigger acts two-way for switch cases, so we add FLIPMAP off event to
                            // anti-events array.
                            if(activator == ActivatorType::Switch)
                            {
                                snprintf(buf, 128, "   setFlipMap(%d, switch_mask, 1); \n   setFlipState(%d, true); \n", operands, operands);
                                single_events += buf;
                            }
                            else
                            {
                                snprintf(buf, 128, "   setFlipMap(%d, 0x%02X, 0); \n   setFlipState(%d, true); \n", operands, trigger_mask, operands);
                                single_events += buf;
                            }
                            break;

                        case TR_FD_TRIGFUNC_FLIPON:
                            // FLIP_ON trigger acts one-way even in switch cases, i.e. if you un-pull
                            // the switch with FLIP_ON trigger, room will remain flipped.
                            snprintf(buf, 128, "   setFlipState(%d, true); \n", operands);
                            single_events += buf;
                            break;

                        case TR_FD_TRIGFUNC_FLIPOFF:
                            // FLIP_OFF trigger acts one-way even in switch cases, i.e. if you un-pull
                            // the switch with FLIP_OFF trigger, room will remain unflipped.
                            snprintf(buf, 128, "   setFlipState(%d, false); \n", operands);
                            single_events += buf;
                            break;

                        case TR_FD_TRIGFUNC_LOOKAT:
                            snprintf(buf, 128, "   setCamTarget(%d, %d); \n", operands, timer_field);
                            single_events += buf;
                            break;

                        case TR_FD_TRIGFUNC_ENDLEVEL:
                            snprintf(buf, 128, "   setLevel(%d); \n", operands);
                            single_events += buf;
                            break;

                        case TR_FD_TRIGFUNC_PLAYTRACK:
                            // Override for looped BGM tracks in TR1: if there are any sectors
                            // triggering looped tracks, ignore it, as BGM is always set in script.
                            if(engine::Engine::instance.m_world.m_engineVersion < loader::Engine::TR2)
                            {
                                audio::StreamType looped;
                                engine_lua.getSoundtrack(operands, nullptr, nullptr, &looped);
                                if(looped == audio::StreamType::Background)
                                    break;
                            }

                            snprintf(buf, 128, "   playStream(%d, 0x%02X); \n", operands, (trigger_mask << 1) + only_once);
                            single_events += buf;
                            break;

                        case TR_FD_TRIGFUNC_FLIPEFFECT:
                            snprintf(buf, 128, "   doEffect(%d, entity_index, %d); \n", operands, timer_field);
                            cont_events += buf;
                            break;

                        case TR_FD_TRIGFUNC_SECRET:
                            snprintf(buf, 128, "   findSecret(%d); \n", operands);
                            single_events += buf;
                            break;

                        case TR_FD_TRIGFUNC_CLEARBODIES:
                            snprintf(buf, 128, "   clearBodies(); \n");
                            single_events += buf;
                            break;

                        case TR_FD_TRIGFUNC_FLYBY:
                        {
                            entry++;
                            uint8_t flyby_once = (*entry & 0x0100) >> 8;
                            cont_bit = (*entry & 0x8000) >> 15;

                            snprintf(buf, 128, "   playFlyby(%d, %d); \n", operands, flyby_once);
                            cont_events += buf;
                        }
                        break;

                        case TR_FD_TRIGFUNC_CUTSCENE:
                            snprintf(buf, 128, "   playCutscene(%d); \n", operands);
                            single_events += buf;
                            break;

                        default: // UNKNOWN!
                            break;
                    };
                } while(!cont_bit && entry < end_p);

                if(!script.empty())
                {
                    script += header;

                    // Heavy trigger and antitrigger item events are engaged ONLY
                    // once, when triggering item is approaching sector. Hence, we
                    // copy item events to single events and nullify original item
                    // events sequence to prevent it to be merged into continous
                    // events.

                    if((sub_function == TR_FD_TRIGTYPE_HEAVY) ||
                       (sub_function == TR_FD_TRIGTYPE_HEAVYANTITRIGGER))
                    {
                        if(action_type == ActionType::Anti)
                        {
                            single_events += anti_events;
                        }
                        else
                        {
                            single_events += item_events;
                        }

                        anti_events.clear();
                        item_events.clear();
                    }

                    if(activator == ActivatorType::Normal)    // Ordinary trigger cases.
                    {
                        if(!single_events.empty())
                        {
                            if(condition)
                                once_condition += " ";
                            once_condition += " if(not getEntitySectorStatus(entity_index)) then \n";
                            script += once_condition;
                            script += single_events;
                            script += "   setEntitySectorStatus(entity_index, true); \n";

                            if(condition)
                            {
                                script += "  end;\n"; // First ENDIF is tabbed for extra condition.
                            }
                            else
                            {
                                script += " end;\n";
                            }
                        }

                        // Item commands kind depends on action type. If type is ANTI, then item
                        // antitriggering is engaged. If type is normal, ordinary triggering happens
                        // in cycle with other continous commands. It is needed to prevent timer dispatch
                        // before activator leaves trigger sector.

                        if(action_type == ActionType::Anti)
                        {
                            script += anti_events;
                        }
                        else
                        {
                            script += item_events;
                        }

                        script += cont_events;
                        if(condition)
                            script += " end;\n"; // Additional ENDIF for extra condition.
                    }
                    else    // SWITCH, KEY and ITEM cases.
                    {
                        script += single_events;
                        script += item_events;
                        script += cont_events;
                        if(action_type == ActionType::Switch && activator == ActivatorType::Switch)
                        {
                            script += buf2;
                            if(engine::Engine::instance.m_world.m_engineVersion < loader::Engine::TR3 || !only_once)
                            {
                                script += single_events;
                                script += anti_events;    // Single/continous events are engaged along with
                                script += cont_events;    // antitriggered items, as described above.
                            }
                        }
                        script += " end;\n";
                    }

                    script += "return 1;\nend }\n";  // Finalize the entry.
                }

                if(action_type != ActionType::Bypass)
                {
                    // Sys_DebugLog("triggers.lua", script);    // Debug!
                    engine_lua.doString(script);
                }
            }
            break;

            case TR_FD_FUNC_DEATH:
                sector.flags |= SECTOR_FLAG_DEATH;
                break;

            case TR_FD_FUNC_CLIMB:
                // First 4 sector flags are similar to subfunction layout.
                sector.flags |= sub_function;
                break;

            case TR_FD_FUNC_MONKEY:
                sector.flags |= SECTOR_FLAG_CLIMB_CEILING;
                break;

            case TR_FD_FUNC_MINECART_LEFT:
                // Minecart left (TR3) and trigger triggerer mark (TR4-5) has the same flag value.
                // We re-parse them properly here.
                if(engine < loader::Engine::TR4)
                {
                    sector.flags |= SECTOR_FLAG_MINECART_LEFT;
                }
                else
                {
                    sector.flags |= SECTOR_FLAG_TRIGGERER_MARK;
                }
                break;

            case TR_FD_FUNC_MINECART_RIGHT:
                // Minecart right (TR3) and beetle mark (TR4-5) has the same flag value.
                // We re-parse them properly here.
                if(engine < loader::Engine::TR4)
                {
                    sector.flags |= SECTOR_FLAG_MINECART_RIGHT;
                }
                else
                {
                    sector.flags |= SECTOR_FLAG_BEETLE_MARK;
                }
                break;

            default:
                // Other functions are TR3+ collisional triangle functions.
                if((function >= TR_FD_FUNC_FLOORTRIANGLE_NW) &&
                   (function <= TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE))
                {
                    entry--;    // Go back, since these functions are parsed differently.

                    end_bit = (*entry & 0x8000) >> 15;      // 0b10000000 00000000

#if 0
                    int16_t  slope_t01 = ((*entry) & 0x7C00) >> 10;      // 0b01111100 00000000
                    int16_t  slope_t00 = ((*entry) & 0x03E0) >> 5;       // 0b00000011 11100000
                    // uint16_t slope_func = ((*entry) & 0x001F);            // 0b00000000 00011111

                    // t01/t02 are 5-bit values, where sign is specified by 0x10 mask.

                    if(slope_t01 & 0x10) slope_t01 |= 0xFFF0;
                    if(slope_t00 & 0x10) slope_t00 |= 0xFFF0;
#endif

                    entry++;

                    uint16_t slope_t13 = (*entry & 0xF000) >> 12;      // 0b11110000 00000000
                    uint16_t slope_t12 = (*entry & 0x0F00) >> 8;       // 0b00001111 00000000
                    uint16_t slope_t11 = (*entry & 0x00F0) >> 4;       // 0b00000000 11110000
                    uint16_t slope_t10 = *entry & 0x000F;            // 0b00000000 00001111

                    entry++;

                    float overall_adjustment = static_cast<float>(std::max({ slope_t10, slope_t11, slope_t12, slope_t13 })) * MeteringStep;

                    if((function == TR_FD_FUNC_FLOORTRIANGLE_NW) ||
                       (function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW) ||
                       (function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE))
                    {
                        sector.floor_diagonal_type = DiagonalType::NW;

                        sector.floor_corners[0][2] -= overall_adjustment - static_cast<glm::float_t>(slope_t12) * MeteringStep;
                        sector.floor_corners[1][2] -= overall_adjustment - static_cast<glm::float_t>(slope_t13) * MeteringStep;
                        sector.floor_corners[2][2] -= overall_adjustment - static_cast<glm::float_t>(slope_t10) * MeteringStep;
                        sector.floor_corners[3][2] -= overall_adjustment - static_cast<glm::float_t>(slope_t11) * MeteringStep;

                        if(function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW)
                        {
                            sector.floor_penetration_config = PenetrationConfig::DoorVerticalA;
                        }
                        else if(function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE)
                        {
                            sector.floor_penetration_config = PenetrationConfig::DoorVerticalB;
                        }
                        else
                        {
                            sector.floor_penetration_config = PenetrationConfig::Solid;
                        }
                    }
                    else if((function == TR_FD_FUNC_FLOORTRIANGLE_NE) ||
                            (function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW) ||
                            (function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE))
                    {
                        sector.floor_diagonal_type = DiagonalType::NE;

                        sector.floor_corners[0][2] -= overall_adjustment - static_cast<glm::float_t>(slope_t12) * MeteringStep;
                        sector.floor_corners[1][2] -= overall_adjustment - static_cast<glm::float_t>(slope_t13) * MeteringStep;
                        sector.floor_corners[2][2] -= overall_adjustment - static_cast<glm::float_t>(slope_t10) * MeteringStep;
                        sector.floor_corners[3][2] -= overall_adjustment - static_cast<glm::float_t>(slope_t11) * MeteringStep;

                        if(function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW)
                        {
                            sector.floor_penetration_config = PenetrationConfig::DoorVerticalA;
                        }
                        else if(function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE)
                        {
                            sector.floor_penetration_config = PenetrationConfig::DoorVerticalB;
                        }
                        else
                        {
                            sector.floor_penetration_config = PenetrationConfig::Solid;
                        }
                    }
                    else if((function == TR_FD_FUNC_CEILINGTRIANGLE_NW) ||
                            (function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW) ||
                            (function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE))
                    {
                        sector.ceiling_diagonal_type = DiagonalType::NW;

                        sector.ceiling_corners[0][2] += overall_adjustment - static_cast<glm::float_t>(slope_t11 * MeteringStep);
                        sector.ceiling_corners[1][2] += overall_adjustment - static_cast<glm::float_t>(slope_t10 * MeteringStep);
                        sector.ceiling_corners[2][2] += overall_adjustment - static_cast<glm::float_t>(slope_t13 * MeteringStep);
                        sector.ceiling_corners[3][2] += overall_adjustment - static_cast<glm::float_t>(slope_t12 * MeteringStep);

                        if(function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW)
                        {
                            sector.ceiling_penetration_config = PenetrationConfig::DoorVerticalA;
                        }
                        else if(function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE)
                        {
                            sector.ceiling_penetration_config = PenetrationConfig::DoorVerticalB;
                        }
                        else
                        {
                            sector.ceiling_penetration_config = PenetrationConfig::Solid;
                        }
                    }
                    else if((function == TR_FD_FUNC_CEILINGTRIANGLE_NE) ||
                            (function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW) ||
                            (function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE))
                    {
                        sector.ceiling_diagonal_type = DiagonalType::NE;

                        sector.ceiling_corners[0][2] += overall_adjustment - static_cast<glm::float_t>(slope_t11 * MeteringStep);
                        sector.ceiling_corners[1][2] += overall_adjustment - static_cast<glm::float_t>(slope_t10 * MeteringStep);
                        sector.ceiling_corners[2][2] += overall_adjustment - static_cast<glm::float_t>(slope_t13 * MeteringStep);
                        sector.ceiling_corners[3][2] += overall_adjustment - static_cast<glm::float_t>(slope_t12 * MeteringStep);

                        if(function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW)
                        {
                            sector.ceiling_penetration_config = PenetrationConfig::DoorVerticalA;
                        }
                        else if(function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE)
                        {
                            sector.ceiling_penetration_config = PenetrationConfig::DoorVerticalB;
                        }
                        else
                        {
                            sector.ceiling_penetration_config = PenetrationConfig::Solid;
                        }
                    }
                }
                else
                {
                    // Unknown floordata function!
                }
                break;
        };
        ret++;
    } while(!end_bit && entry < end_p);

    return ret;
}

void Res_Sector_FixHeights(RoomSector& sector)
{
    if(sector.floor == MeteringWallHeight)
    {
        sector.floor_penetration_config = PenetrationConfig::Wall;
    }
    if(sector.ceiling == MeteringWallHeight)
    {
        sector.ceiling_penetration_config = PenetrationConfig::Wall;
    }

    // Fix non-material crevices

    for(size_t i = 0; i < 4; i++)
    {
        if(sector.ceiling_corners[i][2] == sector.floor_corners[i][2])
            sector.ceiling_corners[i][2] += LaraHangVerticalEpsilon;
    }
}

void GenerateAnimCommands(SkeletalModel& model)
{
    if(engine::Engine::instance.m_world.m_animCommands.empty())
    {
        return;
    }
    //Sys_DebugLog("anim_transform.txt", "MODEL[%d]", model.id);
    for(size_t anim = 0; anim < model.m_animations.size(); anim++)
    {
        if(model.m_animations[anim].animationCommandCount > 255)
        {
            continue;                                                           // If no anim commands or current anim has more than 255 (according to TRosettaStone).
        }

        animation::Animation* af = &model.m_animations[anim];
        if(af->animationCommandCount == 0)
            continue;

        BOOST_ASSERT(af->animationCommand < engine::Engine::instance.m_world.m_animCommands.size());
        int16_t *pointer = &engine::Engine::instance.m_world.m_animCommands[af->animationCommand];

        for(size_t i = 0; i < af->animationCommandCount; i++)
        {
            const auto command = static_cast<animation::AnimCommandOpcode>(*pointer);
            ++pointer;
            switch(command)
            {
                /*
                 * End-of-anim commands:
                 */
                case animation::AnimCommandOpcode::SetPosition:
                    af->finalAnimCommands.push_back({ command, pointer[0], pointer[1], pointer[2] });
                    // ConsoleInfo::instance().printf("ACmd MOVE: anim = %d, x = %d, y = %d, z = %d", static_cast<int>(anim), pointer[0], pointer[1], pointer[2]);
                    pointer += 3;
                    break;

                case animation::AnimCommandOpcode::SetVelocity:
                    af->finalAnimCommands.push_back({ command, pointer[0], pointer[1], 0 });
                    // ConsoleInfo::instance().printf("ACmd JUMP: anim = %d, vVert = %d, vHoriz = %d", static_cast<int>(anim), pointer[0], pointer[1]);
                    pointer += 2;
                    break;

                case animation::AnimCommandOpcode::EmptyHands:
                    af->finalAnimCommands.push_back({ command, 0, 0, 0 });
                    // ConsoleInfo::instance().printf("ACmd EMTYHANDS: anim = %d", static_cast<int>(anim));
                    break;

                case animation::AnimCommandOpcode::Kill:
                    af->finalAnimCommands.push_back({ command, 0, 0, 0 });
                    // ConsoleInfo::instance().printf("ACmd KILL: anim = %d", static_cast<int>(anim));
                    break;

                    /*
                     * Per frame commands:
                     */
                case animation::AnimCommandOpcode::PlaySound:
                    if(pointer[0] < static_cast<int>(af->getFrameDuration()))
                    {
                        af->animCommands(pointer[0]).push_back({ command, pointer[1], 0, 0 });
                    }
                    // ConsoleInfo::instance().printf("ACmd PLAYSOUND: anim = %d, frame = %d of %d", static_cast<int>(anim), pointer[0], static_cast<int>(af->frames.size()));
                    pointer += 2;
                    break;

                case animation::AnimCommandOpcode::PlayEffect:
                    if(pointer[0] < static_cast<int>(af->getFrameDuration()))
                    {
                        af->animCommands(pointer[0]).push_back({ command, pointer[1], 0, 0 });
                    }
                    //                    ConsoleInfo::instance().printf("ACmd FLIPEFFECT: anim = %d, frame = %d of %d", static_cast<int>(anim), pointer[0], static_cast<int>(af->frames.size()));
                    pointer += 2;
                    break;
            }
        }
    }
}

RoomSector* TR_GetRoomSector(uint32_t room_id, int sx, int sy)
{
    if(room_id >= engine::Engine::instance.m_world.m_rooms.size())
    {
        return nullptr;
    }

    auto room = engine::Engine::instance.m_world.m_rooms[room_id];
    return room->getSector(sx, sy);
}

void lua_SetSectorFloorConfig(int id, int sx, int sy, lua::Value pen, lua::Value diag, lua::Value floor, float z0, float z1, float z2, float z3)
{
    RoomSector* rs = TR_GetRoomSector(id, sx, sy);
    if(rs == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_SECTOR_INFO);
        return;
    }

    if(pen.is<lua::Integer>())   rs->floor_penetration_config = static_cast<PenetrationConfig>(pen.toInt());
    if(diag.is<lua::Integer>())  rs->floor_diagonal_type = static_cast<DiagonalType>(diag.toInt());
    if(floor.is<int32_t>())      rs->floor = floor.to<int32_t>();
    rs->floor_corners[0] = { z0,z1,z2 };
    rs->floor_corners[0][3] = z3;
}

void lua_SetSectorCeilingConfig(int id, int sx, int sy, lua::Value pen, lua::Value diag, lua::Value ceil, float z0, float z1, float z2, float z3)
{
    RoomSector* rs = TR_GetRoomSector(id, sx, sy);
    if(rs == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_SECTOR_INFO);
        return;
    }

    if(pen.is<lua::Integer>())  rs->ceiling_penetration_config = static_cast<PenetrationConfig>(pen.toInt());
    if(diag.is<lua::Integer>()) rs->ceiling_diagonal_type = static_cast<DiagonalType>(diag.toInt());
    if(ceil.is<int32_t>())      rs->ceiling = ceil.to<int32_t>();

    rs->ceiling_corners[0] = { z0,z1,z2 };
    rs->ceiling_corners[0][3] = z3;
}

void lua_SetSectorPortal(int id, int sx, int sy, uint32_t p)
{
    RoomSector* rs = TR_GetRoomSector(id, sx, sy);
    if(rs == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_SECTOR_INFO);
        return;
    }

    if(p < engine::Engine::instance.m_world.m_rooms.size())
    {
        rs->portal_to_room = p;
    }
}

void lua_SetSectorFlags(int id, int sx, int sy, lua::Value fpflag, lua::Value ftflag, lua::Value cpflag, lua::Value ctflag)
{
    RoomSector* rs = TR_GetRoomSector(id, sx, sy);
    if(rs == nullptr)
    {
        Console::instance().warning(SYSWARN_WRONG_SECTOR_INFO);
        return;
    }

    if(fpflag.is<lua::Integer>())  rs->floor_penetration_config = static_cast<PenetrationConfig>(fpflag.toInt());
    if(ftflag.is<lua::Integer>())  rs->floor_diagonal_type = static_cast<DiagonalType>(ftflag.toInt());
    if(cpflag.is<lua::Integer>())  rs->ceiling_penetration_config = static_cast<PenetrationConfig>(cpflag.toInt());
    if(ctflag.is<lua::Integer>())  rs->ceiling_diagonal_type = static_cast<DiagonalType>(ctflag.toInt());
}

void Res_AutoexecOpen(loader::Game engine_version)
{
    std::string autoexecScript = engine::Engine::instance.getAutoexecName(engine_version, std::string());

    if(boost::filesystem::is_regular_file(autoexecScript))
    {
        try
        {
            engine_lua.doFile(autoexecScript);
        }
        catch(lua::RuntimeError& error)
        {
            BOOST_LOG_TRIVIAL(error) << error.what();
        }
        catch(lua::LoadError& error)
        {
            BOOST_LOG_TRIVIAL(error) << error.what();
        }
    }
}

void TR_GenWorld(World& world, const std::unique_ptr<loader::Level>& tr)
{
    world.m_engineVersion = loader::gameToEngine(tr->m_gameVersion);

    Res_AutoexecOpen(tr->m_gameVersion);    // Open and do preload autoexec.
    engine_lua.call("autoexec_PreLoad");
    gui::Gui::instance->drawLoadScreen(150);

    Res_GenRBTrees(world);
    gui::Gui::instance->drawLoadScreen(200);

    TR_GenTextures(world, tr);
    gui::Gui::instance->drawLoadScreen(300);

    TR_GenAnimCommands(world, tr);
    gui::Gui::instance->drawLoadScreen(310);

    TR_GenAnimTextures(world, tr);
    gui::Gui::instance->drawLoadScreen(320);

    TR_GenMeshes(world, tr);
    gui::Gui::instance->drawLoadScreen(400);

    TR_GenSprites(world, tr);
    gui::Gui::instance->drawLoadScreen(420);

    TR_GenBoxes(world, tr);
    gui::Gui::instance->drawLoadScreen(440);

    TR_GenCameras(world, tr);
    gui::Gui::instance->drawLoadScreen(460);

    TR_GenRooms(world, tr);
    gui::Gui::instance->drawLoadScreen(500);

    Res_GenRoomFlipMap(world);
    gui::Gui::instance->drawLoadScreen(520);

    TR_GenSkeletalModels(world, tr);
    gui::Gui::instance->drawLoadScreen(600);

    TR_GenEntities(world, tr);
    gui::Gui::instance->drawLoadScreen(650);

    Res_GenBaseItems(world);
    gui::Gui::instance->drawLoadScreen(680);

    Res_GenSpritesBuffer(world);        // Should be done ONLY after TR_GenEntities.
    gui::Gui::instance->drawLoadScreen(700);

    TR_GenRoomProperties(world, tr);
    gui::Gui::instance->drawLoadScreen(750);

    Res_GenRoomCollision(world);
    gui::Gui::instance->drawLoadScreen(800);

    world.m_audioEngine.load(world, tr);
    gui::Gui::instance->drawLoadScreen(850);

    world.m_skyBox = Res_GetSkybox(world);
    gui::Gui::instance->drawLoadScreen(860);

    Res_GenEntityFunctions(world.m_entities);
    gui::Gui::instance->drawLoadScreen(910);

    Res_GenVBOs(world);
    gui::Gui::instance->drawLoadScreen(950);

    engine_lua.doFile("scripts/autoexec.lua");  // Postload autoexec.
    engine_lua.call("autoexec_PostLoad");
    gui::Gui::instance->drawLoadScreen(960);

    Res_FixRooms(world);                        // Fix initial room states
    gui::Gui::instance->drawLoadScreen(970);
}

void Res_GenRBTrees(World& world)
{
    world.m_entities.clear();
    world.m_nextEntityId = 0;
    world.m_items.clear();
}

void TR_GenRooms(World& world, const std::unique_ptr<loader::Level>& tr)
{
    world.m_rooms.resize(tr->m_rooms.size());
    for(size_t i = 0; i < world.m_rooms.size(); ++i)
        world.m_rooms[i] = std::make_shared<Room>(i);
    for(size_t i = 0; i < world.m_rooms.size(); i++)
    {
        world.m_rooms[i]->load(world, tr);
    }
}

void Res_GenRoomCollision(World& world)
{
    for(std::shared_ptr<Room> room : world.m_rooms)
    {
        room->generateCollisionShape();
    }
}

void TR_GenRoomProperties(World& world, const std::unique_ptr<loader::Level>& tr)
{
    for(size_t i = 0; i < world.m_rooms.size(); i++)
    {
        world.m_rooms[i]->generateProperties(world, tr->m_rooms[i], tr->m_floorData, loader::gameToEngine(tr->m_gameVersion));
    }
}

void Res_GenRoomFlipMap(World& world)
{
    // Flipmap count is hardcoded, as no original levels contain such info.
    world.m_flipData.resize(FLIPMAP_MAX_NUMBER);
}

void TR_GenBoxes(World& world, const std::unique_ptr<loader::Level>& tr)
{
    world.m_roomBoxes.clear();

    for(size_t i = 0; i < tr->m_boxes.size(); i++)
    {
        world.m_roomBoxes.emplace_back();
        RoomBox& room = world.m_roomBoxes.back();
        room.overlap_index = tr->m_boxes[i].overlap_index;
        room.true_floor = -tr->m_boxes[i].true_floor;
        room.x_min = tr->m_boxes[i].xmin;
        room.x_max = tr->m_boxes[i].xmax;
        room.y_min = -static_cast<int>(tr->m_boxes[i].zmax);
        room.y_max = -static_cast<int>(tr->m_boxes[i].zmin);
    }
}

void TR_GenCameras(World& world, const std::unique_ptr<loader::Level>& tr)
{
    world.m_camerasAndSinks.clear();

    for(size_t i = 0; i < tr->m_cameras.size(); i++)
    {
        world.m_camerasAndSinks.emplace_back();
        world.m_camerasAndSinks[i].position.x = tr->m_cameras[i].x;
        world.m_camerasAndSinks[i].position.y = tr->m_cameras[i].z;
        world.m_camerasAndSinks[i].position.z = -tr->m_cameras[i].y;
        world.m_camerasAndSinks[i].room_or_strength = tr->m_cameras[i].room;
        world.m_camerasAndSinks[i].flag_or_zone = tr->m_cameras[i].unknown1;
    }
}

/**
 * sprites loading, works correct in TR1 - TR5
 */
void TR_GenSprites(World& world, const std::unique_ptr<loader::Level>& tr)
{
    if(tr->m_spriteTextures.empty())
    {
        world.m_sprites.clear();
        return;
    }

    for(size_t i = 0; i < tr->m_spriteTextures.size(); i++)
    {
        world.m_sprites.emplace_back();
        auto s = &world.m_sprites.back();

        auto tr_st = &tr->m_spriteTextures[i];

        s->left = tr_st->left_side;
        s->right = tr_st->right_side;
        s->top = tr_st->top_side;
        s->bottom = tr_st->bottom_side;

        world.m_textureAtlas->getSpriteCoordinates(i, s->texture, s->tex_coord);
    }

    for(uint32_t i = 0; i < tr->m_spriteSequences.size(); i++)
    {
        if(tr->m_spriteSequences[i].offset >= 0 && static_cast<uint32_t>(tr->m_spriteSequences[i].offset) < world.m_sprites.size())
        {
            world.m_sprites[tr->m_spriteSequences[i].offset].id = tr->m_spriteSequences[i].object_id;
        }
    }
}

void Res_GenSpritesBuffer(World& world)
{
    for(auto room : world.m_rooms)
        room->generateSpritesBuffer();
}

void TR_GenTextures(World& world, const std::unique_ptr<loader::Level>& tr)
{
    int border_size = glm::clamp(render::renderer.settings().texture_border, 0, 64);

    world.m_textureAtlas.reset(new BorderedTextureAtlas(border_size,
                                                   render::renderer.settings().save_texture_memory,
                                                   tr->m_textures,
                                                   tr->m_objectTextures,
                                                   tr->m_spriteTextures));

    world.m_textures.resize(world.m_textureAtlas->getNumAtlasPages() + 1);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelZoom(1, 1);
    world.m_textureAtlas->createTextures(world.m_textures.data(), 1);

    // white texture data for coloured polygons and debug lines.
    GLubyte whtx[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    // Select mipmap mode
    switch(render::renderer.settings().mipmap_mode)
    {
        case 0:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            break;

        case 1:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            break;

        case 2:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            break;

        case 3:
        default:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
    };

    // Set mipmaps number
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, render::renderer.settings().mipmaps);

    // Set anisotropy degree
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, render::renderer.settings().anisotropy);

    // Read lod bias
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, render::renderer.settings().lod_bias);

    glBindTexture(GL_TEXTURE_2D, world.m_textures.back());          // solid color =)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, whtx);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, whtx);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whtx);
    //glDisable(GL_TEXTURE_2D); // Why it is here? It is blocking loading screen.
}

/**   Animated textures loading.
  *   Natively, animated textures stored as a stream of bitu16s, which
  *   is then parsed on the fly. What we do is parse this stream to the
  *   proper structures to be used later within renderer.
  */
void TR_GenAnimTextures(World& world, const std::unique_ptr<loader::Level>& tr)
{
    int32_t uvrotate_script = 0;

    core::Polygon uvCoord0;
    uvCoord0.vertices.resize(3);

    core::Polygon uvCoordJ;
    uvCoordJ.vertices.resize(3);

    const uint16_t* pointer = tr->m_animatedTextures.data();
    const uint16_t num_uvrotates = tr->m_animatedTexturesUvCount;

    const uint16_t num_sequences = *pointer++;   // First word in a stream is sequence count.

    world.m_textureAnimations.resize(num_sequences);

    for(size_t i = 0; i < world.m_textureAnimations.size(); i++)
    {
        animation::TextureAnimationSequence* seq = &world.m_textureAnimations[i];
        seq->keyFrames.resize(*pointer++ + 1);
        seq->textureIndices.resize(seq->keyFrames.size());

        // Fill up new sequence with frame list.

        for(size_t& frame : seq->textureIndices)
        {
            frame = *pointer++;  // Add one frame.
        }

        // UVRotate textures case.
        // In TR4-5, it is possible to define special UVRotate animation mode.
        // It is specified by num_uvrotates variable. If sequence belongs to
        // UVRotate range, each frame will be divided in half and continously
        // scrolled from one part to another by shifting UV coordinates.
        // In OpenTomb, we can have BOTH UVRotate and classic frames mode
        // applied to the same sequence, but there we specify compatibility
        // method for TR4-5.

        engine_lua["UVRotate"].get(uvrotate_script);

        if(i < num_uvrotates)
        {
            seq->frame_lock = false; // by default anim is playing

            seq->uvrotate = true;
            // Get texture height and divide it in half.
            // This way, we get a reference value which is used to identify
            // if scrolling is completed or not.
            seq->keyFrames.resize(8);
            seq->uvrotateMax = world.m_textureAtlas->getTextureHeight(seq->textureIndices[0]) / 2;
            seq->uvrotateSpeed = seq->uvrotateMax / static_cast<glm::float_t>(seq->keyFrames.size());
            seq->textureIndices.resize(8);

            if(uvrotate_script > 0)
            {
                seq->textureType = animation::TextureAnimationType::Forward;
            }
            else if(uvrotate_script < 0)
            {
                seq->textureType = animation::TextureAnimationType::Backward;
            }

            engine::Engine::instance.m_world.m_textureAtlas->getCoordinates(seq->textureIndices[0], false, uvCoord0, 0, true);
            for(size_t j = 0; j < seq->keyFrames.size(); j++)
            {
                engine::Engine::instance.m_world.m_textureAtlas->getCoordinates(seq->textureIndices[0], false, uvCoordJ, j * seq->uvrotateSpeed, true);
                seq->keyFrames[j].textureIndex = uvCoordJ.textureIndex;

                ///@PARANOID: texture transformation may be not only move

                glm::mat2 textureSize0;
                textureSize0[0] = uvCoord0.vertices[1].tex_coord - uvCoord0.vertices[0].tex_coord;
                textureSize0[1] = uvCoord0.vertices[2].tex_coord - uvCoord0.vertices[0].tex_coord;

                glm::mat2 textureSizeJ;
                textureSizeJ[0] = uvCoordJ.vertices[1].tex_coord - uvCoordJ.vertices[0].tex_coord;
                textureSizeJ[1] = uvCoordJ.vertices[2].tex_coord - uvCoordJ.vertices[0].tex_coord;

                glm::float_t d = glm::determinant(textureSize0);

                seq->keyFrames[j].coordinateTransform[0][0] = (textureSizeJ[0][0] * textureSize0[1][1] - textureSize0[0][1] * textureSizeJ[1][0]) / d;
                seq->keyFrames[j].coordinateTransform[1][0] = -(textureSizeJ[0][1] * textureSize0[1][1] - textureSize0[0][1] * textureSizeJ[1][1]) / d;
                seq->keyFrames[j].coordinateTransform[0][1] = -(textureSize0[0][0] * textureSizeJ[1][0] - textureSizeJ[0][0] * textureSize0[1][0]) / d;
                seq->keyFrames[j].coordinateTransform[1][1] = (textureSize0[0][0] * textureSizeJ[1][1] - textureSizeJ[0][1] * textureSize0[1][0]) / d;

                seq->keyFrames[j].move.x = uvCoordJ.vertices[0].tex_coord[0] - glm::dot(uvCoord0.vertices[0].tex_coord, seq->keyFrames[j].coordinateTransform[0]);
                seq->keyFrames[j].move.y = uvCoordJ.vertices[0].tex_coord[1] - glm::dot(uvCoord0.vertices[0].tex_coord, seq->keyFrames[j].coordinateTransform[1]);
            }
        }
        else
        {
            engine::Engine::instance.m_world.m_textureAtlas->getCoordinates(seq->textureIndices[0], false, uvCoord0);
            for(size_t j = 0; j < seq->keyFrames.size(); j++)
            {
                engine::Engine::instance.m_world.m_textureAtlas->getCoordinates(seq->textureIndices[j], false, uvCoordJ);
                seq->keyFrames[j].textureIndex = uvCoordJ.textureIndex;

                ///@PARANOID: texture transformation may be not only move

                glm::mat2 textureSize0;
                textureSize0[0] = uvCoord0.vertices[1].tex_coord - uvCoord0.vertices[0].tex_coord;
                textureSize0[1] = uvCoord0.vertices[2].tex_coord - uvCoord0.vertices[0].tex_coord;

                glm::mat2 textureSizeJ;
                textureSizeJ[0] = uvCoordJ.vertices[1].tex_coord - uvCoordJ.vertices[0].tex_coord;
                textureSizeJ[1] = uvCoordJ.vertices[2].tex_coord - uvCoordJ.vertices[0].tex_coord;

                glm::float_t d = glm::determinant(textureSize0);

                seq->keyFrames[j].coordinateTransform[0][0] = (textureSizeJ[0][0] * textureSize0[1][1] - textureSize0[0][1] * textureSizeJ[1][0]) / d;
                seq->keyFrames[j].coordinateTransform[1][0] = -(textureSizeJ[0][1] * textureSize0[1][1] - textureSize0[0][1] * textureSizeJ[1][1]) / d;
                seq->keyFrames[j].coordinateTransform[0][1] = -(textureSize0[0][0] * textureSizeJ[1][0] - textureSizeJ[0][0] * textureSize0[1][0]) / d;
                seq->keyFrames[j].coordinateTransform[1][1] = (textureSize0[0][0] * textureSizeJ[1][1] - textureSizeJ[0][1] * textureSize0[1][0]) / d;

                seq->keyFrames[j].move.x = uvCoordJ.vertices[0].tex_coord[0] - glm::dot(uvCoord0.vertices[0].tex_coord, seq->keyFrames[j].coordinateTransform[0]);
                seq->keyFrames[j].move.y = uvCoordJ.vertices[0].tex_coord[1] - glm::dot(uvCoord0.vertices[0].tex_coord, seq->keyFrames[j].coordinateTransform[1]);
            }
        }
    }
}

/**   Assign animated texture to a polygon.
  *   While in original TRs we had TexInfo abstraction layer to refer texture,
  *   in OpenTomb we need to re-think animated texture concept to work on a
  *   per-polygon basis. For this, we scan all animated texture lists for
  *   same TexInfo index that is applied to polygon, and if corresponding
  *   animation list is found, we assign it to polygon.
  */
bool SetAnimTexture(core::Polygon& polygon, uint32_t tex_index, const World& world)
{
    polygon.textureAnimationId.reset();

    for(size_t i = 0; i < world.m_textureAnimations.size(); i++)
    {
        for(size_t j = 0; j < world.m_textureAnimations[i].keyFrames.size(); j++)
        {
            if(world.m_textureAnimations[i].textureIndices[j] != tex_index)
                continue;

            // If we have found assigned texture ID in animation texture lists,
            // we assign corresponding animation sequence to this polygon,
            // additionally specifying frame offset.
            polygon.textureAnimationId = i;  // Animation sequence ID.
            polygon.startFrame = j;     // Animation frame offset.
            return true;
        }
    }

    return false;   // No such TexInfo found in animation textures lists.
}

void TR_GenMeshes(World& world, const std::unique_ptr<loader::Level>& tr)
{
    world.m_meshes.resize(tr->m_meshes.size());
    ObjectId i = 0;
    for(std::shared_ptr<core::BaseMesh>& baseMesh : world.m_meshes)
    {
        baseMesh = std::make_shared<core::BaseMesh>();
        TR_GenMesh(world, i++, baseMesh, tr);
    }
}

void tr_copyNormals(core::Polygon& polygon, const core::BaseMesh& mesh, const uint16_t *mesh_vertex_indices)
{
    for(size_t i = 0; i < polygon.vertices.size(); ++i)
    {
        polygon.vertices[i].normal = mesh.m_vertices[mesh_vertex_indices[i]].normal;
    }
}

void tr_accumulateNormals(const loader::Mesh& tr_mesh, core::BaseMesh& mesh, int numCorners, const uint16_t *vertex_indices, core::Polygon& p)
{
    p.vertices.resize(numCorners);

    for(int i = 0; i < numCorners; i++)
    {
        p.vertices[i].position = util::convert(tr_mesh.vertices[vertex_indices[i]]);
    }
    p.updateNormal();

    for(int i = 0; i < numCorners; i++)
    {
        mesh.m_vertices[vertex_indices[i]].normal += p.plane.normal;
    }
}

void tr_setupColoredFace(const loader::Mesh& tr_mesh, const std::unique_ptr<loader::Level>& tr, core::BaseMesh& mesh, const uint16_t *vertex_indices, unsigned color, core::Polygon& p)
{
    for(size_t i = 0; i < p.vertices.size(); i++)
    {
        p.vertices[i].color[0] = tr->m_palette->color[color].r / 255.0f;
        p.vertices[i].color[1] = tr->m_palette->color[color].g / 255.0f;
        p.vertices[i].color[2] = tr->m_palette->color[color].b / 255.0f;
        if(tr_mesh.lights.size() == tr_mesh.vertices.size())
        {
            p.vertices[i].color[0] = p.vertices[i].color[0] * 1.0f - tr_mesh.lights[vertex_indices[i]] / 8192.0f;
            p.vertices[i].color[1] = p.vertices[i].color[1] * 1.0f - tr_mesh.lights[vertex_indices[i]] / 8192.0f;
            p.vertices[i].color[2] = p.vertices[i].color[2] * 1.0f - tr_mesh.lights[vertex_indices[i]] / 8192.0f;
        }
        p.vertices[i].color[3] = 1.0f;

        p.vertices[i].tex_coord[0] = (i & 2) ? 1.0f : 0.0f;
        p.vertices[i].tex_coord[1] = i >= 2 ? 1.0f : 0.0f;
    }
    mesh.m_usesVertexColors = true;
}

void tr_setupTexturedFace(const loader::Mesh& tr_mesh, core::BaseMesh& mesh, const uint16_t *vertex_indices, core::Polygon& p)
{
    for(size_t i = 0; i < p.vertices.size(); i++)
    {
        if(tr_mesh.lights.size() != tr_mesh.vertices.size())
        {
            p.vertices[i].color = { 1,1,1,1 };
            continue;
        }

        p.vertices[i].color[0] = 1.0f - tr_mesh.lights[vertex_indices[i]] / 8192.0f;
        p.vertices[i].color[1] = 1.0f - tr_mesh.lights[vertex_indices[i]] / 8192.0f;
        p.vertices[i].color[2] = 1.0f - tr_mesh.lights[vertex_indices[i]] / 8192.0f;
        p.vertices[i].color[3] = 1.0f;

        mesh.m_usesVertexColors = true;
    }
}

void TR_GenMesh(World& world, ObjectId mesh_index, std::shared_ptr<core::BaseMesh> mesh, const std::unique_ptr<loader::Level>& tr)
{
    const uint32_t tex_mask = world.m_engineVersion == loader::Engine::TR4 ? loader::TextureIndexMaskTr4 : loader::TextureIndexMask;

    /* TR WAD FORMAT DOCUMENTATION!
     * tr4_face[3,4]_t:
     * flipped texture & 0x8000 (1 bit  ) - horizontal flipping.
     * shape texture   & 0x7000 (3 bits ) - texture sample shape.
     * index texture   & $0FFF  (12 bits) - texture sample index.
     *
     * if bit [15] is set, as in ( texture and $8000 ), it indicates that the texture
     * sample must be flipped horizontally prior to be used.
     * Bits [14..12] as in ( texture and $7000 ), are used to store the texture
     * shape, given by: ( texture and $7000 ) shr 12.
     * The valid values are: 0, 2, 4, 6, 7, as assigned to a square starting from
     * the top-left corner and going clockwise: 0, 2, 4, 6 represent the positions
     * of the square angle of the triangles, 7 represents a quad.
     */

    const loader::Mesh& tr_mesh = tr->m_meshes[mesh_index];
    mesh->m_center[0] = tr_mesh.centre.x;
    mesh->m_center[1] = -tr_mesh.centre.z;
    mesh->m_center[2] = tr_mesh.centre.y;
    mesh->m_radius = tr_mesh.collision_size;
    mesh->m_texturePageCount = world.m_textureAtlas->getNumAtlasPages() + 1;

    mesh->m_vertices.resize(tr_mesh.vertices.size());
    auto vertex = mesh->m_vertices.data();
    for(size_t i = 0; i < mesh->m_vertices.size(); i++, vertex++)
    {
        vertex->position = util::convert(tr_mesh.vertices[i]);
        vertex->normal = { 0,0,0 };                                          // paranoid
    }

    mesh->updateBoundingBox();

    mesh->m_polygons.clear();

    /*
     * textured triangles
     */
    for(size_t i = 0; i < tr_mesh.textured_triangles.size(); ++i)
    {
        mesh->m_polygons.emplace_back();
        core::Polygon &p = mesh->m_polygons.back();

        auto face3 = &tr_mesh.textured_triangles[i];
        auto tex = &tr->m_objectTextures[face3->texture & tex_mask];

        p.isDoubleSided = (face3->texture & 0x8000) != 0;    // CORRECT, BUT WRONG IN TR3-5

        SetAnimTexture(p, face3->texture & tex_mask, world);

        if(face3->lighting & 0x01)
        {
            p.blendMode = loader::BlendingMode::Multiply;
        }
        else
        {
            p.blendMode = tex->transparency_flags;
        }

        tr_accumulateNormals(tr_mesh, *mesh, 3, face3->vertices, p);
        tr_setupTexturedFace(tr_mesh, *mesh, face3->vertices, p);

        world.m_textureAtlas->getCoordinates(face3->texture & tex_mask, 0, p);
    }

    /*
     * coloured triangles
     */
    for(size_t i = 0; i < tr_mesh.coloured_triangles.size(); ++i)
    {
        mesh->m_polygons.emplace_back();
        core::Polygon &p = mesh->m_polygons.back();

        auto face3 = &tr_mesh.coloured_triangles[i];
        auto col = face3->texture & 0xff;
        p.textureIndex = world.m_textureAtlas->getNumAtlasPages();
        p.blendMode = loader::BlendingMode::Opaque;
        p.textureAnimationId.reset();

        tr_accumulateNormals(tr_mesh, *mesh, 3, face3->vertices, p);
        tr_setupColoredFace(tr_mesh, tr, *mesh, face3->vertices, col, p);
    }

    /*
     * textured rectangles
     */
    for(size_t i = 0; i < tr_mesh.textured_rectangles.size(); ++i)
    {
        mesh->m_polygons.emplace_back();
        core::Polygon &p = mesh->m_polygons.back();

        auto face4 = &tr_mesh.textured_rectangles[i];
        auto tex = &tr->m_objectTextures[face4->texture & tex_mask];

        p.isDoubleSided = (face4->texture & 0x8000) != 0;    // CORRECT, BUT WRONG IN TR3-5

        SetAnimTexture(p, face4->texture & tex_mask, world);

        if(face4->lighting & 0x01)
        {
            p.blendMode = loader::BlendingMode::Multiply;
        }
        else
        {
            p.blendMode = tex->transparency_flags;
        }

        tr_accumulateNormals(tr_mesh, *mesh, 4, face4->vertices, p);
        tr_setupTexturedFace(tr_mesh, *mesh, face4->vertices, p);

        world.m_textureAtlas->getCoordinates(face4->texture & tex_mask, 0, p);
    }

    /*
     * coloured rectangles
     */
    for(size_t i = 0; i < tr_mesh.coloured_rectangles.size(); i++)
    {
        mesh->m_polygons.emplace_back();
        core::Polygon &p = mesh->m_polygons.back();

        auto face4 = &tr_mesh.coloured_rectangles[i];
        auto col = face4->texture & 0xff;
        p.vertices.resize(4);
        p.textureIndex = world.m_textureAtlas->getNumAtlasPages();
        p.blendMode = loader::BlendingMode::Opaque;
        p.textureAnimationId.reset();

        tr_accumulateNormals(tr_mesh, *mesh, 4, face4->vertices, p);
        tr_setupColoredFace(tr_mesh, tr, *mesh, face4->vertices, col, p);
    }

    /*
     * let us normalise normales %)
     */
    for(core::Vertex& v : mesh->m_vertices)
    {
        v.normal = glm::normalize(v.normal);
    }

    /*
     * triangles
     */
    auto p = mesh->m_polygons.begin();
    for(size_t i = 0; i < tr_mesh.textured_triangles.size(); i++, ++p)
    {
        tr_copyNormals(*p, *mesh, tr_mesh.textured_triangles[i].vertices);
    }

    for(size_t i = 0; i < tr_mesh.coloured_triangles.size(); i++, ++p)
    {
        tr_copyNormals(*p, *mesh, tr_mesh.coloured_triangles[i].vertices);
    }

    /*
     * rectangles
     */
    for(size_t i = 0; i < tr_mesh.textured_rectangles.size(); i++, ++p)
    {
        tr_copyNormals(*p, *mesh, tr_mesh.textured_rectangles[i].vertices);
    }

    for(size_t i = 0; i < tr_mesh.coloured_rectangles.size(); i++, ++p)
    {
        tr_copyNormals(*p, *mesh, tr_mesh.coloured_rectangles[i].vertices);
    }

    mesh->m_vertices.clear();
    mesh->genFaces();
    mesh->polySortInMesh();
}

void tr_setupRoomVertices(World& world, const std::unique_ptr<loader::Level>& tr, loader::Room& tr_room, const std::shared_ptr<core::BaseMesh>& mesh, int numCorners, const uint16_t *vertices, uint16_t masked_texture, core::Polygon& p)
{
    p.vertices.resize(numCorners);

    for(int i = 0; i < numCorners; i++)
    {
        p.vertices[i].position = util::convert(tr_room.vertices[vertices[i]].vertex);
    }
    p.updateNormal();

    for(int i = 0; i < numCorners; i++)
    {
        mesh->m_vertices[vertices[i]].normal += p.plane.normal;
        p.vertices[i].normal = p.plane.normal;
        p.vertices[i].color = util::convert(tr_room.vertices[vertices[i]].colour);
    }

    loader::ObjectTexture *tex = &tr->m_objectTextures[masked_texture];
    SetAnimTexture(p, masked_texture, world);
    p.blendMode = tex->transparency_flags;

    world.m_textureAtlas->getCoordinates(masked_texture, 0, p);
}

void Res_GenVBOs(World& world)
{
    for(uint32_t i = 0; i < world.m_meshes.size(); i++)
    {
        if(world.m_meshes[i]->m_vertices.empty() && !world.m_meshes[i]->m_animatedVertices.empty())
            continue;

        world.m_meshes[i]->genVBO();
    }

    for(uint32_t i = 0; i < world.m_rooms.size(); i++)
    {
        if(world.m_rooms[i]->getMesh() && (!world.m_rooms[i]->getMesh()->m_vertices.empty() || !world.m_rooms[i]->getMesh()->m_animatedVertices.empty()))
        {
            world.m_rooms[i]->getMesh()->genVBO();
        }
    }
}

void Res_GenBaseItems(World& world)
{
    engine_lua["genBaseItems"]();

    if(!world.m_items.empty())
    {
        Res_EntityToItem(world.m_items);
    }
}

void Res_FixRooms(World& world)
{
    for(size_t i = 0; i < world.m_rooms.size(); i++)
    {
        auto r = world.m_rooms[i];
        if(r->getBaseRoom() != nullptr)
        {
            r->disable();    // Disable current room
        }

        // Isolated rooms may be used for rolling ball trick (for ex., LEVEL4.PHD).
        // Hence, this part is commented.

        /*
        if((r->portal_count == 0) && (world.room_count > 1))
        {
            Room_Disable(r);
        }
        */
    }
}

long int TR_GetOriginalAnimationFrameOffset(uint32_t offset, uint32_t anim, const std::unique_ptr<loader::Level>& tr)
{
    loader::Animation *tr_animation;

    if(anim >= tr->m_animations.size())
    {
        return -1;
    }

    tr_animation = &tr->m_animations[anim];
    if(anim + 1 == tr->m_animations.size())
    {
        if(offset < tr_animation->frame_offset)
        {
            return -2;
        }
    }
    else
    {
        if(offset < tr_animation->frame_offset && offset >= (tr_animation + 1)->frame_offset)
        {
            return -2;
        }
    }

    return tr_animation->frame_offset;
}

std::shared_ptr<SkeletalModel> Res_GetSkybox(World& world)
{
    switch(world.m_engineVersion)
    {
        case loader::Engine::TR2:
            return world.getModelByID(TR_ITEM_SKYBOX_TR2);

        case loader::Engine::TR3:
            return world.getModelByID(TR_ITEM_SKYBOX_TR3);

        case loader::Engine::TR4:
            return world.getModelByID(TR_ITEM_SKYBOX_TR4);

        case loader::Engine::TR5:
            return world.getModelByID(TR_ITEM_SKYBOX_TR5);

        default:
            return nullptr;
    }
}

void TR_GenAnimCommands(World& world, const std::unique_ptr<loader::Level>& tr)
{
    world.m_animCommands = std::move(tr->m_animCommands);
}

void TR_GenSkeletalModel(World& world, size_t model_num, SkeletalModel& model, const std::unique_ptr<loader::Level>& tr, size_t meshCount)
{
    const std::unique_ptr<loader::Moveable>& tr_moveable = tr->m_moveables[model_num];  // original tr structure

    model.m_collisionMap.resize(model.m_meshes.size());
    std::iota(model.m_collisionMap.begin(), model.m_collisionMap.end(), 0);

    model.m_meshes.resize(meshCount);

    const uint32_t *mesh_index = &tr->m_meshIndices[tr_moveable->starting_mesh];

    for(size_t k = 0; k < model.m_meshes.size(); k++)
    {
        SkeletalModel::MeshReference* tree_tag = &model.m_meshes[k];
        tree_tag->mesh_base = world.m_meshes[mesh_index[k]];
        if(k == 0)
        {
            tree_tag->flag = 0x02;
            continue;
        }

        uint32_t *tr_mesh_tree = tr->m_meshTreeData.data() + tr_moveable->mesh_tree_index + (k - 1) * 4;
        tree_tag->flag = tr_mesh_tree[0] & 0xFF;
        tree_tag->offset[0] = static_cast<float>(static_cast<int32_t>(tr_mesh_tree[1]));
        tree_tag->offset[1] = static_cast<float>(static_cast<int32_t>(tr_mesh_tree[3]));
        tree_tag->offset[2] = -static_cast<float>(static_cast<int32_t>(tr_mesh_tree[2]));
    }

    /*
     * =================    now, animation loading    ========================
     */

    if(tr_moveable->animation_index >= tr->m_animations.size())
    {
        /*
         * model has no start offset and any animation
         */
        model.m_animations.resize(1);
        model.m_animations.front().setDuration(1, 1, 1);
        animation::SkeletonKeyFrame& keyFrame = model.m_animations.front().rawKeyFrame(0);

        model.m_animations.front().id = 0;
        model.m_animations.front().next_anim = nullptr;
        model.m_animations.front().next_frame = 0;
        model.m_animations.front().stateChanges.clear();

        keyFrame.boneKeyFrames.resize(model.m_meshes.size());

        keyFrame.position = { 0,0,0 };

        for(size_t k = 0; k < keyFrame.boneKeyFrames.size(); k++)
        {
            SkeletalModel::MeshReference* mesh = &model.m_meshes[k];
            animation::BoneKeyFrame& boneKeyFrame = keyFrame.boneKeyFrames[k];

            boneKeyFrame.qrotate = util::vec4_SetTRRotations({ 0,0,0 });
            boneKeyFrame.offset = mesh->offset;
        }
        return;
    }
    //Sys_DebugLog(LOG_FILENAME, "model = %d, anims = %d", tr_moveable->object_id, GetNumAnimationsForMoveable(tr, model_num));
    model.m_animations.resize(TR_GetNumAnimationsForMoveable(tr, model_num));
    if(model.m_animations.empty())
    {
        /*
         * the animation count must be >= 1
         */
        model.m_animations.resize(1);
    }

    /*
     *   Ok, let us calculate animations;
     *   there is no difficult:
     * - first 9 words are bounding box and frame offset coordinates.
     * - 10's word is a rotations count, must be equal to number of meshes in model.
     *   BUT! only in TR1. In TR2 - TR5 after first 9 words begins next section.
     * - in the next follows rotation's data. one word - one rotation, if rotation is one-axis (one angle).
     *   two words in 3-axis rotations (3 angles). angles are calculated with bit mask.
     */
    for(size_t i = 0; i < model.m_animations.size(); i++)
    {
        animation::Animation* anim = &model.m_animations[i];
        loader::Animation *tr_animation = &tr->m_animations[tr_moveable->animation_index + i];

        uint32_t frame_offset = tr_animation->frame_offset / 2;
        uint16_t l_start = 0x09;

        if(tr->m_gameVersion == loader::Game::TR1 || tr->m_gameVersion == loader::Game::TR1Demo || tr->m_gameVersion == loader::Game::TR1UnfinishedBusiness)
        {
            l_start = 0x0A;
        }

        anim->id = i;
        BOOST_LOG_TRIVIAL(debug) << "Anim " << i << " stretch factor = " << int(tr_animation->frame_rate) << ", frame count = " << (tr_animation->frame_end - tr_animation->frame_start + 1);

        anim->speed_x = tr_animation->speed;
        anim->accel_x = tr_animation->accel;
        anim->speed_y = tr_animation->accel_lateral;
        anim->accel_y = tr_animation->speed_lateral;

        anim->animationCommand = tr_animation->anim_command;
        anim->animationCommandCount = tr_animation->num_anim_commands;
        anim->state_id = static_cast<LaraState>(tr_animation->state_id);

        //        anim->frames.resize(TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index + i));
                // FIXME: number of frames is always (frame_end - frame_start + 1)
                // this matters for transitional anims, which run through their frame length with the same anim frame.
                // This should ideally be solved without filling identical frames,
                // but due to the amount of currFrame-indexing, waste dummy frames for now:
                // (I haven't seen this for framerate==1 animation, but it would be possible,
                //  also, resizing now saves re-allocations in interpolateFrames later)

        const size_t keyFrameCount = TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index + i);
        anim->setDuration(tr_animation->frame_end - tr_animation->frame_start + 1, keyFrameCount, tr_animation->frame_rate);

        //Sys_DebugLog(LOG_FILENAME, "Anim[%d], %d", tr_moveable->animation_index, TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index));

        // Parse AnimCommands
        // Max. amount of AnimCommands is 255, larger numbers are considered as 0.
        // See http://evpopov.com/dl/TR4format.html#Animations for details.
        // FIXME: This is done here only to adjust relative frame indices
        //        see GenerateAnimCommands()
        if(anim->animationCommandCount > 0 && anim->animationCommandCount <= 255)
        {
            // Calculate current animation anim command block offset.
            BOOST_ASSERT(anim->animationCommand < world.m_animCommands.size());
            int16_t *pointer = &world.m_animCommands[anim->animationCommand];

            for(size_t count = 0; count < anim->animationCommandCount; count++)
            {
                const auto command = static_cast<animation::AnimCommandOpcode>(*pointer);
                ++pointer;
                switch(command)
                {
                    case animation::AnimCommandOpcode::PlayEffect:
                    case animation::AnimCommandOpcode::PlaySound:
                        // Recalculate absolute frame number to relative.
                        pointer[0] -= tr_animation->frame_start;
                        pointer += 2;
                        break;

                    case animation::AnimCommandOpcode::SetPosition:
                        // Parse through 3 operands.
                        pointer += 3;
                        break;

                    case animation::AnimCommandOpcode::SetVelocity:
                        // Parse through 2 operands.
                        pointer += 2;
                        break;

                    default:
                        // All other commands have no operands.
                        break;
                }
            }
        }

        if(anim->getFrameDuration() == 0)
        {
            /*
             * number of animations must be >= 1, because frame contains base model offset
             */
            anim->setDuration(1, 1, anim->getStretchFactor());
        }

        /*
         * let us begin to load animations
         */
        for(size_t j = 0; j < anim->getKeyFrameCount(); ++j, frame_offset += tr_animation->frame_size)
        {
            animation::SkeletonKeyFrame* keyFrame = &anim->rawKeyFrame(j);
            // !Need bonetags in empty frames:
            keyFrame->boneKeyFrames.resize(model.m_meshes.size());

            if(j >= keyFrameCount)
            {
                BOOST_LOG_TRIVIAL(warning) << "j=" << j << ", keyFrameCount=" << keyFrameCount << ", anim->getKeyFrameCount()=" << anim->getKeyFrameCount();
                continue; // only create bone_tags for rate>1 fill-frames
            }

            keyFrame->position = { 0,0,0 };
            TR_GetBFrameBB_Pos(tr, frame_offset, *keyFrame);

            if(frame_offset >= tr->m_frameData.size())
            {
                for(size_t k = 0; k < keyFrame->boneKeyFrames.size(); k++)
                {
                    animation::BoneKeyFrame* boneKeyFrame = &keyFrame->boneKeyFrames[k];
                    boneKeyFrame->qrotate = util::vec4_SetTRRotations({ 0,0,0 });
                    boneKeyFrame->offset = model.m_meshes[k].offset;
                }
                continue;
            }

            uint16_t l = l_start;
            uint16_t temp1, temp2;
            float ang;

            for(size_t k = 0; k < keyFrame->boneKeyFrames.size(); k++)
            {
                animation::BoneKeyFrame* bone_tag = &keyFrame->boneKeyFrames[k];
                bone_tag->qrotate = util::vec4_SetTRRotations({ 0,0,0 });
                bone_tag->offset = model.m_meshes[k].offset;

                if(loader::gameToEngine(tr->m_gameVersion) == loader::Engine::TR1)
                {
                    temp2 = tr->m_frameData[frame_offset + l];
                    l++;
                    temp1 = tr->m_frameData[frame_offset + l];
                    l++;
                    glm::vec3 rot;
                    rot[0] = static_cast<float>((temp1 & 0x3ff0) >> 4);
                    rot[2] = -static_cast<float>(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                    rot[1] = static_cast<float>(temp2 & 0x03ff);
                    rot *= 360.0 / 1024.0;
                    bone_tag->qrotate = util::vec4_SetTRRotations(rot);
                }
                else
                {
                    temp1 = tr->m_frameData[frame_offset + l];
                    l++;
                    if(tr->m_gameVersion >= loader::Game::TR4)
                    {
                        ang = static_cast<float>(temp1 & 0x0fff);
                        ang *= 360.0 / 4096.0;
                    }
                    else
                    {
                        ang = static_cast<float>(temp1 & 0x03ff);
                        ang *= 360.0 / 1024.0;
                    }

                    switch(temp1 & 0xc000)
                    {
                        case 0x4000:    // x only
                            bone_tag->qrotate = util::vec4_SetTRRotations({ ang,0,0 });
                            break;

                        case 0x8000:    // y only
                            bone_tag->qrotate = util::vec4_SetTRRotations({ 0,0,-ang });
                            break;

                        case 0xc000:    // z only
                            bone_tag->qrotate = util::vec4_SetTRRotations({ 0,ang,0 });
                            break;

                        default:
                        {        // all three
                            temp2 = tr->m_frameData[frame_offset + l];
                            glm::vec3 rot;
                            rot[0] = static_cast<float>((temp1 & 0x3ff0) >> 4);
                            rot[2] = -static_cast<float>(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                            rot[1] = static_cast<float>(temp2 & 0x03ff);
                            rot *= 360.0 / 1024.0;
                            bone_tag->qrotate = util::vec4_SetTRRotations(rot);
                            l++;
                            break;
                        }
                    };
                }
            }
        }
    }

    /*
     * state change's loading
     */
#ifdef LOG_ANIM_DISPATCHES
    if(model.animations.size() > 1)
    {
        BOOST_LOG_TRIVIAL(debug) << "MODEL[" << model_num << "], anims = " << model.animations.size();
    }
#endif
    for(size_t i = 0; i < model.m_animations.size(); i++)
    {
        animation::Animation* anim = &model.m_animations[i];
        anim->stateChanges.clear();

        loader::Animation *tr_animation = &tr->m_animations[tr_moveable->animation_index + i];
        int16_t animId = tr_animation->next_animation - tr_moveable->animation_index;
        animId &= 0x7fff; // this masks out the sign bit
        BOOST_ASSERT(animId >= 0);
        if(static_cast<size_t>(animId) < model.m_animations.size())
        {
            anim->next_anim = &model.m_animations[animId];
            anim->next_frame = tr_animation->next_frame - tr->m_animations[tr_animation->next_animation].frame_start;
            anim->next_frame %= anim->next_anim->getFrameDuration();
#ifdef LOG_ANIM_DISPATCHES
            BOOST_LOG_TRIVIAL(debug) << "ANIM[" << i << "], next_anim = " << anim->next_anim->id << ", next_frame = " << anim->next_frame;
#endif
        }
        else
        {
            anim->next_anim = nullptr;
            anim->next_frame = 0;
        }

        anim->stateChanges.clear();

        if(tr_animation->num_state_changes > 0 && model.m_animations.size() > 1)
        {
#ifdef LOG_ANIM_DISPATCHES
            BOOST_LOG_TRIVIAL(debug) << "ANIM[" << i << "], next_anim = " << (anim->next_anim ? anim->next_anim->id : -1) << ", next_frame = " << anim->next_frame;
#endif
            for(uint16_t j = 0; j < tr_animation->num_state_changes; j++)
            {
                loader::StateChange *tr_sch = &tr->m_stateChanges[j + tr_animation->state_change_offset];
                if(anim->findStateChangeByID(static_cast<LaraState>(tr_sch->state_id)) != nullptr)
                {
                    BOOST_LOG_TRIVIAL(warning) << "Multiple state changes for id " << tr_sch->state_id;
                }
                animation::StateChange* sch_p = &anim->stateChanges[static_cast<LaraState>(tr_sch->state_id)];
                sch_p->id = static_cast<LaraState>(tr_sch->state_id);
                sch_p->dispatches.clear();
                for(uint16_t l = 0; l < tr_sch->num_anim_dispatches; l++)
                {
                    loader::AnimDispatch *tr_adisp = &tr->m_animDispatches[tr_sch->anim_dispatch + l];
                    uint16_t next_anim = tr_adisp->next_animation & 0x7fff;
                    uint16_t next_anim_ind = next_anim - (tr_moveable->animation_index & 0x7fff);
                    if(next_anim_ind >= model.m_animations.size())
                        continue;

                    sch_p->dispatches.emplace_back();

                    animation::AnimationDispatch* adsp = &sch_p->dispatches.back();
                    size_t next_frames_count = model.m_animations[next_anim - tr_moveable->animation_index].getFrameDuration();
                    size_t next_frame = tr_adisp->next_frame - tr->m_animations[next_anim].frame_start;

                    uint16_t low = tr_adisp->low - tr_animation->frame_start;
                    uint16_t high = tr_adisp->high - tr_animation->frame_start;

                    // this is not good: frame_high can be frame_end+1 (for last-frame-loop statechanges,
                    // secifically fall anims (75,77 etc), which may fail to change state),
                    // And: if theses values are > framesize, then they're probably faulty and won't be fixed by modulo anyway:
//                        adsp->frame_low = low  % anim->frames.size();
//                        adsp->frame_high = (high - 1) % anim->frames.size();
                    if(low > anim->getFrameDuration() || high > anim->getFrameDuration())
                    {
                        //Sys_Warn("State range out of bounds: anim: %d, stid: %d, low: %d, high: %d", anim->id, sch_p->id, low, high);
                        Console::instance().printf("State range out of bounds: anim: %d, stid: %d, low: %d, high: %d, duration: %d, timestretch: %d", anim->id, sch_p->id, low, high, int(anim->getFrameDuration()), int(anim->getStretchFactor()));
                    }
                    adsp->start = low;
                    adsp->end = high;
                    BOOST_ASSERT(next_anim >= tr_moveable->animation_index);
                    adsp->next.animation = next_anim - tr_moveable->animation_index;
                    adsp->next.frame = next_frame % next_frames_count;

#ifdef LOG_ANIM_DISPATCHES
                    BOOST_LOG_TRIVIAL(debug) << "anim_disp["
                        << l
                        << "], duration = "
                        << anim->getFrameDuration()
                        << ": interval["
                        << adsp->frame_low
                        << ".."
                        << adsp->frame_high
                        << "], next_anim = "
                        << adsp->next.animation
                        << ", next_frame = "
                        << adsp->next.frame;
#endif
                }
            }
        }
    }
    GenerateAnimCommands(model);
}

size_t TR_GetNumAnimationsForMoveable(const std::unique_ptr<loader::Level>& tr, size_t moveable_ind)
{
    const std::unique_ptr<loader::Moveable>& curr_moveable = tr->m_moveables[moveable_ind];

    if(curr_moveable->animation_index == 0xFFFF)
    {
        return 0;
    }

    if(moveable_ind == tr->m_moveables.size() - 1)
    {
        if(tr->m_animations.size() < curr_moveable->animation_index)
        {
            return 1;
        }

        return tr->m_animations.size() - curr_moveable->animation_index;
    }

    const loader::Moveable* next_moveable = tr->m_moveables[moveable_ind + 1].get();
    if(next_moveable->animation_index == 0xFFFF)
    {
        if(moveable_ind + 2 < tr->m_moveables.size())                              // I hope there is no two neighboard movables with animation_index'es == 0xFFFF
        {
            next_moveable = tr->m_moveables[moveable_ind + 2].get();
        }
        else
        {
            return 1;
        }
    }

    return std::min(static_cast<size_t>(next_moveable->animation_index), tr->m_animations.size()) - curr_moveable->animation_index;
}

// Returns real animation frame count

size_t TR_GetNumFramesForAnimation(const std::unique_ptr<loader::Level>& tr, size_t animation_ind)
{
    loader::Animation* curr_anim = &tr->m_animations[animation_ind];
    if(curr_anim->frame_size <= 0)
    {
        return 1;                                                               // impossible!
    }

    if(animation_ind == tr->m_animations.size() - 1)
    {
        size_t ret = 2 * tr->m_frameData.size() - curr_anim->frame_offset;
        ret /= curr_anim->frame_size * 2;                                       /// it is fully correct!
        return ret;
    }

    loader::Animation* next_anim = &tr->m_animations[animation_ind + 1];
    size_t ret = next_anim->frame_offset - curr_anim->frame_offset;
    ret /= curr_anim->frame_size * 2;

    return ret;
}

void TR_GetBFrameBB_Pos(const std::unique_ptr<loader::Level>& tr, size_t frame_offset, animation::SkeletonKeyFrame& keyFrame)
{
    if(frame_offset < tr->m_frameData.size())
    {
        const int16_t* frame = &tr->m_frameData[frame_offset];

        keyFrame.boundingBox.min[0] = frame[0];   // x_min
        keyFrame.boundingBox.min[1] = frame[4];   // y_min
        keyFrame.boundingBox.min[2] = -frame[3];  // z_min

        keyFrame.boundingBox.max[0] = frame[1];   // x_max
        keyFrame.boundingBox.max[1] = frame[5];   // y_max
        keyFrame.boundingBox.max[2] = -frame[2];  // z_max

        keyFrame.position[0] = frame[6];
        keyFrame.position[1] = frame[8];
        keyFrame.position[2] = -frame[7];
    }
    else
    {
        BOOST_LOG_TRIVIAL(warning) << "Reading animation data beyond end of frame data: offset = " << frame_offset << ", size = " << tr->m_frameData.size();
        keyFrame.boundingBox.min = { 0,0,0 };
        keyFrame.boundingBox.max = { 0,0,0 };
        keyFrame.position = { 0,0,0 };
    }
}

void TR_GenSkeletalModels(World& world, const std::unique_ptr<loader::Level>& tr)
{
    for(size_t i = 0; i < tr->m_moveables.size(); i++)
    {
        const loader::Moveable& tr_moveable = *tr->m_moveables[i];
        std::shared_ptr<SkeletalModel> smodel = std::make_shared<SkeletalModel>(tr_moveable.object_id);
        TR_GenSkeletalModel(world, i, *smodel, tr, tr_moveable.num_meshes);
        smodel->updateTransparencyFlag();
        world.m_skeletalModels[smodel->getId()] = smodel;
    }
}

void TR_GenEntities(World& world, const std::unique_ptr<loader::Level>& tr)
{
    for(ObjectId i = 0; i < tr->m_items.size(); i++)
    {
        loader::Item *tr_item = &tr->m_items[i];
        std::shared_ptr<Entity> entity = tr_item->object_id == 0 ? std::make_shared<Character>(i) : std::make_shared<Entity>(i);
        entity->m_transform[3][0] = tr_item->position.x;
        entity->m_transform[3][1] = -tr_item->position.z;
        entity->m_transform[3][2] = tr_item->position.y;
        entity->m_angles[0] = tr_item->rotation;
        entity->m_angles[1] = 0;
        entity->m_angles[2] = 0;
        entity->updateTransform();
        if(tr_item->room >= 0 && static_cast<uint32_t>(tr_item->room) < world.m_rooms.size())
        {
            entity->setRoom(world.m_rooms[tr_item->room].get());
        }
        else
        {
            entity->setRoom(nullptr);
        }

        entity->m_triggerLayout = tr_item->getActivationMask();   ///@FIXME: Ignore INVISIBLE and CLEAR BODY flags for a moment.
        entity->m_OCB = tr_item->ocb;
        entity->m_timer = 0.0;

        entity->setCollisionType(CollisionType::Kinematic);
        entity->setCollisionShape(CollisionShape::TriMeshConvex);
        entity->m_moveType = MoveType::StaticPos;
        entity->m_inertiaLinear = 0.0;
        entity->m_inertiaAngular[0] = 0.0;
        entity->m_inertiaAngular[1] = 0.0;

        entity->m_skeleton.setModel(world.getModelByID(static_cast<ModelId>(tr_item->object_id)));

        if(entity->m_skeleton.getModel() == nullptr)
        {
            ModelId id = engine_lua.call("getOverridedID", static_cast<int>(loader::gameToEngine(tr->m_gameVersion)), tr_item->object_id).to<ModelId>();
            entity->m_skeleton.setModel(world.getModelByID(id));
        }

        lua::Value replace_anim_id = engine_lua.call("getOverridedAnim", static_cast<int>(loader::gameToEngine(tr->m_gameVersion)), tr_item->object_id);
        if(!replace_anim_id.isNil())
        {
            auto replace_anim_model = world.getModelByID(replace_anim_id.to<ModelId>());
            BOOST_ASSERT(replace_anim_model != nullptr);
            BOOST_ASSERT(entity->m_skeleton.getModel() != nullptr);
            std::swap(entity->m_skeleton.getModel()->m_animations, replace_anim_model->m_animations);
        }

        if(entity->m_skeleton.getModel() == nullptr)
        {
            // SPRITE LOADING
            core::Sprite* sp = world.getSpriteByID(tr_item->object_id);
            if(sp && entity->getRoom())
            {
                entity->getRoom()->addSprite(sp, glm::vec3(entity->m_transform[3]));
            }

            continue;                                                           // that entity has no model. may be it is a some trigger or look at object
        }

        if(tr->m_gameVersion < loader::Game::TR2 && tr_item->object_id == 83)                ///@FIXME: brutal magick hardcode! ;-)
        {
            // skip PSX save model
            continue;
        }

        entity->m_skeleton.fromModel(entity->m_skeleton.getModel());

        if(tr_item->object_id != 0)                                             // Lara is unical model
        {
            entity->setAnimation(0, 0);                                      // Set zero animation and zero frame

            Res_SetEntityProperties(entity);
            entity->rebuildBoundingBox();
            entity->m_skeleton.genRigidBody(*entity);

            entity->getRoom()->addEntity(entity.get());
            world.addEntity(entity);
            continue;
        }

        std::shared_ptr<Character> lara = std::dynamic_pointer_cast<Character>(entity);
        BOOST_ASSERT(lara != nullptr);

        lara->m_moveType = MoveType::OnFloor;
        world.m_character = lara;
        lara->setCollisionType(CollisionType::Actor);
        lara->setCollisionShape(CollisionShape::TriMeshConvex);
        lara->m_typeFlags |= ENTITY_TYPE_TRIGGER_ACTIVATOR;

        engine_lua.set("player", lara->getId());

        std::shared_ptr<SkeletalModel> laraModel = world.getModelByID(0);

        switch(loader::gameToEngine(tr->m_gameVersion))
        {
            case loader::Engine::TR1:
                if(engine::Gameflow::instance.getLevelID() == 0)
                {
                    if(std::shared_ptr<SkeletalModel> LM = world.getModelByID(TR_ITEM_LARA_SKIN_ALTERNATE_TR1))
                    {
                        // In TR1, Lara has unified head mesh for all her alternate skins.
                        // Hence, we copy all meshes except head, to prevent Potato Raider bug.
                        laraModel->setMeshes(LM->m_meshes, laraModel->m_meshes.size() - 1);
                    }
                }
                break;

            case loader::Engine::TR2:
                break;

            case loader::Engine::TR3:
                if(std::shared_ptr<SkeletalModel> LM = world.getModelByID(TR_ITEM_LARA_SKIN_TR3))
                {
                    laraModel->setMeshes(LM->m_meshes, laraModel->m_meshes.size());
                    auto tmp = world.getModelByID(11);                   // moto / quadro cycle animations
                    if(tmp)
                    {
                        tmp->setMeshes(LM->m_meshes, laraModel->m_meshes.size());
                    }
                }
                break;

            case loader::Engine::TR4:
            case loader::Engine::TR5:
                // base skeleton meshes
                if(std::shared_ptr<SkeletalModel> LM = world.getModelByID(TR_ITEM_LARA_SKIN_TR45))
                {
                    laraModel->setMeshes(LM->m_meshes, laraModel->m_meshes.size());
                }
                // skin skeleton meshes
                if(std::shared_ptr<SkeletalModel> LM = world.getModelByID(TR_ITEM_LARA_SKIN_JOINTS_TR45))
                {
                    laraModel->setSkinnedMeshes(LM->m_meshes, laraModel->m_meshes.size());
                }
                laraModel->fillSkinnedMeshMap();
                break;

            case loader::Engine::Unknown:
                break;
        };

        lara->m_skeleton.copyMeshBinding(lara->m_skeleton.getModel(), true);

        world.m_character->setAnimation(animation::TR_ANIMATION_LARA_STAY_IDLE, 0);
        lara->m_skeleton.genRigidBody(*lara);
        lara->m_skeleton.createGhosts(*lara);
        lara->setHeight(768.0f);
    }
}

void Res_EntityToItem(std::map<ObjectId, std::shared_ptr<BaseItem> >& map)
{
    for(std::shared_ptr<BaseItem> item : map | boost::adaptors::map_values)
    {
        for(const std::shared_ptr<Room>& room : engine::Engine::instance.m_world.m_rooms)
        {
            for(Object* object : room->getObjects())
            {
                Entity* ent = dynamic_cast<Entity*>(object);
                if(!ent)
                    continue;

                if(ent->m_skeleton.getModel()->getId() != item->world_model_id)
                    continue;

                if(engine_lua["entity_funcs"][ent->getId()].is<lua::Nil>())
                    engine_lua["entity_funcs"].set(ent->getId(), lua::Table());

                engine_lua["pickup_init"](ent->getId(), item->id);

                ent->m_skeleton.disableCollision();
            }
        }
    }
}
} // namespace world
