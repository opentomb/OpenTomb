#pragma once

#include "character_controller.h"
#include "world/entity.h"
#include "gl_font.h"
#include "render/render.h"

#include <list>

struct InventoryNode;

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

// Horizontal alignment is simple side alignment, like in original TRs.
// It means that X coordinate will be either used for left, right or
// center orientation.
enum class LineAlignment
{
    Left,
    Right,
    Center
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

enum class MenuItemType
{
    System,
    Supply,
    Quest,
    Invalid
};

inline MenuItemType nextItemType(MenuItemType t)
{
    switch(t)
    {
        case MenuItemType::System: return MenuItemType::Supply;
        case MenuItemType::Supply: return MenuItemType::Quest;
        default: return MenuItemType::Invalid;
    }
}

inline MenuItemType previousItemType(MenuItemType t)
{
    switch(t)
    {
        case MenuItemType::Supply: return MenuItemType::System;
        case MenuItemType::Quest: return MenuItemType::Supply;
        default: return MenuItemType::Invalid;
    }
}


// OpenTomb has three types of fonts - primary, secondary and console
// font. This should be enough for most of the cases. However, user
// can generate and use additional font types via script, but engine
// behaviour with extra font types is undefined.

enum class FontType
{
    Primary,
    Secondary,
    Console
};

// This is predefined enumeration of font styles, which can be extended
// with user-defined script functions.
///@TODO: add system message console style
enum class FontStyle
{
    ConsoleInfo,
    ConsoleWarning,
    ConsoleEvent,
    ConsoleNotify,
    MenuTitle,
    MenuHeading1,
    MenuHeading2,
    MenuItemActive,
    MenuItemInactive,
    MenuContent,
    StatsTitle,
    StatsContent,
    Notifier,
    SavegameList,
    Generic,
    Sentinel
};


// Font struct contains additional field for font type which is
// used to dynamically create or delete fonts.

struct Font
{
    FontType                   index;
    uint16_t                    size;
    std::shared_ptr<FontTexture> gl_font;
};

// Font style is different to font itself - whereas engine can have
// only three fonts, there could be unlimited amount of font styles.
// Font style management is done via font manager.

struct FontStyleData
{
    FontStyle                  index;          // Unique index which is used to identify style.

    GLfloat                     color[4];
    GLfloat                     real_color[4];
    GLfloat                     rect_color[4];
    GLfloat                     rect_border;

    bool                        shadowed;
    bool                        rect;
    bool                        fading;         // TR4-like looped fading font effect.
    bool                        hidden;         // Used to bypass certain GUI lines easily.
};

// Font manager is a singleton class which is used to manage all in-game fonts
// and font styles. Every time you want to change font or style, font manager
// functions should be used.

class FontManager
{
public:
    FontManager();
    ~FontManager();

    bool             AddFont(const FontType index,
                             const uint32_t size,
                             const char* path);
    bool             RemoveFont(const FontType index);
    FontTexture*     GetFont(const FontType index);

    bool             AddFontStyle(const FontStyle index,
                                  const GLfloat R, const GLfloat G, const GLfloat B, const GLfloat A,
                                  const bool shadow, const bool fading,
                                  const bool rect, const GLfloat rect_border,
                                  const GLfloat rect_R, const GLfloat rect_G, const GLfloat rect_B, const GLfloat rect_A,
                                  const bool hide);
    bool             RemoveFontStyle(const FontStyle index);
    FontStyleData*  GetFontStyle(const FontStyle index);

    uint32_t         GetFontCount()
    {
        return fonts.size();
    }
    uint32_t         GetFontStyleCount()
    {
        return styles.size();
    }

    void             Update(); // Do fading routine here, etc. Put into Gui_Update, maybe...
    void             Resize(); // Resize fonts on window resize event.

private:
    Font*            GetFontAddress(const FontType index);

    GLfloat          mFadeValue; // Multiplier used with font RGB values to animate fade.
    bool             mFadeDirection;

    std::list<FontStyleData> styles;

    std::list<Font>  fonts;

    FT_Library       font_library;  // GLF font library unit.
};

struct TextLine
{
    std::string                 text;

    FontType                    font_id;
    FontStyle                   style_id;

    GLfloat                     X;
    HorizontalAnchor            Xanchor;
    GLfloat                     absXoffset;
    GLfloat                     Y;
    VerticalAnchor              Yanchor;
    GLfloat                     absYoffset;

