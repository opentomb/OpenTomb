
#ifndef ENGINE_GUI_H
#define ENGINE_GUI_H

#include "ftgl/FTGLTextureFont.h"
#include "entity.h"
#include "render.h"
#include "character_controller.h"

// OpenTomb has three types of fonts - primary, secondary and console
// font. This should be enough for most of the cases. However, user 
// can generate and use additional font types via script, but engine
// behaviour with extra font types is undefined.

enum font_Type
{
    FONT_PRIMARY,
    FONT_SECONDARY,
    FONT_CONSOLE
};

#define GUI_MAX_FONTS 8     // 8 fonts is PLENTY.

// This is predefined enumeration of font styles, which can be extended
// with user-defined script functions.

enum font_Style
{
        FONTSTYLE_CONSOLE_INFO,
        FONTSTYLE_CONSOLE_WARNING,
        FONTSTYLE_CONSOLE_EVENT,
        FONTSTYLE_CONSOLE_NOTIFY,
        FONTSTYLE_MENU_TITLE,
        FONTSTYLE_MENU_HEADING1,
        FONTSTYLE_MENU_HEADING2,
        FONTSTYLE_MENU_ITEM_ACTIVE,
        FONTSTYLE_MENU_ITEM_INACTIVE,
        FONTSTYLE_MENU_CONTENT,
        FONTSTYLE_STATS_TITLE,
        FONTSTYLE_STATS_CONTENT,
        FONTSTYLE_NOTIFIER,
        FONTSTYLE_SAVEGAMELIST,
        FONTSTYLE_GENERIC
};

#define GUI_MAX_FONTSTYLES 32   // Who even needs so many? 

// Font struct contains additional field for font type which is
// used to dynamically create or delete fonts.

typedef struct gui_font_s
{
    font_Type           index;
    uint16_t            size;
    FTGLTextureFont    *font;
}gui_font_t, *gui_font_p;

// Font style is different to font itself - whereas engine can have
// only three fonts, there could be unlimited amount of font styles.
// Font style management is done via font manager.

typedef struct gui_fontstyle_s
{
    font_Style  index;          // Unique index which is used to identify style.
    
    GLfloat     color[4];
    GLfloat     real_color[4];
    GLfloat     rect_color[4];
    GLfloat     rect_border;
    
    bool        shadowed;
    bool        rect;
    bool        fading;         // TR4-like looped fading font effect.
    bool        hidden;         // Used to bypass certain GUI lines easily.
} gui_fontstyle_t, *gui_fontstyle_p;

#define GUI_FONT_FADE_SPEED 1.0 // Global fading style speed.
#define GUI_FONT_FADE_MIN   0.3 // Minimum fade multiplier.

#define GUI_FONT_SHADOW_TRANSPARENCY 0.7


// Font manager is a singleton class which is used to manage all in-game fonts
// and font styles. Every time you want to change font or style, font manager
// functions should be used.

class gui_FontManager
{
public:
    gui_FontManager();
   ~gui_FontManager();
    
    bool             AddFont(const font_Type index,
                             const uint32_t size,
                             const char* path);
    bool             RemoveFont(const font_Type index);
    FTGLTextureFont* GetFont(const font_Type index);
    
    bool             AddFontStyle(const font_Style index,
                                  const GLfloat R, const GLfloat G, const GLfloat B, const GLfloat A,
                                  const bool shadow, const bool fading,
                                  const bool rect, const GLfloat rect_border,
                                  const GLfloat rect_R, const GLfloat rect_G, const GLfloat rect_B, const GLfloat rect_A,
                                  const bool hide);
    bool             RemoveFontStyle(const font_Style index);
    gui_fontstyle_s* GetFontStyle(const font_Style index);
    
    uint32_t         GetFontCount()
    {
        return font_count;
    }
    uint32_t         GetFontStyleCount()
    {
        return style_count;
    }
    
    void             Update(); // Do fading routine here, etc. Put into Gui_Update, maybe...
    
private:
    gui_font_s*      GetFontAddress(const font_Type index);
    
    GLfloat          mFadeValue; // Multiplier used with font RGB values to animate fade.
    bool             mFadeDirection;
    
    uint32_t         style_count;
    gui_fontstyle_p  styles;
    
    uint32_t         font_count;
    gui_font_p       fonts;
};


typedef struct gui_text_line_s
{
    char                       *text;
    FTGLTextureFont            *font;
    gui_fontstyle_s            *style;
    
    uint16_t                    buf_size;
    int8_t                      show;

    GLint                       x;
    GLint                       y;

    GLfloat                     rect[4];            //x0, yo, x1, y1

    struct gui_text_line_s     *next;
    struct gui_text_line_s     *prev;
} gui_text_line_t, *gui_text_line_p;


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

