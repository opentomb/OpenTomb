#pragma once

#include "fontmanager.h"
#include "gl_font.h"
#include "render/render.h"

namespace gui
{

namespace
{
constexpr int MaxTempLines = 256;

// Screen metering resolution specifies user-friendly relative dimensions of screen,
// which are not dependent on screen resolution. They're primarily used to parse
// bar and string dimensions.

constexpr float ScreenMeteringResolution = 1000.0f;

constexpr int MaxFonts = 8;     // 8 fonts is PLENTY.

constexpr int MinFontSize = 1;
constexpr int MaxFontSize = 72;

constexpr float FontFadeSpeed = 1.0f;                 // Global fading style speed.
constexpr float FontFadeMin   = 0.3f;                 // Minimum fade multiplier.

constexpr float FontShadowTransparency    =  0.7f;
constexpr float FontShadowVerticalShift   = -0.9f;
constexpr float FontShadowHorizontalShift =  0.7f;

// Default line size is generally used for static in-game strings. Strings
// that are created dynamically may have variable string sizes.

constexpr int LineDefaultSize = 128;
} // anonymous namespace

// Anchoring is needed to link specific GUI element to specific screen position,
// independent of screen resolution and aspect ratio. Vertical and horizontal
// anchorings are seperated, so you can link element at any place - top, bottom,
// center, left or right.
enum class VerticalAnchor
{
    Top,
    Bottom,
    Center
};

enum class HorizontalAnchor
{
    Left,
    Right,
    Center
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

namespace
{
// Offscreen divider specifies how far item notifier will be placed from
// the final slide position. Usually it's enough to be 1/8 of the screen
// width, but if you want to increase or decrease notifier size, you must
// change this value properly.

constexpr float NotifierOffscreenDivider = 8.0f;

// Notifier show time is a time notifier stays on screen (excluding slide
// effect). Maybe it's better to move it to script later.

constexpr float NotifierShowtime = 2.0f;
} // anonymous namespace

void initFontManager();

void init();
void destroy();

extern std::unique_ptr<FontManager> fontManager;

/**
 * Helper method to setup OpenGL state for console drawing.
 *
 * Either changes to 2D matrix state (is_gui = 1) or away from it (is_gui = 0). Does not do any drawing.
 */
void switchGLMode(bool is_gui);

/**
 * Draws wireframe of this frustum.
 *
 * Expected state:
 *  - Vertex array is enabled, color, tex coord, normal disabled
 *  - No vertex buffer object is bound
 *  - Texturing is disabled
 *  - Alpha test is disabled
 *  - Blending is enabled
 *  - Lighting is disabled
 * Ignored state:
 *  - Currently bound texture.
 *  - Currently bound element buffer.
 *  - Depth test enabled (disables it, then restores)
 *  - Vertex pointer (changes it)
 *  - Matrices (changes them, restores)
 *  - Line width (changes it, then restores)
 *  - Current color (changes it)
 * Changed state:
 *  - Current position will be arbitrary.
 *  - Vertex pointer will be arbitray.
 *  - Current color will be arbitray (set by console)
 *  - Blend mode will be SRC_ALPHA, ONE_MINUS_SRC_ALPHA (set by console)
 */
void render();

/**
 *  Draw simple rectangle.
 *  Only state it changes is the blend mode, according to blendMode value.
 */
void drawRect(const GLfloat &x, const GLfloat &y,
              const GLfloat &width, const GLfloat &height,
              const GLfloat colorUpperLeft[], const GLfloat colorUpperRight[],
              const GLfloat colorLowerLeft[], const GLfloat colorLowerRight[],
              const loader::BlendingMode blendMode,
              const GLuint texture = 0);

/**
 * General GUI drawing routines.
 */
void drawLoadScreen(int value);
void drawInventory();

/**
 * General GUI update routines.
 */
bool update();
void resize();  // Called every resize event.

extern glm::mat4 guiProjectionMatrix;

} // namespace gui
