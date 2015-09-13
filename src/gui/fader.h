#pragma once

#include <cstdint>

#include <GL/glew.h>

#include "loader/datatypes.h"

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

    void Show();                  // Shows and updates fader.
    void Engage(FaderDir fade_dir);    // Resets and starts fader.
    void Cut();                   // Immediately cuts fader.

    FaderStatus getStatus();              // Get current state of the fader.

    void SetScaleMode(FaderScale mode = FaderScale::Zoom);
    void SetColor(uint8_t R, uint8_t G, uint8_t B, FaderCorner corner = FaderCorner::None);
    void SetBlendingMode(loader::BlendingMode mode = loader::BlendingMode::Opaque);
    void SetAlpha(uint8_t alpha = 255);
    void SetSpeed(uint16_t fade_speed, uint16_t fade_speed_secondary = 200);
    void SetDelay(uint32_t delay_msec);

    bool SetTexture(const char* texture_path);

private:
    void SetAspect();
    bool DropTexture();

    GLfloat         mTopLeftColor[4];       // All colors are defined separately, for
    GLfloat         mTopRightColor[4];      // further possibility of advanced full
    GLfloat         mBottomLeftColor[4];    // screen effects with gradients.
    GLfloat         mBottomRightColor[4];

    loader::BlendingMode mBlendingMode;     // Fader's blending mode.

    GLfloat         mCurrentAlpha;          // Current alpha value.
    GLfloat         mMaxAlpha;              // Maximum reachable alpha value.
    GLfloat         mSpeed;                 // Fade speed.
    GLfloat         mSpeedSecondary;        // Secondary speed - used with TIMED type.

    GLuint          mTexture;               // Texture (optional).
    uint16_t        mTextureWidth;
    uint16_t        mTextureHeight;
    bool            mTextureWide;           // Set, if texture width is greater than height.
    float           mTextureAspectRatio;    // Pre-calculated aspect ratio.
    FaderScale      mTextureScaleMode;      // Fader texture's scale mode.

    bool            mActive;                // Specifies if fader active or not.
    bool            mComplete;              // Specifies if fading is complete or not.
    FaderDir        mDirection;             // Specifies fade direction.

    float           mCurrentTime;           // Current fader time.
    float           mMaxTime;               // Maximum delay time.
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
               uint16_t fadein_speed, uint16_t fadeout_speed);

} // namespace gui
