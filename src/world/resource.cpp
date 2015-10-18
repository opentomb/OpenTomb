#include "resource.h"

#include "LuaState.h"
#include "loader/level.h"

#include "animation/animids.h"
#include "bordered_texture_atlas.h"
#include "engine/engine.h"
#include "engine/gameflow.h"
#include "engine/system.h"
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
#include <glm/gtx/io.hpp>

#include <boost/log/trivial.hpp>

using gui::Console;

namespace world
{
    void Res_SetEntityProperties(std::shared_ptr<Entity> ent)
    {
        if(ent->m_skeleton.getModel() != nullptr && engine_lua["getEntityModelProperties"].is<lua::Callable>())
        {
            uint16_t flg;
            int collisionType, collisionShape;
            lua::tie(collisionType, collisionShape, ent->m_visible, flg) = engine_lua.call("getEntityModelProperties", static_cast<int>(engine::engine_world.engineVersion), ent->m_skeleton.getModel()->id);
            ent->setCollisionType(static_cast<world::CollisionType>(collisionType));
            ent->setCollisionShape(static_cast<world::CollisionShape>(collisionShape));

            ent->m_visible = !ent->m_visible;
            ent->m_typeFlags |= flg;
        }
    }

    void Res_SetEntityFunction(std::shared_ptr<Entity> ent)
    {
        if(ent->m_skeleton.getModel())
        {
            const char* funcName = engine_lua.call("getEntityFunction", static_cast<int>(engine::engine_world.engineVersion), ent->m_skeleton.getModel()->id);
            if(funcName)
                Res_CreateEntityFunc(engine_lua, funcName ? funcName : std::string(), ent->getId());
        }
    }

    void Res_CreateEntityFunc(script::ScriptEngine& state, const std::string& func_name, int entity_id)
    {
        if(state["entity_funcs"][entity_id].is<lua::Nil>())
            state["entity_funcs"].set(entity_id, lua::Table());
        state[(func_name + "_init").c_str()](entity_id);
    }

    void Res_GenEntityFunctions(std::map<uint32_t, std::shared_ptr<Entity> > &entities)
    {
        if(entities.empty()) return;

        for(const auto& pair : entities)
            Res_SetEntityFunction(pair.second);
    }

    void Res_SetStaticMeshProperties(std::shared_ptr<StaticMesh> r_static)
    {
        lua::Integer _collision_type, _collision_shape;
        lua::Boolean _hide;
        lua::tie(_collision_type, _collision_shape, _hide) = engine_lua.call("getStaticMeshProperties", r_static->getId());

        if(_collision_type > 0)
        {
            r_static->setCollisionType(static_cast<world::CollisionType>(_collision_type));
            r_static->setCollisionShape(static_cast<world::CollisionShape>(_collision_shape));
            r_static->hide = _hide;
        }
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

    void Res_Sector_SetTweenFloorConfig(SectorTween *tween)
    {
        if(tween->floor_corners[0][2] > tween->floor_corners[1][2])
        {
            std::swap(tween->floor_corners[0][2], tween->floor_corners[1][2]);
            std::swap(tween->floor_corners[2][2], tween->floor_corners[3][2]);
        }

        if(tween->floor_corners[3][2] > tween->floor_corners[2][2])
        {
            tween->floor_tween_type = TweenType::TwoTriangles;              // like a butterfly
        }
        else if((tween->floor_corners[0][2] != tween->floor_corners[1][2]) &&
                (tween->floor_corners[2][2] != tween->floor_corners[3][2]))
        {
            tween->floor_tween_type = TweenType::Quad;
        }
        else if(tween->floor_corners[0][2] != tween->floor_corners[1][2])
        {
            tween->floor_tween_type = TweenType::TriangleLeft;
        }
        else if(tween->floor_corners[2][2] != tween->floor_corners[3][2])
        {
            tween->floor_tween_type = TweenType::TriangleRight;
        }
        else
        {
            tween->floor_tween_type = TweenType::None;
        }
    }

    void Res_Sector_SetTweenCeilingConfig(SectorTween *tween)
    {
        if(tween->ceiling_corners[0][2] > tween->ceiling_corners[1][2])
        {
            std::swap(tween->ceiling_corners[0][2], tween->ceiling_corners[1][2]);
            std::swap(tween->ceiling_corners[2][2], tween->ceiling_corners[3][2]);
        }

        if(tween->ceiling_corners[3][2] > tween->ceiling_corners[2][2])
        {
            tween->ceiling_tween_type = TweenType::TwoTriangles;            // like a butterfly
        }
        else if((tween->ceiling_corners[0][2] != tween->ceiling_corners[1][2]) &&
                (tween->ceiling_corners[2][2] != tween->ceiling_corners[3][2]))
        {
            tween->ceiling_tween_type = TweenType::Quad;
        }
        else if(tween->ceiling_corners[0][2] != tween->ceiling_corners[1][2])
        {
            tween->ceiling_tween_type = TweenType::TriangleLeft;
        }
        else if(tween->ceiling_corners[2][2] != tween->ceiling_corners[3][2])
        {
            tween->ceiling_tween_type = TweenType::TriangleRight;
        }
        else
        {
            tween->ceiling_tween_type = TweenType::None;
        }
    }

    bool Res_Sector_IsWall(RoomSector* ws, RoomSector* ns)
    {
        if((ws->portal_to_room < 0) && (ns->portal_to_room < 0) && (ws->floor_penetration_config == PenetrationConfig::Wall))
        {
            return true;
        }

        if((ns->portal_to_room < 0) && (ns->floor_penetration_config != PenetrationConfig::Wall) && (ws->portal_to_room >= 0))
        {
            ws = ws->checkPortalPointer();
            if((ws->floor_penetration_config == PenetrationConfig::Wall) || !ns->is2SidePortals(ws))
            {
                return true;
            }
        }

        return false;
    }

    ///@TODO: resolve floor >> ceiling case
    std::vector<SectorTween> Res_Sector_GenTweens(std::shared_ptr<Room> room)
    {
        std::vector<SectorTween> result;
        for(size_t h = 0; h < room->sectors.shape()[1] - 1; h++)
        {
            for(size_t w = 0; w < room->sectors.shape()[0] - 1; w++)
            {
                result.emplace_back();
                SectorTween* room_tween = &result.back();
                // Init X-plane tween [ | ]

                RoomSector* current_heightmap = &room->sectors[w][h];
                RoomSector* next_heightmap = current_heightmap + 1;
                char joined_floors = 0;
                char joined_ceilings = 0;

                /* XY corners coordinates must be calculated from native room sector */
                room_tween->floor_corners[0][1] = current_heightmap->floor_corners[0][1];
                room_tween->floor_corners[1][1] = room_tween->floor_corners[0][1];
                room_tween->floor_corners[2][1] = room_tween->floor_corners[0][1];
                room_tween->floor_corners[3][1] = room_tween->floor_corners[0][1];
                room_tween->floor_corners[0][0] = current_heightmap->floor_corners[0][0];
                room_tween->floor_corners[1][0] = room_tween->floor_corners[0][0];
                room_tween->floor_corners[2][0] = current_heightmap->floor_corners[1][0];
                room_tween->floor_corners[3][0] = room_tween->floor_corners[2][0];

                room_tween->ceiling_corners[0][1] = current_heightmap->ceiling_corners[0][1];
                room_tween->ceiling_corners[1][1] = room_tween->ceiling_corners[0][1];
                room_tween->ceiling_corners[2][1] = room_tween->ceiling_corners[0][1];
                room_tween->ceiling_corners[3][1] = room_tween->ceiling_corners[0][1];
                room_tween->ceiling_corners[0][0] = current_heightmap->ceiling_corners[0][0];
                room_tween->ceiling_corners[1][0] = room_tween->ceiling_corners[0][0];
                room_tween->ceiling_corners[2][0] = current_heightmap->ceiling_corners[1][0];
                room_tween->ceiling_corners[3][0] = room_tween->ceiling_corners[2][0];

                if(w > 0)
                {
                    if((next_heightmap->floor_penetration_config != PenetrationConfig::Wall) || (current_heightmap->floor_penetration_config != PenetrationConfig::Wall))                                                           // Init X-plane tween [ | ]
                    {
                        if(Res_Sector_IsWall(next_heightmap, current_heightmap))
                        {
                            room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                            room_tween->floor_corners[1][2] = current_heightmap->ceiling_corners[0][2];
                            room_tween->floor_corners[2][2] = current_heightmap->ceiling_corners[1][2];
                            room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                            Res_Sector_SetTweenFloorConfig(room_tween);
                            room_tween->ceiling_tween_type = TweenType::None;
                            joined_floors = 1;
                            joined_ceilings = 1;
                        }
                        else if(Res_Sector_IsWall(current_heightmap, next_heightmap))
                        {
                            room_tween->floor_corners[0][2] = next_heightmap->floor_corners[3][2];
                            room_tween->floor_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                            room_tween->floor_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                            room_tween->floor_corners[3][2] = next_heightmap->floor_corners[2][2];
                            Res_Sector_SetTweenFloorConfig(room_tween);
                            room_tween->ceiling_tween_type = TweenType::None;
                            joined_floors = 1;
                            joined_ceilings = 1;
                        }
                        else
                        {
                            /************************** SECTION WITH DROPS CALCULATIONS **********************/
                            if(((current_heightmap->portal_to_room < 0) && ((next_heightmap->portal_to_room < 0))) || current_heightmap->is2SidePortals(next_heightmap))
                            {
                                current_heightmap = current_heightmap->checkPortalPointer();
                                next_heightmap = next_heightmap->checkPortalPointer();
                                if((current_heightmap->portal_to_room < 0) && (next_heightmap->portal_to_room < 0) && (current_heightmap->floor_penetration_config != PenetrationConfig::Wall) && (next_heightmap->floor_penetration_config != PenetrationConfig::Wall))
                                {
                                    if((current_heightmap->floor_penetration_config == PenetrationConfig::Solid) || (next_heightmap->floor_penetration_config == PenetrationConfig::Solid))
                                    {
                                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                                        room_tween->floor_corners[1][2] = next_heightmap->floor_corners[3][2];
                                        room_tween->floor_corners[2][2] = next_heightmap->floor_corners[2][2];
                                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                                        Res_Sector_SetTweenFloorConfig(room_tween);
                                        joined_floors = 1;
                                    }
                                    if((current_heightmap->ceiling_penetration_config == PenetrationConfig::Solid) || (next_heightmap->ceiling_penetration_config == PenetrationConfig::Solid))
                                    {
                                        room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[0][2];
                                        room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                                        room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                                        room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[1][2];
                                        Res_Sector_SetTweenCeilingConfig(room_tween);
                                        joined_ceilings = 1;
                                    }
                                }
                            }
                        }
                    }

                    current_heightmap = &room->sectors[w][h];
                    next_heightmap = current_heightmap + 1;
                    if((joined_floors == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                    {
                        char valid = 0;
                        if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_above != nullptr) && (current_heightmap->floor_penetration_config == PenetrationConfig::Solid))
                        {
                            next_heightmap = next_heightmap->checkPortalPointer();
                            if(next_heightmap->owner_room->getId() == current_heightmap->sector_above->owner_room->getId())
                            {
                                valid = 1;
                            }
                            if(valid == 0)
                            {
                                RoomSector* rs = current_heightmap->sector_above->owner_room->getSectorRaw(next_heightmap->position);
                                if(rs && (static_cast<uint32_t>(rs->portal_to_room) == next_heightmap->owner_room->getId()))
                                {
                                    valid = 1;
                                }
                            }
                        }

                        if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_above != nullptr) && (next_heightmap->floor_penetration_config == PenetrationConfig::Solid))
                        {
                            current_heightmap = current_heightmap->checkPortalPointer();
                            if(current_heightmap->owner_room->getId() == next_heightmap->sector_above->owner_room->getId())
                            {
                                valid = 1;
                            }
                            if(valid == 0)
                            {
                                RoomSector* rs = next_heightmap->sector_above->owner_room->getSectorRaw(current_heightmap->position);
                                if(rs && (static_cast<uint32_t>(rs->portal_to_room) == current_heightmap->owner_room->getId()))
                                {
                                    valid = 1;
                                }
                            }
                        }

                        if((valid == 1) && (current_heightmap->floor_penetration_config != PenetrationConfig::Wall) && (next_heightmap->floor_penetration_config != PenetrationConfig::Wall))
                        {
                            room_tween->floor_corners[0][2] = current_heightmap->floor_corners[0][2];
                            room_tween->floor_corners[1][2] = next_heightmap->floor_corners[3][2];
                            room_tween->floor_corners[2][2] = next_heightmap->floor_corners[2][2];
                            room_tween->floor_corners[3][2] = current_heightmap->floor_corners[1][2];
                            Res_Sector_SetTweenFloorConfig(room_tween);
                        }
                    }

                    current_heightmap = &room->sectors[w][h];
                    next_heightmap = current_heightmap + 1;
                    if((joined_ceilings == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                    {
                        char valid = 0;
                        if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_below != nullptr) && (current_heightmap->ceiling_penetration_config == PenetrationConfig::Solid))
                        {
                            next_heightmap = next_heightmap->checkPortalPointer();
                            if(next_heightmap->owner_room->getId() == current_heightmap->sector_below->owner_room->getId())
                            {
                                valid = 1;
                            }
                            if(valid == 0)
                            {
                                RoomSector* rs = current_heightmap->sector_below->owner_room->getSectorRaw(next_heightmap->position);
                                if(rs && (static_cast<uint32_t>(rs->portal_to_room) == next_heightmap->owner_room->getId()))
                                {
                                    valid = 1;
                                }
                            }
                        }

                        if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_below != nullptr) && (next_heightmap->floor_penetration_config == PenetrationConfig::Solid))
                        {
                            current_heightmap = current_heightmap->checkPortalPointer();
                            if(current_heightmap->owner_room->getId() == next_heightmap->sector_below->owner_room->getId())
                            {
                                valid = 1;
                            }
                            if(valid == 0)
                            {
                                RoomSector* rs = next_heightmap->sector_below->owner_room->getSectorRaw(current_heightmap->position);
                                if(rs && (static_cast<uint32_t>(rs->portal_to_room) == current_heightmap->owner_room->getId()))
                                {
                                    valid = 1;
                                }
                            }
                        }

                        if((valid == 1) && (current_heightmap->floor_penetration_config != PenetrationConfig::Wall) && (next_heightmap->floor_penetration_config != PenetrationConfig::Wall))
                        {
                            room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[0][2];
                            room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[3][2];
                            room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[2][2];
                            room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[1][2];
                            Res_Sector_SetTweenCeilingConfig(room_tween);
                        }
                    }
                }

                /*****************************************************************************************************
                 ********************************   CENTRE  OF  THE  ALGORITHM   *************************************
                 *****************************************************************************************************/

