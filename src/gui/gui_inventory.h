
#ifndef ENGINE_GUI_INVENTORY_H
#define ENGINE_GUI_INVENTORY_H

#include <stdint.h>
#include "../core/gl_text.h"

struct inventory_node_s;


#define GUI_MENU_ITEMTYPE_SYSTEM 0
#define GUI_MENU_ITEMTYPE_SUPPLY 1
#define GUI_MENU_ITEMTYPE_QUEST  2

// Offscreen divider specifies how far item notifier will be placed from
// the final slide position. Usually it's enough to be 1/8 of the screen
// width, but if you want to increase or decrease notifier size, you must
// change this value properly.

#define GUI_NOTIFIER_OFFSCREEN_DIVIDER 8.0f

// Notifier show time is a time notifier stays on screen (excluding slide
// effect). Maybe it's better to move it to script later.

#define GUI_NOTIFIER_SHOWTIME 2.0f

class gui_ItemNotifier
{
public:
    gui_ItemNotifier();

    void    Start(int item, float time);
    void    Reset();
    void    Animate(float time);
    void    Draw();

    void    SetRot(float X, float Y);
    void    SetSize(float size);
    void    SetRotateTime(float time);

private:
    bool    mActive;
    int     mItem;

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

void Gui_InitNotifier();

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

    void setInventory(struct inventory_node_s **i, uint32_t owner_id);
    void setTitle(int items_type);
    void frame(float time);
    void render();

    gl_text_line_t              mLabel_Title;
    char                        mLabel_Title_text[GUI_LINE_DEFAULTSIZE];
    gl_text_line_t              mLabel_ItemName;
    char                        mLabel_ItemName_text[GUI_LINE_DEFAULTSIZE];

private:
    struct inventory_node_s   **mInventory;
    uint32_t                    mOwnerId;
    int                         mCurrentState;
    int                         mNextState;

    int                         mCurrentItemsType;
    int                         mNextItemsType;
    int                         mCurrentItemsCount;
    int                         mSelectedItem;

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

    int getItemElementsCountByType(int type);
    int getPreviousItemsType(int curr_type);
    int getNextItemsType(int curr_type);
    void updateCurrentRing();
    void frameStates(float time);
    void frameItems(float time);
    void restoreItemAngle(float time);
};


extern gui_InventoryManager  *main_inventory_manager;

/**
 * Item notifier functions.
 */
void Gui_NotifierStart(int item);
void Gui_NotifierStop();

/**
 * General GUI drawing routines.
 */
void Gui_DrawInventory(float time);
void Gui_DrawNotifier(float time);

#endif
