#include "itemnotifier.h"

#include "engine/engine.h"
#include "engine/system.h"
#include "gui.h"

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

void ItemNotifier::Start(int item, float time)
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

    if(mRotateTime)
    {
        mCurrRotX += (engine::engine_frame_time * mRotateTime);
        //mCurrRotY += (engine_frame_time * mRotateTime);

        mCurrRotX = (mCurrRotX > 360.0) ? (mCurrRotX - 360.0f) : (mCurrRotX);
        //mCurrRotY = (mCurrRotY > 360.0)?(mCurrRotY - 360.0):(mCurrRotY);
    }

    if(mCurrTime == 0)
    {
        float step = (mCurrPosX - mEndPosX) * (engine::engine_frame_time * 4.0f);
        step = std::max(0.5f, step);

        mCurrPosX -= step;
        mCurrPosX = (mCurrPosX < mEndPosX) ? (mEndPosX) : (mCurrPosX);

        if(mCurrPosX == mEndPosX)
            mCurrTime += engine::engine_frame_time;
    }
    else if(mCurrTime < mShowTime)
    {
        mCurrTime += engine::engine_frame_time;
    }
    else
    {
        float step = (mCurrPosX - mEndPosX) * (engine::engine_frame_time * 4.0f);
        step = std::max(0.5f, step);

        mCurrPosX += step;
        mCurrPosX = (mCurrPosX > mStartPosX) ? (mStartPosX) : (mCurrPosX);

        if(mCurrPosX == mStartPosX)
            Reset();
    }
}

void ItemNotifier::Reset()
{
    mActive = false;
    mCurrTime = 0.0;
    mCurrRotX = 0.0;
    mCurrRotY = 0.0;

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

    int anim = item->bf->animations.current_animation;
    int frame = item->bf->animations.current_frame;
    btScalar time = item->bf->animations.frame_time;

    item->bf->animations.current_animation = 0;
    item->bf->animations.current_frame = 0;
    item->bf->animations.frame_time = 0.0;

    itemFrame(item->bf.get(), 0.0);
    btTransform matrix;
    matrix.setIdentity();
    util::Mat4_Translate(matrix, mCurrPosX, mPosY, -2048.0);
    util::Mat4_RotateY(matrix, mCurrRotX + mRotX);
    util::Mat4_RotateX(matrix, mCurrRotY + mRotY);
    renderItem(item->bf.get(), mSize, matrix);

    item->bf->animations.current_animation = anim;
    item->bf->animations.current_frame = frame;
    item->bf->animations.frame_time = time;
}

void ItemNotifier::SetPos(float X, float Y)
{
    mAbsPosX = X;
    mAbsPosY = 1000.0f - Y;
}

void ItemNotifier::SetRot(float X, float Y)
{
    mRotX = X;
    mRotY = Y;
}

void ItemNotifier::SetSize(float size)
{
    mSize = size;
}

void ItemNotifier::SetRotateTime(float time)
{
    mRotateTime = (1000.0f / time) * 360.0f;
}

void initNotifier()
{
    g_notifier.SetPos(850.0, 850.0);
    g_notifier.SetRot(180.0, 270.0);
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
