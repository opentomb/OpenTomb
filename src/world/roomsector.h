#pragma once

#include "object.h"

#include <boost/optional.hpp>
#include <glm/glm.hpp>

#include <cstdint>

namespace world
{

class Room;

enum class PenetrationConfig;
enum class DiagonalType;

struct RoomSector
{
    uint32_t                    trig_index; // Trigger function index.
    int32_t                     box_index;

    uint32_t                    flags;      // Climbability, death etc.
    uint32_t                    material;   // Footstep sound and footsteps.

    int32_t                     floor;
    int32_t                     ceiling;

    const RoomSector* sector_below;
    const RoomSector* sector_above;
    Room* owner_room;    // Room that contain this sector

    size_t index_x;
    size_t index_y;
    glm::vec3 position;

    glm::vec3                   ceiling_corners[4];
    DiagonalType                ceiling_diagonal_type;
    PenetrationConfig           ceiling_penetration_config;

    glm::vec3                   floor_corners[4];
    DiagonalType                floor_diagonal_type;
    PenetrationConfig           floor_penetration_config;

    boost::optional<ObjectId> portal_to_room;

    const RoomSector* getLowestSector() const;
    const RoomSector* getHighestSector() const;

    const RoomSector* checkFlip() const;
    const RoomSector* checkBaseRoom() const;
    const RoomSector* checkAlternateRoom() const;
    const RoomSector* checkPortalPointerRaw();
    const RoomSector* checkPortalPointer() const;
    bool is2SidePortals(const RoomSector* s2) const;
    bool similarCeiling(const RoomSector* s2, bool ignore_doors) const;
    bool similarFloor(const RoomSector* s2, bool ignore_doors) const;
    glm::vec3 getFloorPoint() const;
    glm::vec3 getCeilingPoint() const;

    glm::vec3 getHighestFloorCorner() const
    {
        glm::vec3 r1 = floor_corners[0][2] > floor_corners[1][2] ? floor_corners[0] : floor_corners[1];
        glm::vec3 r2 = floor_corners[2][2] > floor_corners[3][2] ? floor_corners[2] : floor_corners[3];

        return r1[2] > r2[2] ? r1 : r2;
    }

    glm::vec3 getLowestCeilingCorner() const
    {
        glm::vec3 r1 = ceiling_corners[0][2] > ceiling_corners[1][2] ? ceiling_corners[1] : ceiling_corners[0];
        glm::vec3 r2 = ceiling_corners[2][2] > ceiling_corners[3][2] ? ceiling_corners[3] : ceiling_corners[2];

        return r1[2] > r2[2] ? r2 : r1;
    }
};

}
