#include "progressbar.h"

#include "character_controller.h"
#include "engine/system.h"
#include "world/character.h"

#include <boost/range/adaptors.hpp>

namespace gui
{
namespace
{
std::map<BarType, ProgressBar> g_bar;
}

ProgressBar::ProgressBar()
{
    // Set up some defaults.
    m_visible = false;
    m_alternate = false;
    m_invert = false;
    m_vertical = false;
    m_forced = false;

    // Initialize parameters.
    // By default, bar is initialized with TR5-like health bar properties.
    setPosition(HorizontalAnchor::Left, 20, VerticalAnchor::Top, 20);
    setSize(250, 25, 3);
    setColor(BarColorType::BaseMain, 255, 50, 50, 150);
    setColor(BarColorType::BaseFade, 100, 255, 50, 150);
    setColor(BarColorType::AltMain, 255, 180, 0, 220);
    setColor(BarColorType::AltFade, 255, 255, 0, 220);
    setColor(BarColorType::BackMain, 0, 0, 0, 160);
    setColor(BarColorType::BackFade, 60, 60, 60, 130);
    setColor(BarColorType::BorderMain, 200, 200, 200, 50);
    setColor(BarColorType::BorderFade, 80, 80, 80, 100);
    setValues(1000, 300);
    setBlink(util::MilliSeconds(300));
    setExtrude(true, 100);
    setAutoshow(true, util::MilliSeconds(5000), true, util::MilliSeconds(1000));
}

// Resize bar.
// This function should be called every time resize event occurs.

void ProgressBar::resize()
{
    recalculateSize();
    recalculatePosition();
}

// Set specified color.
void ProgressBar::setColor(BarColorType colType,
                               uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    glm::float_t maxColValue = 255.0;

    switch(colType)
    {
        case BarColorType::BaseMain:
            m_baseMainColor[0] = static_cast<float>(R) / maxColValue;
            m_baseMainColor[1] = static_cast<float>(G) / maxColValue;
            m_baseMainColor[2] = static_cast<float>(B) / maxColValue;
            m_baseMainColor[3] = static_cast<float>(A) / maxColValue;
            m_baseMainColorAlpha = m_baseMainColor[3];
            break;
        case BarColorType::BaseFade:
            m_baseFadeColor[0] = static_cast<float>(R) / maxColValue;
            m_baseFadeColor[1] = static_cast<float>(G) / maxColValue;
            m_baseFadeColor[2] = static_cast<float>(B) / maxColValue;
            m_baseFadeColor[3] = static_cast<float>(A) / maxColValue;
            m_baseFadeColorAlpha = m_baseFadeColor[3];
            break;
        case BarColorType::AltMain:
            m_altMainColor[0] = static_cast<float>(R) / maxColValue;
            m_altMainColor[1] = static_cast<float>(G) / maxColValue;
            m_altMainColor[2] = static_cast<float>(B) / maxColValue;
            m_altMainColor[3] = static_cast<float>(A) / maxColValue;
            m_altMainColorAlpha = m_altMainColor[3];
            break;
        case BarColorType::AltFade:
            m_altFadeColor[0] = static_cast<float>(R) / maxColValue;
            m_altFadeColor[1] = static_cast<float>(G) / maxColValue;
            m_altFadeColor[2] = static_cast<float>(B) / maxColValue;
            m_altFadeColor[3] = static_cast<float>(A) / maxColValue;
            m_altFadeColorAlpha = m_altFadeColor[3];
            break;
        case BarColorType::BackMain:
            m_backMainColor[0] = static_cast<float>(R) / maxColValue;
            m_backMainColor[1] = static_cast<float>(G) / maxColValue;
            m_backMainColor[2] = static_cast<float>(B) / maxColValue;
            m_backMainColor[3] = static_cast<float>(A) / maxColValue;
            m_backMainColorAlpha = m_backMainColor[3];
            break;
        case BarColorType::BackFade:
            m_backFadeColor[0] = static_cast<float>(R) / maxColValue;
            m_backFadeColor[1] = static_cast<float>(G) / maxColValue;
            m_backFadeColor[2] = static_cast<float>(B) / maxColValue;
            m_backFadeColor[3] = static_cast<float>(A) / maxColValue;
            m_backFadeColorAlpha = m_backFadeColor[3];
            break;
        case BarColorType::BorderMain:
            m_borderMainColor[0] = static_cast<float>(R) / maxColValue;
            m_borderMainColor[1] = static_cast<float>(G) / maxColValue;
            m_borderMainColor[2] = static_cast<float>(B) / maxColValue;
            m_borderMainColor[3] = static_cast<float>(A) / maxColValue;
            m_borderMainColorAlpha = m_borderMainColor[3];
            break;
        case BarColorType::BorderFade:
            m_borderFadeColor[0] = static_cast<float>(R) / maxColValue;
            m_borderFadeColor[1] = static_cast<float>(G) / maxColValue;
            m_borderFadeColor[2] = static_cast<float>(B) / maxColValue;
            m_borderFadeColor[3] = static_cast<float>(A) / maxColValue;
            m_borderFadeColorAlpha = m_borderFadeColor[3];
            break;
        default:
            break;
    }
}

void ProgressBar::setPosition(HorizontalAnchor anchor_X, glm::float_t offset_X, VerticalAnchor anchor_Y, glm::float_t offset_Y)
{
    m_xAnchor = anchor_X;
    m_yAnchor = anchor_Y;
    m_absXoffset = offset_X;
    m_absYoffset = offset_Y;

    recalculatePosition();
}

// Set bar size
void ProgressBar::setSize(glm::float_t width, glm::float_t height, glm::float_t borderSize)
{
    // Absolute values are needed to recalculate actual bar size according to resolution.
    m_absWidth = width;
    m_absHeight = height;
    m_absBorderSize = borderSize;

    recalculateSize();
}

// Recalculate size, according to viewport resolution.
void ProgressBar::recalculateSize()
{
    m_width = static_cast<float>(m_absWidth)  * engine::screen_info.scale_factor;
    m_height = static_cast<float>(m_absHeight) * engine::screen_info.scale_factor;

    m_borderWidth = static_cast<float>(m_absBorderSize)  * engine::screen_info.scale_factor;
    m_borderHeight = static_cast<float>(m_absBorderSize)  * engine::screen_info.scale_factor;

    // Calculate range unit, according to maximum bar value set up.
    // If bar alignment is set to horizontal, calculate it from bar width.
    // If bar is vertical, then calculate it from height.

    m_rangeUnit = !m_vertical ? m_width / m_maxValue : m_height / m_maxValue;
}

// Recalculate position, according to viewport resolution.
void ProgressBar::recalculatePosition()
{
    switch(m_xAnchor)
    {
        case HorizontalAnchor::Left:
            m_x = static_cast<float>(m_absXoffset + m_absBorderSize) * engine::screen_info.scale_factor;
            break;
        case HorizontalAnchor::Center:
            m_x = (static_cast<float>(engine::screen_info.w) - static_cast<float>(m_absWidth + m_absBorderSize * 2) * engine::screen_info.scale_factor) / 2 +
                static_cast<float>(m_absXoffset) * engine::screen_info.scale_factor;
            break;
        case HorizontalAnchor::Right:
            m_x = static_cast<float>(engine::screen_info.w) - static_cast<float>(m_absXoffset + m_absWidth + m_absBorderSize * 2) * engine::screen_info.scale_factor;
            break;
    }

    switch(m_yAnchor)
    {
        case VerticalAnchor::Top:
            m_y = static_cast<float>(engine::screen_info.h) - static_cast<float>(m_absYoffset + m_absHeight + m_absBorderSize * 2) * engine::screen_info.scale_factor;
            break;
        case VerticalAnchor::Center:
            m_y = (static_cast<float>(engine::screen_info.h) - static_cast<float>(m_absHeight + m_absBorderSize * 2) * engine::screen_info.h_unit) / 2 +
                static_cast<float>(m_absYoffset) * engine::screen_info.scale_factor;
            break;
        case VerticalAnchor::Bottom:
            m_y = (m_absYoffset + m_absBorderSize) * engine::screen_info.scale_factor;
            break;
    }
}

// Set maximum and warning state values.
void ProgressBar::setValues(glm::float_t maxValue, glm::float_t warnValue)
{
    m_maxValue = maxValue;
    m_warnValue = warnValue;

    recalculateSize();  // We need to recalculate size, because max. value is changed.
}

// Set warning state blinking interval.
void ProgressBar::setBlink(util::Duration interval)
{
    m_blinkInterval = interval;
    m_blinkCnt = interval;  // Also reset blink counter.
}

// Set extrude overlay effect parameters.
void ProgressBar::setExtrude(bool enabled, uint8_t depth)
{
    m_extrude = enabled;
    m_extrudeDepth = glm::vec4(0.0f);    // Set all colors to 0.
    m_extrudeDepth[3] = static_cast<glm::float_t>(depth) / 255.0f;        // We need only alpha transparency.
    m_extrudeDepthAlpha = m_extrudeDepth[3];
}

// Set autoshow and fade parameters.
// Please note that fade parameters are actually independent of autoshow.
void ProgressBar::setAutoshow(bool enabled, util::Duration delay, bool fade, util::Duration fadeDelay)
{
    m_autoShow = enabled;

    m_autoShowDelay = delay;
    m_autoShowCnt = delay;     // Also reset autoshow counter.

    m_autoShowFade = fade;
    m_autoShowFadeDelay = fadeDelay;
    m_autoShowFadeLength = util::Duration(0); // Initially, it's 0.
}

// Main bar show procedure.
// Draws a bar with a given value. Please note that it also accepts float,
// so effectively you can create bars for floating-point parameters.
void ProgressBar::show(glm::float_t value)
{
    // Initial value limiters (to prevent bar overflow).
    value = glm::clamp(value, 0.0f, m_maxValue);

    // Enable blink mode, if value is gone below warning value.
    m_blink = value <= m_warnValue;

    if(m_autoShow)   // Check autoshow visibility conditions.
    {
        // 0. If bar drawing was forced, then show a bar without additional
        //    autoshow delay set. This condition has to be overwritten by
        //    any other conditions, that's why it is set first.
        if(m_forced)
        {
            m_visible = true;
            m_forced = false;
        }
        else
        {
            m_visible = false;
        }

        // 1. If bar value gone less than warning value, we show it
        //    in any case, bypassing all other conditions.
        if(value <= m_warnValue)
            m_visible = true;

        // 2. Check if bar's value changed,
        //    and if so, start showing it automatically for a given delay time.
        if(m_lastValue != value)
        {
            m_lastValue = value;
            m_visible = true;
            m_autoShowCnt = m_autoShowDelay;
        }

        // 3. If autoshow time is up, then we hide bar,
        //    otherwise decrease delay counter.
        if(m_autoShowCnt.count() > 0)
        {
            m_visible = true;
            m_autoShowCnt -= engine::engine_frame_time;

            if(m_autoShowCnt.count() <= 0)
            {
                m_autoShowCnt = util::Duration(0);
                m_visible = false;
            }
        }
    } // end if(AutoShow)

    if(m_autoShowFade)   // Process fade-in and fade-out effect, if enabled.
    {
        if(!m_visible)
        {
            // If visibility flag is off and bar is still on-screen, gradually decrease
            // fade counter, else simply don't draw anything and exit.
            if(m_autoShowFadeLength.count() == 0)
            {
                return;
            }
            else
            {
                m_autoShowFadeLength -= engine::engine_frame_time;
                if(m_autoShowFadeLength.count() < 0)
                    m_autoShowFadeLength = util::Duration(0);
            }
        }
        else
        {
            // If visibility flag is on, and bar is not yet fully visible, gradually
            // increase fade counter, until it's 1 (i. e. fully opaque).
            if(m_autoShowFadeLength < m_autoShowFadeDelay)
            {
                m_autoShowFadeLength += engine::engine_frame_time;
                if(m_autoShowFadeLength > m_autoShowFadeDelay)
                    m_autoShowFadeLength = m_autoShowFadeDelay;
            }
        } // end if(!Visible)

        // Multiply all layers' alpha by current fade counter.
        m_baseMainColor[3] = m_baseMainColorAlpha * m_autoShowFadeLength / m_autoShowFadeDelay;
        m_baseFadeColor[3] = m_baseFadeColorAlpha * m_autoShowFadeLength / m_autoShowFadeDelay;
        m_altMainColor[3] = m_altMainColorAlpha * m_autoShowFadeLength / m_autoShowFadeDelay;
        m_altFadeColor[3] = m_altFadeColorAlpha * m_autoShowFadeLength / m_autoShowFadeDelay;
        m_backMainColor[3] = m_backMainColorAlpha * m_autoShowFadeLength / m_autoShowFadeDelay;
        m_backFadeColor[3] = m_backFadeColorAlpha * m_autoShowFadeLength / m_autoShowFadeDelay;
        m_borderMainColor[3] = m_borderMainColorAlpha * m_autoShowFadeLength / m_autoShowFadeDelay;
        m_borderFadeColor[3] = m_borderFadeColorAlpha * m_autoShowFadeLength / m_autoShowFadeDelay;
        m_extrudeDepth[3] = m_extrudeDepthAlpha * m_autoShowFadeLength / m_autoShowFadeDelay;
    }
    else
    {
        if(!m_visible)
            return;   // Obviously, quit, if bar is not visible.
    } // end if(mAutoShowFade)

    // Draw border rect.
    // Border rect should be rendered first, as it lies beneath actual bar,
    // and additionally, we need to show it in any case, even if bar is in
    // warning state (blinking).
    drawRect(m_x, m_y, m_width + m_borderWidth * 2, m_height + m_borderHeight * 2,
                 m_borderMainColor, m_borderMainColor,
                 m_borderFadeColor, m_borderFadeColor,
                 loader::BlendingMode::Opaque);

    // SECTION FOR BASE BAR RECTANGLE.

    // We check if bar is in a warning state. If it is, we blink it continously.
    if(m_blink)
    {
        m_blinkCnt -= engine::engine_frame_time;
        if(m_blinkCnt > m_blinkInterval)
        {
            value = 0; // Force zero value, which results in empty bar.
        }
        else if(m_blinkCnt.count() <= 0)
        {
            m_blinkCnt = m_blinkInterval * 2;
        }
    }

    // If bar value is zero, just render background overlay and immediately exit.
    // It is needed in case bar is used as a simple UI box to bypass unnecessary calculations.
    if(!value)
    {
        // Draw full-sized background rect (instead of base bar rect)
        drawRect(m_x + m_borderWidth, m_y + m_borderHeight, m_width, m_height,
                     m_backMainColor, m_vertical ? m_backFadeColor : m_backMainColor,
                     m_vertical ? m_backMainColor : m_backFadeColor, m_backFadeColor,
                     loader::BlendingMode::Opaque);
        return;
    }

    // Calculate base bar width, according to current value and range unit.
    m_baseSize = m_rangeUnit * value;
    m_baseRatio = value / m_maxValue;

    glm::float_t RectAnchor;           // Anchor to stick base bar rect, according to Invert flag.
    glm::vec4 RectFirstColor;    // Used to recalculate gradient, according to current value.
    glm::vec4 RectSecondColor;

    // If invert decrease direction style flag is set, we position bar in a way
    // that it seems like it's decreasing to another side, and also swap main / fade colours.
    if(m_invert)
    {
        RectFirstColor = m_alternate ? m_altMainColor : m_baseMainColor;

        // Main-fade gradient is recalculated according to current / maximum value ratio.
        RectSecondColor = m_alternate
                        ? m_baseRatio * m_altFadeColor + (1 - m_baseRatio) * m_altMainColor
                        : m_baseRatio * m_baseFadeColor + (1 - m_baseRatio) * m_baseMainColor;
    }
    else
    {
        RectSecondColor = m_alternate ? m_altMainColor : m_baseMainColor;

        // Main-fade gradient is recalculated according to current / maximum value ratio.
        RectFirstColor = m_alternate
                       ? m_baseRatio * m_altFadeColor + (1 - m_baseRatio) * m_altMainColor
                       : m_baseRatio * m_baseFadeColor + (1 - m_baseRatio) * m_baseMainColor;
    } // end if(Invert)

    // We need to reset Alternate flag each frame, cause behaviour is immediate.

    m_alternate = false;

    // If vertical style flag is set, we draw bar base top-bottom, else we draw it left-right.
    if(m_vertical)
    {
        RectAnchor = (m_invert ? m_y + m_height - m_baseSize : m_y) + m_borderHeight;

        // Draw actual bar base.
        drawRect(m_x + m_borderWidth, RectAnchor,
                     m_width, m_baseSize,
                     RectFirstColor, RectFirstColor,
                     RectSecondColor, RectSecondColor,
                     loader::BlendingMode::Opaque);

        // Draw background rect.
        drawRect(m_x + m_borderWidth,
                     m_invert ? m_y + m_borderHeight : RectAnchor + m_baseSize,
                     m_width, m_height - m_baseSize,
                     m_backMainColor, m_backFadeColor,
                     m_backMainColor, m_backFadeColor,
                     loader::BlendingMode::Opaque);

        if(m_extrude)    // Draw extrude overlay, if flag is set.
        {
            glm::vec4 transparentColor{ 0.0f };  // Used to set counter-shade to transparent.

            drawRect(m_x + m_borderWidth, RectAnchor,
                         m_width / 2, m_baseSize,
                         m_extrudeDepth, transparentColor,
                         m_extrudeDepth, transparentColor,
                         loader::BlendingMode::Opaque);
            drawRect(m_x + m_borderWidth + m_width / 2, RectAnchor,
                         m_width / 2, m_baseSize,
                         transparentColor, m_extrudeDepth,
                         transparentColor, m_extrudeDepth,
                         loader::BlendingMode::Opaque);
        }
    }
    else
    {
        RectAnchor = (m_invert ? m_x + m_width - m_baseSize : m_x) + m_borderWidth;

        // Draw actual bar base.
        drawRect(RectAnchor, m_y + m_borderHeight,
                     m_baseSize, m_height,
                     RectSecondColor, RectFirstColor,
                     RectSecondColor, RectFirstColor,
                     loader::BlendingMode::Opaque);

        // Draw background rect.
        drawRect(m_invert ? m_x + m_borderWidth : RectAnchor + m_baseSize,
                     m_y + m_borderHeight,
                     m_width - m_baseSize, m_height,
                     m_backMainColor, m_backMainColor,
                     m_backFadeColor, m_backFadeColor,
                     loader::BlendingMode::Opaque);

        if(m_extrude)    // Draw extrude overlay, if flag is set.
        {
            glm::vec4 transparentColor{ 0.0f };  // Used to set counter-shade to transparent.

            drawRect(RectAnchor, m_y + m_borderHeight,
                         m_baseSize, m_height / 2,
                         transparentColor, transparentColor,
                         m_extrudeDepth, m_extrudeDepth,
                         loader::BlendingMode::Opaque);
            drawRect(RectAnchor, m_y + m_borderHeight + m_height / 2,
                         m_baseSize, m_height / 2,
                         m_extrudeDepth, m_extrudeDepth,
                         transparentColor, transparentColor,
                         loader::BlendingMode::Opaque);
        }
    } // end if(Vertical)
}

void initBars()
{
    {
        ProgressBar& bar = g_bar[BarType::Health];
        bar.m_visible = false;
        bar.m_alternate = false;
        bar.m_invert = false;
        bar.m_vertical = false;

        bar.setSize(250, 15, 3);
        bar.setPosition(HorizontalAnchor::Left, 30, VerticalAnchor::Top, 30);
        bar.setColor(BarColorType::BaseMain, 255, 50, 50, 200);
        bar.setColor(BarColorType::BaseFade, 100, 255, 50, 200);
        bar.setColor(BarColorType::AltMain, 255, 180, 0, 255);
        bar.setColor(BarColorType::AltFade, 255, 255, 0, 255);
        bar.setColor(BarColorType::BackMain, 0, 0, 0, 160);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 130);
        bar.setColor(BarColorType::BorderMain, 200, 200, 200, 50);
        bar.setColor(BarColorType::BorderFade, 80, 80, 80, 100);
        bar.setValues(LARA_PARAM_HEALTH_MAX, LARA_PARAM_HEALTH_MAX / 3);
        bar.setBlink(util::MilliSeconds(300));
        bar.setExtrude(true, 100);
        bar.setAutoshow(true, util::MilliSeconds(2000), true, util::MilliSeconds(400));
    }
    {
        ProgressBar& bar = g_bar[BarType::Air];
        bar.m_visible = false;
        bar.m_alternate = false;
        bar.m_invert = true;
        bar.m_vertical = false;

        bar.setSize(250, 15, 3);
        bar.setPosition(HorizontalAnchor::Right, 30, VerticalAnchor::Top, 30);
        bar.setColor(BarColorType::BaseMain, 0, 50, 255, 200);
        bar.setColor(BarColorType::BaseFade, 190, 190, 255, 200);
        bar.setColor(BarColorType::BackMain, 0, 0, 0, 160);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 130);
        bar.setColor(BarColorType::BorderMain, 200, 200, 200, 50);
        bar.setColor(BarColorType::BorderFade, 80, 80, 80, 100);
        bar.setValues(LARA_PARAM_AIR_MAX, LARA_PARAM_AIR_MAX / 3);
        bar.setBlink(util::MilliSeconds(300));
        bar.setExtrude(true, 100);
        bar.setAutoshow(true, util::MilliSeconds(2000), true, util::MilliSeconds(400));
    }
    {
        ProgressBar& bar = g_bar[BarType::Stamina];
        bar.m_visible = false;
        bar.m_alternate = false;
        bar.m_invert = false;
        bar.m_vertical = false;

        bar.setSize(250, 15, 3);
        bar.setPosition(HorizontalAnchor::Left, 30, VerticalAnchor::Top, 55);
        bar.setColor(BarColorType::BaseMain, 255, 100, 50, 200);
        bar.setColor(BarColorType::BaseFade, 255, 200, 0, 200);
        bar.setColor(BarColorType::BackMain, 0, 0, 0, 160);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 130);
        bar.setColor(BarColorType::BorderMain, 110, 110, 110, 100);
        bar.setColor(BarColorType::BorderFade, 60, 60, 60, 180);
        bar.setValues(LARA_PARAM_STAMINA_MAX, 0);
        bar.setExtrude(true, 100);
        bar.setAutoshow(true, util::MilliSeconds(500), true, util::MilliSeconds(300));
    }
    {
        ProgressBar& bar = g_bar[BarType::Warmth];
        bar.m_visible = false;
        bar.m_alternate = false;
        bar.m_invert = true;
        bar.m_vertical = false;

        bar.setSize(250, 15, 3);
        bar.setPosition(HorizontalAnchor::Right, 30, VerticalAnchor::Top, 55);
        bar.setColor(BarColorType::BaseMain, 255, 0, 255, 255);
        bar.setColor(BarColorType::BaseFade, 190, 120, 255, 255);
        bar.setColor(BarColorType::BackMain, 0, 0, 0, 160);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 130);
        bar.setColor(BarColorType::BorderMain, 200, 200, 200, 50);
        bar.setColor(BarColorType::BorderFade, 80, 80, 80, 100);
        bar.setValues(LARA_PARAM_WARMTH_MAX, LARA_PARAM_WARMTH_MAX / 3);
        bar.setBlink(util::MilliSeconds(200));
        bar.setExtrude(true, 60);
        bar.setAutoshow(true, util::MilliSeconds(500), true, util::MilliSeconds(300));
    }
    {
        ProgressBar& bar = g_bar[BarType::Loading];
        bar.m_visible = true;
        bar.m_alternate = false;
        bar.m_invert = false;
        bar.m_vertical = false;

        bar.setSize(800, 25, 3);
        bar.setPosition(HorizontalAnchor::Center, 0, VerticalAnchor::Bottom, 40);
        bar.setColor(BarColorType::BaseMain, 255, 225, 127, 230);
        bar.setColor(BarColorType::BaseFade, 255, 187, 136, 230);
        bar.setColor(BarColorType::BackMain, 30, 30, 30, 100);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 100);
        bar.setColor(BarColorType::BorderMain, 200, 200, 200, 80);
        bar.setColor(BarColorType::BorderFade, 80, 80, 80, 80);
        bar.setValues(1000, 0);
        bar.setExtrude(true, 70);
        bar.setAutoshow(false, util::MilliSeconds(500), false, util::MilliSeconds(300));
    }
}

void drawBars()
{
    if(engine::engine_world.character)
    {
        if(engine::engine_world.character->m_currentWeaponState > world::WeaponState::HideToReady)
            g_bar[BarType::Health].m_forced = true;

        if(engine::engine_world.character->getParam(world::PARAM_POISON) > 0.0)
            g_bar[BarType::Health].m_alternate = true;

        g_bar[BarType::Air].show(engine::engine_world.character->getParam(world::PARAM_AIR));
        g_bar[BarType::Stamina].show(engine::engine_world.character->getParam(world::PARAM_STAMINA));
        g_bar[BarType::Health].show(engine::engine_world.character->getParam(world::PARAM_HEALTH));
        g_bar[BarType::Warmth].show(engine::engine_world.character->getParam(world::PARAM_WARMTH));
    }
}

void showLoadingProgressBar(int value)
{
    g_bar[BarType::Loading].show(value);
}

void resizeProgressBars()
{
    for(ProgressBar& bar : g_bar | boost::adaptors::map_values)
    {
        bar.resize();
    }
}

} // namespace gui