                result.emplace_back();
                room_tween = &result.back();
                current_heightmap = &room->sectors[w][h];
                next_heightmap = &room->sectors[w + 1][h];
                room_tween->floor_corners[0][0] = current_heightmap->floor_corners[1][0];
                room_tween->floor_corners[1][0] = room_tween->floor_corners[0][0];
                room_tween->floor_corners[2][0] = room_tween->floor_corners[0][0];
                room_tween->floor_corners[3][0] = room_tween->floor_corners[0][0];
                room_tween->floor_corners[0][1] = current_heightmap->floor_corners[1][1];
                room_tween->floor_corners[1][1] = room_tween->floor_corners[0][1];
                room_tween->floor_corners[2][1] = current_heightmap->floor_corners[2][1];
                room_tween->floor_corners[3][1] = room_tween->floor_corners[2][1];

                room_tween->ceiling_corners[0][0] = current_heightmap->ceiling_corners[1][0];
                room_tween->ceiling_corners[1][0] = room_tween->ceiling_corners[0][0];
                room_tween->ceiling_corners[2][0] = room_tween->ceiling_corners[0][0];
                room_tween->ceiling_corners[3][0] = room_tween->ceiling_corners[0][0];
                room_tween->ceiling_corners[0][1] = current_heightmap->ceiling_corners[1][1];
                room_tween->ceiling_corners[1][1] = room_tween->ceiling_corners[0][1];
                room_tween->ceiling_corners[2][1] = current_heightmap->ceiling_corners[2][1];
                room_tween->ceiling_corners[3][1] = room_tween->ceiling_corners[2][1];

                joined_floors = 0;
                joined_ceilings = 0;

                if(h > 0)
                {
                    if((next_heightmap->floor_penetration_config != PenetrationConfig::Wall) || (current_heightmap->floor_penetration_config != PenetrationConfig::Wall))
                    {
                        // Init Y-plane tween  [ - ]
                        if(Res_Sector_IsWall(next_heightmap, current_heightmap))
                        {
                            room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                            room_tween->floor_corners[1][2] = current_heightmap->ceiling_corners[1][2];
                            room_tween->floor_corners[2][2] = current_heightmap->ceiling_corners[2][2];
                            room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                            Res_Sector_SetTweenFloorConfig(room_tween);
                            room_tween->ceiling_tween_type = TweenType::None;
                            joined_floors = 1;
                            joined_ceilings = 1;
                        }
                        else if(Res_Sector_IsWall(current_heightmap, next_heightmap))
                        {
                            room_tween->floor_corners[0][2] = next_heightmap->floor_corners[0][2];
                            room_tween->floor_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                            room_tween->floor_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                            room_tween->floor_corners[3][2] = next_heightmap->floor_corners[3][2];
                            Res_Sector_SetTweenFloorConfig(room_tween);
                            room_tween->ceiling_tween_type = TweenType::None;
                            joined_floors = 1;
                            joined_ceilings = 1;
                        }
                        else
                        {
                            /************************** BIG SECTION WITH DROPS CALCULATIONS **********************/
                            if(((current_heightmap->portal_to_room < 0) && ((next_heightmap->portal_to_room < 0))) || current_heightmap->is2SidePortals(next_heightmap))
                            {
                                current_heightmap = current_heightmap->checkPortalPointer();
                                next_heightmap = next_heightmap->checkPortalPointer();
                                if((current_heightmap->portal_to_room < 0) && (next_heightmap->portal_to_room < 0) && (current_heightmap->floor_penetration_config != PenetrationConfig::Wall) && (next_heightmap->floor_penetration_config != PenetrationConfig::Wall))
                                {
                                    if((current_heightmap->floor_penetration_config == PenetrationConfig::Solid) || (next_heightmap->floor_penetration_config == PenetrationConfig::Solid))
                                    {
                                        room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                                        room_tween->floor_corners[1][2] = next_heightmap->floor_corners[0][2];
                                        room_tween->floor_corners[2][2] = next_heightmap->floor_corners[3][2];
                                        room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                                        Res_Sector_SetTweenFloorConfig(room_tween);
                                        joined_floors = 1;
                                    }
                                    if((current_heightmap->ceiling_penetration_config == PenetrationConfig::Solid) || (next_heightmap->ceiling_penetration_config == PenetrationConfig::Solid))
                                    {
                                        room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[1][2];
                                        room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                                        room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                                        room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[2][2];
                                        Res_Sector_SetTweenCeilingConfig(room_tween);
                                        joined_ceilings = 1;
                                    }
                                }
                            }
                        }
                    }

                    current_heightmap = &room->sectors[w][h];
                    next_heightmap = &room->sectors[w + 1][h];
                    if((joined_floors == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                    {
                        char valid = 0;
                        if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_above != nullptr) && (current_heightmap->floor_penetration_config == PenetrationConfig::Solid))
                        {
                            next_heightmap = next_heightmap->checkPortalPointer();
                            if(next_heightmap->owner_room->getId() == current_heightmap->sector_above->owner_room->getId())
                            {
                                valid = 1;
                            }
                            if(valid == 0)
                            {
                                RoomSector* rs = current_heightmap->sector_above->owner_room->getSectorRaw(next_heightmap->position);
                                if(rs && (static_cast<uint32_t>(rs->portal_to_room) == next_heightmap->owner_room->getId()))
                                {
                                    valid = 1;
                                }
                            }
                        }

                        if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_above != nullptr) && (next_heightmap->floor_penetration_config == PenetrationConfig::Solid))
                        {
                            current_heightmap = current_heightmap->checkPortalPointer();
                            if(current_heightmap->owner_room->getId() == next_heightmap->sector_above->owner_room->getId())
                            {
                                valid = 1;
                            }
                            if(valid == 0)
                            {
                                RoomSector* rs = next_heightmap->sector_above->owner_room->getSectorRaw(current_heightmap->position);
                                if(rs && (static_cast<uint32_t>(rs->portal_to_room) == current_heightmap->owner_room->getId()))
                                {
                                    valid = 1;
                                }
                            }
                        }

