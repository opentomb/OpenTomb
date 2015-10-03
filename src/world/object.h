#pragma once

#include <cstdint>

namespace world
{
struct Room;

enum class CollisionShape
{
    Box,
    BoxBase,      //!< use single box collision
    Sphere,
    TriMesh,      //!< for static objects and room's!
    TriMeshConvex //!< for dynamic objects
};

enum class CollisionType
{
    None,
    Static,    //!< static object - never moved
    Kinematic, //!< doors and other moveable statics
    Dynamic,   //!< hellow full physics interaction
    Actor,     //!< actor, enemies, NPC, animals
    Vehicle,   //!< car, moto, bike
    Ghost      //!< no fix character position, but works in collision callbacks and interacts with dynamic objects
};

class Object
{
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

public:
    virtual ~Object() = default;

    world::Room* getRoom() const noexcept
    {
        return m_room;
    }

    void setRoom(world::Room* room) noexcept
    {
        m_room = room;
    }

    CollisionType getCollisionType() const noexcept
    {
        return m_collisionType;
    }

    void setCollisionType(CollisionType type) noexcept
    {
        m_collisionType = type;
    }

    CollisionShape getCollisionShape() const noexcept
    {
        return m_collisionShape;
    }

    void setCollisionShape(CollisionShape shape) noexcept
    {
        m_collisionShape = shape;
    }

    uint32_t getId() const noexcept
    {
        return m_id;
    }

protected:
    explicit Object(uint32_t id, Room* room = nullptr)
        : m_id(id)
        , m_room(room)
    {
    }

private:
    const uint32_t m_id;                 // Unique entity ID

    world::Room* m_room = nullptr;
    CollisionType m_collisionType = CollisionType::None;
    CollisionShape m_collisionShape = CollisionShape::Box;
};

class BulletObject : public Object
{
public:
    explicit BulletObject(Room* room)
        : Object(0, room)
    {
    }
};

} // namespace world
