#pragma once

#include "gui.h"

namespace gui
{
// These are the bars that are always exist in GUI.
// Scripted bars could be created and drawn separately later.

enum class BarType
{
    Health,     // TR 1-5
    Air,        // TR 1-5, alternate state - gas (TR5)
    Stamina,    // TR 3-5
    Warmth,     // TR 3 only
    Loading,
    Sentinel
};

// Bar color types.
// Each bar part basically has two colours - main and fade.

enum class BarColorType
{
    BaseMain,
    BaseFade,
    AltMain,
    AltFade,
    BackMain,
    BackFade,
    BorderMain,
    BorderFade
};

class ProgressBar
{
public:
    ProgressBar();  // Bar constructor.

    void Show(float value);    // Main show bar procedure.
    void Resize();

    void SetColor(BarColorType colType, uint8_t R, uint8_t G, uint8_t B, uint8_t alpha);
    void SetSize(float width, float height, float borderSize);
    void SetPosition(HorizontalAnchor anchor_X, float offset_X, VerticalAnchor anchor_Y, float offset_Y);
    void SetValues(float maxValue, float warnValue);
    void SetBlink(int interval);
    void SetExtrude(bool enabled, uint8_t depth);
    void SetAutoshow(bool enabled, int delay, bool fade, int fadeDelay);

    bool          Forced;               // Forced flag is set when bar is strictly drawn.
    bool          Visible;              // Is it visible or not.
    bool          Alternate;            // Alternate state, in which bar changes color to AltColor.

    bool          Invert;               // Invert decrease direction flag.
    bool          Vertical;             // Change bar style to vertical.

private:
    void          RecalculateSize();    // Recalculate size and position.
    void          RecalculatePosition();

    float         mX;                   // Horizontal position.
    float         mY;                   // Vertical position.
    float         mWidth;               // Real width.
    float         mHeight;              // Real height.
    float         mBorderWidth;         // Real border size (horizontal).
    float         mBorderHeight;        // Real border size (vertical).

    HorizontalAnchor mXanchor;          // Horizontal anchoring: left, right or center.
    VerticalAnchor   mYanchor;          // Vertical anchoring: top, bottom or center.
    float         mAbsXoffset;          // Absolute (resolution-independent) X offset.
    float         mAbsYoffset;          // Absolute Y offset.
    float         mAbsWidth;            // Absolute width.
    float         mAbsHeight;           // Absolute height.
    float         mAbsBorderSize;       // Absolute border size (horizontal).

    float         mBaseMainColor[5];    // Color at the min. of bar.
    float         mBaseFadeColor[5];    // Color at the max. of bar.
    float         mAltMainColor[5];     // Alternate main color.
    float         mAltFadeColor[5];     // Alternate fade color.
    float         mBackMainColor[5];    // Background main color.
    float         mBackFadeColor[5];    // Background fade color.
    float         mBorderMainColor[5];  // Border main color.
    float         mBorderFadeColor[5];  // Border fade color.

    // int8_t        mBaseBlendingMode;    // Blending modes for all bar parts.
    // int8_t        mBackBlendingMode;    // Note there is no alt. blending mode, cause
    // int8_t        mBorderBlendingMode;  // base and alt are actually the same part.

    bool          mExtrude;             // Extrude effect.
    float         mExtrudeDepth[5];     // Extrude effect depth.

    float         mMaxValue;            // Maximum possible value.
    float         mWarnValue;           // Warning value, at which bar begins to blink.
    float         mLastValue;           // Last value back-up for autoshow on change event.

    bool          mBlink;               // Warning state (blink) flag.
    float         mBlinkInterval;       // Blink interval (speed).
    float         mBlinkCnt;            // Blink counter.

    bool          mAutoShow;            // Autoshow on change flag.
    float         mAutoShowDelay;       // How long bar will stay on-screen in AutoShow mode.
    float         mAutoShowCnt;         // Auto-show counter.
    bool          mAutoShowFade;        // Fade flag.
    float         mAutoShowFadeDelay;   // Fade length.
    float         mAutoShowFadeCnt;     // Fade progress counter.

    float         mRangeUnit;           // Range unit used to set base bar size.
    float         mBaseSize;            // Base bar size.
    float         mBaseRatio;           // Max. / actual value ratio.
};

void initBars();
void drawBars();
void showLoadingProgressBar(int value);
void resizeProgressBars();

} // namespace gui
