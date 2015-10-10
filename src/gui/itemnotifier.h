#pragma once

#include <glm/glm.hpp>

namespace gui
{

class ItemNotifier
{
public:
    ItemNotifier();

    void    Start(int item, float time);
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

    float   mShowTime;
    float   mCurrTime;
};

void initNotifier();
void notifierStart(int item);
void notifierStop();
void drawNotifier();

} // namespace gui
