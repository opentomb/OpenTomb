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
    Effect,       // Effect fader (flashes, etc.)
    Sun,          // Sun fader (engages on looking at the sun)
    Vignette,     // Just for fun - death fader.
    LoadScreen,   // Loading screen
    Black,        // Classic black fader
    Sentinel
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

    glm::vec4 m_topLeftColor;       // All colors are defined separately, for
    glm::vec4 m_topRightColor;      // further possibility of advanced full
    glm::vec4 m_bottomLeftColor;    // screen effects with gradients.
    glm::vec4 m_bottomRightColor;

    loader::BlendingMode m_blendingMode;     // Fader's blending mode.

    glm::float_t m_currentAlpha;          // Current alpha value.
    glm::float_t m_maxAlpha;              // Maximum reachable alpha value.
    util::Duration  m_speed;                 // Fade speed.
    util::Duration  m_speedSecondary;        // Secondary speed - used with TIMED type.

    GLuint          m_texture;               // Texture (optional).
    uint16_t        m_textureWidth;
    uint16_t        m_textureHeight;
    bool            m_textureWide;           // Set, if texture width is greater than height.
    float           m_textureAspectRatio;    // Pre-calculated aspect ratio.
    FaderScale      m_textureScaleMode;      // Fader texture's scale mode.

    bool            m_active;                // Specifies if fader active or not.
    bool            m_complete;              // Specifies if fading is complete or not.
    FaderDir        m_direction;             // Specifies fade direction.

    util::Duration m_currentTime;           // Current fader time.
    util::Duration m_maxTime;               // Maximum delay time.
};

void initFaders();
void drawFaders();
void destroyFaders();
void showLoadScreenFader();

bool fadeStart(FaderType fader, FaderDir fade_direction);
bool fadeStop(FaderType fader);
bool fadeAssignPic(FaderType fader, const std::string &pic_name);
FaderStatus getFaderStatus(FaderType fader);
void fadeSetup(FaderType fader,
               uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, loader::BlendingMode blending_mode,
               util::Duration fadein_speed, util::Duration fadeout_speed);

} // namespace gui
