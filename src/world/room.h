#pragma once

#include "core/boundingbox.h"
#include "world/core/light.h"
#include "object.h"
#include "portal.h"

#include <cstdint>
#include <memory>
#include <vector>

#include <boost/multi_array.hpp>
#include <boost/optional.hpp>

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
struct BaseMesh;
class Frustum;
} // namespace core

struct StaticMesh;
struct RoomSprite;
struct Entity;
struct World;

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

    RoomSector        *sector_below;
    RoomSector        *sector_above;
    std::shared_ptr<Room> owner_room;    // Room that contain this sector

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

    RoomSector* getLowestSector();
    RoomSector* getHighestSector();

    RoomSector* checkFlip();
    RoomSector* checkBaseRoom();
    RoomSector* checkAlternateRoom();
    RoomSector* checkPortalPointerRaw();
    RoomSector* checkPortalPointer();
    bool is2SidePortals(RoomSector* s2);
    bool similarCeiling(RoomSector* s2, bool ignore_doors) const;
    bool similarFloor(RoomSector* s2, bool ignore_doors) const;
    glm::vec3 getFloorPoint();
    glm::vec3 getCeilingPoint();

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

struct Room : public Object
{
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
    std::shared_ptr<Room> m_alternateRoom;
    std::shared_ptr<Room> m_baseRoom; // room->m_alternateRoom->m_baseRoom

    boost::multi_array<RoomSector, 2> m_sectors;

    std::vector<Room*> m_nearRooms;
    std::vector<std::shared_ptr<Room>> m_overlappedRooms;
    std::unique_ptr<btRigidBody> m_btBody;

    explicit Room(uint32_t id, Room* room = nullptr)
        : Object(id, room)
    {
    }

    ~Room();

    void enable();
    void disable();
    void swapToAlternate();
    void swapToBase();
    Room *checkFlip();
    void swapPortals(std::shared_ptr<Room> dest_room); //Swap room portals of input room to destination room
    void swapObjects(std::shared_ptr<Room> dest_room);   //Swap room items of input room to destination room
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

    RoomSector* getSectorRaw(const glm::vec3 &pos);
    RoomSector* getSectorXYZ(const glm::vec3 &pos);

    void genMesh(World& world, const std::unique_ptr<loader::Level>& tr);
};

btCollisionShape* BT_CSfromHeightmap(const boost::multi_array<RoomSector, 2>& heightmap, const std::vector<SectorTween> &tweens, bool useCompression, bool buildBvh);

} // namespace world
