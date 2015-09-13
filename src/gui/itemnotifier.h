#pragma once

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
    void    SetRot(float X, float Y);
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

    float   mRotX;
    float   mRotY;
    float   mCurrRotX;
    float   mCurrRotY;

    float   mSize;

    float   mShowTime;
    float   mCurrTime;
    float   mRotateTime;
};

void initNotifier();
void notifierStart(int item);
void notifierStop();
void drawNotifier();

} // namespace gui
