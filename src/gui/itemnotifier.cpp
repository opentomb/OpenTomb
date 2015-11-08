#include "itemnotifier.h"

#include "engine/engine.h"
#include "engine/system.h"
#include "gui.h"

#include <glm/gtc/matrix_transform.hpp>

namespace gui
{
namespace
{
    ItemNotifier g_notifier;
}

ItemNotifier::ItemNotifier()
{
    SetPos(850, 850);
    SetRot(0, 0);
    SetSize(1.0);
    SetRotateTime(1000.0);

    mItem = 0;
    mActive = false;
}

void ItemNotifier::Start(int item, util::Duration time)
{
    Reset();

    mItem = item;
    mShowTime = time;
    mActive = true;
}

void ItemNotifier::Animate()
{
    if(!mActive)
    {
        return;
    }

    if(!util::fuzzyZero(m_rotationSpeed))
    {
        m_currentRotation.x = glm::mod(m_currentRotation.x + util::toSeconds(engine::engine_frame_time) * m_rotationSpeed, glm::radians(360.0f));
    }

    if(util::fuzzyZero(mCurrTime.count()))
    {
        float step = (mCurrPosX - mEndPosX) * util::toSeconds(engine::engine_frame_time) * 4.0f;
        step = std::max(0.5f, step);

        mCurrPosX -= step;
        mCurrPosX = glm::min(mCurrPosX, mEndPosX);

        if(mCurrPosX == mEndPosX)
            mCurrTime += engine::engine_frame_time;
    }
    else if(mCurrTime < mShowTime)
    {
        mCurrTime += engine::engine_frame_time;
    }
    else
    {
        float step = (mCurrPosX - mEndPosX) * util::toSeconds(engine::engine_frame_time * 4);
        step = std::max(0.5f, step);

        mCurrPosX += step;
        mCurrPosX = glm::min(mCurrPosX, mStartPosX);

        if(mCurrPosX == mStartPosX)
            Reset();
    }
}

void ItemNotifier::Reset()
{
    mActive = false;
    mCurrTime = util::Duration(0);
    m_currentRotation = {0,0};

    mEndPosX = (static_cast<float>(engine::screen_info.w) / ScreenMeteringResolution) * mAbsPosX;
    mPosY = (static_cast<float>(engine::screen_info.h) / ScreenMeteringResolution) * mAbsPosY;
    mCurrPosX = engine::screen_info.w + (static_cast<float>(engine::screen_info.w) / NotifierOffscreenDivider * mSize);
    mStartPosX = mCurrPosX;    // Equalize current and start positions.
}

void ItemNotifier::Draw()
{
    if(!mActive)
        return;

    auto item = engine::engine_world.getBaseItemByID(mItem);
    if(!item)
        return;

    const auto anim = item->bf->getCurrentAnimation();
    const auto frame = item->bf->getCurrentFrame();

    item->bf->setCurrentAnimation(0);
    item->bf->setCurrentFrame(0);

    item->bf->itemFrame(util::Duration(0));
    glm::mat4 matrix(1.0f);
    matrix = glm::translate(matrix, { mCurrPosX, mPosY, -2048.0 });
    matrix = glm::rotate(matrix, m_currentRotation.x + m_rotation.x, { 0,1,0 });
    matrix = glm::rotate(matrix, m_currentRotation.y + m_rotation.y, { 1,0,0 });
    render::renderItem(item->bf.get(), mSize, matrix, gui::guiProjectionMatrix);

    item->bf->setCurrentAnimation(anim);
    item->bf->setCurrentFrame(frame);
}

void ItemNotifier::SetPos(float X, float Y)
{
    mAbsPosX = X;
    mAbsPosY = 1000.0f - Y;
}

void ItemNotifier::SetRot(glm::float_t X, glm::float_t Y)
{
    m_rotation = {X,Y};
}

void ItemNotifier::SetSize(float size)
{
    mSize = size;
}

void ItemNotifier::SetRotateTime(float time)
{
    m_rotationSpeed = (1000.0f / time) * glm::radians(360.0f);
}

void initNotifier()
{
    g_notifier.SetPos(850.0, 850.0);
    g_notifier.SetRot(glm::radians(180.0f), glm::radians(270.0f));
    g_notifier.SetSize(128.0);
    g_notifier.SetRotateTime(2500.0);
}

void notifierStart(int item)
{
    g_notifier.Start(item, NotifierShowtime);
}

void notifierStop()
{
    g_notifier.Reset();
}

void drawNotifier()
{
    g_notifier.Draw();
    g_notifier.Animate();
}

} // namespace gui
