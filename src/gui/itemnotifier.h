#pragma once

#include "util/helpers.h"

#include <glm/glm.hpp>

namespace gui
{

class ItemNotifier
{
public:
    ItemNotifier();

    void    Start(int item, util::Duration time);
    void    Reset();
    void    Animate();
    void    Draw();

    void    SetPos(float X, float Y);
    void    SetRot(glm::float_t X, glm::float_t Y);
    void    SetSize(float size);
    void    SetRotateTime(float time);

private:
    bool    mActive;
    int     mItem;

    float   mAbsPosY;
    float   mAbsPosX;

    float   mPosY;
    float   mStartPosX;
    float   mEndPosX;
    float   mCurrPosX;

    glm::vec2 m_rotation{0,0};
    glm::vec2 m_currentRotation{0,0};
    glm::float_t m_rotationSpeed = 0;

    float   mSize;

    util::Duration mShowTime;
    util::Duration mCurrTime;
};

void initNotifier();
void notifierStart(int item);
void notifierStop();
void drawNotifier();

} // namespace gui
