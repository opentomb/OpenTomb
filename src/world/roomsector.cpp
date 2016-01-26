#include "roomsector.h"

#include "engine/engine.h"
#include "resource.h"
#include "room.h"

namespace world
{

const RoomSector* RoomSector::checkPortalPointerRaw()
{
    if(portal_to_room)
    {
        std::shared_ptr<Room> r = engine::Engine::instance.m_world.m_rooms[*portal_to_room];
        int ind_x = static_cast<int>((position[0] - r->getModelMatrix()[3][0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->getModelMatrix()[3][1]) / MeteringSectorSize);
        if(ind_x >= 0 && static_cast<size_t>(ind_x) < r->getSectors().shape()[0] && ind_y >= 0 && static_cast<size_t>(ind_y) < r->getSectors().shape()[1])
        {
            return &r->getSectors()[ind_x][ind_y];
        }
    }

    return this;
}

const RoomSector* RoomSector::checkPortalPointer() const
{
    if(portal_to_room)
    {
        const Room* r = engine::Engine::instance.m_world.m_rooms[*portal_to_room].get();
        if(owner_room->getBaseRoom() != nullptr && r->getAlternateRoom() != nullptr)
        {
            r = r->getAlternateRoom();
        }
        else if(owner_room->getAlternateRoom() != nullptr && r->getBaseRoom() != nullptr)
        {
            r = r->getBaseRoom();
        }
        int ind_x = static_cast<int>((position[0] - r->getModelMatrix()[3][0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->getModelMatrix()[3][1]) / MeteringSectorSize);
        if(ind_x >= 0 && static_cast<size_t>(ind_x) < r->getSectors().shape()[0] && ind_y >= 0 && static_cast<size_t>(ind_y) < r->getSectors().shape()[1])
        {
            return &r->getSectors()[ind_x][ind_y];
        }
    }

    return this;
}

const RoomSector* RoomSector::checkBaseRoom() const
{
    if(owner_room->getBaseRoom() != nullptr)
    {
        const Room* r = owner_room->getBaseRoom();
        int ind_x = static_cast<int>((position[0] - r->getModelMatrix()[3][0]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->getModelMatrix()[3][1]) / MeteringSectorSize);
        if(ind_x >= 0 && static_cast<size_t>(ind_x) < r->getSectors().shape()[0] && ind_y >= 0 && static_cast<size_t>(ind_y) < r->getSectors().shape()[1])
        {
            return &r->getSectors()[ind_x][ind_y];
        }
    }

    return this;
}

const RoomSector* RoomSector::checkAlternateRoom() const
{
    if(owner_room->getAlternateRoom() != nullptr)
    {
        const Room* r = owner_room->getAlternateRoom();
        int ind_x = static_cast<int>((position[0] - r->getModelMatrix()[3][1]) / MeteringSectorSize);
        int ind_y = static_cast<int>((position[1] - r->getModelMatrix()[3][1]) / MeteringSectorSize);
        if(ind_x >= 0 && static_cast<size_t>(ind_x) < r->getSectors().shape()[0] && ind_y >= 0 && static_cast<size_t>(ind_y) < r->getSectors().shape()[1])
        {
            return &r->getSectors()[ind_x][ind_y];
        }
    }

    return this;
}

bool RoomSector::is2SidePortals(const RoomSector* s2) const
{
    const RoomSector* s1 = checkPortalPointer();
    s2 = s2->checkPortalPointer();

    if(owner_room == s2->owner_room)
    {
        return false;
    }

    const RoomSector* s1p = s2->owner_room->getSectorRaw(position);
    const RoomSector* s2p = s1->owner_room->getSectorRaw(s2->position);

    // 2 next conditions are the stick for TR5 door-roll-wall
    if(!s1p->portal_to_room)
    {
        s1p = s1p->checkAlternateRoom();
        if(!s1p->portal_to_room)
        {
            return false;
        }
    }
    if(!s2p->portal_to_room)
    {
        s2p = s2p->checkAlternateRoom();
        if(!s2p->portal_to_room)
        {
            return false;
        }
    }

    if((s1p->checkPortalPointer() == s1->checkBaseRoom() && s2p->checkPortalPointer() == s2->checkBaseRoom()) ||
       (s1p->checkPortalPointer() == s1->checkAlternateRoom() && s2p->checkPortalPointer() == s2->checkAlternateRoom()))
    {
        return true;
    }

    return false;
}

bool RoomSector::similarCeiling(const RoomSector* s2, bool ignore_doors) const
{
    if(!s2)
        return false;
    if(this == s2)
        return true;

    if(ceiling != s2->ceiling ||
       ceiling_penetration_config == PenetrationConfig::Wall ||
       s2->ceiling_penetration_config == PenetrationConfig::Wall ||
       (!ignore_doors && (sector_above || s2->sector_above)))
        return false;

    for(int i = 0; i < 4; i++)
    {
        if(ceiling_corners[i].z != s2->ceiling_corners[i].z)
            return false;
    }

    return true;
}

bool RoomSector::similarFloor(const RoomSector* s2, bool ignore_doors) const
{
    if(!s2) return false;
    if(this == s2) return true;

    if(floor != s2->floor ||
       floor_penetration_config == PenetrationConfig::Wall ||
       s2->floor_penetration_config == PenetrationConfig::Wall ||
       (!ignore_doors && (sector_below || s2->sector_below)))
        return false;

    for(int i = 0; i < 4; i++)
    {
        if(floor_corners[i].z != s2->floor_corners[i].z)
            return false;
    }

    return true;
}

glm::vec3 RoomSector::getFloorPoint() const
{
    return getLowestSector()->getHighestFloorCorner();
}

glm::vec3 RoomSector::getCeilingPoint() const
{
    return getHighestSector()->getLowestCeilingCorner();
}

const RoomSector* RoomSector::checkFlip() const
{
    if(owner_room->isActive())
        return this;

    if(owner_room->getBaseRoom() && owner_room->getBaseRoom()->isActive())
    {
        return &owner_room->getBaseRoom()->getSectors()[index_x][index_y];
    }
    else if(owner_room->getAlternateRoom() && owner_room->getAlternateRoom()->isActive())
    {
        return &owner_room->getAlternateRoom()->getSectors()[index_x][index_y];
    }
    else
    {
        return this;
    }
}

const RoomSector* RoomSector::getLowestSector() const
{
    const RoomSector* lowest_sector = this;

    while(const RoomSector* below = lowest_sector->sector_below)
    {
        const RoomSector* flipped = below->checkFlip();
        if(!flipped)
            break;
        lowest_sector = flipped;
    }

    return lowest_sector->checkFlip();
}

const RoomSector* RoomSector::getHighestSector() const
{
    const RoomSector* highest_sector = this;

    while(const RoomSector* above = highest_sector->sector_above)
    {
        const RoomSector* flipped = above->checkFlip();
        if(!flipped)
            break;
        highest_sector = flipped;
    }

    return highest_sector;
}

}
