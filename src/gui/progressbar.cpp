#include "progressbar.h"

#include "character_controller.h"
#include "engine/system.h"
#include "world/character.h"

namespace gui
{
namespace
{
std::map<BarType, ProgressBar> g_bar;
}

ProgressBar::ProgressBar()
{
    // Set up some defaults.
    Visible = false;
    Alternate = false;
    Invert = false;
    Vertical = false;
    Forced = false;

    // Initialize parameters.
    // By default, bar is initialized with TR5-like health bar properties.
    SetPosition(HorizontalAnchor::Left, 20, VerticalAnchor::Top, 20);
    SetSize(250, 25, 3);
    SetColor(BarColorType::BaseMain, 255, 50, 50, 150);
    SetColor(BarColorType::BaseFade, 100, 255, 50, 150);
    SetColor(BarColorType::AltMain, 255, 180, 0, 220);
    SetColor(BarColorType::AltFade, 255, 255, 0, 220);
    SetColor(BarColorType::BackMain, 0, 0, 0, 160);
    SetColor(BarColorType::BackFade, 60, 60, 60, 130);
    SetColor(BarColorType::BorderMain, 200, 200, 200, 50);
    SetColor(BarColorType::BorderFade, 80, 80, 80, 100);
    SetValues(1000, 300);
    SetBlink(util::MilliSeconds(300));
    SetExtrude(true, 100);
    SetAutoshow(true, util::MilliSeconds(5000), true, util::MilliSeconds(1000));
}

// Resize bar.
// This function should be called every time resize event occurs.

void ProgressBar::Resize()
{
    RecalculateSize();
    RecalculatePosition();
}

// Set specified color.
void ProgressBar::SetColor(BarColorType colType,
                               uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    float maxColValue = 255.0;

    switch(colType)
    {
        case BarColorType::BaseMain:
            mBaseMainColor[0] = static_cast<float>(R) / maxColValue;
            mBaseMainColor[1] = static_cast<float>(G) / maxColValue;
            mBaseMainColor[2] = static_cast<float>(B) / maxColValue;
            mBaseMainColor[3] = static_cast<float>(A) / maxColValue;
            mBaseMainColor[4] = mBaseMainColor[3];
            break;
        case BarColorType::BaseFade:
            mBaseFadeColor[0] = static_cast<float>(R) / maxColValue;
            mBaseFadeColor[1] = static_cast<float>(G) / maxColValue;
            mBaseFadeColor[2] = static_cast<float>(B) / maxColValue;
            mBaseFadeColor[3] = static_cast<float>(A) / maxColValue;
            mBaseFadeColor[4] = mBaseFadeColor[3];
            break;
        case BarColorType::AltMain:
            mAltMainColor[0] = static_cast<float>(R) / maxColValue;
            mAltMainColor[1] = static_cast<float>(G) / maxColValue;
            mAltMainColor[2] = static_cast<float>(B) / maxColValue;
            mAltMainColor[3] = static_cast<float>(A) / maxColValue;
            mAltMainColor[4] = mAltMainColor[3];
            break;
        case BarColorType::AltFade:
            mAltFadeColor[0] = static_cast<float>(R) / maxColValue;
            mAltFadeColor[1] = static_cast<float>(G) / maxColValue;
            mAltFadeColor[2] = static_cast<float>(B) / maxColValue;
            mAltFadeColor[3] = static_cast<float>(A) / maxColValue;
            mAltFadeColor[4] = mAltFadeColor[3];
            break;
        case BarColorType::BackMain:
            mBackMainColor[0] = static_cast<float>(R) / maxColValue;
            mBackMainColor[1] = static_cast<float>(G) / maxColValue;
            mBackMainColor[2] = static_cast<float>(B) / maxColValue;
            mBackMainColor[3] = static_cast<float>(A) / maxColValue;
            mBackMainColor[4] = mBackMainColor[3];
            break;
        case BarColorType::BackFade:
            mBackFadeColor[0] = static_cast<float>(R) / maxColValue;
            mBackFadeColor[1] = static_cast<float>(G) / maxColValue;
            mBackFadeColor[2] = static_cast<float>(B) / maxColValue;
            mBackFadeColor[3] = static_cast<float>(A) / maxColValue;
            mBackFadeColor[4] = mBackFadeColor[3];
            break;
        case BarColorType::BorderMain:
            mBorderMainColor[0] = static_cast<float>(R) / maxColValue;
            mBorderMainColor[1] = static_cast<float>(G) / maxColValue;
            mBorderMainColor[2] = static_cast<float>(B) / maxColValue;
            mBorderMainColor[3] = static_cast<float>(A) / maxColValue;
            mBorderMainColor[4] = mBorderMainColor[3];
            break;
        case BarColorType::BorderFade:
            mBorderFadeColor[0] = static_cast<float>(R) / maxColValue;
            mBorderFadeColor[1] = static_cast<float>(G) / maxColValue;
            mBorderFadeColor[2] = static_cast<float>(B) / maxColValue;
            mBorderFadeColor[3] = static_cast<float>(A) / maxColValue;
            mBorderFadeColor[4] = mBorderFadeColor[3];
            break;
        default:
            break;
    }
}

void ProgressBar::SetPosition(HorizontalAnchor anchor_X, float offset_X, VerticalAnchor anchor_Y, float offset_Y)
{
    mXanchor = anchor_X;
    mYanchor = anchor_Y;
    mAbsXoffset = offset_X;
    mAbsYoffset = offset_Y;

    RecalculatePosition();
}

// Set bar size
void ProgressBar::SetSize(float width, float height, float borderSize)
{
    // Absolute values are needed to recalculate actual bar size according to resolution.
    mAbsWidth = width;
    mAbsHeight = height;
    mAbsBorderSize = borderSize;

    RecalculateSize();
}

// Recalculate size, according to viewport resolution.
void ProgressBar::RecalculateSize()
{
    mWidth = static_cast<float>(mAbsWidth)  * engine::screen_info.scale_factor;
    mHeight = static_cast<float>(mAbsHeight) * engine::screen_info.scale_factor;

    mBorderWidth = static_cast<float>(mAbsBorderSize)  * engine::screen_info.scale_factor;
    mBorderHeight = static_cast<float>(mAbsBorderSize)  * engine::screen_info.scale_factor;

    // Calculate range unit, according to maximum bar value set up.
    // If bar alignment is set to horizontal, calculate it from bar width.
    // If bar is vertical, then calculate it from height.

    mRangeUnit = (!Vertical) ? ((mWidth) / mMaxValue) : ((mHeight) / mMaxValue);
}

// Recalculate position, according to viewport resolution.
void ProgressBar::RecalculatePosition()
{
    switch(mXanchor)
    {
        case HorizontalAnchor::Left:
            mX = static_cast<float>(mAbsXoffset + mAbsBorderSize) * engine::screen_info.scale_factor;
            break;
        case HorizontalAnchor::Center:
            mX = (static_cast<float>(engine::screen_info.w) - (static_cast<float>(mAbsWidth + mAbsBorderSize * 2) * engine::screen_info.scale_factor)) / 2 +
                (static_cast<float>(mAbsXoffset) * engine::screen_info.scale_factor);
            break;
        case HorizontalAnchor::Right:
            mX = static_cast<float>(engine::screen_info.w) - static_cast<float>(mAbsXoffset + mAbsWidth + mAbsBorderSize * 2) * engine::screen_info.scale_factor;
            break;
    }

    switch(mYanchor)
    {
        case VerticalAnchor::Top:
            mY = static_cast<float>(engine::screen_info.h) - static_cast<float>(mAbsYoffset + mAbsHeight + mAbsBorderSize * 2) * engine::screen_info.scale_factor;
            break;
        case VerticalAnchor::Center:
            mY = (static_cast<float>(engine::screen_info.h) - (static_cast<float>(mAbsHeight + mAbsBorderSize * 2) * engine::screen_info.h_unit)) / 2 +
                (static_cast<float>(mAbsYoffset) * engine::screen_info.scale_factor);
            break;
        case VerticalAnchor::Bottom:
            mY = (mAbsYoffset + mAbsBorderSize) * engine::screen_info.scale_factor;
            break;
    }
}

// Set maximum and warning state values.
void ProgressBar::SetValues(float maxValue, float warnValue)
{
    mMaxValue = maxValue;
    mWarnValue = warnValue;

    RecalculateSize();  // We need to recalculate size, because max. value is changed.
}

// Set warning state blinking interval.
void ProgressBar::SetBlink(util::Duration interval)
{
    mBlinkInterval = interval;
    mBlinkCnt = interval;  // Also reset blink counter.
}

// Set extrude overlay effect parameters.
void ProgressBar::SetExtrude(bool enabled, uint8_t depth)
{
    mExtrude = enabled;
    memset(mExtrudeDepth, 0, sizeof(float) * 5);    // Set all colors to 0.
    mExtrudeDepth[3] = static_cast<float>(depth) / 255.0f;        // We need only alpha transparency.
    mExtrudeDepth[4] = mExtrudeDepth[3];
}

// Set autoshow and fade parameters.
// Please note that fade parameters are actually independent of autoshow.
void ProgressBar::SetAutoshow(bool enabled, util::Duration delay, bool fade, util::Duration fadeDelay)
{
    mAutoShow = enabled;

    mAutoShowDelay = delay;
    mAutoShowCnt = delay;     // Also reset autoshow counter.

    mAutoShowFade = fade;
    mAutoShowFadeDelay = fadeDelay;
    mAutoShowFadeLength = util::Duration(0); // Initially, it's 0.
}

// Main bar show procedure.
// Draws a bar with a given value. Please note that it also accepts float,
// so effectively you can create bars for floating-point parameters.
void ProgressBar::Show(float value)
{
    // Initial value limiters (to prevent bar overflow).
    value = glm::clamp(value, 0.0f, mMaxValue);

    // Enable blink mode, if value is gone below warning value.
    mBlink = (value <= mWarnValue) ? (true) : (false);

    if(mAutoShow)   // Check autoshow visibility conditions.
    {
        // 0. If bar drawing was forced, then show a bar without additional
        //    autoshow delay set. This condition has to be overwritten by
        //    any other conditions, that's why it is set first.
        if(Forced)
        {
            Visible = true;
            Forced = false;
        }
        else
        {
            Visible = false;
        }

        // 1. If bar value gone less than warning value, we show it
        //    in any case, bypassing all other conditions.
        if(value <= mWarnValue)
            Visible = true;

        // 2. Check if bar's value changed,
        //    and if so, start showing it automatically for a given delay time.
        if(mLastValue != value)
        {
            mLastValue = value;
            Visible = true;
            mAutoShowCnt = mAutoShowDelay;
        }

        // 3. If autoshow time is up, then we hide bar,
        //    otherwise decrease delay counter.
        if(mAutoShowCnt.count() > 0)
        {
            Visible = true;
            mAutoShowCnt -= engine::engine_frame_time;

            if(mAutoShowCnt.count() <= 0)
            {
                mAutoShowCnt = util::Duration(0);
                Visible = false;
            }
        }
    } // end if(AutoShow)

    if(mAutoShowFade)   // Process fade-in and fade-out effect, if enabled.
    {
        if(!Visible)
        {
            // If visibility flag is off and bar is still on-screen, gradually decrease
            // fade counter, else simply don't draw anything and exit.
            if(mAutoShowFadeLength.count() == 0)
            {
                return;
            }
            else
            {
                mAutoShowFadeLength -= engine::engine_frame_time;
                if(mAutoShowFadeLength.count() < 0)
                    mAutoShowFadeLength = util::Duration(0);
            }
        }
        else
        {
            // If visibility flag is on, and bar is not yet fully visible, gradually
            // increase fade counter, until it's 1 (i. e. fully opaque).
            if(mAutoShowFadeLength < mAutoShowFadeDelay)
            {
                mAutoShowFadeLength += engine::engine_frame_time;
                if(mAutoShowFadeLength > mAutoShowFadeDelay)
                    mAutoShowFadeLength = mAutoShowFadeDelay;
            }
        } // end if(!Visible)

        // Multiply all layers' alpha by current fade counter.
        mBaseMainColor[3] = mBaseMainColor[4] * mAutoShowFadeLength / mAutoShowFadeDelay;
        mBaseFadeColor[3] = mBaseFadeColor[4] * mAutoShowFadeLength / mAutoShowFadeDelay;
        mAltMainColor[3] = mAltMainColor[4] * mAutoShowFadeLength / mAutoShowFadeDelay;
        mAltFadeColor[3] = mAltFadeColor[4] * mAutoShowFadeLength / mAutoShowFadeDelay;
        mBackMainColor[3] = mBackMainColor[4] * mAutoShowFadeLength / mAutoShowFadeDelay;
        mBackFadeColor[3] = mBackFadeColor[4] * mAutoShowFadeLength / mAutoShowFadeDelay;
        mBorderMainColor[3] = mBorderMainColor[4] * mAutoShowFadeLength / mAutoShowFadeDelay;
        mBorderFadeColor[3] = mBorderFadeColor[4] * mAutoShowFadeLength / mAutoShowFadeDelay;
        mExtrudeDepth[3] = mExtrudeDepth[4] * mAutoShowFadeLength / mAutoShowFadeDelay;
    }
    else
    {
        if(!Visible) return;   // Obviously, quit, if bar is not visible.
    } // end if(mAutoShowFade)

    // Draw border rect.
    // Border rect should be rendered first, as it lies beneath actual bar,
    // and additionally, we need to show it in any case, even if bar is in
    // warning state (blinking).
    drawRect(mX, mY, mWidth + (mBorderWidth * 2), mHeight + (mBorderHeight * 2),
                 mBorderMainColor, mBorderMainColor,
                 mBorderFadeColor, mBorderFadeColor,
                 loader::BlendingMode::Opaque);

    // SECTION FOR BASE BAR RECTANGLE.

    // We check if bar is in a warning state. If it is, we blink it continously.
    if(mBlink)
    {
        mBlinkCnt -= engine::engine_frame_time;
        if(mBlinkCnt > mBlinkInterval)
        {
            value = 0; // Force zero value, which results in empty bar.
        }
        else if(mBlinkCnt.count() <= 0)
        {
            mBlinkCnt = mBlinkInterval * 2;
        }
    }

    // If bar value is zero, just render background overlay and immediately exit.
    // It is needed in case bar is used as a simple UI box to bypass unnecessary calculations.
    if(!value)
    {
        // Draw full-sized background rect (instead of base bar rect)
        drawRect(mX + mBorderWidth, mY + mBorderHeight, mWidth, mHeight,
                     mBackMainColor, (Vertical) ? (mBackFadeColor) : (mBackMainColor),
                     (Vertical) ? (mBackMainColor) : (mBackFadeColor), mBackFadeColor,
                     loader::BlendingMode::Opaque);
        return;
    }

    // Calculate base bar width, according to current value and range unit.
    mBaseSize = mRangeUnit * value;
    mBaseRatio = value / mMaxValue;

    float RectAnchor;           // Anchor to stick base bar rect, according to Invert flag.
    float RectFirstColor[4];    // Used to recalculate gradient, according to current value.
    float RectSecondColor[4];

    // If invert decrease direction style flag is set, we position bar in a way
    // that it seems like it's decreasing to another side, and also swap main / fade colours.
    if(Invert)
    {
        memcpy(RectFirstColor,
               (Alternate) ? (mAltMainColor) : (mBaseMainColor),
               sizeof(float) * 4);

        // Main-fade gradient is recalculated according to current / maximum value ratio.
        for(int i = 0; i <= 3; i++)
            RectSecondColor[i] = (Alternate) ? ((mBaseRatio * mAltFadeColor[i]) + ((1 - mBaseRatio) * mAltMainColor[i]))
            : ((mBaseRatio * mBaseFadeColor[i]) + ((1 - mBaseRatio) * mBaseMainColor[i]));
    }
    else
    {
        memcpy(RectSecondColor,
               (Alternate) ? (mAltMainColor) : (mBaseMainColor),
               sizeof(float) * 4);

        // Main-fade gradient is recalculated according to current / maximum value ratio.
        for(int i = 0; i <= 3; i++)
            RectFirstColor[i] = (Alternate) ? ((mBaseRatio * mAltFadeColor[i]) + ((1 - mBaseRatio) * mAltMainColor[i]))
            : ((mBaseRatio * mBaseFadeColor[i]) + ((1 - mBaseRatio) * mBaseMainColor[i]));
    } // end if(Invert)

    // We need to reset Alternate flag each frame, cause behaviour is immediate.

    Alternate = false;

    // If vertical style flag is set, we draw bar base top-bottom, else we draw it left-right.
    if(Vertical)
    {
        RectAnchor = ((Invert) ? (mY + mHeight - mBaseSize) : (mY)) + mBorderHeight;

        // Draw actual bar base.
        drawRect(mX + mBorderWidth, RectAnchor,
                     mWidth, mBaseSize,
                     RectFirstColor, RectFirstColor,
                     RectSecondColor, RectSecondColor,
                     loader::BlendingMode::Opaque);

        // Draw background rect.
        drawRect(mX + mBorderWidth,
                     (Invert) ? (mY + mBorderHeight) : (RectAnchor + mBaseSize),
                     mWidth, mHeight - mBaseSize,
                     mBackMainColor, mBackFadeColor,
                     mBackMainColor, mBackFadeColor,
                     loader::BlendingMode::Opaque);

        if(mExtrude)    // Draw extrude overlay, if flag is set.
        {
            float transparentColor[4] = { 0 };  // Used to set counter-shade to transparent.

            drawRect(mX + mBorderWidth, RectAnchor,
                         mWidth / 2, mBaseSize,
                         mExtrudeDepth, transparentColor,
                         mExtrudeDepth, transparentColor,
                         loader::BlendingMode::Opaque);
            drawRect(mX + mBorderWidth + mWidth / 2, RectAnchor,
                         mWidth / 2, mBaseSize,
                         transparentColor, mExtrudeDepth,
                         transparentColor, mExtrudeDepth,
                         loader::BlendingMode::Opaque);
        }
    }
    else
    {
        RectAnchor = ((Invert) ? (mX + mWidth - mBaseSize) : (mX)) + mBorderWidth;

        // Draw actual bar base.
        drawRect(RectAnchor, mY + mBorderHeight,
                     mBaseSize, mHeight,
                     RectSecondColor, RectFirstColor,
                     RectSecondColor, RectFirstColor,
                     loader::BlendingMode::Opaque);

        // Draw background rect.
        drawRect((Invert) ? (mX + mBorderWidth) : (RectAnchor + mBaseSize),
                     mY + mBorderHeight,
                     mWidth - mBaseSize, mHeight,
                     mBackMainColor, mBackMainColor,
                     mBackFadeColor, mBackFadeColor,
                     loader::BlendingMode::Opaque);

        if(mExtrude)    // Draw extrude overlay, if flag is set.
        {
            float transparentColor[4] = { 0 };  // Used to set counter-shade to transparent.

            drawRect(RectAnchor, mY + mBorderHeight,
                         mBaseSize, mHeight / 2,
                         transparentColor, transparentColor,
                         mExtrudeDepth, mExtrudeDepth,
                         loader::BlendingMode::Opaque);
            drawRect(RectAnchor, mY + mBorderHeight + (mHeight / 2),
                         mBaseSize, mHeight / 2,
                         mExtrudeDepth, mExtrudeDepth,
                         transparentColor, transparentColor,
                         loader::BlendingMode::Opaque);
        }
    } // end if(Vertical)
}

void initBars()
{
    {
        const auto i = BarType::Health;
        g_bar[i].Visible = false;
        g_bar[i].Alternate = false;
        g_bar[i].Invert = false;
        g_bar[i].Vertical = false;

        g_bar[i].SetSize(250, 15, 3);
        g_bar[i].SetPosition(HorizontalAnchor::Left, 30, VerticalAnchor::Top, 30);
        g_bar[i].SetColor(BarColorType::BaseMain, 255, 50, 50, 200);
        g_bar[i].SetColor(BarColorType::BaseFade, 100, 255, 50, 200);
        g_bar[i].SetColor(BarColorType::AltMain, 255, 180, 0, 255);
        g_bar[i].SetColor(BarColorType::AltFade, 255, 255, 0, 255);
        g_bar[i].SetColor(BarColorType::BackMain, 0, 0, 0, 160);
        g_bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 130);
        g_bar[i].SetColor(BarColorType::BorderMain, 200, 200, 200, 50);
        g_bar[i].SetColor(BarColorType::BorderFade, 80, 80, 80, 100);
        g_bar[i].SetValues(LARA_PARAM_HEALTH_MAX, LARA_PARAM_HEALTH_MAX / 3);
        g_bar[i].SetBlink(util::MilliSeconds(300));
        g_bar[i].SetExtrude(true, 100);
        g_bar[i].SetAutoshow(true, util::MilliSeconds(2000), true, util::MilliSeconds(400));
    }
    {
        const auto i = BarType::Air;
        g_bar[i].Visible = false;
        g_bar[i].Alternate = false;
        g_bar[i].Invert = true;
        g_bar[i].Vertical = false;

        g_bar[i].SetSize(250, 15, 3);
        g_bar[i].SetPosition(HorizontalAnchor::Right, 30, VerticalAnchor::Top, 30);
        g_bar[i].SetColor(BarColorType::BaseMain, 0, 50, 255, 200);
        g_bar[i].SetColor(BarColorType::BaseFade, 190, 190, 255, 200);
        g_bar[i].SetColor(BarColorType::BackMain, 0, 0, 0, 160);
        g_bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 130);
        g_bar[i].SetColor(BarColorType::BorderMain, 200, 200, 200, 50);
        g_bar[i].SetColor(BarColorType::BorderFade, 80, 80, 80, 100);
        g_bar[i].SetValues(LARA_PARAM_AIR_MAX, (LARA_PARAM_AIR_MAX / 3));
        g_bar[i].SetBlink(util::MilliSeconds(300));
        g_bar[i].SetExtrude(true, 100);
        g_bar[i].SetAutoshow(true, util::MilliSeconds(2000), true, util::MilliSeconds(400));
    }
    {
        const auto i = BarType::Stamina;
        g_bar[i].Visible = false;
        g_bar[i].Alternate = false;
        g_bar[i].Invert = false;
        g_bar[i].Vertical = false;

        g_bar[i].SetSize(250, 15, 3);
        g_bar[i].SetPosition(HorizontalAnchor::Left, 30, VerticalAnchor::Top, 55);
        g_bar[i].SetColor(BarColorType::BaseMain, 255, 100, 50, 200);
        g_bar[i].SetColor(BarColorType::BaseFade, 255, 200, 0, 200);
        g_bar[i].SetColor(BarColorType::BackMain, 0, 0, 0, 160);
        g_bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 130);
        g_bar[i].SetColor(BarColorType::BorderMain, 110, 110, 110, 100);
        g_bar[i].SetColor(BarColorType::BorderFade, 60, 60, 60, 180);
        g_bar[i].SetValues(LARA_PARAM_STAMINA_MAX, 0);
        g_bar[i].SetExtrude(true, 100);
        g_bar[i].SetAutoshow(true, util::MilliSeconds(500), true, util::MilliSeconds(300));
    }
    {
        const auto i = BarType::Warmth;
        g_bar[i].Visible = false;
        g_bar[i].Alternate = false;
        g_bar[i].Invert = true;
        g_bar[i].Vertical = false;

        g_bar[i].SetSize(250, 15, 3);
        g_bar[i].SetPosition(HorizontalAnchor::Right, 30, VerticalAnchor::Top, 55);
        g_bar[i].SetColor(BarColorType::BaseMain, 255, 0, 255, 255);
        g_bar[i].SetColor(BarColorType::BaseFade, 190, 120, 255, 255);
        g_bar[i].SetColor(BarColorType::BackMain, 0, 0, 0, 160);
        g_bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 130);
        g_bar[i].SetColor(BarColorType::BorderMain, 200, 200, 200, 50);
        g_bar[i].SetColor(BarColorType::BorderFade, 80, 80, 80, 100);
        g_bar[i].SetValues(LARA_PARAM_WARMTH_MAX, LARA_PARAM_WARMTH_MAX / 3);
        g_bar[i].SetBlink(util::MilliSeconds(200));
        g_bar[i].SetExtrude(true, 60);
        g_bar[i].SetAutoshow(true, util::MilliSeconds(500), true, util::MilliSeconds(300));
    }
    {
        const auto i = BarType::Loading;
        g_bar[i].Visible = true;
        g_bar[i].Alternate = false;
        g_bar[i].Invert = false;
        g_bar[i].Vertical = false;

        g_bar[i].SetSize(800, 25, 3);
        g_bar[i].SetPosition(HorizontalAnchor::Center, 0, VerticalAnchor::Bottom, 40);
        g_bar[i].SetColor(BarColorType::BaseMain, 255, 225, 127, 230);
        g_bar[i].SetColor(BarColorType::BaseFade, 255, 187, 136, 230);
        g_bar[i].SetColor(BarColorType::BackMain, 30, 30, 30, 100);
        g_bar[i].SetColor(BarColorType::BackFade, 60, 60, 60, 100);
        g_bar[i].SetColor(BarColorType::BorderMain, 200, 200, 200, 80);
        g_bar[i].SetColor(BarColorType::BorderFade, 80, 80, 80, 80);
        g_bar[i].SetValues(1000, 0);
        g_bar[i].SetExtrude(true, 70);
        g_bar[i].SetAutoshow(false, util::MilliSeconds(500), false, util::MilliSeconds(300));
    }
}

void drawBars()
{
    if(engine::engine_world.character)
    {
        if(engine::engine_world.character->m_currentWeaponState > world::WeaponState::HideToReady)
            g_bar[BarType::Health].Forced = true;

        if(engine::engine_world.character->getParam(world::PARAM_POISON) > 0.0)
            g_bar[BarType::Health].Alternate = true;

        g_bar[BarType::Air].Show(engine::engine_world.character->getParam(world::PARAM_AIR));
        g_bar[BarType::Stamina].Show(engine::engine_world.character->getParam(world::PARAM_STAMINA));
        g_bar[BarType::Health].Show(engine::engine_world.character->getParam(world::PARAM_HEALTH));
        g_bar[BarType::Warmth].Show(engine::engine_world.character->getParam(world::PARAM_WARMTH));
    }
}

void showLoadingProgressBar(int value)
{
    g_bar[BarType::Loading].Show(value);
}

void resizeProgressBars()
{
    for(auto& i : g_bar)
    {
        i.second.Resize();
    }
}

} // namespace gui
