
#ifndef ENGINE_GUI_H
#define ENGINE_GUI_H


typedef struct gui_text_line_s
{
    char                       *text;
    uint16_t                    buf_size;
    int8_t                      show_rect;
    int8_t                      show;

    GLint                       x;
    GLint                       y;

    GLfloat                     rect_color[4];
    GLfloat                     font_color[4];
    GLfloat                     rect_border;
    GLfloat                     rect[4];                                        //x0, yo, x1, y1

    struct gui_text_line_s     *next;
    struct gui_text_line_s     *prev;
} gui_text_line_t, *gui_text_line_p;

// Immutable bars enumeration.
// These are the bars that are always exist in GUI.
// Scripted bars could be created and drawn separately later.
enum Bars
{
    BAR_HEALTH,     // TR 1-5
    BAR_AIR,        // TR 1-5, alternate state - gas (TR5)
    BAR_SPRINT,     // TR 3-5
    BAR_FREEZE,     // TR 3 only
    BAR_LASTINDEX
};

// Bar color types.
// Each bar part basically has two colours - main and fade.
enum BarColorType
{
    BASE_MAIN,
    BASE_FADE,
    ALT_MAIN,
    ALT_FADE,
    BACK_MAIN,
    BACK_FADE,
    BORDER_MAIN,
    BORDER_FADE
};

// Main bar class.
class ProgressBar
{
public:
    ProgressBar();  // Bar constructor.

    void Show(float value);    // Main show bar procedure.

    void SetColor(BarColorType colType, uint8_t R, uint8_t G, uint8_t B, uint8_t alpha);
    void SetDimensions(float X, float Y, float width, float height, float borderSize);
    void SetValues(float maxValue, float warnValue);
    void SetBlink(int interval);
    void SetExtrude(bool enabled, uint8_t depth);
    void SetAutoshow(bool enabled, int delay, bool fade, int fadeDelay);

    bool          Visible;              // Is it visible or not.
    bool          Alternate;            // Alternate state, in which bar changes color to AltColor.

    bool          Invert;               // Invert decrease direction flag.
    bool          Vertical;             // Change bar style to vertical.

private:

    bool          UpdateResolution      // Try to update bar resolution.
                       (int ScrWidth,
                        int ScrHeight);

    void          RecalculateSize();    // Recalculate size and position.
    void          RecalculatePosition();

    float         mX;                   // Real X position.
    float         mY;                   // Real Y position.
    float         mWidth;               // Real width.
    float         mHeight;              // Real height.
    float         mBorderWidth;         // Real border size (horizontal).
    float         mBorderHeight;        // Real border size (vertical).

    float         mAbsX;                // Absolute (resolution-independent) X position.
    float         mAbsY;                // Absolute Y position.
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

    int8_t        mBaseBlendingMode;    // Blending modes for all bar parts.
    int8_t        mBackBlendingMode;    // Note there is no alt. blending mode, cause
    int8_t        mBorderBlendingMode;  // base and alt are actually the same part.

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

    int           mLastScrWidth;        // Back-up to check resolution change.
    int           mLastScrHeight;
};

void Gui_Init();
void Gui_Destroy();

void Gui_AddLine(gui_text_line_p line);
void Gui_DeleteLine(gui_text_line_p line);
void Gui_RenderStringLine(gui_text_line_p l);
void Gui_RenderStrings();

/*
 * Calculates rect coordinates around the text
 */
gui_text_line_p Gui_StringAutoRect(gui_text_line_p l);
/**
 * Draws text using a current console font.
 */
gui_text_line_p Gui_OutTextXY(int x, int y, const char *fmt, ...);

/**
 * Helper method to setup OpenGL state for console drawing.
 *
 * Either changes to 2D matrix state (is_gui = 1) or away from it (is_gui = 0). Does not do any drawing.
 */
void Gui_SwitchConGLMode(char is_gui);

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
void Gui_Render();
/**
*  Draw simple rectangle.
*  Only state it changes is the blend mode, according to blendMode value.
*/

void Gui_DrawRect(const GLfloat &x, const GLfloat &y,
                  const GLfloat &width, const GLfloat &height,
                  const GLfloat colorUpperLeft[], const GLfloat colorUpperRight[],
                  const GLfloat colorLowerLeft[], const GLfloat colorLowerRight[],
                  const int &blendMode);
void Gui_DrawCrosshair();
void Gui_DrawBars();
#endif
