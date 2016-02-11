#pragma once

#include "gui/textline.h"
#include "util/helpers.h"

#include "world/object.h"

#include <map>

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
    size_t count = 0;
    size_t max_count = 0;
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

namespace engine
{
class Engine;
}

/*
 * Other inventory renderer class
 */
class InventoryManager
{
    TRACK_LIFETIME();

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
    engine::Engine* m_engine;

    std::map<world::ObjectId, InventoryNode> m_inventory{};
    InventoryState              m_currentState = InventoryState::Disabled;
    InventoryState              m_nextState = InventoryState::Disabled;
    int                         m_nextItemsCount = 0;

    MenuItemType                m_currentItemsType = MenuItemType::System;
    int                         m_currentItemsCount = 0;
    int                         m_itemsOffset = 0;

    util::Duration              m_ringRotatePeriod = util::MilliSeconds(500);
    util::Duration              m_ringTime = util::Duration(0);
    float                       m_ringAngle = 0;
    float                       m_ringVerticalAngle = 0;
    float                       m_ringAngleStep = 0;
    float                       m_baseRingRadius = 600.0f;
    float                       m_ringRadius = 600.0f;
    float                       m_verticalOffset = 0;

    util::Duration              m_itemRotatePeriod = util::Seconds(4);
    util::Duration              m_itemTime = util::Duration(0);
    float                       m_itemAngle = 0;

    int getItemsTypeCount(MenuItemType type) const;
    void restoreItemAngle();

public:
    gui::TextLine             m_labelTitle;
    gui::TextLine             m_labelItemName;

    InventoryManager(engine::Engine* engine);
    ~InventoryManager();

    InventoryState getCurrentState() const
    {
        return m_currentState;
    }

    InventoryState getNextState() const
    {
        return m_nextState;
    }

    void send(InventoryState state)
    {
        m_nextState = state;
    }

    MenuItemType getItemsType() const
    {
        return m_currentItemsType;
    }

    MenuItemType setItemsType(MenuItemType type);
    void disable();
    void setTitle(MenuItemType items_type);
    void frame();
    void render();

    size_t addItem(world::ObjectId id, size_t count)
    {
        return m_inventory[id].count += count;
    }

    size_t remove(world::ObjectId id, size_t count)
    {
        if(m_inventory[id].count < count)
        {
            m_inventory[id].count = 0;
            return 0;
        }

        return m_inventory[id].count -= count;
    }

    void clear()
    {
        m_inventory.clear();
    }

    size_t count(world::ObjectId id) const
    {
        auto it = m_inventory.find(id);
        if(it == m_inventory.end())
            return 0;

        return it->second.count;
    }

    void print() const;

    void saveGame(std::ostream& f, world::ObjectId oid) const;
};
