#pragma once

#include "loader/datatypes.h"
#include "util/helpers.h"

#include <glm/glm.hpp>
#include <GL/glew.h>

#include <cstdint>

namespace gui
{
// These faders always exist in engine, and rarely you will need more than these.
enum class FaderType
{
    Effect,       //!< Effect fader (flashes, etc.)
    Sun,          //!< Sun fader (engages on looking at the sun)
    Vignette,     //!< Just for fun - death fader.
    LoadScreen,   //!< Loading screen
    Black         //!< Classic black fader
};

enum class FaderDir
{
    In,   // Normal fade-in.
    Out,  // Normal fade-out.
    Timed // Timed fade: in -> stay -> out.
};

// Scale type specifies how textures with various aspect ratios will be
// handled. If scale type is set to ZOOM, texture will be zoomed up to
// current screen's aspect ratio. If type is LETTERBOX, empty spaces
// will be filled with bars of fader's color. If type is STRETCH, image
// will be simply stretched across whole screen.
// ZOOM type is the best shot for loading screens, while LETTERBOX is
// needed for pictures with crucial info that shouldn't be cut by zoom,
// and STRETCH type is usable for full-screen effects, like vignette.

enum class FaderScale
{
    Zoom,
    LetterBox,
    Stretch
};

enum class FaderStatus
{
    Invalid,
    Idle,
    Fading,
    Complete
};

enum class FaderCorner
{
    None,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

// Fader is a simple full-screen rectangle, which always sits above the scene,
// and, when activated, either shows or hides gradually - hence, creating illusion
// of fade in and fade out effects.
// TR1-3 had only one type of fader - black one, which was activated on level
// transitions. Since TR4, additional colored fader was introduced to emulate
// various full-screen effects (flashes, flares, and so on).
// With OpenTomb, we extend fader functionality to support not only simple dip to
// color effect, but also various advanced parameters - texture, delay and variable
// fade-in and fade-out speeds.

class Fader
{
public:
    Fader();                  // Fader constructor.

    void show();                  // Shows and updates fader.
    void engage(FaderDir fade_dir);    // Resets and starts fader.
    void cut();                   // Immediately cuts fader.

    FaderStatus getStatus() const;              // Get current state of the fader.

    void setScaleMode(FaderScale mode = FaderScale::Zoom);
    void setColor(uint8_t R, uint8_t G, uint8_t B, FaderCorner corner = FaderCorner::None);
    void setBlendingMode(loader::BlendingMode mode = loader::BlendingMode::Opaque);
    void setAlpha(uint8_t alpha = 255);
    void setSpeed(util::Duration fade_speed, util::Duration fade_speed_secondary = util::MilliSeconds(200));
    void setDelay(util::Duration delay_msec);

    bool setTexture(const std::string &texture_path);

private:
    void setAspect();
    bool dropTexture();

    glm::vec4 m_topLeftColor{0};       // All colors are defined separately, for
    glm::vec4 m_topRightColor{0};      // further possibility of advanced full
    glm::vec4 m_bottomLeftColor{0};    // screen effects with gradients.
    glm::vec4 m_bottomRightColor{0};

    loader::BlendingMode m_blendingMode = loader::BlendingMode::Opaque;     // Fader's blending mode.

    glm::float_t m_currentAlpha;          // Current alpha value.
    glm::float_t m_maxAlpha{1};              // Maximum reachable alpha value.
    util::Duration m_speed = util::MilliSeconds(500);                 // Fade speed.
    util::Duration m_speedSecondary = util::MilliSeconds(200);        // Secondary speed - used with TIMED type.

    GLuint m_texture = 0;               // Texture (optional).
    int m_textureWidth;
    int m_textureHeight;
    bool m_textureWide;           // Set, if texture width is greater than height.
    float m_textureAspectRatio;    // Pre-calculated aspect ratio.
    FaderScale m_textureScaleMode;      // Fader texture's scale mode.

    bool m_active = false;                // Specifies if fader active or not.
    bool m_complete = true;              // Specifies if fading is complete or not.
    FaderDir m_direction = FaderDir::In;             // Specifies fade direction.

    util::Duration m_currentTime;           // Current fader time.
    util::Duration m_maxTime{0};               // Maximum delay time.
};

} // namespace gui
