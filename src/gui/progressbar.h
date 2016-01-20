#pragma once

#include "gui.h"

#include <glm/glm.hpp>

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

    void show(glm::float_t value);    // Main show bar procedure.
    void resize();

    void setColor(BarColorType colType, uint8_t R, uint8_t G, uint8_t B, uint8_t alpha);
    void setSize(glm::float_t width, glm::float_t height, glm::float_t borderSize);
    void setPosition(HorizontalAnchor anchor_X, glm::float_t offset_X, VerticalAnchor anchor_Y, glm::float_t offset_Y);
    void setValues(glm::float_t maxValue, glm::float_t warnValue);
    void setBlink(util::Duration interval);
    void setExtrude(bool enabled, uint8_t depth);
    void setAutoshow(bool enabled, util::Duration delay, bool fade, util::Duration fadeDelay);

    bool          m_forced;               // Forced flag is set when bar is strictly drawn.
    bool          m_visible;              // Is it visible or not.
    bool          m_alternate;            // Alternate state, in which bar changes color to AltColor.

    bool          m_invert;               // Invert decrease direction flag.
    bool          m_vertical;             // Change bar style to vertical.

private:
    void          recalculateSize();    // Recalculate size and position.
    void          recalculatePosition();

    float         m_x;                   // Horizontal position.
    float         m_y;                   // Vertical position.
    float         m_width;               // Real width.
    float         m_height;              // Real height.
    float         m_borderWidth;         // Real border size (horizontal).
    float         m_borderHeight;        // Real border size (vertical).

    HorizontalAnchor m_xAnchor;          // Horizontal anchoring: left, right or center.
    VerticalAnchor   m_yAnchor;          // Vertical anchoring: top, bottom or center.
    float         m_absXoffset;          // Absolute (resolution-independent) X offset.
    float         m_absYoffset;          // Absolute Y offset.
    float         m_absWidth;            // Absolute width.
    float         m_absHeight;           // Absolute height.
    float         m_absBorderSize;       // Absolute border size (horizontal).

    glm::vec4 m_baseMainColor;    // Color at the min. of bar.
    glm::float_t m_baseMainColorAlpha;
    glm::vec4 m_baseFadeColor;    // Color at the max. of bar.
    glm::float_t m_baseFadeColorAlpha;
    glm::vec4 m_altMainColor;     // Alternate main color.
    glm::float_t m_altMainColorAlpha;
    glm::vec4 m_altFadeColor;     // Alternate fade color.
    glm::float_t m_altFadeColorAlpha;
    glm::vec4 m_backMainColor;    // Background main color.
    glm::float_t m_backMainColorAlpha;
    glm::vec4 m_backFadeColor;    // Background fade color.
    glm::float_t m_backFadeColorAlpha;
    glm::vec4 m_borderMainColor;  // Border main color.
    glm::float_t m_borderMainColorAlpha;
    glm::vec4 m_borderFadeColor;  // Border fade color.
    glm::float_t m_borderFadeColorAlpha;

    bool m_extrude;             // Extrude effect.
    glm::vec4 m_extrudeDepth;     // Extrude effect depth.
    glm::float_t m_extrudeDepthAlpha;

    glm::float_t m_maxValue;            // Maximum possible value.
    glm::float_t m_warnValue;           // Warning value, at which bar begins to blink.
    glm::float_t m_lastValue;           // Last value back-up for autoshow on change event.

    bool m_blink;               // Warning state (blink) flag.
    util::Duration m_blinkInterval;       // Blink interval (speed).
    util::Duration m_blinkCnt;            // Blink counter.

    bool m_autoShow;            // Autoshow on change flag.
    util::Duration m_autoShowDelay;       // How long bar will stay on-screen in AutoShow mode.
    util::Duration m_autoShowCnt;         // Auto-show counter.
    bool m_autoShowFade;        // Fade flag.
    util::Duration m_autoShowFadeDelay;   // Fade length.
    util::Duration m_autoShowFadeLength;     // Fade progress counter.

    glm::float_t m_rangeUnit;           // Range unit used to set base bar size.
    glm::float_t m_baseSize;            // Base bar size.
    glm::float_t m_baseRatio;           // Max. / actual value ratio.
};

void initBars();
void drawBars();
void showLoadingProgressBar(int value);
void resizeProgressBars();

} // namespace gui
