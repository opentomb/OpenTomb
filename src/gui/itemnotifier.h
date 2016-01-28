#pragma once

#include "util/helpers.h"
#include "world/object.h"

#include <glm/glm.hpp>

namespace engine
{
class Engine;
}

namespace gui
{
class ItemNotifier
{
public:
    explicit ItemNotifier(engine::Engine* engine);

    void    start(world::ObjectId item, util::Duration time = util::Seconds(2));
    void    reset();
    void    animate();
    void    draw() const;

    void    setPos(glm::float_t X, glm::float_t Y);
    void    setRotation(glm::float_t X, glm::float_t Y);
    void    setSize(glm::float_t size);
    void    setRotateTime(util::Duration time);

private:
    engine::Engine* m_engine;

    bool    m_active = false;
    world::ObjectId m_item = 0;

    glm::vec2 m_absPos;

    glm::float_t   m_posY;
    glm::float_t   m_startPosX;
    glm::float_t   m_endPosX;
    glm::float_t   m_currPosX;

    glm::vec2 m_rotation{ 0,0 };
    glm::vec2 m_currentAngle{ 0,0 };
    glm::float_t m_radPerSecond = 0;

    glm::float_t m_size;

    util::Duration m_showTime;
    util::Duration m_currTime;
};
} // namespace gui
