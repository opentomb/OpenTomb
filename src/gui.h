
#ifndef ENGINE_GUI_H
#define ENGINE_GUI_H

#include "core/gl_font.h"
#include "core/console.h"
#include "entity.h"
#include "render.h"
#include "character_controller.h"


#define GUI_MAX_TEMP_LINES   (256)

// Screen metering resolution specifies user-friendly relative dimensions of screen,
// which are not dependent on screen resolution. They're primarily used to parse
// bar and string dimensions.

#define GUI_SCREEN_METERING_RESOLUTION 1000.0

// Anchoring is needed to link specific GUI element to specific screen position,
// independent of screen resolution and aspect ratio. Vertical and horizontal
// anchorings are seperated, so you can link element at any place - top, bottom,
// center, left or right.

#define GUI_ANCHOR_VERT_TOP         0
#define GUI_ANCHOR_VERT_BOTTOM      1
#define GUI_ANCHOR_VERT_CENTER      2

#define GUI_ANCHOR_HOR_LEFT         0
#define GUI_ANCHOR_HOR_RIGHT        1
#define GUI_ANCHOR_HOR_CENTER       2


struct inventory_node_s;

// Horizontal alignment is simple side alignment, like in original TRs.
// It means that X coordinate will be either used for left, right or
// center orientation.

#define GUI_LINE_ALIGN_LEFT   0
#define GUI_LINE_ALIGN_RIGHT  1
#define GUI_LINE_ALIGN_CENTER 2

// Default line size is generally used for static in-game strings. Strings
// that are created dynamically may have variable string sizes.

#define GUI_LINE_DEFAULTSIZE 128

typedef struct gui_text_line_s
{
    char                       *text;
    uint16_t                    text_size;

    uint16_t                    font_id;
    uint16_t                    style_id;

    GLfloat                     X;
    uint8_t                     Xanchor;
    GLfloat                     absXoffset;
    GLfloat                     Y;
    uint8_t                     Yanchor;
    GLfloat                     absYoffset;

    GLfloat                     rect[4];    //x0, yo, x1, y1

    int8_t                      show;

    struct gui_text_line_s     *next;
    struct gui_text_line_s     *prev;
} gui_text_line_t, *gui_text_line_p;


typedef struct gui_rect_s
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
    gui_text_line_s            *lines;

    int8_t                      state;      // Opening / static / closing
    int8_t                      show;
    GLfloat                     current_alpha;

    int8_t                      focused;
    int8_t                      focus_index;

    int8_t                      selectable;
    int8_t                      selection_index;

    char                       *lua_click_function;
} gui_rect_t, *gui_rect_p;


#define GUI_MENU_ITEMTYPE_SYSTEM 0
#define GUI_MENU_ITEMTYPE_SUPPLY 1
#define GUI_MENU_ITEMTYPE_QUEST  2

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

// Main bar class.

class gui_ProgressBar
{
public:
    gui_ProgressBar();  // Bar constructor.

    void Show(float value);    // Main show bar procedure.
    void Resize();

    void SetColor(BarColorType colType, uint8_t R, uint8_t G, uint8_t B, uint8_t alpha);
    void SetSize(float width, float height, float borderSize);
    void SetPosition(int8_t anchor_X, float offset_X, int8_t anchor_Y, float offset_Y);
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

    int8_t        mXanchor;             // Horizontal anchoring: left, right or center.
    int8_t        mYanchor;             // Vertical anchoring: top, bottom or center.
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
};

// Offscreen divider specifies how far item notifier will be placed from
// the final slide position. Usually it's enough to be 1/8 of the screen
// width, but if you want to increase or decrease notifier size, you must
// change this value properly.

#define GUI_NOTIFIER_OFFSCREEN_DIVIDER 8.0

// Notifier show time is a time notifier stays on screen (excluding slide
// effect). Maybe it's better to move it to script later.

#define GUI_NOTIFIER_SHOWTIME 2.0

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

void Gui_Init();
void Gui_Destroy();

void Gui_InitBars();
void Gui_InitNotifier();
void Gui_InitTempLines();

void Gui_AddLine(gui_text_line_p line);
void Gui_DeleteLine(gui_text_line_p line);
void Gui_MoveLine(gui_text_line_p line);
void Gui_RenderStringLine(gui_text_line_p l);
void Gui_RenderStrings();

/**
 * Inventory rendering / manipulation functions
 */
void Item_Frame(struct ss_bone_frame_s *bf, float time);
void Gui_RenderItem(struct ss_bone_frame_s *bf, float size, const float *mvMatrix);
/*
 * Inventory renderer class
 */
class gui_InventoryManager
{
private:
    struct inventory_node_s   **mInventory;
    int                         mCurrentState;
    int                         mNextState;
    int                         mNextItemsCount;

    int                         mCurrentItemsType;
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
    
    int getItemsTypeCount(int type);
    void restoreItemAngle(float time);
    
public:
    enum inventoryState
    {
        INVENTORY_DISABLED = 0,
        INVENTORY_IDLE,
        INVENTORY_OPEN,
        INVENTORY_CLOSE,
        INVENTORY_R_LEFT,
        INVENTORY_R_RIGHT,
        INVENTORY_UP,
        INVENTORY_DOWN,
        INVENTORY_ACTIVATE
    };

    gui_text_line_s             mLabel_Title;
    char                        mLabel_Title_text[GUI_LINE_DEFAULTSIZE];
    gui_text_line_s             mLabel_ItemName;
    char                        mLabel_ItemName_text[GUI_LINE_DEFAULTSIZE];

    gui_InventoryManager();
   ~gui_InventoryManager();

    int getCurrentState()
    {
        return mCurrentState;
    }

    int getNextState()
    {
        return mNextState;
    }

    void send(inventoryState state)
    {
        mNextState = state;
    }

    int getItemsType()
    {
        return mCurrentItemsType;
    }
    
    int setItemsType(int type);
    void setInventory(struct inventory_node_s **i);
    void setTitle(int items_type);
    void frame(float time);
    void render();
};


extern gui_InventoryManager  *main_inventory_manager;

/**
 * Draws text using a FONT_SECONDARY.
 */
gui_text_line_p Gui_OutTextXY(GLfloat x, GLfloat y, const char *fmt, ...);

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
 * Item notifier functions.
 */
void Gui_NotifierStart(int item);
void Gui_NotifierStop();

/**
 * General GUI drawing routines.
 */
void Gui_DrawCrosshair();
void Gui_DrawBars();
void Gui_DrawLoadScreen(int value);
bool Gui_LoadScreenAssignPic(const char* pic_name);
void Gui_DrawInventory();
void Gui_DrawNotifier();

/**
 * General GUI update routines.
 */
void Gui_Update();
void Gui_Resize();  // Called every resize event.

#endif