    GLfloat                     rect[4];    //x0, yo, x1, y1

    bool                        show;

    TextLine     *next;
    TextLine     *prev;
};

struct Rect
{
    GLfloat                     rect[4];
    GLfloat                     absRect[4];

    GLfloat                     X;  GLfloat absX;
    GLfloat                     Y;  GLfloat absY;
    int8_t                      align;

    GLuint                      texture;
    GLfloat                     color[16]; // TL, TR, BL, BR x 4
    uint32_t                    blending_mode;

    int16_t                     line_count;
    TextLine            *lines;

    int8_t                      state;      // Opening / static / closing
    int8_t                      show;
    GLfloat                     current_alpha;

    int8_t                      focused;
    int8_t                      focus_index;

    int8_t                      selectable;
    int8_t                      selection_index;

    char                       *lua_click_function;
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

// Immutable fader enumeration.
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

// Main fader class.

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

// Immutable bars enumeration.
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

// Main bar class.

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

void initFontManager();

void init();
void destroy();

void initBars();
void initFaders();
void initNotifier();
void initTempLines();
void fillCrosshairBuffer();

void addLine(TextLine* line);
void deleteLine(TextLine* line);
void moveLine(TextLine* line);
void renderStringLine(TextLine* l);
void renderStrings();

/**
 * Inventory rendering / manipulation functions
 */
void Item_Frame(world::animation::SSBoneFrame *bf, btScalar time);
void renderItem(world::animation::SSBoneFrame *bf, btScalar size, const btTransform &mvMatrix);
/*
 * Other inventory renderer class
 */
class InventoryManager
{
public:
    enum class InventoryState
    {
        Disabled = 0,
        Idle,
        Open,
        Closed,
        RLeft,
        RRight,
        Up,
        Down,
        Activate
    };

private:
    std::list<InventoryNode>* mInventory;
    InventoryState              mCurrentState;
    InventoryState              mNextState;
    int                         mNextItemsCount;

    MenuItemType                mCurrentItemsType;
    int                         mCurrentItemsCount;
    int                         mItemsOffset;

    float                       mRingRotatePeriod;
    float                       mRingTime;
    float                       mRingAngle;
    float                       mRingVerticalAngle;
    float                       mRingAngleStep;
    float                       mBaseRingRadius;
    float                       mRingRadius;
    float                       mVerticalOffset;

    float                       mItemRotatePeriod;
    float                       mItemTime;
    float                       mItemAngle;

    int getItemsTypeCount(MenuItemType type);
    void restoreItemAngle(float time);

public:
    TextLine             mLabel_Title;
    TextLine             mLabel_ItemName;

    InventoryManager();
    ~InventoryManager();

    InventoryState getCurrentState()
    {
        return mCurrentState;
    }

    InventoryState getNextState()
    {
        return mNextState;
    }

    void send(InventoryState state)
    {
        mNextState = state;
    }

    MenuItemType getItemsType()
    {
        return mCurrentItemsType;
    }

    MenuItemType setItemsType(MenuItemType type);
    void setInventory(std::list<InventoryNode> *i);
    void setTitle(MenuItemType items_type);
    void frame(float time);
    void render();
};

extern InventoryManager  *main_inventory_manager;
extern FontManager       *fontManager;

/**
 * Draws text using a FontType::Secondary.
 */
TextLine* drawText(GLfloat x, GLfloat y, const char *fmt, ...);

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
 *  Fader functions.
 */
bool fadeStart(FaderType fader, FaderDir fade_direction);
bool fadeStop(FaderType fader);
bool fadeAssignPic(FaderType fader, const std::string &pic_name);
FaderStatus getFaderStatus(FaderType fader);
void fadeSetup(FaderType fader,
               uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, loader::BlendingMode blending_mode,
               uint16_t fadein_speed, uint16_t fadeout_speed);

/**
 * Item notifier functions.
 */
void notifierStart(int item);
void notifierStop();

/**
 * General GUI drawing routines.
 */
void drawCrosshair();
void drawFaders();
void drawBars();
void drawLoadScreen(int value);
void drawInventory();
void drawNotifier();

/**
 * General GUI update routines.
 */
void update();
void resize();  // Called every resize event.

} // namespace gui
