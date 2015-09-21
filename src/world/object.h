#pragma once

namespace world
{
class Room;

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

protected:
    explicit Object(Room* room = nullptr)
        : m_room(room)
    {
    }

private:
    world::Room* m_room = nullptr;
};

} // namespace world
