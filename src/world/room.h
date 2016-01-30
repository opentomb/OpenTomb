#pragma once

#include "core/boundingbox.h"
#include "world/core/light.h"
#include "object.h"
#include "portal.h"
#include "roomsector.h"

#include <cstdint>
#include <memory>
#include <vector>

#include <boost/multi_array.hpp>

class btRigidBody;

namespace loader
{
class Level;
}

namespace world
{
struct SectorTween;

namespace core
{
struct SpriteBuffer;
class BaseMesh;
class Frustum;
struct Sprite;
} // namespace core

struct StaticMesh;
struct RoomSprite;
class Entity;

class Room : public Object
{
public:
    using SectorArray = boost::multi_array<RoomSector, 2>;

private:
    uint32_t                    m_flags = 0; //!< room's type + water, wind info
    int16_t                     m_lightMode = 0; //!< (present only in TR2: 0 is normal, 1 is flickering(?), 2 and 3 are uncertain)
    loader::ReverbType          m_reverbType = loader::ReverbType::Outside;
    uint8_t                     m_waterScheme = 0;
    uint8_t                     m_alternateGroup = 0;

    bool m_active = false;
    bool m_hide = true;
    std::shared_ptr<core::BaseMesh> m_mesh; //!< room's base mesh
    core::SpriteBuffer* m_spriteBuffer = nullptr;     //!< Render data for sprites
    std::vector<RoomSprite> m_sprites;

    std::vector<std::shared_ptr<StaticMesh>> m_staticMeshes;

    std::vector<world::Object*> m_objects;

    core::BoundingBox m_boundingBox;
    glm::mat4 m_modelMatrix;

    glm::vec3 m_ambientLighting;

    std::vector<core::Light> m_lights;

    std::vector<Portal> m_portals;
    Room* m_alternateRoom = nullptr;
    Room* m_baseRoom = nullptr; // room->m_alternateRoom->m_baseRoom

    SectorArray m_sectors;

    std::vector<Room*> m_nearRooms;
    std::vector<std::shared_ptr<Room>> m_overlappedRooms;
    std::unique_ptr<btRigidBody> m_btBody;

public:
    explicit Room(ObjectId id, World* world, Room* room = nullptr)
        : Object(id, world, room)
    {
    }

    ~Room();

    bool isActive() const noexcept
    {
        return m_active;
    }

    const Room* getBaseRoom() const noexcept
    {
        return m_baseRoom;
    }

    const Room* getAlternateRoom() const noexcept
    {
        return m_alternateRoom;
    }

    const std::shared_ptr<core::BaseMesh>& getMesh() const noexcept
    {
        return m_mesh;
    }

    const glm::mat4& getModelMatrix() const noexcept
    {
        return m_modelMatrix;
    }

    const glm::vec3& getAmbientLighting() const noexcept
    {
        return m_ambientLighting;
    }

    uint32_t getFlags() const noexcept
    {
        return m_flags;
    }

    const std::vector<world::Object*>& getObjects() const noexcept
    {
        return m_objects;
    }

    loader::ReverbType getReverbType() const noexcept
    {
        return m_reverbType;
    }

    const SectorArray& getSectors() const noexcept
    {
        return m_sectors;
    }

    const core::BoundingBox& getBoundingBox() const noexcept
    {
        return m_boundingBox;
    }

    const std::vector<Room*>& getNearRooms() const noexcept
    {
        return m_nearRooms;
    }

    uint8_t getAlternateGroup() const noexcept
    {
        return m_alternateGroup;
    }

    const std::vector<Portal>& getPortals() const noexcept
    {
        return m_portals;
    }

    const std::vector<core::Light>& getLights() const noexcept
    {
        return m_lights;
    }

    const std::vector<std::shared_ptr<StaticMesh>>& getStaticMeshes() const
    {
        return m_staticMeshes;
    }

    int16_t getLightMode() const noexcept
    {
        return m_lightMode;
    }

    const std::vector<RoomSprite>& getSprites() const noexcept
    {
        return m_sprites;
    }

    const core::SpriteBuffer* getSpriteBuffer() const noexcept
    {
        return m_spriteBuffer;
    }

    void addSprite(core::Sprite* sprite, const glm::vec3& pos);

    void load(const std::unique_ptr<loader::Level>& tr);
    std::vector<SectorTween> generateTweens() const;
    void generateCollisionShape();
    void initVerticalSectorRelations(const loader::Room& tr);
    void generateSpritesBuffer();
    void generateProperties(const loader::Room& tr, const loader::FloorData& floorData, loader::Engine engine);

    void enable();
    void disable();
    void swapToAlternate();
    void swapToBase();
    Room* checkFlip();
    const Room* checkFlip() const
    {
        return const_cast<Room*>(this)->checkFlip();
    }

    void swapPortals(Room* dest_room); //Swap room portals of input room to destination room
    void swapObjects(Room* dest_room);   //Swap room items of input room to destination room
    void buildNearRoomsList();
    void buildOverlappedRoomsList();

    bool isJoined(Room *r2);
    bool overlaps(Room *r1);
    bool isInNearRoomsList(const Room &r) const;
    bool hasSector(size_t x, size_t y) const;//If this room contains a sector
    void addEntity(Entity *entity);
    bool removeEntity(Entity *entity);
    void addToNearRoomsList(Room* r);

    bool contains(const glm::vec3& dot) const
    {
        return m_boundingBox.contains(dot);
    }

    const RoomSector* getSectorRaw(const glm::vec3 &pos) const;
    const RoomSector* getSectorXYZ(const glm::vec3 &pos) const;

    void genMesh(const std::unique_ptr<loader::Level>& tr);

    RoomSector* getSector(int x, int y)
    {
        if(x < 0 || static_cast<size_t>(x) >= m_sectors.shape()[0] || y < 0 || static_cast<size_t>(y) >= m_sectors.shape()[1])
        {
            return nullptr;
        }

        return &m_sectors[x][y];
    }

    const RoomSector* getSector(int x, int y) const
    {
        return const_cast<Room*>(this)->getSector(x, y);
    }
};

btCollisionShape* BT_CSfromHeightmap(const Room::SectorArray& heightmap, const std::vector<SectorTween> &tweens, bool useCompression, bool buildBvh);
} // namespace world
