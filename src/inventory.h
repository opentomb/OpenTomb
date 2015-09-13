#pragma once

#include "gui/textline.h"

#include <list>

#define ITEM_COMPASS  1     // Aka Watch in TR2-3, Timex in TR5
#define ITEM_PASSPORT 2     // Exists only in TR1-3, not used in TR4 (diary)
#define ITEM_LARAHOME 3
#define ITEM_VIDEO    4     // Video settings. Exists only in TR1-3.
#define ITEM_AUDIO    5     // Audio settings. Exists only in TR1-3.
#define ITEM_CONTROLS 6     // Control settings. Exists only in TR1-3.
#define ITEM_LOAD     7     // Load game. Exists only in TR4-5.
#define ITEM_SAVE     8     // Save game. Exists only in TR4-5.
#define ITEM_MAP      9     // Map item only existed in TR1-3, not used.

#define ITEM_PISTOLS    10  // Exists in all game versions
#define ITEM_SHOTGUN    11  // Exists in all game versions
#define ITEM_MAGNUMS    12  // Aka Automags in TR2, Desert Eagle in TR3/5, Revolver in TR4/5.
#define ITEM_UZIS       13  // Exists in all game versions
#define ITEM_M16        14  // Exists since TR2, aka MP5 in TR3, aka H&K in TR5
#define ITEM_GRENADEGUN 15  // Exists since TR2, through all game versions
#define ITEM_ROCKETGUN  16  // Exists in TR3 only
#define ITEM_HARPOONGUN 17  // Exists in TR2-3 only
#define ITEM_CROSSBOW   18  // Exists since TR4, aka Grappling Gun in TR5.

#define ITEM_LASERSIGHT 20
#define ITEM_BINOCULARS 21
#define ITEM_SILENCER   22  // Exists only in TR5, not used.

#define ITEM_PISTOL_AMMO 30
#define ITEM_SHOTGUN_NORMAL_AMMO 31
#define ITEM_SHOTGUN_WIDESHOT_AMMO 32
#define ITEM_MAGNUM_AMMO 33
#define ITEM_UZI_AMMO 34
#define ITEM_M16_AMMO 35
#define ITEM_GRENADEGUN_NORMAL_AMMO 36
#define ITEM_GRENADEGUN_SUPER_AMMO 37
#define ITEM_GRENADEGUN_FLASH_AMMO 38
#define ITEM_ROCKETGUN_AMMO 39
#define ITEM_HARPOONGUN_AMMO 40
#define ITEM_CROSSBOW_NORMAL_AMMO 41
#define ITEM_CROSSBOW_POISON_AMMO 42
#define ITEM_CROSSBOW_EXPLOSIVE_AMMO 43

#define ITEM_FLARES 45
#define ITEM_SINGLE_FLARE 46
#define ITEM_TORCH 47

#define ITEM_SMALL_MEDIPACK 50
#define ITEM_LARGE_MEDIPACK 51

#define ITEM_SECRET_1 120
#define ITEM_SECRET_2 121
#define ITEM_SECRET_3 122

struct InventoryNode
{
    uint32_t id;
    int32_t count;
    uint32_t max_count;
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
    std::list<InventoryNode>*   m_inventory;
    InventoryState              m_currentState;
    InventoryState              m_nextState;
    int                         m_nextItemsCount;

    MenuItemType                m_currentItemsType;
    int                         m_currentItemsCount;
    int                         m_itemsOffset;

    float                       m_ringRotatePeriod;
    float                       m_ringTime;
    float                       m_ringAngle;
    float                       m_ringVerticalAngle;
    float                       m_ringAngleStep;
    float                       m_baseRingRadius;
    float                       m_ringRadius;
    float                       m_verticalOffset;

    float                       m_itemRotatePeriod;
    float                       m_itemTime;
    float                       m_itemAngle;

    int getItemsTypeCount(MenuItemType type);
    void restoreItemAngle(float time);

public:
    gui::TextLine             mLabel_Title;
    gui::TextLine             mLabel_ItemName;

    InventoryManager();
    ~InventoryManager();

    InventoryState getCurrentState()
    {
        return m_currentState;
    }

    InventoryState getNextState()
    {
        return m_nextState;
    }

    void send(InventoryState state)
    {
        m_nextState = state;
    }

    MenuItemType getItemsType()
    {
        return m_currentItemsType;
    }

    MenuItemType setItemsType(MenuItemType type);
    void setInventory(std::list<InventoryNode> *i);
    void setTitle(MenuItemType items_type);
    void frame(float time);
    void render();
};

extern InventoryManager  *main_inventory_manager;