enum Faders
{
    FADER_EFFECT,       // Effect fader (flashes, etc.)
    FADER_SUN,          // Sun fader (engages on looking at the sun)
    FADER_VIGNETTE,     // Just for fun - death fader.
    FADER_LOADSCREEN,   // Classic black fader for level transition
    FADER_LASTINDEX
};

#define TR_FADER_DIR_IN    0    // Normal fade-in.
#define TR_FADER_DIR_OUT   1    // Normal fade-out.
#define TR_FADER_DIR_TIMED 2    // Timed fade: in -> stay -> out.

// Scale type specifies how textures with various aspect ratios will be
// handled. If scale type is set to ZOOM, texture will be zoomed up to
// current screen's aspect ratio. If type is LETTERBOX, empty spaces
// will be filled with bars of fader's color. If type is STRETCH, image
// will be simply stretched across whole screen.
// ZOOM type is the best shot for loading screens, while LETTERBOX is
// needed for pictures with crucial info that shouldn't be cut by zoom,
// and STRETCH type is usable for full-screen effects, like vignette.

#define TR_FADER_SCALE_ZOOM      0
#define TR_FADER_SCALE_LETTERBOX 1
#define TR_FADER_SCALE_STRETCH   2

#define TR_FADER_STATUS_IDLE     0
#define TR_FADER_STATUS_FADING   1
#define TR_FADER_STATUS_COMPLETE 2

#define TR_FADER_CORNER_TOPLEFT     0
#define TR_FADER_CORNER_TOPRIGHT    1
#define TR_FADER_CORNER_BOTTOMLEFT  2
#define TR_FADER_CORNER_BOTTOMRIGHT 3

// Main fader class.

class gui_Fader
{
public:
    gui_Fader();                  // Fader constructor.

    void Show();                  // Shows and updates fader.
    void Engage(int fade_dir);    // Resets and starts fader.
    void Cut();                   // Immediately cuts fader.
    
    int  IsFading();              // Get current state of the fader.
    
    void SetScaleMode(uint8_t mode = TR_FADER_SCALE_ZOOM);
    void SetColor(uint8_t R, uint8_t G, uint8_t B, int corner = -1);
    void SetBlendingMode(uint32_t mode = BM_OPAQUE);
    void SetAlpha(uint8_t alpha  = 255);
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
    
    uint32_t        mBlendingMode;          // Fader's blending mode.
    
    GLfloat         mCurrentAlpha;          // Current alpha value.
    GLfloat         mMaxAlpha;              // Maximum reachable alpha value.
    GLfloat         mSpeed;                 // Fade speed.
    GLfloat         mSpeedSecondary;        // Secondary speed - used with TIMED type.
    
    GLuint          mTexture;               // Texture (optional).
    uint16_t        mTextureWidth;
    uint16_t        mTextureHeight;
    bool            mTextureWide;           // Set, if texture width is greater than height.
    float           mTextureAspectRatio;    // Pre-calculated aspect ratio.
    uint8_t         mTextureScaleMode;      // Fader texture's scale mode.
    
    bool            mActive;                // Specifies if fader active or not.
    bool            mComplete;              // Specifies if fading is complete or not.
    int8_t          mDirection;             // Specifies fade direction.
    
    float           mCurrentTime;           // Current fader time.
    float           mMaxTime;               // Maximum delay time.
};


// Immutable bars enumeration.
// These are the bars that are always exist in GUI.
// Scripted bars could be created and drawn separately later.