                        if((valid == 1) && (current_heightmap->floor_penetration_config != PenetrationConfig::Wall) && (next_heightmap->floor_penetration_config != PenetrationConfig::Wall))
                        {
                            room_tween->floor_corners[0][2] = current_heightmap->floor_corners[1][2];
                            room_tween->floor_corners[1][2] = next_heightmap->floor_corners[0][2];
                            room_tween->floor_corners[2][2] = next_heightmap->floor_corners[3][2];
                            room_tween->floor_corners[3][2] = current_heightmap->floor_corners[2][2];
                            Res_Sector_SetTweenFloorConfig(room_tween);
                        }
                    }

                    current_heightmap = &room->sectors[w][h];
                    next_heightmap = &room->sectors[w + 1][h];
                    if((joined_ceilings == 0) && ((current_heightmap->portal_to_room < 0) || (next_heightmap->portal_to_room < 0)))
                    {
                        char valid = 0;
                        if((next_heightmap->portal_to_room >= 0) && (current_heightmap->sector_below != nullptr) && (current_heightmap->ceiling_penetration_config == PenetrationConfig::Solid))
                        {
                            next_heightmap = next_heightmap->checkPortalPointer();
                            if(next_heightmap->owner_room->getId() == current_heightmap->sector_below->owner_room->getId())
                            {
                                valid = 1;
                            }
                            if(valid == 0)
                            {
                                RoomSector* rs = current_heightmap->sector_below->owner_room->getSectorRaw(next_heightmap->position);
                                if(rs && (static_cast<uint32_t>(rs->portal_to_room) == next_heightmap->owner_room->getId()))
                                {
                                    valid = 1;
                                }
                            }
                        }

                        if((current_heightmap->portal_to_room >= 0) && (next_heightmap->sector_below != nullptr) && (next_heightmap->floor_penetration_config == PenetrationConfig::Solid))
                        {
                            current_heightmap = current_heightmap->checkPortalPointer();
                            if(current_heightmap->owner_room->getId() == next_heightmap->sector_below->owner_room->getId())
                            {
                                valid = 1;
                            }
                            if(valid == 0)
                            {
                                RoomSector* rs = next_heightmap->sector_below->owner_room->getSectorRaw(current_heightmap->position);
                                if(rs && (static_cast<uint32_t>(rs->portal_to_room) == current_heightmap->owner_room->getId()))
                                {
                                    valid = 1;
                                }
                            }
                        }

                        if((valid == 1) && (current_heightmap->floor_penetration_config != PenetrationConfig::Wall) && (next_heightmap->floor_penetration_config != PenetrationConfig::Wall))
                        {
                            room_tween->ceiling_corners[0][2] = current_heightmap->ceiling_corners[1][2];
                            room_tween->ceiling_corners[1][2] = next_heightmap->ceiling_corners[0][2];
                            room_tween->ceiling_corners[2][2] = next_heightmap->ceiling_corners[3][2];
                            room_tween->ceiling_corners[3][2] = current_heightmap->ceiling_corners[2][2];
                            Res_Sector_SetTweenCeilingConfig(room_tween);
                        }
                    }
                }
            }    ///END for
        }    ///END for
        return result;
    }

    bool Res_IsEntityProcessed(int32_t *lookup_table, uint16_t entity_index)
    {
        // Fool-proof check for entity existence. Fixes LOTS of stray non-existent
        // entity #256 occurences in original games (primarily TR4-5).

        if(!engine::engine_world.getEntityByID(entity_index))
            return true;

        int32_t *curr_table_index = lookup_table;

        while(*curr_table_index != -1)
        {
            if(*curr_table_index == static_cast<int32_t>(entity_index))
                return true;
            curr_table_index++;
        }

        *curr_table_index = static_cast<int32_t>(entity_index);
        return false;
    }

    int TR_Sector_TranslateFloorData(RoomSector* sector, const std::unique_ptr<loader::Level>& tr)
    {
        if(!sector || (sector->trig_index <= 0) || (sector->trig_index >= tr->m_floorData.size()))
        {
            return 0;
        }

        sector->flags = 0;  // Clear sector flags before parsing.

        /*
         * PARSE FUNCTIONS
         */

        uint16_t *end_p = tr->m_floorData.data() + tr->m_floorData.size() - 1;
        uint16_t *entry = tr->m_floorData.data() + sector->trig_index;

        int ret = 0;
        uint16_t end_bit;

        do
        {
            // TR1 - TR2
            //function = (*entry) & 0x00FF;                   // 0b00000000 11111111
            //sub_function = ((*entry) & 0x7F00) >> 8;        // 0b01111111 00000000

            //TR3+, but works with TR1 - TR2
            uint16_t function = ((*entry) & 0x001F);             // 0b00000000 00011111
            // uint16_t function_value = ((*entry) & 0x00E0) >> 5;        // 0b00000000 11100000  TR3+
            uint16_t sub_function = ((*entry) & 0x7F00) >> 8;        // 0b01111111 00000000

            end_bit = ((*entry) & 0x8000) >> 15;       // 0b10000000 00000000

            entry++;

            switch(function)
            {
                case TR_FD_FUNC_PORTALSECTOR:          // PORTAL DATA
                    if(sub_function == 0x00)
                    {
                        if(*entry < engine::engine_world.rooms.size())
                        {
                            sector->portal_to_room = *entry;
                            sector->floor_penetration_config = PenetrationConfig::Ghost;
                            sector->ceiling_penetration_config = PenetrationConfig::Ghost;
                        }
                        entry++;
                    }
                    break;

                case TR_FD_FUNC_FLOORSLANT:          // FLOOR SLANT
                    if(sub_function == 0x00)
                    {
                        int8_t raw_y_slant = (*entry & 0x00FF);
                        int8_t raw_x_slant = ((*entry & 0xFF00) >> 8);

                        sector->floor_diagonal_type = DiagonalType::None;
                        sector->floor_penetration_config = PenetrationConfig::Solid;

                        if(raw_x_slant > 0)
                        {
                            sector->floor_corners[2][2] -= (static_cast<glm::float_t>(raw_x_slant) * MeteringStep);
                            sector->floor_corners[3][2] -= (static_cast<glm::float_t>(raw_x_slant) * MeteringStep);
                        }
                        else if(raw_x_slant < 0)
                        {
                            sector->floor_corners[0][2] -= (glm::abs(static_cast<glm::float_t>(raw_x_slant)) * MeteringStep);
                            sector->floor_corners[1][2] -= (glm::abs(static_cast<glm::float_t>(raw_x_slant)) * MeteringStep);
                        }

                        if(raw_y_slant > 0)
                        {
                            sector->floor_corners[0][2] -= (static_cast<glm::float_t>(raw_y_slant) * MeteringStep);
                            sector->floor_corners[3][2] -= (static_cast<glm::float_t>(raw_y_slant) * MeteringStep);
                        }
                        else if(raw_y_slant < 0)
                        {
                            sector->floor_corners[1][2] -= (glm::abs(static_cast<glm::float_t>(raw_y_slant)) * MeteringStep);
                            sector->floor_corners[2][2] -= (glm::abs(static_cast<glm::float_t>(raw_y_slant)) * MeteringStep);
                        }

                        entry++;
                    }
                    break;

                case TR_FD_FUNC_CEILINGSLANT:          // CEILING SLANT
                    if(sub_function == 0x00)
                    {
                        int8_t raw_y_slant = (*entry & 0x00FF);
                        int8_t raw_x_slant = ((*entry & 0xFF00) >> 8);

                        sector->ceiling_diagonal_type = DiagonalType::None;
                        sector->ceiling_penetration_config = PenetrationConfig::Solid;

                        if(raw_x_slant > 0)
                        {
                            sector->ceiling_corners[3][2] += (static_cast<glm::float_t>(raw_x_slant) * MeteringStep);
                            sector->ceiling_corners[2][2] += (static_cast<glm::float_t>(raw_x_slant) * MeteringStep);
                        }
                        else if(raw_x_slant < 0)
                        {
                            sector->ceiling_corners[1][2] += (glm::abs(static_cast<glm::float_t>(raw_x_slant)) * MeteringStep);
                            sector->ceiling_corners[0][2] += (glm::abs(static_cast<glm::float_t>(raw_x_slant)) * MeteringStep);
                        }

                        if(raw_y_slant > 0)
                        {
                            sector->ceiling_corners[1][2] += (static_cast<glm::float_t>(raw_y_slant) * MeteringStep);
                            sector->ceiling_corners[2][2] += (static_cast<glm::float_t>(raw_y_slant) * MeteringStep);
                        }
                        else if(raw_y_slant < 0)
                        {
                            sector->ceiling_corners[0][2] += (glm::abs(static_cast<glm::float_t>(raw_y_slant)) * MeteringStep);
                            sector->ceiling_corners[3][2] += (glm::abs(static_cast<glm::float_t>(raw_y_slant)) * MeteringStep);
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
                    int condition = 0;                        // No condition by default.
                    int mask_mode = AMASK_OP_OR;              // Activation mask by default.

                    int8_t  timer_field = (*entry) & 0x00FF;          // Used as common parameter for some commands.
                    uint8_t trigger_mask = ((*entry) & 0x3E00) >> 9;
                    uint8_t only_once = ((*entry) & 0x0100) >> 8;    // Lock out triggered items after activation.

                    // Processed entities lookup array initialization.

                    int32_t ent_lookup_table[64];
                    memset(ent_lookup_table, 0xFF, sizeof(int32_t) * 64);

                    // Activator type is LARA for all triggers except HEAVY ones, which are triggered by
                    // some specific entity classes.

                    int activator_type = ((sub_function == TR_FD_TRIGTYPE_HEAVY) ||
                                          (sub_function == TR_FD_TRIGTYPE_HEAVYANTITRIGGER) ||
                                          (sub_function == TR_FD_TRIGTYPE_HEAVYSWITCH)) ? TR_ACTIVATORTYPE_MISC : TR_ACTIVATORTYPE_LARA;

                    // Table cell header.

                    snprintf(buf, 256, "trigger_list[%d] = {activator_type = %d, func = function(entity_index) \n",
                             sector->trig_index, activator_type);

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
                            snprintf(buf, 128, " if(getEntityMoveType(entity_index) == %d) then \n", MoveType::OnFloor);
                            if(sub_function == TR_FD_TRIGTYPE_ANTIPAD)
                                action_type = ActionType::Anti;
                            condition = 1;  // Set additional condition.
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
                            condition = 1;  // Set additional condition.
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
                            condition = 1;  // Set additional condition.
                            break;

                        case TR_FD_TRIGTYPE_TIGHTROPE:
                            // Check state range for triggering entity.
                            snprintf(buf, 128, " local state = getEntityState(entity_index) \n if((state >= %d) and (state <= %d)) then \n", static_cast<int>(LaraState::TIGHTROPE_IDLE), static_cast<int>(LaraState::TIGHTROPE_EXIT));
                            condition = 1;  // Set additional condition.
                            break;
                        case TR_FD_TRIGTYPE_CRAWLDUCK:
                            // Check state range for triggering entity.
                            snprintf(buf, 128, " local state = getEntityState(entity_index) \n if((state >= %d) and (state <= %d)) then \n", TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_BEGIN, TR_ANIMATION_LARA_CRAWL_SMASH_LEFT);
                            condition = 1;  // Set additional condition.
                            break;
                    }

                    header += buf;    // Add condition to header.

                    uint16_t cont_bit;
                    uint16_t argn = 0;

                    // Now parse operand chain for trigger function!

                    do
                    {
                        entry++;

                        uint16_t trigger_function = (((*entry) & 0x7C00)) >> 10;    // 0b01111100 00000000
                        uint16_t operands = (*entry) & 0x03FF;                      // 0b00000011 11111111
                        cont_bit = ((*entry) & 0x8000) >> 15;              // 0b10000000 00000000

                        switch(trigger_function)
                        {
                            case TR_FD_TRIGFUNC_OBJECT:         // ACTIVATE / DEACTIVATE object
                                // If activator is specified, first item operand counts as activator index (except
                                // heavy switch case, which is ordinary heavy trigger case with certain differences).
                                if((argn == 0) && activator != ActivatorType::Normal)
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
                                                if(engine::engine_world.engineVersion >= loader::Engine::TR3 && only_once)
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
                                uint8_t cam_index = (*entry) & 0x007F;
                                entry++;
                                uint8_t cam_timer = ((*entry) & 0x00FF);
                                uint8_t cam_once = ((*entry) & 0x0100) >> 8;
                                uint8_t cam_zoom = engine::engine_world.engineVersion < loader::Engine::TR2
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
                                if(engine::engine_world.engineVersion < loader::Engine::TR2)
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
                                uint8_t flyby_once = ((*entry) & 0x0100) >> 8;
                                cont_bit = ((*entry) & 0x8000) >> 15;

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
                            if((action_type == ActionType::Switch) && (activator == ActivatorType::Switch))
                            {
                                script += buf2;
                                if(engine::engine_world.engineVersion < loader::Engine::TR3 || !only_once)
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
                    sector->flags |= SECTOR_FLAG_DEATH;
                    break;

                case TR_FD_FUNC_CLIMB:
                    // First 4 sector flags are similar to subfunction layout.
                    sector->flags |= sub_function;
                    break;

                case TR_FD_FUNC_MONKEY:
                    sector->flags |= SECTOR_FLAG_CLIMB_CEILING;
                    break;

                case TR_FD_FUNC_MINECART_LEFT:
                    // Minecart left (TR3) and trigger triggerer mark (TR4-5) has the same flag value.
                    // We re-parse them properly here.
                    if(tr->m_gameVersion < loader::Game::TR4)
                    {
                        sector->flags |= SECTOR_FLAG_MINECART_LEFT;
                    }
                    else
                    {
                        sector->flags |= SECTOR_FLAG_TRIGGERER_MARK;
                    }
                    break;

                case TR_FD_FUNC_MINECART_RIGHT:
                    // Minecart right (TR3) and beetle mark (TR4-5) has the same flag value.
                    // We re-parse them properly here.
                    if(tr->m_gameVersion < loader::Game::TR4)
                    {
                        sector->flags |= SECTOR_FLAG_MINECART_RIGHT;
                    }
                    else
                    {
                        sector->flags |= SECTOR_FLAG_BEETLE_MARK;
                    }
                    break;

                default:
                    // Other functions are TR3+ collisional triangle functions.
                    if((function >= TR_FD_FUNC_FLOORTRIANGLE_NW) &&
                       (function <= TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE))
                    {
                        entry--;    // Go back, since these functions are parsed differently.

                        end_bit = ((*entry) & 0x8000) >> 15;      // 0b10000000 00000000

#if 0
                        int16_t  slope_t01 = ((*entry) & 0x7C00) >> 10;      // 0b01111100 00000000
                        int16_t  slope_t00 = ((*entry) & 0x03E0) >> 5;       // 0b00000011 11100000
                        // uint16_t slope_func = ((*entry) & 0x001F);            // 0b00000000 00011111

                        // t01/t02 are 5-bit values, where sign is specified by 0x10 mask.

                        if(slope_t01 & 0x10) slope_t01 |= 0xFFF0;
                        if(slope_t00 & 0x10) slope_t00 |= 0xFFF0;
#endif

                        entry++;

                        uint16_t slope_t13 = ((*entry) & 0xF000) >> 12;      // 0b11110000 00000000
                        uint16_t slope_t12 = ((*entry) & 0x0F00) >> 8;       // 0b00001111 00000000
                        uint16_t slope_t11 = ((*entry) & 0x00F0) >> 4;       // 0b00000000 11110000
                        uint16_t slope_t10 = ((*entry) & 0x000F);            // 0b00000000 00001111

                        entry++;

                        float overall_adjustment = static_cast<float>(std::max({slope_t10, slope_t11, slope_t12, slope_t13})) * MeteringStep;

                        if((function == TR_FD_FUNC_FLOORTRIANGLE_NW) ||
                           (function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW) ||
                           (function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE))
                        {
                            sector->floor_diagonal_type = DiagonalType::NW;

                            sector->floor_corners[0][2] -= overall_adjustment - (static_cast<glm::float_t>(slope_t12) * MeteringStep);
                            sector->floor_corners[1][2] -= overall_adjustment - (static_cast<glm::float_t>(slope_t13) * MeteringStep);
                            sector->floor_corners[2][2] -= overall_adjustment - (static_cast<glm::float_t>(slope_t10) * MeteringStep);
                            sector->floor_corners[3][2] -= overall_adjustment - (static_cast<glm::float_t>(slope_t11) * MeteringStep);

                            if(function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW)
                            {
                                sector->floor_penetration_config = PenetrationConfig::DoorVerticalA;
                            }
                            else if(function == TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE)
                            {
                                sector->floor_penetration_config = PenetrationConfig::DoorVerticalB;
                            }
                            else
                            {
                                sector->floor_penetration_config = PenetrationConfig::Solid;
                            }
                        }
                        else if((function == TR_FD_FUNC_FLOORTRIANGLE_NE) ||
                                (function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW) ||
                                (function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE))
                        {
                            sector->floor_diagonal_type = DiagonalType::NE;

                            sector->floor_corners[0][2] -= overall_adjustment - (static_cast<glm::float_t>(slope_t12) * MeteringStep);
                            sector->floor_corners[1][2] -= overall_adjustment - (static_cast<glm::float_t>(slope_t13) * MeteringStep);
                            sector->floor_corners[2][2] -= overall_adjustment - (static_cast<glm::float_t>(slope_t10) * MeteringStep);
                            sector->floor_corners[3][2] -= overall_adjustment - (static_cast<glm::float_t>(slope_t11) * MeteringStep);

                            if(function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW)
                            {
                                sector->floor_penetration_config = PenetrationConfig::DoorVerticalA;
                            }
                            else if(function == TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE)
                            {
                                sector->floor_penetration_config = PenetrationConfig::DoorVerticalB;
                            }
                            else
                            {
                                sector->floor_penetration_config = PenetrationConfig::Solid;
                            }
                        }
                        else if((function == TR_FD_FUNC_CEILINGTRIANGLE_NW) ||
                                (function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW) ||
                                (function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE))
                        {
                            sector->ceiling_diagonal_type = DiagonalType::NW;

                            sector->ceiling_corners[0][2] += overall_adjustment - static_cast<glm::float_t>(slope_t11 * MeteringStep);
                            sector->ceiling_corners[1][2] += overall_adjustment - static_cast<glm::float_t>(slope_t10 * MeteringStep);
                            sector->ceiling_corners[2][2] += overall_adjustment - static_cast<glm::float_t>(slope_t13 * MeteringStep);
                            sector->ceiling_corners[3][2] += overall_adjustment - static_cast<glm::float_t>(slope_t12 * MeteringStep);

                            if(function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW)
                            {
                                sector->ceiling_penetration_config = PenetrationConfig::DoorVerticalA;
                            }
                            else if(function == TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE)
                            {
                                sector->ceiling_penetration_config = PenetrationConfig::DoorVerticalB;
                            }
                            else
                            {
                                sector->ceiling_penetration_config = PenetrationConfig::Solid;
                            }
                        }
                        else if((function == TR_FD_FUNC_CEILINGTRIANGLE_NE) ||
                                (function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW) ||
                                (function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE))
                        {
                            sector->ceiling_diagonal_type = DiagonalType::NE;

                            sector->ceiling_corners[0][2] += overall_adjustment - static_cast<glm::float_t>(slope_t11 * MeteringStep);
                            sector->ceiling_corners[1][2] += overall_adjustment - static_cast<glm::float_t>(slope_t10 * MeteringStep);
                            sector->ceiling_corners[2][2] += overall_adjustment - static_cast<glm::float_t>(slope_t13 * MeteringStep);
                            sector->ceiling_corners[3][2] += overall_adjustment - static_cast<glm::float_t>(slope_t12 * MeteringStep);

                            if(function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW)
                            {
                                sector->ceiling_penetration_config = PenetrationConfig::DoorVerticalA;
                            }
                            else if(function == TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE)
                            {
                                sector->ceiling_penetration_config = PenetrationConfig::DoorVerticalB;
                            }
                            else
                            {
                                sector->ceiling_penetration_config = PenetrationConfig::Solid;
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

    void Res_Sector_FixHeights(RoomSector* sector)
    {
        if(sector->floor == MeteringWallHeight)
        {
            sector->floor_penetration_config = PenetrationConfig::Wall;
        }
        if(sector->ceiling == MeteringWallHeight)
        {
            sector->ceiling_penetration_config = PenetrationConfig::Wall;
        }

        // Fix non-material crevices

        for(size_t i = 0; i < 4; i++)
        {
            if(sector->ceiling_corners[i][2] == sector->floor_corners[i][2])
                sector->ceiling_corners[i][2] += LaraHangVerticalEpsilon;
        }
    }

    void GenerateAnimCommands(SkeletalModel* model)
    {
        if(engine::engine_world.anim_commands.empty())
        {
            return;
        }
        //Sys_DebugLog("anim_transform.txt", "MODEL[%d]", model->id);
        for(size_t anim = 0; anim < model->animations.size(); anim++)
        {
            if(model->animations[anim].num_anim_commands > 255)
            {
                continue;                                                           // If no anim commands or current anim has more than 255 (according to TRosettaStone).
            }

            animation::AnimationFrame* af = &model->animations[anim];
            if(af->num_anim_commands == 0)
                continue;

            BOOST_ASSERT(af->anim_command < engine::engine_world.anim_commands.size());
            int16_t *pointer = &engine::engine_world.anim_commands[af->anim_command];

            for(uint32_t i = 0; i < af->num_anim_commands; i++)
            {
                const auto command = static_cast<animation::AnimCommandOpcode>(*pointer);
                ++pointer;
                switch(command)
                {
                    /*
                     * End-of-anim commands:
                     */
                    case animation::AnimCommandOpcode::SetPosition:
                        af->animCommands.push_back({ command, pointer[0], pointer[1], pointer[2] });
                        // ConsoleInfo::instance().printf("ACmd MOVE: anim = %d, x = %d, y = %d, z = %d", static_cast<int>(anim), pointer[0], pointer[1], pointer[2]);
                        pointer += 3;
                        break;

                    case animation::AnimCommandOpcode::SetVelocity:
                        af->animCommands.push_back({ command, pointer[0], pointer[1], 0 });
                        // ConsoleInfo::instance().printf("ACmd JUMP: anim = %d, vVert = %d, vHoriz = %d", static_cast<int>(anim), pointer[0], pointer[1]);
                        pointer += 2;
                        break;

                    case animation::AnimCommandOpcode::EmptyHands:
                        af->animCommands.push_back({ command, 0, 0, 0 });
                        // ConsoleInfo::instance().printf("ACmd EMTYHANDS: anim = %d", static_cast<int>(anim));
                        break;

                    case animation::AnimCommandOpcode::Kill:
                        af->animCommands.push_back({ command, 0, 0, 0 });
                        // ConsoleInfo::instance().printf("ACmd KILL: anim = %d", static_cast<int>(anim));
                        break;

                        /*
                         * Per frame commands:
                         */
                    case animation::AnimCommandOpcode::PlaySound:
                        if(pointer[0] < static_cast<int>(af->keyFrames.size()))
                        {
                            af->keyFrames[pointer[0]].animCommands.push_back({ command, pointer[1], 0, 0 });
                        }
                        // ConsoleInfo::instance().printf("ACmd PLAYSOUND: anim = %d, frame = %d of %d", static_cast<int>(anim), pointer[0], static_cast<int>(af->frames.size()));
                        pointer += 2;
                        break;

                    case animation::AnimCommandOpcode::PlayEffect:
                        if(pointer[0] < static_cast<int>(af->keyFrames.size()))
                        {
                            af->keyFrames[pointer[0]].animCommands.push_back({ command, pointer[1], 0, 0 });
                        }
                        //                    ConsoleInfo::instance().printf("ACmd FLIPEFFECT: anim = %d, frame = %d of %d", static_cast<int>(anim), pointer[0], static_cast<int>(af->frames.size()));
                        pointer += 2;
                        break;
                }
            }
        }
    }

    bool TR_IsSectorsIn2SideOfPortal(RoomSector* s1, RoomSector* s2, const Portal& p)
    {
        if(util::fuzzyEqual(s1->position[0], s2->position[0]) && !util::fuzzyEqual(s1->position[1], s2->position[1]) && glm::abs(p.normal[1]) > 0.99)
        {
            glm::float_t min_x, max_x, min_y, max_y;
            max_x = min_x = p.vertices.front().x;
            for(const auto& v : p.vertices)
            {
                if(v.x > max_x)
                {
                    max_x = v.x;
                }
                if(v.x < min_x)
                {
                    min_x = v.x;
                }
            }
            if(s1->position[1] > s2->position[1])
            {
                min_y = s2->position[1];
                max_y = s1->position[1];
            }
            else
            {
                min_y = s1->position[1];
                max_y = s2->position[1];
            }

            if((s1->position[0] < max_x) && (s1->position[0] > min_x) && (p.center[1] < max_y) && (p.center[1] > min_y))
            {
                return true;
            }
        }
        else if(!util::fuzzyEqual(s1->position[0], s2->position[0]) && util::fuzzyEqual(s1->position[1], s2->position[1]) && glm::abs(p.normal[0]) > 0.99)
        {
            glm::float_t min_x, max_x, min_y, max_y;
            max_y = min_y = p.vertices.front().y;
            for(const auto& v : p.vertices)
            {
                if(v.y > max_y)
                {
                    max_y = v.y;
                }
                if(v.y < min_y)
                {
                    min_y = v.y;
                }
            }
            if(s1->position[0] > s2->position[0])
            {
                min_x = s2->position[0];
                max_x = s1->position[0];
            }
            else
            {
                min_x = s1->position[0];
                max_x = s2->position[0];
            }

            if((p.center[0] < max_x) && (p.center[0] > min_x) && (s1->position[1] < max_y) && (s1->position[1] > min_y))
            {
                return true;
            }
        }

        return false;
    }

    void TR_Sector_Calculate(World *world, const std::unique_ptr<loader::Level>& tr, long int room_index)
    {
        std::shared_ptr<Room> room = world->rooms[room_index];
        loader::Room *tr_room = &tr->m_rooms[room_index];

        /*
         * Sectors loading
         */

        for(size_t i = 0; i < room->sectors.num_elements(); i++)
        {
            RoomSector* sector = &room->sectors[i/room->sectors.shape()[1]][i%room->sectors.shape()[1]];
            /*
             * Let us fill pointers to sectors above and sectors below
             */

            uint8_t rp = tr_room->sector_list[i].room_below;
            sector->sector_below = nullptr;
            if(rp < world->rooms.size() && rp != 255)
            {
                sector->sector_below = world->rooms[rp]->getSectorRaw(sector->position);
            }
            rp = tr_room->sector_list[i].room_above;
            sector->sector_above = nullptr;
            if(rp < world->rooms.size() && rp != 255)
            {
                sector->sector_above = world->rooms[rp]->getSectorRaw(sector->position);
            }

            RoomSector* near_sector = nullptr;

            /**** OX *****/
            if((sector->index_y > 0) && (sector->index_y < room->sectors.shape()[1] - 1) && (sector->index_x == 0))
            {
                near_sector = sector + room->sectors.shape()[1];
            }
            if((sector->index_y > 0) && (sector->index_y < room->sectors.shape()[1] - 1) && (sector->index_x == room->sectors.shape()[0] - 1))
            {
                near_sector = sector - room->sectors.shape()[1];
            }
            /**** OY *****/
            if((sector->index_x > 0) && (sector->index_x < room->sectors.shape()[0] - 1) && (sector->index_y == 0))
            {
                near_sector = sector + 1;
            }
            if((sector->index_x > 0) && (sector->index_x < room->sectors.shape()[0] - 1) && (sector->index_y == room->sectors.shape()[1] - 1))
            {
                near_sector = sector - 1;
            }

            if((near_sector != nullptr) && (sector->portal_to_room >= 0))
            {
                for(const Portal& p : room->portals)
                {
                    if(util::fuzzyZero(p.normal[2]))
                    {
                        RoomSector* dst = p.destination ? p.destination->getSectorRaw(sector->position) : nullptr;
                        RoomSector* orig_dst = engine::engine_world.rooms[sector->portal_to_room]->getSectorRaw(sector->position);

                        if((dst != nullptr) && (dst->portal_to_room < 0) && (dst->floor != MeteringWallHeight) && (dst->ceiling != MeteringWallHeight) && (static_cast<uint32_t>(sector->portal_to_room) != p.destination->getId()) && (dst->floor < orig_dst->floor) && TR_IsSectorsIn2SideOfPortal(near_sector, dst, p))
                        {
                            sector->portal_to_room = p.destination->getId();
                        }
                    }
                }
            }
        }
    }

    RoomSector* TR_GetRoomSector(uint32_t room_id, int sx, int sy)
    {
        if(room_id >= engine::engine_world.rooms.size())
        {
            return nullptr;
        }

        auto room = engine::engine_world.rooms[room_id];
        if(sx < 0 || static_cast<size_t>(sx) >= room->sectors.shape()[0] || sy < 0 || static_cast<size_t>(sy) >= room->sectors.shape()[1])
        {
            return nullptr;
        }

        return &room->sectors[sx][sy];
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
        if(floor.is<lua::Integer>()) rs->floor = floor;
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
        if(ceil.is<lua::Integer>()) rs->ceiling = ceil;

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

        if(p < engine::engine_world.rooms.size())
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
        std::string temp_script_name = engine::getAutoexecName(engine_version, std::string());

        if(engine::fileExists(temp_script_name, false))
        {
            try
            {
                engine_lua.doFile(temp_script_name);
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

    void TR_GenWorld(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        world->engineVersion = loader::gameToEngine(tr->m_gameVersion);

        Res_AutoexecOpen(tr->m_gameVersion);    // Open and do preload autoexec.
        engine_lua.call("autoexec_PreLoad");
        gui::drawLoadScreen(150);

        Res_GenRBTrees(world);
        gui::drawLoadScreen(200);

        TR_GenTextures(world, tr);
        gui::drawLoadScreen(300);

        TR_GenAnimCommands(world, tr);
        gui::drawLoadScreen(310);

        TR_GenAnimTextures(world, tr);
        gui::drawLoadScreen(320);

        TR_GenMeshes(world, tr);
        gui::drawLoadScreen(400);

        TR_GenSprites(world, tr);
        gui::drawLoadScreen(420);

        TR_GenBoxes(world, tr);
        gui::drawLoadScreen(440);

        TR_GenCameras(world, tr);
        gui::drawLoadScreen(460);

        TR_GenRooms(world, tr);
        gui::drawLoadScreen(500);

        Res_GenRoomFlipMap(world);
        gui::drawLoadScreen(520);

        TR_GenSkeletalModels(world, tr);
        gui::drawLoadScreen(600);

        TR_GenEntities(world, tr);
        gui::drawLoadScreen(650);

        Res_GenBaseItems(world);
        gui::drawLoadScreen(680);

        Res_GenSpritesBuffer(world);        // Should be done ONLY after TR_GenEntities.
        gui::drawLoadScreen(700);

        TR_GenRoomProperties(world, tr);
        gui::drawLoadScreen(750);

        Res_GenRoomCollision(world);
        gui::drawLoadScreen(800);

        world->audioEngine.load(world, tr);
        gui::drawLoadScreen(850);

        world->sky_box = Res_GetSkybox(world, world->engineVersion);
        gui::drawLoadScreen(860);

        Res_GenEntityFunctions(world->entity_tree);
        gui::drawLoadScreen(910);

        Res_GenVBOs(world);
        gui::drawLoadScreen(950);

        engine_lua.doFile("scripts/autoexec.lua");  // Postload autoexec.
        engine_lua.call("autoexec_PostLoad");
        gui::drawLoadScreen(960);

        Res_FixRooms(world);                        // Fix initial room states
        gui::drawLoadScreen(970);
    }

    void Res_GenRBTrees(World *world)
    {
        world->entity_tree.clear();
        world->next_entity_id = 0;
        world->items_tree.clear();
    }

    void TR_GenRooms(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        world->rooms.resize(tr->m_rooms.size());
        for(uint32_t i = 0; i < world->rooms.size(); ++i)
            world->rooms[i] = std::make_shared<Room>(i);
        for(uint32_t i = 0; i < world->rooms.size(); i++)
        {
            TR_GenRoom(world->rooms[i], world, tr);
        }
    }

    void TR_GenRoom(std::shared_ptr<Room>& room, World *world, const std::unique_ptr<loader::Level>& tr)
    {
        room->active = true;
        room->flags = tr->m_rooms[room->getId()].flags;
        room->light_mode = tr->m_rooms[room->getId()].light_mode;
        room->reverb_info = tr->m_rooms[room->getId()].reverb_info;
        room->water_scheme = tr->m_rooms[room->getId()].water_scheme;
        room->alternate_group = tr->m_rooms[room->getId()].alternate_group;

        room->transform = glm::translate(glm::mat4(1.0f), { tr->m_rooms[room->getId()].offset.x, -tr->m_rooms[room->getId()].offset.z, tr->m_rooms[room->getId()].offset.y });
        room->ambient_lighting[0] = tr->m_rooms[room->getId()].light_colour.r * 2;
        room->ambient_lighting[1] = tr->m_rooms[room->getId()].light_colour.g * 2;
        room->ambient_lighting[2] = tr->m_rooms[room->getId()].light_colour.b * 2;
        room->setRoom(room.get());
        room->near_room_list.clear();
        room->overlapped_room_list.clear();

        room->genMesh(world, room->getId(), tr);

        room->bt_body.reset();

        /*
         *  let us load static room meshes
         */
        room->static_mesh.clear();

        const loader::Room *tr_room = &tr->m_rooms[room->getId()];
        for(uint16_t i = 0; i < tr_room->static_meshes.size(); i++)
        {
            const loader::StaticMesh* tr_static = tr->findStaticMeshById(tr_room->static_meshes[i].object_id);
            if(tr_static == nullptr)
            {
                continue;
            }
            room->static_mesh.emplace_back(std::make_shared<StaticMesh>(tr_room->static_meshes[i].object_id));
            std::shared_ptr<StaticMesh> r_static = room->static_mesh.back();
            r_static->setRoom(room.get());
            r_static->mesh = world->meshes[tr->m_meshIndices[tr_static->mesh]];
            r_static->position[0] = tr_room->static_meshes[i].position.x;
            r_static->position[1] = -tr_room->static_meshes[i].position.z;
            r_static->position[2] = tr_room->static_meshes[i].position.y;
            r_static->rotation[0] = tr_room->static_meshes[i].rotation;
            r_static->rotation[1] = 0.0;
            r_static->rotation[2] = 0.0;
            r_static->tint[0] = tr_room->static_meshes[i].tint.r * 2;
            r_static->tint[1] = tr_room->static_meshes[i].tint.g * 2;
            r_static->tint[2] = tr_room->static_meshes[i].tint.b * 2;
            r_static->tint[3] = tr_room->static_meshes[i].tint.a * 2;

            r_static->collisionBoundingBox.min[0] = tr_static->collision_box[0].x;
            r_static->collisionBoundingBox.min[1] = -tr_static->collision_box[0].z;
            r_static->collisionBoundingBox.min[2] = tr_static->collision_box[1].y;
            r_static->collisionBoundingBox.max[0] = tr_static->collision_box[1].x;
            r_static->collisionBoundingBox.max[1] = -tr_static->collision_box[1].z;
            r_static->collisionBoundingBox.max[2] = tr_static->collision_box[0].y;

            r_static->visibleBoundingBox.min[0] = tr_static->visibility_box[0].x;
            r_static->visibleBoundingBox.min[1] = -tr_static->visibility_box[0].z;
            r_static->visibleBoundingBox.min[2] = tr_static->visibility_box[1].y;

            r_static->visibleBoundingBox.max[0] = tr_static->visibility_box[1].x;
            r_static->visibleBoundingBox.max[1] = -tr_static->visibility_box[1].z;
            r_static->visibleBoundingBox.max[2] = tr_static->visibility_box[0].y;

            r_static->obb.transform = &room->static_mesh[i]->transform;
            r_static->obb.radius = room->static_mesh[i]->mesh->m_radius;
            r_static->transform = glm::rotate(glm::translate(glm::mat4(1.0f), r_static->position), glm::radians(r_static->rotation[0]), { 0,0,1 });
            r_static->was_rendered = 0;
            r_static->obb.rebuild(r_static->visibleBoundingBox);
            r_static->obb.doTransform();

            r_static->bt_body = nullptr;
            r_static->hide = false;

            // Disable static mesh collision, if flag value is 3 (TR1) or all bounding box
            // coordinates are equal (TR2-5).

            if(tr_static->flags == 3 ||
               (    tr_static->collision_box[0].x == -tr_static->collision_box[0].y
                 && tr_static->collision_box[0].y ==  tr_static->collision_box[0].z
                 && tr_static->collision_box[1].x == -tr_static->collision_box[1].y
                 && tr_static->collision_box[1].y ==  tr_static->collision_box[1].z ))
            {
                r_static->setCollisionType(world::CollisionType::None);
            }
            else
            {
                r_static->setCollisionType(world::CollisionType::Static);
                r_static->setCollisionShape(world::CollisionShape::Box);
            }

            // Set additional static mesh properties from level script override.

            Res_SetStaticMeshProperties(r_static);

            // Set static mesh collision.
            if(r_static->getCollisionType() != world::CollisionType::None)
            {
                btCollisionShape* cshape;
                switch(r_static->getCollisionShape())
                {
                    case world::CollisionShape::Box:
                        cshape = core::BT_CSfromBBox(r_static->collisionBoundingBox, true, true);
                        break;

                    case world::CollisionShape::BoxBase:
                        cshape = core::BT_CSfromBBox(r_static->mesh->boundingBox, true, true);
                        break;

                    case world::CollisionShape::TriMesh:
                        cshape = core::BT_CSfromMesh(r_static->mesh, true, true, true);
                        break;

                    case world::CollisionShape::TriMeshConvex:
                        cshape = core::BT_CSfromMesh(r_static->mesh, true, true, false);
                        break;

                    default:
                        cshape = nullptr;
                        break;
                };

                if(cshape)
                {
                    btTransform startTransform;
                    startTransform.setFromOpenGLMatrix(glm::value_ptr(r_static->transform));
                    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
                    btVector3 localInertia(0, 0, 0);
                    r_static->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
                    engine::bt_engine_dynamicsWorld->addRigidBody(r_static->bt_body, COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
                    r_static->bt_body->setUserPointer(r_static.get());
                }
            }
        }

        /*
         * sprites loading section
         */
        for(uint32_t i = 0; i < tr_room->sprites.size(); i++)
        {
            room->sprites.emplace_back();
            if((tr_room->sprites[i].texture >= 0) && (static_cast<uint32_t>(tr_room->sprites[i].texture) < world->sprites.size()))
            {
                room->sprites[i].sprite = &world->sprites[tr_room->sprites[i].texture];
                room->sprites[i].pos = util::convert(tr_room->vertices[tr_room->sprites[i].vertex].vertex);
                room->sprites[i].pos += glm::vec3(room->transform[3]);
            }
        }

        /*
         * let us load sectors
         */
        room->sectors.resize(boost::extents[tr_room->num_xsectors][tr_room->num_zsectors]);

        /*
         * base sectors information loading and collisional mesh creation
         */

         // To avoid manipulating with unnecessary information, we declare simple
         // heightmap here, which will be operated with sector and floordata parsing,
         // then vertical inbetween polys will be constructed, and Bullet collisional
         // object will be created. Afterwards, this heightmap also can be used to
         // quickly detect slopes for pushable blocks and other entities that rely on
         // floor level.

        for(size_t i = 0; i < room->sectors.num_elements(); i++)
        {
            const auto indexX = i / room->sectors.shape()[1];
            const auto indexY = i % room->sectors.shape()[1];
            RoomSector* const sector = &room->sectors[indexX][indexY];
            // Filling base sectors information.

            sector->index_x = indexX;
            sector->index_y = indexY;

            sector->position[0] = room->transform[3][0] + sector->index_x * MeteringSectorSize + 0.5f * MeteringSectorSize;
            sector->position[1] = room->transform[3][1] + sector->index_y * MeteringSectorSize + 0.5f * MeteringSectorSize;
            sector->position[2] = 0.5f * (tr_room->y_bottom + tr_room->y_top);

            sector->owner_room = room;

            if(tr->m_gameVersion < loader::Game::TR3)
            {
                sector->box_index = tr_room->sector_list[i].box_index;
                sector->material = SECTOR_MATERIAL_STONE;
            }
            else
            {
                sector->box_index = (tr_room->sector_list[i].box_index & 0xFFF0) >> 4;
                sector->material = tr_room->sector_list[i].box_index & 0x000F;
            }

            if(sector->box_index == 0xFFFF) sector->box_index = -1;

            sector->flags = 0;  // Clear sector flags.

            sector->floor = -MeteringStep * static_cast<int>(tr_room->sector_list[i].floor);
            sector->ceiling = -MeteringStep * static_cast<int>(tr_room->sector_list[i].ceiling);
            sector->trig_index = tr_room->sector_list[i].fd_index;

            // BUILDING CEILING HEIGHTMAP.

            // Penetration config is used later to build inbetween vertical collision polys.
            // If sector's penetration config is a wall, we simply build a vertical plane to
            // isolate this sector from top to bottom. Also, this allows to trick out wall
            // sectors inside another wall sectors to be ignored completely when building
            // collisional mesh.
            // Door penetration config means that we should either ignore sector collision
            // completely (classic door) or ignore one of the triangular sector parts (TR3+).

            if(sector->ceiling == MeteringWallHeight)
            {
                sector->ceiling_penetration_config = PenetrationConfig::Wall;
            }
            else if(tr_room->sector_list[i].room_above != 0xFF)
            {
                sector->ceiling_penetration_config = PenetrationConfig::Ghost;
            }
            else
            {
                sector->ceiling_penetration_config = PenetrationConfig::Solid;
            }

            // Reset some sector parameters to avoid garbaged memory issues.

            sector->portal_to_room = -1;
            sector->ceiling_diagonal_type = DiagonalType::None;
            sector->floor_diagonal_type = DiagonalType::None;

            // Now, we define heightmap cells position and draft (flat) height.
            // Draft height is derived from sector's floor and ceiling values, which are
            // copied into heightmap cells Y coordinates. As result, we receive flat
            // heightmap cell, which will be operated later with floordata.

            sector->ceiling_corners[0][0] = sector->index_x * MeteringSectorSize;
            sector->ceiling_corners[0][1] = sector->index_y * MeteringSectorSize + MeteringSectorSize;
            sector->ceiling_corners[0][2] = sector->ceiling;

            sector->ceiling_corners[1][0] = sector->index_x * MeteringSectorSize + MeteringSectorSize;
            sector->ceiling_corners[1][1] = sector->index_y * MeteringSectorSize + MeteringSectorSize;
            sector->ceiling_corners[1][2] = sector->ceiling;

            sector->ceiling_corners[2][0] = sector->index_x * MeteringSectorSize + MeteringSectorSize;
            sector->ceiling_corners[2][1] = sector->index_y * MeteringSectorSize;
            sector->ceiling_corners[2][2] = sector->ceiling;

            sector->ceiling_corners[3][0] = sector->index_x * MeteringSectorSize;
            sector->ceiling_corners[3][1] = sector->index_y * MeteringSectorSize;
            sector->ceiling_corners[3][2] = sector->ceiling;

            // BUILDING FLOOR HEIGHTMAP.

            // Features same steps as for the ceiling.

            if(sector->floor == MeteringWallHeight)
            {
                sector->floor_penetration_config = PenetrationConfig::Wall;
            }
            else if(tr_room->sector_list[i].room_below != 0xFF)
            {
                sector->floor_penetration_config = PenetrationConfig::Ghost;
            }
            else
            {
                sector->floor_penetration_config = PenetrationConfig::Solid;
            }

            sector->floor_corners[0][0] = sector->index_x * MeteringSectorSize;
            sector->floor_corners[0][1] = sector->index_y * MeteringSectorSize + MeteringSectorSize;
            sector->floor_corners[0][2] = sector->floor;

            sector->floor_corners[1][0] = sector->index_x * MeteringSectorSize + MeteringSectorSize;
            sector->floor_corners[1][1] = sector->index_y * MeteringSectorSize + MeteringSectorSize;
            sector->floor_corners[1][2] = sector->floor;

            sector->floor_corners[2][0] = sector->index_x * MeteringSectorSize + MeteringSectorSize;
            sector->floor_corners[2][1] = sector->index_y * MeteringSectorSize;
            sector->floor_corners[2][2] = sector->floor;

            sector->floor_corners[3][0] = sector->index_x * MeteringSectorSize;
            sector->floor_corners[3][1] = sector->index_y * MeteringSectorSize;
            sector->floor_corners[3][2] = sector->floor;
        }

        /*
         *  load lights
         */
        room->lights.resize(tr_room->lights.size());

        for(size_t i = 0; i < tr_room->lights.size(); i++)
        {
            room->lights[i].light_type = tr_room->lights[i].getLightType();

            room->lights[i].position[0] = tr_room->lights[i].position.x;
            room->lights[i].position[1] = -tr_room->lights[i].position.z;
            room->lights[i].position[2] = tr_room->lights[i].position.y;

            if(room->lights[i].light_type == loader::LightType::Shadow)
            {
                room->lights[i].colour[0] = -(tr_room->lights[i].color.r / 255.0f) * tr_room->lights[i].intensity;
                room->lights[i].colour[1] = -(tr_room->lights[i].color.g / 255.0f) * tr_room->lights[i].intensity;
                room->lights[i].colour[2] = -(tr_room->lights[i].color.b / 255.0f) * tr_room->lights[i].intensity;
                room->lights[i].colour[3] = 1.0f;
            }
            else
            {
                room->lights[i].colour[0] = (tr_room->lights[i].color.r / 255.0f) * tr_room->lights[i].intensity;
                room->lights[i].colour[1] = (tr_room->lights[i].color.g / 255.0f) * tr_room->lights[i].intensity;
                room->lights[i].colour[2] = (tr_room->lights[i].color.b / 255.0f) * tr_room->lights[i].intensity;
                room->lights[i].colour[3] = 1.0f;
            }

            room->lights[i].inner = tr_room->lights[i].r_inner;
            room->lights[i].outer = tr_room->lights[i].r_outer;
            room->lights[i].length = tr_room->lights[i].length;
            room->lights[i].cutoff = tr_room->lights[i].cutoff;

            room->lights[i].falloff = 0.001f / room->lights[i].outer;
        }

        /*
         * portals loading / calculation!!!
         */
        for(size_t i = 0; i < tr_room->portals.size(); i++)
        {
            std::shared_ptr<Room> r_dest = world->rooms[tr_room->portals[i].adjoining_room];
            room->portals.emplace_back(tr_room->portals[i], r_dest.get(), room->transform);
        }

        /*
         * room borders calculation
         */
        room->boundingBox.min[2] = tr_room->y_bottom;
        room->boundingBox.max[2] = tr_room->y_top;

        room->boundingBox.min[0] = room->transform[3][0] + MeteringSectorSize;
        room->boundingBox.min[1] = room->transform[3][1] + MeteringSectorSize;
        room->boundingBox.max[0] = room->transform[3][0] + MeteringSectorSize * room->sectors.shape()[0] - MeteringSectorSize;
        room->boundingBox.max[1] = room->transform[3][1] + MeteringSectorSize * room->sectors.shape()[1] - MeteringSectorSize;

        /*
         * alternate room pointer calculation if one exists.
         */
        room->alternate_room = nullptr;
        room->base_room = nullptr;

        if((tr_room->alternate_room >= 0) && (static_cast<uint32_t>(tr_room->alternate_room) < tr->m_rooms.size()))
        {
            room->alternate_room = world->rooms[tr_room->alternate_room];
        }
    }

    void Res_GenRoomCollision(World *world)
    {
        for(std::shared_ptr<Room> room : world->rooms)
        {
            // Inbetween polygons array is later filled by loop which scans adjacent
            // sector heightmaps and fills the gaps between them, thus creating inbetween
            // polygon. Inbetweens can be either quad (if all four corner heights are
            // different), triangle (if one corner height is similar to adjacent) or
            // ghost (if corner heights are completely similar). In case of quad inbetween,
            // two triangles are added to collisional trimesh, in case of triangle inbetween,
            // we add only one, and in case of ghost inbetween, we ignore it.

            // Most difficult task with converting floordata collision to trimesh collision is
            // building inbetween polygons which will block out gaps between sector heights.
            std::vector<SectorTween> room_tween = Res_Sector_GenTweens(room);

            // Final step is sending actual sectors to Bullet collision model. We do it here.

            btCollisionShape *cshape = BT_CSfromHeightmap(room->sectors, room_tween, true, true);

            if(!cshape)
                continue;

            btVector3 localInertia(0, 0, 0);
            btTransform tr;
            tr.setFromOpenGLMatrix(glm::value_ptr(room->transform));
            btDefaultMotionState* motionState = new btDefaultMotionState(tr);
            room->bt_body.reset(new btRigidBody(0.0, motionState, cshape, localInertia));
            engine::bt_engine_dynamicsWorld->addRigidBody(room->bt_body.get(), COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
            room->bt_body->setUserPointer(room.get());
            room->bt_body->setRestitution(1.0);
            room->bt_body->setFriction(1.0);
            room->setCollisionType(world::CollisionType::Static);                    // meshtree
            room->setCollisionShape(world::CollisionShape::TriMesh);
        }
    }

    void TR_GenRoomProperties(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        for(uint32_t i = 0; i < world->rooms.size(); i++)
        {
            std::shared_ptr<Room> r = world->rooms[i];
            if(r->alternate_room != nullptr)
            {
                r->alternate_room->base_room = r;   // Refill base room pointer.
            }

            // Fill heightmap and translate floordata.
            for(auto column : r->sectors)
            {
                for(RoomSector& sector : column)
                {
                    TR_Sector_TranslateFloorData(&sector, tr);
                    Res_Sector_FixHeights(&sector);
                }
            }

            // Generate links to the near rooms.
            r->buildNearRoomsList();
            // Generate links to the overlapped rooms.
            r->buildOverlappedRoomsList();

            // Basic sector calculations.
            TR_Sector_Calculate(world, tr, i);
        }
    }

    void Res_GenRoomFlipMap(World *world)
    {
        // Flipmap count is hardcoded, as no original levels contain such info.
        world->flip_data.resize(FLIPMAP_MAX_NUMBER);
    }

    void TR_GenBoxes(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        world->room_boxes.clear();

        for(uint32_t i = 0; i < tr->m_boxes.size(); i++)
        {
            world->room_boxes.emplace_back();
            auto& room = world->room_boxes.back();
            room.overlap_index = tr->m_boxes[i].overlap_index;
            room.true_floor = -tr->m_boxes[i].true_floor;
            room.x_min = tr->m_boxes[i].xmin;
            room.x_max = tr->m_boxes[i].xmax;
            room.y_min = -static_cast<int>(tr->m_boxes[i].zmax);
            room.y_max = -static_cast<int>(tr->m_boxes[i].zmin);
        }
    }

    void TR_GenCameras(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        world->cameras_sinks.clear();

        for(uint32_t i = 0; i < tr->m_cameras.size(); i++)
        {
            world->cameras_sinks.emplace_back();
            world->cameras_sinks[i].x = tr->m_cameras[i].x;
            world->cameras_sinks[i].y = tr->m_cameras[i].z;
            world->cameras_sinks[i].z = -tr->m_cameras[i].y;
            world->cameras_sinks[i].room_or_strength = tr->m_cameras[i].room;
            world->cameras_sinks[i].flag_or_zone = tr->m_cameras[i].unknown1;
        }
    }

    /**
     * sprites loading, works correct in TR1 - TR5
     */
    void TR_GenSprites(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        if(tr->m_spriteTextures.empty())
        {
            world->sprites.clear();
            return;
        }

        for(size_t i = 0; i < tr->m_spriteTextures.size(); i++)
        {
            world->sprites.emplace_back();
            auto s = &world->sprites.back();

            auto tr_st = &tr->m_spriteTextures[i];

            s->left = tr_st->left_side;
            s->right = tr_st->right_side;
            s->top = tr_st->top_side;
            s->bottom = tr_st->bottom_side;

            world->tex_atlas->getSpriteCoordinates(i, s->texture, s->tex_coord);
        }

        for(uint32_t i = 0; i < tr->m_spriteSequences.size(); i++)
        {
            if((tr->m_spriteSequences[i].offset >= 0) && (static_cast<uint32_t>(tr->m_spriteSequences[i].offset) < world->sprites.size()))
            {
                world->sprites[tr->m_spriteSequences[i].offset].id = tr->m_spriteSequences[i].object_id;
            }
        }
    }

    void Res_GenSpritesBuffer(World *world)
    {
        for(auto room : world->rooms)
            Res_GenRoomSpritesBuffer(room);
    }

    void TR_GenTextures(World* world, const std::unique_ptr<loader::Level>& tr)
    {
        int border_size = glm::clamp(render::renderer.settings().texture_border, 0, 64);

        world->tex_atlas.reset(new BorderedTextureAtlas(border_size,
                                                        render::renderer.settings().save_texture_memory,
                                                        tr->m_textures,
                                                        tr->m_objectTextures,
                                                        tr->m_spriteTextures));

        world->textures.resize(world->tex_atlas->getNumAtlasPages() + 1);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelZoom(1, 1);
        world->tex_atlas->createTextures(world->textures.data(), 1);

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

        glBindTexture(GL_TEXTURE_2D, world->textures.back());          // solid color =)
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
    void TR_GenAnimTextures(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        int32_t uvrotate_script = 0;

        core::Polygon p0;
        p0.vertices.resize(3);

        core::Polygon p;
        p.vertices.resize(3);

        const uint16_t* pointer = tr->m_animatedTextures.data();
        const uint16_t num_uvrotates = tr->m_animatedTexturesUvCount;

        const uint16_t num_sequences = *(pointer++);   // First word in a stream is sequence count.

        world->anim_sequences.resize(num_sequences);

        for(size_t i = 0; i < world->anim_sequences.size(); i++)
        {
            animation::AnimSeq* seq = &world->anim_sequences[i];
            seq->frames.resize(*(pointer++) + 1);
            seq->frame_list.resize(seq->frames.size());

            // Fill up new sequence with frame list.
            seq->anim_type = animation::AnimTextureType::Forward;
            seq->frame_lock = false; // by default anim is playing
            seq->uvrotate = false; // by default uvrotate
            seq->reverse_direction = false; // Needed for proper reverse-type start-up.
            seq->frame_rate = util::MilliSeconds(50);  // Should be passed as 1 / FPS.
            seq->frame_time = util::Duration(0);   // Reset frame time to initial state.
            seq->current_frame = 0;     // Reset current frame to zero.

            for(size_t j = 0; j < seq->frames.size(); j++)
            {
                seq->frame_list[j] = *(pointer++);  // Add one frame.
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
                seq->frames.resize(8);
                seq->uvrotate_max = world->tex_atlas->getTextureHeight(seq->frame_list[0]) / 2;
                seq->uvrotate_speed = seq->uvrotate_max / static_cast<glm::float_t>(seq->frames.size());
                seq->frame_list.resize(8);

                if(uvrotate_script > 0)
                {
                    seq->anim_type = animation::AnimTextureType::Forward;
                }
                else if(uvrotate_script < 0)
                {
                    seq->anim_type = animation::AnimTextureType::Backward;
                }

                engine::engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p, 0, true);
                for(uint16_t j = 0; j < seq->frames.size(); j++)
                {
                    engine::engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p, j * seq->uvrotate_speed, true);
                    seq->frames[j].tex_ind = p.tex_index;

                    GLfloat A0[2], B0[2], A[2], B[2], d;                            ///@PARANOID: texture transformation may be not only move
                    A0[0] = p0.vertices[1].tex_coord[0] - p0.vertices[0].tex_coord[0];
                    A0[1] = p0.vertices[1].tex_coord[1] - p0.vertices[0].tex_coord[1];
                    B0[0] = p0.vertices[2].tex_coord[0] - p0.vertices[0].tex_coord[0];
                    B0[1] = p0.vertices[2].tex_coord[1] - p0.vertices[0].tex_coord[1];

                    A[0] = p.vertices[1].tex_coord[0] - p.vertices[0].tex_coord[0];
                    A[1] = p.vertices[1].tex_coord[1] - p.vertices[0].tex_coord[1];
                    B[0] = p.vertices[2].tex_coord[0] - p.vertices[0].tex_coord[0];
                    B[1] = p.vertices[2].tex_coord[1] - p.vertices[0].tex_coord[1];

                    d = A0[0] * B0[1] - A0[1] * B0[0];
                    seq->frames[j].mat[0 + 0 * 2] = (A[0] * B0[1] - A0[1] * B[0]) / d;
                    seq->frames[j].mat[1 + 0 * 2] = -(A[1] * B0[1] - A0[1] * B[1]) / d;
                    seq->frames[j].mat[0 + 1 * 2] = -(A0[0] * B[0] - A[0] * B0[0]) / d;
                    seq->frames[j].mat[1 + 1 * 2] = (A0[0] * B[1] - A[1] * B0[0]) / d;

                    seq->frames[j].move[0] = p.vertices[0].tex_coord[0] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[0 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[0 + 1 * 2]);
                    seq->frames[j].move[1] = p.vertices[0].tex_coord[1] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[1 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[1 + 1 * 2]);
                }
            }
            else
            {
                engine::engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p0);
                for(uint16_t j = 0; j < seq->frames.size(); j++)
                {
                    engine::engine_world.tex_atlas->getCoordinates(seq->frame_list[j], false, &p);
                    seq->frames[j].tex_ind = p.tex_index;

                    GLfloat A0[2], B0[2], A[2], B[2], d;                            ///@PARANOID: texture transformation may be not only move
                    A0[0] = p0.vertices[1].tex_coord[0] - p0.vertices[0].tex_coord[0];
                    A0[1] = p0.vertices[1].tex_coord[1] - p0.vertices[0].tex_coord[1];
                    B0[0] = p0.vertices[2].tex_coord[0] - p0.vertices[0].tex_coord[0];
                    B0[1] = p0.vertices[2].tex_coord[1] - p0.vertices[0].tex_coord[1];

                    A[0] = p.vertices[1].tex_coord[0] - p.vertices[0].tex_coord[0];
                    A[1] = p.vertices[1].tex_coord[1] - p.vertices[0].tex_coord[1];
                    B[0] = p.vertices[2].tex_coord[0] - p.vertices[0].tex_coord[0];
                    B[1] = p.vertices[2].tex_coord[1] - p.vertices[0].tex_coord[1];

                    d = A0[0] * B0[1] - A0[1] * B0[0];
                    seq->frames[j].mat[0 + 0 * 2] = (A[0] * B0[1] - A0[1] * B[0]) / d;
                    seq->frames[j].mat[1 + 0 * 2] = -(A[1] * B0[1] - A0[1] * B[1]) / d;
                    seq->frames[j].mat[0 + 1 * 2] = -(A0[0] * B[0] - A[0] * B0[0]) / d;
                    seq->frames[j].mat[1 + 1 * 2] = (A0[0] * B[1] - A[1] * B0[0]) / d;

                    seq->frames[j].move[0] = p.vertices[0].tex_coord[0] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[0 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[0 + 1 * 2]);
                    seq->frames[j].move[1] = p.vertices[0].tex_coord[1] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[1 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[1 + 1 * 2]);
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
    bool SetAnimTexture(core::Polygon *polygon, uint32_t tex_index, World *world)
    {
        polygon->anim_id = 0;                           // Reset to 0 by default.

        for(uint32_t i = 0; i < world->anim_sequences.size(); i++)
        {
            for(uint16_t j = 0; j < world->anim_sequences[i].frames.size(); j++)
            {
                if(world->anim_sequences[i].frame_list[j] == tex_index)
                {
                    // If we have found assigned texture ID in animation texture lists,
                    // we assign corresponding animation sequence to this polygon,
                    // additionally specifying frame offset.
                    polygon->anim_id = i + 1;  // Animation sequence ID.
                    polygon->frame_offset = j;     // Animation frame offset.
                    return true;
                }
            }
        }

        return false;   // No such TexInfo found in animation textures lists.
    }

    void TR_GenMeshes(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        world->meshes.resize(tr->m_meshes.size());
        size_t i = 0;
        for(std::shared_ptr<core::BaseMesh>& baseMesh : world->meshes)
        {
            baseMesh = std::make_shared<core::BaseMesh>();
            TR_GenMesh(world, i++, baseMesh, tr);
        }
    }

    void tr_copyNormals(core::Polygon *polygon, const std::shared_ptr<core::BaseMesh>& mesh, const uint16_t *mesh_vertex_indices)
    {
        for(size_t i = 0; i < polygon->vertices.size(); ++i)
        {
            polygon->vertices[i].normal = mesh->m_vertices[mesh_vertex_indices[i]].normal;
        }
    }

    void tr_accumulateNormals(loader::Mesh *tr_mesh, core::BaseMesh* mesh, int numCorners, const uint16_t *vertex_indices, core::Polygon *p)
    {
        p->vertices.resize(numCorners);

        for(int i = 0; i < numCorners; i++)
        {
            p->vertices[i].position = util::convert(tr_mesh->vertices[vertex_indices[i]]);
        }
        p->updateNormal();

        for(int i = 0; i < numCorners; i++)
        {
            mesh->m_vertices[vertex_indices[i]].normal += p->plane.normal;
        }
    }

    void tr_setupColoredFace(loader::Mesh *tr_mesh, const std::unique_ptr<loader::Level>& tr, core::BaseMesh* mesh, const uint16_t *vertex_indices, unsigned color, core::Polygon *p)
    {
        for(size_t i = 0; i < p->vertices.size(); i++)
        {
            p->vertices[i].color[0] = tr->m_palette.colour[color].r / 255.0f;
            p->vertices[i].color[1] = tr->m_palette.colour[color].g / 255.0f;
            p->vertices[i].color[2] = tr->m_palette.colour[color].b / 255.0f;
            if(tr_mesh->lights.size() == tr_mesh->vertices.size())
            {
                p->vertices[i].color[0] = p->vertices[i].color[0] * 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
                p->vertices[i].color[1] = p->vertices[i].color[1] * 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
                p->vertices[i].color[2] = p->vertices[i].color[2] * 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
            }
            p->vertices[i].color[3] = 1.0f;

            p->vertices[i].tex_coord[0] = (i & 2) ? 1.0f : 0.0f;
            p->vertices[i].tex_coord[1] = i >= 2 ? 1.0f : 0.0f;
        }
        mesh->m_usesVertexColors = true;
    }

    void tr_setupTexturedFace(loader::Mesh *tr_mesh, core::BaseMesh* mesh, const uint16_t *vertex_indices, core::Polygon *p)
    {
        for(size_t i = 0; i < p->vertices.size(); i++)
        {
            if(tr_mesh->lights.size() == tr_mesh->vertices.size())
            {
                p->vertices[i].color[0] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
                p->vertices[i].color[1] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
                p->vertices[i].color[2] = 1.0f - (tr_mesh->lights[vertex_indices[i]] / (8192.0f));
                p->vertices[i].color[3] = 1.0f;

                mesh->m_usesVertexColors = true;
            }
            else
            {
                p->vertices[i].color = {1,1,1,1};
            }
        }
    }

    void TR_GenMesh(World *world, size_t mesh_index, std::shared_ptr<core::BaseMesh> mesh, const std::unique_ptr<loader::Level>& tr)
    {
        const uint32_t tex_mask = (world->engineVersion == loader::Engine::TR4) ? (loader::TextureIndexMaskTr4) : (loader::TextureIndexMask);

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

        loader::Mesh* tr_mesh = &tr->m_meshes[mesh_index];
        mesh->m_id = static_cast<uint32_t>(mesh_index);
        mesh->m_center[0] = tr_mesh->centre.x;
        mesh->m_center[1] = -tr_mesh->centre.z;
        mesh->m_center[2] = tr_mesh->centre.y;
        mesh->m_radius = tr_mesh->collision_size;
        mesh->m_texturePageCount = static_cast<uint32_t>(world->tex_atlas->getNumAtlasPages()) + 1;

        mesh->m_vertices.resize(tr_mesh->vertices.size());
        auto vertex = mesh->m_vertices.data();
        for(size_t i = 0; i < mesh->m_vertices.size(); i++, vertex++)
        {
            vertex->position = util::convert(tr_mesh->vertices[i]);
            vertex->normal = {0,0,0};                                          // paranoid
        }

        mesh->updateBoundingBox();

        mesh->m_polygons.clear();

        /*
         * textured triangles
         */
        for(size_t i = 0; i < tr_mesh->textured_triangles.size(); ++i)
        {
            mesh->m_polygons.emplace_back();
            core::Polygon &p = mesh->m_polygons.back();

            auto face3 = &tr_mesh->textured_triangles[i];
            auto tex = &tr->m_objectTextures[face3->texture & tex_mask];

            p.double_side = (face3->texture & 0x8000) != 0;    // CORRECT, BUT WRONG IN TR3-5

            SetAnimTexture(&p, face3->texture & tex_mask, world);

            if(face3->lighting & 0x01)
            {
                p.blendMode = loader::BlendingMode::Multiply;
            }
            else
            {
                p.blendMode = tex->transparency_flags;
            }

            tr_accumulateNormals(tr_mesh, mesh.get(), 3, face3->vertices, &p);
            tr_setupTexturedFace(tr_mesh, mesh.get(), face3->vertices, &p);

            world->tex_atlas->getCoordinates(face3->texture & tex_mask, 0, &p);
        }

        /*
         * coloured triangles
         */
        for(size_t i = 0; i < tr_mesh->coloured_triangles.size(); ++i)
        {
            mesh->m_polygons.emplace_back();
            core::Polygon &p = mesh->m_polygons.back();

            auto face3 = &tr_mesh->coloured_triangles[i];
            auto col = face3->texture & 0xff;
            p.tex_index = static_cast<uint32_t>(world->tex_atlas->getNumAtlasPages());
            p.blendMode = loader::BlendingMode::Opaque;
            p.anim_id = 0;

            tr_accumulateNormals(tr_mesh, mesh.get(), 3, face3->vertices, &p);
            tr_setupColoredFace(tr_mesh, tr, mesh.get(), face3->vertices, col, &p);
        }

        /*
         * textured rectangles
         */
        for(size_t i = 0; i < tr_mesh->textured_rectangles.size(); ++i)
        {
            mesh->m_polygons.emplace_back();
            core::Polygon &p = mesh->m_polygons.back();

            auto face4 = &tr_mesh->textured_rectangles[i];
            auto tex = &tr->m_objectTextures[face4->texture & tex_mask];

            p.double_side = (face4->texture & 0x8000) != 0;    // CORRECT, BUT WRONG IN TR3-5

            SetAnimTexture(&p, face4->texture & tex_mask, world);

            if(face4->lighting & 0x01)
            {
                p.blendMode = loader::BlendingMode::Multiply;
            }
            else
            {
                p.blendMode = tex->transparency_flags;
            }

            tr_accumulateNormals(tr_mesh, mesh.get(), 4, face4->vertices, &p);
            tr_setupTexturedFace(tr_mesh, mesh.get(), face4->vertices, &p);

            world->tex_atlas->getCoordinates(face4->texture & tex_mask, 0, &p);
        }

        /*
         * coloured rectangles
         */
        for(size_t i = 0; i < tr_mesh->coloured_rectangles.size(); i++)
        {
            mesh->m_polygons.emplace_back();
            core::Polygon &p = mesh->m_polygons.back();

            auto face4 = &tr_mesh->coloured_rectangles[i];
            auto col = face4->texture & 0xff;
            p.vertices.resize(4);
            p.tex_index = static_cast<uint32_t>(world->tex_atlas->getNumAtlasPages());
            p.blendMode = loader::BlendingMode::Opaque;
            p.anim_id = 0;

            tr_accumulateNormals(tr_mesh, mesh.get(), 4, face4->vertices, &p);
            tr_setupColoredFace(tr_mesh, tr, mesh.get(), face4->vertices, col, &p);
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
        for(size_t i = 0; i < tr_mesh->textured_triangles.size(); i++, ++p)
        {
            tr_copyNormals(&*p, mesh, tr_mesh->textured_triangles[i].vertices);
        }

        for(size_t i = 0; i < tr_mesh->coloured_triangles.size(); i++, ++p)
        {
            tr_copyNormals(&*p, mesh, tr_mesh->coloured_triangles[i].vertices);
        }

        /*
         * rectangles
         */
        for(size_t i = 0; i < tr_mesh->textured_rectangles.size(); i++, ++p)
        {
            tr_copyNormals(&*p, mesh, tr_mesh->textured_rectangles[i].vertices);
        }

        for(size_t i = 0; i < tr_mesh->coloured_rectangles.size(); i++, ++p)
        {
            tr_copyNormals(&*p, mesh, tr_mesh->coloured_rectangles[i].vertices);
        }

        mesh->m_vertices.clear();
        mesh->genFaces();
        mesh->polySortInMesh();
    }

    void tr_setupRoomVertices(World *world, const std::unique_ptr<loader::Level>& tr, loader::Room *tr_room, const std::shared_ptr<core::BaseMesh>& mesh, int numCorners, const uint16_t *vertices, uint16_t masked_texture, core::Polygon *p)
    {
        p->vertices.resize(numCorners);

        for(int i = 0; i < numCorners; i++)
        {
            p->vertices[i].position = util::convert(tr_room->vertices[vertices[i]].vertex);
        }
        p->updateNormal();

        for(int i = 0; i < numCorners; i++)
        {
            mesh->m_vertices[vertices[i]].normal += p->plane.normal;
            p->vertices[i].normal = p->plane.normal;
            p->vertices[i].color = util::convert(tr_room->vertices[vertices[i]].colour);
        }

        loader::ObjectTexture *tex = &tr->m_objectTextures[masked_texture];
        SetAnimTexture(p, masked_texture, world);
        p->blendMode = tex->transparency_flags;

        world->tex_atlas->getCoordinates(masked_texture, 0, p);
    }

    void Res_GenRoomSpritesBuffer(std::shared_ptr<Room> room)
    {
        // Find the number of different texture pages used and the number of non-null sprites
        size_t highestTexturePageFound = 0;
        int actualSpritesFound = 0;
        for(RoomSprite& sp : room->sprites)
        {
            if(sp.sprite)
            {
                actualSpritesFound += 1;
                highestTexturePageFound = std::max(highestTexturePageFound, sp.sprite->texture);
            }
        }
        if(actualSpritesFound == 0)
        {
            room->sprite_buffer = nullptr;
            return;
        }

        room->sprite_buffer = new core::SpriteBuffer();
        room->sprite_buffer->num_texture_pages = highestTexturePageFound + 1;
        room->sprite_buffer->element_count_per_texture.resize(room->sprite_buffer->num_texture_pages, 0);

        // First collect indices on a per-texture basis
        std::vector<std::vector<uint16_t>> elements_for_texture(highestTexturePageFound + 1);

        std::vector<GLfloat> spriteData(actualSpritesFound * 4 * 7, 0);

        int writeIndex = 0;
        for(const RoomSprite& room_sprite : room->sprites)
        {
            if(room_sprite.sprite)
            {
                int vertexStart = writeIndex;
                // top right
                std::copy_n(glm::value_ptr(room_sprite.pos), 3, &spriteData[writeIndex * 7 + 0]);
                std::copy_n(&room_sprite.sprite->tex_coord[0], 2, &spriteData[writeIndex * 7 + 3]);
                spriteData[writeIndex * 7 + 5] = room_sprite.sprite->right;
                spriteData[writeIndex * 7 + 6] = room_sprite.sprite->top;

                writeIndex += 1;

                // top left
                std::copy_n(glm::value_ptr(room_sprite.pos), 3, &spriteData[writeIndex * 7 + 0]);
                std::copy_n(&room_sprite.sprite->tex_coord[2], 2, &spriteData[writeIndex * 7 + 3]);
                spriteData[writeIndex * 7 + 5] = room_sprite.sprite->left;
                spriteData[writeIndex * 7 + 6] = room_sprite.sprite->top;

                writeIndex += 1;

                // bottom left
                std::copy_n(glm::value_ptr(room_sprite.pos), 3, &spriteData[writeIndex * 7 + 0]);
                std::copy_n(&room_sprite.sprite->tex_coord[4], 2, &spriteData[writeIndex * 7 + 3]);
                spriteData[writeIndex * 7 + 5] = room_sprite.sprite->left;
                spriteData[writeIndex * 7 + 6] = room_sprite.sprite->bottom;

                writeIndex += 1;

                // bottom right
                std::copy_n(glm::value_ptr(room_sprite.pos), 3, &spriteData[writeIndex * 7 + 0]);
                std::copy_n(&room_sprite.sprite->tex_coord[6], 2, &spriteData[writeIndex * 7 + 3]);
                spriteData[writeIndex * 7 + 5] = room_sprite.sprite->right;
                spriteData[writeIndex * 7 + 6] = room_sprite.sprite->bottom;

                writeIndex += 1;

                // Assign indices
                size_t texture = room_sprite.sprite->texture;
                size_t start = room->sprite_buffer->element_count_per_texture[texture];
                size_t newElementCount = start + 6;
                room->sprite_buffer->element_count_per_texture[texture] = newElementCount;
                elements_for_texture[texture].resize(newElementCount);

                elements_for_texture[texture][start + 0] = vertexStart + 0;
                elements_for_texture[texture][start + 1] = vertexStart + 1;
                elements_for_texture[texture][start + 2] = vertexStart + 2;
                elements_for_texture[texture][start + 3] = vertexStart + 2;
                elements_for_texture[texture][start + 4] = vertexStart + 3;
                elements_for_texture[texture][start + 5] = vertexStart + 0;
            }
        }

        // Now flatten all these indices to a single array
        std::vector<uint16_t> elements;
        for(uint32_t i = 0; i <= highestTexturePageFound; i++)
        {
            if(elements_for_texture[i].empty())
            {
                continue;
            }
            BOOST_ASSERT(elements_for_texture[i].size() >= room->sprite_buffer->element_count_per_texture[i]);
            std::copy_n(elements_for_texture[i].begin(), room->sprite_buffer->element_count_per_texture[i], std::back_inserter(elements));
            elements_for_texture[i].clear();
        }
        elements_for_texture.clear();

        // Now load into OpenGL
        GLuint arrayBuffer, elementBuffer;
        glGenBuffers(1, &arrayBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat[7]) * 4 * actualSpritesFound, spriteData.data(), GL_STATIC_DRAW);
        spriteData.clear();

        glGenBuffers(1, &elementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements[0]) * elements.size(), elements.data(), GL_STATIC_DRAW);
        elements.clear();

        render::VertexArrayAttribute attribs[3] = {
            render::VertexArrayAttribute(render::SpriteShaderDescription::vertex_attribs::position,      3, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat[7]), 0),
            render::VertexArrayAttribute(render::SpriteShaderDescription::vertex_attribs::tex_coord,     2, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat[7]), sizeof(GLfloat[3])),
            render::VertexArrayAttribute(render::SpriteShaderDescription::vertex_attribs::corner_offset, 2, GL_FLOAT, false, arrayBuffer, sizeof(GLfloat[7]), sizeof(GLfloat[5]))
        };

        room->sprite_buffer->data.reset(new render::VertexArray(elementBuffer, 3, attribs));
    }

    void Res_GenVBOs(World *world)
    {
        for(uint32_t i = 0; i < world->meshes.size(); i++)
        {
            if(!world->meshes[i]->m_vertices.empty() || !world->meshes[i]->m_animatedVertices.empty())
            {
                world->meshes[i]->genVBO(&render::renderer);
            }
        }

        for(uint32_t i = 0; i < world->rooms.size(); i++)
        {
            if(world->rooms[i]->mesh && (!world->rooms[i]->mesh->m_vertices.empty() || !world->rooms[i]->mesh->m_animatedVertices.empty()))
            {
                world->rooms[i]->mesh->genVBO(&render::renderer);
            }
        }
    }

    void Res_GenBaseItems(World* world)
    {
        engine_lua["genBaseItems"]();

        if(!world->items_tree.empty())
        {
            Res_EntityToItem(world->items_tree);
        }
    }

    void Res_FixRooms(World *world)
    {
        for(uint32_t i = 0; i < world->rooms.size(); i++)
        {
            auto r = world->rooms[i];
            if(r->base_room != nullptr)
            {
                r->disable();    // Disable current room
            }

            // Isolated rooms may be used for rolling ball trick (for ex., LEVEL4.PHD).
            // Hence, this part is commented.

            /*
            if((r->portal_count == 0) && (world->room_count > 1))
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
            if((offset < tr_animation->frame_offset) && (offset >= (tr_animation + 1)->frame_offset))
            {
                return -2;
            }
        }

        return tr_animation->frame_offset;
    }

    SkeletalModel* Res_GetSkybox(World *world, loader::Engine engine_version)
    {
        switch(engine_version)
        {
            case loader::Engine::TR2:
                return world->getModelByID(TR_ITEM_SKYBOX_TR2);

            case loader::Engine::TR3:
                return world->getModelByID(TR_ITEM_SKYBOX_TR3);

            case loader::Engine::TR4:
                return world->getModelByID(TR_ITEM_SKYBOX_TR4);

            case loader::Engine::TR5:
                return world->getModelByID(TR_ITEM_SKYBOX_TR5);

            default:
                return nullptr;
        }
    }

    void TR_GenAnimCommands(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        world->anim_commands = std::move(tr->m_animCommands);
    }

    void TR_GenSkeletalModel(World *world, size_t model_num, SkeletalModel *model, const std::unique_ptr<loader::Level>& tr, size_t meshCount)
    {
        loader::Moveable *tr_moveable = &tr->m_moveables[model_num];  // original tr structure

        model->collision_map.resize(model->meshes.size());
        std::iota(model->collision_map.begin(), model->collision_map.end(), 0);

        model->meshes.resize(meshCount);

        const uint32_t *mesh_index = &tr->m_meshIndices[tr_moveable->starting_mesh];

        for(size_t k = 0; k < model->meshes.size(); k++)
        {
            SkeletalModel::MeshReference* tree_tag = &model->meshes[k];
            tree_tag->mesh_base = world->meshes[mesh_index[k]];
            if(k == 0)
            {
                tree_tag->flag = 0x02;
            }
            else
            {
                uint32_t *tr_mesh_tree = tr->m_meshTreeData.data() + tr_moveable->mesh_tree_index + (k - 1) * 4;
                tree_tag->flag = (tr_mesh_tree[0] & 0xFF);
                tree_tag->offset[0] = static_cast<float>(static_cast<int32_t>(tr_mesh_tree[1]));
                tree_tag->offset[1] = static_cast<float>(static_cast<int32_t>(tr_mesh_tree[3]));
                tree_tag->offset[2] = -static_cast<float>(static_cast<int32_t>(tr_mesh_tree[2]));
            }
        }

        /*
         * =================    now, animation loading    ========================
         */

        if(tr_moveable->animation_index >= tr->m_animations.size())
        {
            /*
             * model has no start offset and any animation
             */
            model->animations.resize(1);
            model->animations.front().keyFrames.resize(1);
            animation::SkeletonKeyFrame* bone_frame = model->animations.front().keyFrames.data();

            model->animations.front().id = 0;
            model->animations.front().next_anim = nullptr;
            model->animations.front().next_frame = 0;
            model->animations.front().stateChanges.clear();
            model->animations.front().original_frame_rate = 1;

            bone_frame->boneKeyFrames.resize(model->meshes.size());

            bone_frame->position = {0,0,0};
            for(size_t k = 0; k < bone_frame->boneKeyFrames.size(); k++)
            {
                SkeletalModel::MeshReference* tree_tag = &model->meshes[k];
                animation::BoneKeyFrame* bone_tag = &bone_frame->boneKeyFrames[k];

                bone_tag->qrotate = util::vec4_SetTRRotations({ 0,0,0 });
                bone_tag->offset = tree_tag->offset;
            }
            return;
        }
        //Sys_DebugLog(LOG_FILENAME, "model = %d, anims = %d", tr_moveable->object_id, GetNumAnimationsForMoveable(tr, model_num));
        model->animations.resize(TR_GetNumAnimationsForMoveable(tr, model_num));
        if(model->animations.empty())
        {
            /*
             * the animation count must be >= 1
             */
            model->animations.resize(1);
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
        for(size_t i = 0; i < model->animations.size(); i++)
        {
            animation::AnimationFrame* anim = &model->animations[i];
            loader::Animation *tr_animation = &tr->m_animations[tr_moveable->animation_index + i];

            uint32_t frame_offset = tr_animation->frame_offset / 2;
            uint16_t l_start = 0x09;

            if(tr->m_gameVersion == loader::Game::TR1 || tr->m_gameVersion == loader::Game::TR1Demo || tr->m_gameVersion == loader::Game::TR1UnfinishedBusiness)
            {
                l_start = 0x0A;
            }

            uint32_t frame_step = tr_animation->frame_size;

            anim->id = i;
            anim->original_frame_rate = tr_animation->frame_rate;

            anim->speed_x = tr_animation->speed;
            anim->accel_x = tr_animation->accel;
            anim->speed_y = tr_animation->accel_lateral;
            anim->accel_y = tr_animation->speed_lateral;

            anim->anim_command = tr_animation->anim_command;
            anim->num_anim_commands = tr_animation->num_anim_commands;
            anim->state_id = static_cast<LaraState>(tr_animation->state_id);

            //        anim->frames.resize(TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index + i));
                    // FIXME: number of frames is always (frame_end - frame_start + 1)
                    // this matters for transitional anims, which run through their frame length with the same anim frame.
                    // This should ideally be solved without filling identical frames,
                    // but due to the amount of currFrame-indexing, waste dummy frames for now:
                    // (I haven't seen this for framerate==1 animation, but it would be possible,
                    //  also, resizing now saves re-allocations in interpolateFrames later)
            anim->keyFrames.resize(tr_animation->frame_end - tr_animation->frame_start + 1);

            size_t numFrameData = TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index + i);
            if(numFrameData > anim->keyFrames.size())
            {
                numFrameData = anim->keyFrames.size();
            }

            //Sys_DebugLog(LOG_FILENAME, "Anim[%d], %d", tr_moveable->animation_index, TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index));

            // Parse AnimCommands
            // Max. amount of AnimCommands is 255, larger numbers are considered as 0.
            // See http://evpopov.com/dl/TR4format.html#Animations for details.
            // FIXME: This is done here only to adjust relative frame indices
            //        see GenerateAnimCommands()
            if((anim->num_anim_commands > 0) && (anim->num_anim_commands <= 255))
            {
                // Calculate current animation anim command block offset.
                BOOST_ASSERT(anim->anim_command < world->anim_commands.size());
                int16_t *pointer = &world->anim_commands[anim->anim_command];

                for(uint32_t count = 0; count < anim->num_anim_commands; count++)
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

            if(anim->keyFrames.empty())
            {
                /*
                 * number of animations must be >= 1, because frame contains base model offset
                 */
                anim->keyFrames.resize(1);
            }

            /*
             * let us begin to load animations
             */
            for(size_t j = 0; j < anim->keyFrames.size(); j++, frame_offset += frame_step)
            {
                animation::SkeletonKeyFrame* bone_frame = &anim->keyFrames[j];
                // !Need bonetags in empty frames:
                bone_frame->boneKeyFrames.resize(model->meshes.size());

                if(j >= numFrameData)
                    continue; // only create bone_tags for rate>1 fill-frames

                bone_frame->position = {0,0,0};
                TR_GetBFrameBB_Pos(tr, frame_offset, bone_frame);

                if(frame_offset >= tr->m_frameData.size())
                {
                    for(size_t k = 0; k < bone_frame->boneKeyFrames.size(); k++)
                    {
                        SkeletalModel::MeshReference* tree_tag = &model->meshes[k];
                        animation::BoneKeyFrame* bone_tag = &bone_frame->boneKeyFrames[k];
                        bone_tag->qrotate = util::vec4_SetTRRotations({ 0,0,0 });
                        bone_tag->offset = tree_tag->offset;
                    }
                }
                else
                {
                    uint16_t l = l_start;
                    uint16_t temp1, temp2;
                    float ang;

                    for(size_t k = 0; k < bone_frame->boneKeyFrames.size(); k++)
                    {
                        SkeletalModel::MeshReference* tree_tag = &model->meshes[k];
                        animation::BoneKeyFrame* bone_tag = &bone_frame->boneKeyFrames[k];
                        bone_tag->qrotate = util::vec4_SetTRRotations({ 0,0,0 });
                        bone_tag->offset = tree_tag->offset;

                        switch(tr->m_gameVersion)
                        {
                            case loader::Game::TR1:                                              /* TR1 */
                            case loader::Game::TR1UnfinishedBusiness:
                            case loader::Game::TR1Demo:
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
                                break;
                            }

                            default:                                                /* TR2 + */
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
                                break;
                        };
                    }
                }
            }
        }

        /*
         * Animations interpolation to 1/30 sec like in original. Needed for correct state change works.
         */
        model->interpolateFrames();
        /*
         * state change's loading
         */
#if LOG_ANIM_DISPATCHES
        if(model->animations.size() > 1)
        {
            Sys_DebugLog(LOG_FILENAME, "MODEL[%d], anims = %d", model_num, model->animations.size());
        }
#endif
        for(size_t i = 0; i < model->animations.size(); i++)
        {
            animation::AnimationFrame* anim = &model->animations[i];
            anim->stateChanges.clear();

            loader::Animation *tr_animation = &tr->m_animations[tr_moveable->animation_index + i];
            int16_t animId = tr_animation->next_animation - tr_moveable->animation_index;
            animId &= 0x7fff; // this masks out the sign bit
            BOOST_ASSERT(animId >= 0);
            if(static_cast<size_t>(animId) < model->animations.size())
            {
                anim->next_anim = &model->animations[animId];
                anim->next_frame = tr_animation->next_frame - tr->m_animations[tr_animation->next_animation].frame_start;
                anim->next_frame %= anim->next_anim->keyFrames.size();
                if(anim->next_frame < 0)
                {
                    anim->next_frame = 0;
                }
#if LOG_ANIM_DISPATCHES
                Sys_DebugLog(LOG_FILENAME, "ANIM[%d], next_anim = %d, next_frame = %d", i, anim->next_anim->id, anim->next_frame);
#endif
            }
            else
            {
                anim->next_anim = nullptr;
                anim->next_frame = 0;
            }

            anim->stateChanges.clear();

            if(tr_animation->num_state_changes > 0 && model->animations.size() > 1)
            {
#if LOG_ANIM_DISPATCHES
                Sys_DebugLog(LOG_FILENAME, "ANIM[%d], next_anim = %d, next_frame = %d", i, (anim->next_anim) ? (anim->next_anim->id) : (-1), anim->next_frame);
#endif
                anim->stateChanges.resize(tr_animation->num_state_changes);

                for(uint16_t j = 0; j < tr_animation->num_state_changes; j++)
                {
                    animation::StateChange* sch_p = &anim->stateChanges[j];
                    loader::StateChange *tr_sch = &tr->m_stateChanges[j + tr_animation->state_change_offset];
                    sch_p->id = static_cast<LaraState>(tr_sch->state_id);
                    sch_p->anim_dispatch.clear();
                    for(uint16_t l = 0; l < tr_sch->num_anim_dispatches; l++)
                    {
                        loader::AnimDispatch *tr_adisp = &tr->m_animDispatches[tr_sch->anim_dispatch + l];
                        uint16_t next_anim = tr_adisp->next_animation & 0x7fff;
                        uint16_t next_anim_ind = next_anim - (tr_moveable->animation_index & 0x7fff);
                        if(next_anim_ind < model->animations.size())
                        {
                            sch_p->anim_dispatch.emplace_back();

                            animation::AnimDispatch* adsp = &sch_p->anim_dispatch.back();
                            size_t next_frames_count = model->animations[next_anim - tr_moveable->animation_index].keyFrames.size();
                            uint16_t next_frame = tr_adisp->next_frame - tr->m_animations[next_anim].frame_start;

                            uint16_t low = tr_adisp->low - tr_animation->frame_start;
                            uint16_t high = tr_adisp->high - tr_animation->frame_start;

                            // this is not good: frame_high can be frame_end+1 (for last-frame-loop statechanges,
                            // secifically fall anims (75,77 etc), which may fail to change state),
                            // And: if theses values are > framesize, then they're probably faulty and won't be fixed by modulo anyway:
    //                        adsp->frame_low = low  % anim->frames.size();
    //                        adsp->frame_high = (high - 1) % anim->frames.size();
                            if(low > anim->keyFrames.size() || high > anim->keyFrames.size())
                            {
                                //Sys_Warn("State range out of bounds: anim: %d, stid: %d, low: %d, high: %d", anim->id, sch_p->id, low, high);
                                Console::instance().printf("State range out of bounds: anim: %d, stid: %d, low: %d, high: %d", anim->id, sch_p->id, low, high);
                            }
                            adsp->frame_low = low;
                            adsp->frame_high = high;
                            adsp->next_anim = next_anim - tr_moveable->animation_index;
                            adsp->next_frame = next_frame % next_frames_count;

#if LOG_ANIM_DISPATCHES
                            Sys_DebugLog(LOG_FILENAME, "anim_disp[%d], frames.size() = %d: interval[%d.. %d], next_anim = %d, next_frame = %d", l,
                                         anim->frames.size(), adsp->frame_low, adsp->frame_high,
                                         adsp->next_anim, adsp->next_frame);
#endif
                        }
                    }
                }
            }
        }
        GenerateAnimCommands(model);
    }

    size_t TR_GetNumAnimationsForMoveable(const std::unique_ptr<loader::Level>& tr, size_t moveable_ind)
    {
        loader::Moveable* curr_moveable = &tr->m_moveables[moveable_ind];

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

        loader::Moveable* next_moveable = &tr->m_moveables[moveable_ind + 1];
        if(next_moveable->animation_index == 0xFFFF)
        {
            if(moveable_ind + 2 < tr->m_moveables.size())                              // I hope there is no two neighboard movables with animation_index'es == 0xFFFF
            {
                next_moveable = &tr->m_moveables[moveable_ind + 2];
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

    void TR_GetBFrameBB_Pos(const std::unique_ptr<loader::Level>& tr, size_t frame_offset, animation::SkeletonKeyFrame *keyFrame)
    {
        if(frame_offset < tr->m_frameData.size())
        {
            unsigned short int *frame = &tr->m_frameData[frame_offset];

            keyFrame->boundingBox.min[0] = (short int)frame[0];   // x_min
            keyFrame->boundingBox.min[1] = (short int)frame[4];   // y_min
            keyFrame->boundingBox.min[2] = -(short int)frame[3];  // z_min

            keyFrame->boundingBox.max[0] = (short int)frame[1];   // x_max
            keyFrame->boundingBox.max[1] = (short int)frame[5];   // y_max
            keyFrame->boundingBox.max[2] = -(short int)frame[2];  // z_max

            keyFrame->position[0] = (short int)frame[6];
            keyFrame->position[1] = (short int)frame[8];
            keyFrame->position[2] = -(short int)frame[7];
        }
        else
        {
            keyFrame->boundingBox.min = {0,0,0};
            keyFrame->boundingBox.max = {0,0,0};
            keyFrame->position = {0,0,0};
        }
    }

    void TR_GenSkeletalModels(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        world->skeletal_models.resize(tr->m_moveables.size());

        for(size_t i = 0; i < tr->m_moveables.size(); i++)
        {
            const loader::Moveable& tr_moveable = tr->m_moveables[i];
            SkeletalModel& smodel = world->skeletal_models[i];
            smodel.id = tr_moveable.object_id;
            TR_GenSkeletalModel(world, i, &smodel, tr, tr_moveable.num_meshes);
            smodel.updateTransparencyFlag();
        }
    }

    void TR_GenEntities(World *world, const std::unique_ptr<loader::Level>& tr)
    {
        for(uint32_t i = 0; i < tr->m_items.size(); i++)
        {
            loader::Item *tr_item = &tr->m_items[i];
            std::shared_ptr<Entity> entity = (tr_item->object_id == 0) ? std::make_shared<Character>(i) : std::make_shared<Entity>(i);
            entity->m_transform[3][0] = tr_item->position.x;
            entity->m_transform[3][1] = -tr_item->position.z;
            entity->m_transform[3][2] = tr_item->position.y;
            entity->m_angles[0] = tr_item->rotation;
            entity->m_angles[1] = 0;
            entity->m_angles[2] = 0;
            entity->updateTransform();
            if((tr_item->room >= 0) && (static_cast<uint32_t>(tr_item->room) < world->rooms.size()))
            {
                entity->setRoom(world->rooms[tr_item->room].get());
            }
            else
            {
                entity->setRoom(nullptr);
            }

            entity->m_triggerLayout = tr_item->getActivationMask();   ///@FIXME: Ignore INVISIBLE and CLEAR BODY flags for a moment.
            entity->m_OCB = tr_item->ocb;
            entity->m_timer = 0.0;

            entity->setCollisionType(world::CollisionType::Kinematic);
            entity->setCollisionShape(world::CollisionShape::TriMeshConvex);
            entity->m_moveType = MoveType::StaticPos;
            entity->m_inertiaLinear = 0.0;
            entity->m_inertiaAngular[0] = 0.0;
            entity->m_inertiaAngular[1] = 0.0;

            entity->m_skeleton.setModel( world->getModelByID(tr_item->object_id) );

            if(entity->m_skeleton.getModel() == nullptr)
            {
                int id = engine_lua.call("getOverridedID", static_cast<int>(loader::gameToEngine(tr->m_gameVersion)), tr_item->object_id);
                entity->m_skeleton.setModel( world->getModelByID(id) );
            }

            int replace_anim_id = engine_lua.call("getOverridedAnim", static_cast<int>(loader::gameToEngine(tr->m_gameVersion)), tr_item->object_id);
            if(replace_anim_id > 0)
            {
                SkeletalModel* replace_anim_model = world->getModelByID(replace_anim_id);
                std::swap(entity->m_skeleton.model()->animations, replace_anim_model->animations);
            }

            if(entity->m_skeleton.getModel() == nullptr)
            {
                // SPRITE LOADING
                core::Sprite* sp = world->getSpriteByID(tr_item->object_id);
                if(sp && entity->getRoom())
                {
                    entity->getRoom()->sprites.emplace_back();
                    RoomSprite& rsp = entity->getRoom()->sprites.back();
                    rsp.sprite = sp;
                    rsp.pos = glm::vec3(entity->m_transform[3]);
                    rsp.was_rendered = false;
                }

                continue;                                                           // that entity has no model. may be it is a some trigger or look at object
            }

            if(tr->m_gameVersion < loader::Game::TR2 && tr_item->object_id == 83)                ///@FIXME: brutal magick hardcode! ;-)
            {
                // skip PSX save model
                continue;
            }

            entity->m_skeleton.fromModel(entity->m_skeleton.model());

            if(0 == tr_item->object_id)                                             // Lara is unical model
            {
                std::shared_ptr<Character> lara = std::dynamic_pointer_cast<Character>(entity);
                BOOST_ASSERT(lara != nullptr);

                lara->m_moveType = MoveType::OnFloor;
                world->character = lara;
                lara->setCollisionType(world::CollisionType::Actor);
                lara->setCollisionShape(world::CollisionShape::TriMeshConvex);
                lara->m_typeFlags |= ENTITY_TYPE_TRIGGER_ACTIVATOR;

                engine_lua.set("player", lara->getId());

                switch(loader::gameToEngine(tr->m_gameVersion))
                {
                    case loader::Engine::TR1:
                        if(engine::Gameflow_Manager.getLevelID() == 0)
                        {
                            if(SkeletalModel* LM = world->getModelByID(TR_ITEM_LARA_SKIN_ALTERNATE_TR1))
                            {
                                // In TR1, Lara has unified head mesh for all her alternate skins.
                                // Hence, we copy all meshes except head, to prevent Potato Raider bug.
                                world->skeletal_models[0].setMeshes(LM->meshes, world->skeletal_models[0].meshes.size() - 1);
                            }
                        }
                        break;

                    case loader::Engine::TR2:
                        break;

                    case loader::Engine::TR3:
                        if(SkeletalModel* LM = world->getModelByID(TR_ITEM_LARA_SKIN_TR3))
                        {
                            world->skeletal_models[0].setMeshes(LM->meshes, world->skeletal_models[0].meshes.size());
                            auto tmp = world->getModelByID(11);                   // moto / quadro cycle animations
                            if(tmp)
                            {
                                tmp->setMeshes(LM->meshes, world->skeletal_models[0].meshes.size());
                            }
                        }
                        break;

                    case loader::Engine::TR4:
                    case loader::Engine::TR5:
                        // base skeleton meshes
                        if(SkeletalModel* LM = world->getModelByID(TR_ITEM_LARA_SKIN_TR45))
                        {
                            world->skeletal_models[0].setMeshes(LM->meshes, world->skeletal_models[0].meshes.size());
                        }
                        // skin skeleton meshes
                        if(SkeletalModel* LM = world->getModelByID(TR_ITEM_LARA_SKIN_JOINTS_TR45))
                        {
                            world->skeletal_models[0].setSkinnedMeshes(LM->meshes, world->skeletal_models[0].meshes.size());
                        }
                        world->skeletal_models[0].fillSkinnedMeshMap();
                        break;

                    case loader::Engine::Unknown:
                        break;
                };

                lara->m_skeleton.copyMeshBinding(lara->m_skeleton.getModel(), true);

                world->character->setAnimation(TR_ANIMATION_LARA_STAY_IDLE, 0);
                lara->m_skeleton.genRigidBody(lara.get(), lara->getCollisionShape(), lara->getCollisionType(), lara->m_transform);
                lara->m_skeleton.createGhosts(lara.get(), lara->m_transform);
                lara->m_height = 768.0;

                continue;
            }

            entity->setAnimation(0, 0);                                      // Set zero animation and zero frame

            Res_SetEntityProperties(entity);
            entity->rebuildBV();
            entity->m_skeleton.genRigidBody(entity.get(), entity->getCollisionShape(), entity->getCollisionType(), entity->m_transform);

            entity->getRoom()->addEntity(entity.get());
            world->addEntity(entity);
        }
    }

    void Res_EntityToItem(std::map<uint32_t, std::shared_ptr<BaseItem> >& map)
    {
        for(auto it : map)
        {
            std::shared_ptr<BaseItem> item = it.second;

            for(const std::shared_ptr<Room>& room : engine::engine_world.rooms)
            {
                for(world::Object* cont : room->containers)
                {
                    Entity* ent = dynamic_cast<Entity*>(cont);
                    if(!ent)
                        continue;

                    if(ent->m_skeleton.getModel()->id != item->world_model_id)
                        continue;

                    if(engine_lua["entity_funcs"][static_cast<lua::Integer>(ent->getId())].is<lua::Nil>())
                        engine_lua["entity_funcs"].set(static_cast<lua::Integer>(ent->getId()), lua::Table());

                    engine_lua["pickup_init"](ent->getId(), item->id);

                    ent->m_skeleton.disableCollision();
                }
            }
        }
    }
} // namespace world
