#include "itemnotifier.h"

#include "engine/engine.h"
#include "engine/system.h"
#include "gui.h"
#include "world/entity.h"

#include <glm/gtc/matrix_transform.hpp>

namespace gui
{
ItemNotifier::ItemNotifier(engine::Engine* engine)
    : m_engine(engine)
{
    BOOST_LOG_TRIVIAL(info) << "Initializing ItemNotifier";

    setPos(850, 850);
    // SetRot(0, 0);
    setRotation(glm::radians(180.0f), glm::radians(270.0f));
    // SetSize(1.0);
    setSize(128.0);
    // SetRotateTime(1000.0);
    setRotateTime(util::Seconds(2.5));
}

void ItemNotifier::start(world::ObjectId item, util::Duration time)
{
    reset();

    m_item = item;
    m_showTime = time;
    m_active = true;
}

void ItemNotifier::animate()
{
    if(!m_active)
    {
        return;
    }

    if(!util::fuzzyZero(m_radPerSecond))
    {
        m_currentAngle.x = glm::mod(m_currentAngle.x + m_engine->getFrameTimeSecs() * m_radPerSecond, glm::radians(360.0f));
    }

    if(util::fuzzyZero(m_currTime.count()))
    {
        glm::float_t step = (m_currPosX - m_endPosX) * m_engine->getFrameTimeSecs() * 4.0f;
        step = std::max(0.5f, step);

        m_currPosX = glm::min(m_currPosX - step, m_endPosX);

        if(util::fuzzyEqual(m_currPosX, m_endPosX))
            m_currTime += m_engine->getFrameTime();
    }
    else if(m_currTime < m_showTime)
    {
        m_currTime += m_engine->getFrameTime();
    }
    else
    {
        glm::float_t step = (m_currPosX - m_endPosX) * m_engine->getFrameTimeSecs() * 4;
        step = std::max(0.5f, step);

        m_currPosX = glm::min(m_currPosX + step, m_startPosX);

        if(m_currPosX == m_startPosX)
            reset();
    }
}

void ItemNotifier::reset()
{
    m_active = false;
    m_currTime = util::Duration(0);
    m_currentAngle = { 0,0 };

    m_endPosX = static_cast<glm::float_t>(m_engine->screen_info.w) / ScreenMeteringResolution * m_absPos.x;
    m_posY = static_cast<float>(m_engine->screen_info.h) / ScreenMeteringResolution * m_absPos.y;
    m_currPosX = m_engine->screen_info.w + static_cast<float>(m_engine->screen_info.w) / NotifierOffscreenDivider * m_size;
    m_startPosX = m_currPosX;    // Equalize current and start positions.
}

void ItemNotifier::draw() const
{
    if(!m_active)
        return;

    auto item = m_engine->m_world.getBaseItemByID(m_item);
    if(!item)
        return;

    const world::animation::AnimationId anim = item->getSkeleton().getCurrentAnimation();
    const auto frame = item->getSkeleton().getCurrentFrame();

    item->getSkeleton().setCurrentAnimation(0);
    item->getSkeleton().setCurrentFrame(0);

    item->getSkeleton().itemFrame(util::Duration(0));
    glm::mat4 matrix(1.0f);
    matrix = glm::translate(matrix, { m_currPosX, m_posY, -2048.0 });
    matrix = glm::rotate(matrix, m_currentAngle.x + m_rotation.x, { 0,1,0 });
    matrix = glm::rotate(matrix, m_currentAngle.y + m_rotation.y, { 1,0,0 });
    render::renderItem(item->getSkeleton(), m_size, matrix, m_engine->m_gui.m_guiProjectionMatrix);

    item->getSkeleton().setCurrentAnimation(anim);
    item->getSkeleton().setCurrentFrame(frame);
}

void ItemNotifier::setPos(glm::float_t X, glm::float_t Y)
{
    m_absPos = { X, 1000.0f - Y };
}

void ItemNotifier::setRotation(glm::float_t X, glm::float_t Y)
{
    m_rotation = { X, Y };
}

void ItemNotifier::setSize(glm::float_t size)
{
    m_size = size;
}

void ItemNotifier::setRotateTime(util::Duration time)
{
    m_radPerSecond = util::Seconds(1) / time * glm::radians(360.0f);
}
} // namespace gui