enum Bars
{
    BAR_HEALTH,     // TR 1-5
    BAR_AIR,        // TR 1-5, alternate state - gas (TR5)
    BAR_STAMINA,    // TR 3-5
    BAR_WARMTH,     // TR 3 only
    BAR_LOADING,
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

// Screen metering resolution specifies user-friendly relative dimensions of screen,
// which are not dependent on screen resolution. They're primarily used to parse
// bar dimensions.

#define TR_GUI_SCREEN_METERING_RESOLUTION 1000.0

// Main bar class.

class gui_ProgressBar
{
public:
    gui_ProgressBar();  // Bar constructor.

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

// Offscreen divider specifies how far item notifier will be placed from
// the final slide position. Usually it's enough to be 1/8 of the screen
// width, but if you want to increase or decrease notifier size, you must
// change this value properly.

#define TR_GUI_OFFSCREEN_DIVIDER 8.0

// Notifier show time is a time notifier stays on screen (excluding slide
// effect). Maybe it's better to move it to script later.

#define TR_GUI_NOTIFIER_SHOWTIME 2.0

class gui_ItemNotifier
{
public:
    gui_ItemNotifier();
    
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

void Gui_InitFontManager();

void Gui_Init();
void Gui_Destroy();

void Gui_InitBars();
void Gui_InitFaders();
void Gui_InitNotifier();
void Gui_InitTempLines();

void Gui_AddLine(gui_text_line_p line);
void Gui_DeleteLine(gui_text_line_p line);
void Gui_RenderStringLine(gui_text_line_p l);
void Gui_RenderStrings();

/**
 * Inventory rendering / manipulation functions
 */
void Item_Frame(struct ss_bone_frame_s *bf, btScalar time);
void Gui_RenderItem(struct ss_bone_frame_s *bf, btScalar size);

typedef struct gui_invmenu_item_ammo_s
{
    inventory_node_s           *linked_item;

    //float                       size;
    uint16_t                    type;
    //uint32_t                    id;
    //uint32_t                    count;
    //uint32_t                    max_count;
    //char                       *name;
    char                       *description;
}gui_invmenu_item_ammo_t, *gui_invmenu_item_ammo_p;

typedef struct gui_invmenu_item_s
{
    inventory_node_s           *linked_item;

    float                       angle;
    int8_t                      angle_dir;              // rotation direction: 0, 1 or -1
    //float                       size;
    //uint16_t                    type;
    //uint32_t                    id;
    //uint32_t                    count;
    //char                       *name;
    char                       *description;
    int8_t                      selected_ammo;

    gui_invmenu_item_ammo_s   **ammo;                   // array of ammo structs
    gui_invmenu_item_s        **combinables;            // array of items it can be combined with
    gui_invmenu_item_s         *next;                   // next item in the row
}gui_invmenu_item_t, *gui_invmenu_item_p;

class gui_InventoryMenu
{
private:
    bool                        mVisible;

    int                         mRowOffset;
    int                         mRow1Max;
    int                         mRow2Max;
    int                         mRow3Max;
    int                         mSelected;
    int                         mMaxItems;

    gui_invmenu_item_s         *mFirstInRow1;
    gui_invmenu_item_s         *mFirstInRow2;
    gui_invmenu_item_s         *mFirstInRow3;

    int                         mFrame;
    int                         mAnim;
    float                       mTime;
    float                       mMovementH;
    float                       mMovementV;
    float                       mMovementC;
    int                         mMovementDirectionH;
    int                         mMovementDirectionV;
    int                         mMovementDirectionC;
    float                       mShiftBig;
    float                       mShiftSmall;
    float                       mAngle;

    int                         mFontSize;
    int                         mFontHeight;
    // background settings
public:
    FTGLTextureFont            *mFont_Primary;               // Texture font renderers
    FTGLTextureFont            *mFont_Secondary;
    gui_fontstyle_s            *mStyle_Title;                // Styles
    gui_fontstyle_s            *mStyle_Heading1;
    gui_fontstyle_s            *mStyle_Heading2;
    gui_fontstyle_s            *mStyle_ItemActive;
    gui_fontstyle_s            *mStyle_ItemInactive;
    gui_fontstyle_s            *mStyle_MenuContent;

    gui_InventoryMenu();
    ~gui_InventoryMenu();

    void DestroyItems();
    void Toggle();
    bool IsVisible()
    {
        return mVisible;
    }
    bool IsMoving()
    {
        if (mMovementH!=0 || mMovementDirectionV!=0 || mMovementDirectionC!=0)
            return true;
        return false;
    }
    void SetRowOffset(int dy)               /// Scrolling inventory
    {
        mRowOffset = dy;
    }
    void AddItem(inventory_node_p item);
    void UpdateItemRemoval(inventory_node_p item);
    void RemoveAllItems();
    void UpdateItemsOrder(int row);
    void MoveSelectHorisontal(int dx);
    void MoveSelectVertical(int dy);

    void UpdateMovements();
    void Render(); // struct inventory_node_s *inv
    // inventory parameters calculation
    // mouse callback
};

extern gui_InventoryMenu     *main_inventory_menu;
extern gui_FontManager       *FontManager;

/**
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
void Gui_SwitchGLMode(char is_gui);

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
                  const int &blendMode,
                  const GLuint texture = 0);
                  
/**
 *  Initiate fade or check if fade is active.
 *  When Gui_Fade function is called without second argument, it will act like
 *  state check, and when second argument is present, it will immediately engage
 *  corresponding fader.
 */
bool Gui_FadeStart(int fader, int fade_direction);
bool Gui_FadeAssignPic(int fader, const char* pic_name);
int  Gui_FadeCheck(int fader);
                  
/**
 * Draw item notifier.
 */
void Gui_StartNotifier(int item);
void Gui_DrawNotifier();
 
/**
 * General GUI drawing routines.
 */
void Gui_DrawCrosshair();
void Gui_DrawFaders();
void Gui_DrawBars();
void Gui_DrawLoadScreen(int value);
void Gui_DrawInventory();

/**
 * General GUI update routine.
 */

void Gui_Update();

#endif
