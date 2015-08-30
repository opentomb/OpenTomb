#include "inventory.h"

#include "script/script.h"
#include "util/vmath.h"
#include "strings.h"

InventoryManager  *main_inventory_manager = nullptr;

/*
 * GUI RENDEDR CLASS
 */
InventoryManager::InventoryManager()
{
    m_currentState = InventoryState::Disabled;
    m_nextState = InventoryState::Disabled;
    m_currentItemsType = MenuItemType::System;
    m_currentItemsCount = 0;
    m_itemsOffset = 0;
    m_nextItemsCount = 0;

    m_ringRotatePeriod = 0.5f;
    m_ringTime = 0.0f;
    m_ringAngle = 0.0f;
    m_ringVerticalAngle = 0.0f;
    m_ringAngleStep = 0.0f;
    m_baseRingRadius = 600.0f;
    m_ringRadius = 600.0f;
    m_verticalOffset = 0.0f;

    m_itemRotatePeriod = 4.0f;
    m_itemTime = 0.0f;
    m_itemAngle = 0.0f;

    m_inventory = nullptr;

    mLabel_Title.X = 0.0f;
    mLabel_Title.Y = 30.0f;
    mLabel_Title.Xanchor = gui::HorizontalAnchor::Center;
    mLabel_Title.Yanchor = gui::VerticalAnchor::Top;

    mLabel_Title.font_id = gui::FontType::Primary;
    mLabel_Title.style_id = gui::FontStyle::MenuTitle;
    mLabel_Title.show = false;

    mLabel_ItemName.X = 0.0f;
    mLabel_ItemName.Y = 50.0f;
    mLabel_ItemName.Xanchor = gui::HorizontalAnchor::Center;
    mLabel_ItemName.Yanchor = gui::VerticalAnchor::Bottom;

    mLabel_ItemName.font_id = gui::FontType::Primary;
    mLabel_ItemName.style_id = gui::FontStyle::MenuContent;
    mLabel_ItemName.show = false;

    gui::addLine(&mLabel_ItemName);
    gui::addLine(&mLabel_Title);
}

InventoryManager::~InventoryManager()
{
    m_currentState = InventoryState::Disabled;
    m_nextState = InventoryState::Disabled;
    m_inventory = nullptr;

    mLabel_ItemName.show = false;
    gui::deleteLine(&mLabel_ItemName);

    mLabel_Title.show = false;
    gui::deleteLine(&mLabel_Title);
}

int InventoryManager::getItemsTypeCount(MenuItemType type)
{
    int ret = 0;
    for(const InventoryNode& i : *m_inventory)
    {
        auto bi = engine::engine_world.getBaseItemByID(i.id);
        if(bi && bi->type == type)
        {
            ret++;
        }
    }
    return ret;
}

void InventoryManager::restoreItemAngle(float time)
{
    if(m_itemAngle > 0.0f)
    {
        if(m_itemAngle <= 180.0f)
        {
            m_itemAngle -= 180.0f * time / m_ringRotatePeriod;
            if(m_itemAngle < 0.0f)
            {
                m_itemAngle = 0.0f;
            }
        }
        else
        {
            m_itemAngle += 180.0f * time / m_ringRotatePeriod;
            if(m_itemAngle >= 360.0f)
            {
                m_itemAngle = 0.0f;
            }
        }
    }
}

void InventoryManager::setInventory(std::list<InventoryNode> *i)
{
    m_inventory = i;
    m_currentState = InventoryState::Disabled;
    m_nextState = InventoryState::Disabled;
}

void InventoryManager::setTitle(MenuItemType items_type)
{
    int string_index;

    switch(items_type)
    {
        case MenuItemType::System:
            string_index = STR_GEN_OPTIONS_TITLE;
            break;

        case MenuItemType::Quest:
            string_index = STR_GEN_ITEMS;
            break;

        case MenuItemType::Supply:
        default:
            string_index = STR_GEN_INVENTORY;
            break;
    }

    char buffer[gui::LineDefaultSize];
    engine_lua.getString(string_index, gui::LineDefaultSize, buffer);
    mLabel_Title.text = buffer;
}

MenuItemType InventoryManager::setItemsType(MenuItemType type)
{
    if(!m_inventory || m_inventory->empty())
    {
        m_currentItemsType = type;
        return type;
    }

    int count = this->getItemsTypeCount(type);
    if(count == 0)
    {
        for(const InventoryNode& i : *m_inventory)
        {
            if(auto bi = engine::engine_world.getBaseItemByID(i.id))
            {
                type = bi->type;
                count = this->getItemsTypeCount(m_currentItemsType);
                break;
            }
        }
    }

    if(count > 0)
    {
        m_currentItemsCount = count;
        m_currentItemsType = type;
        m_ringAngleStep = 360.0f / m_currentItemsCount;
        m_itemsOffset %= count;
        m_ringTime = 0.0f;
        m_ringAngle = 0.0f;
        return type;
    }

    return MenuItemType::Invalid;
}

void InventoryManager::frame(float time)
{
    if(!m_inventory || m_inventory->empty())
    {
        m_currentState = InventoryState::Disabled;
        m_nextState = InventoryState::Disabled;
        return;
    }

    switch(m_currentState)
    {
        case InventoryState::RLeft:
            m_ringTime += time;
            m_ringAngle = m_ringAngleStep * m_ringTime / m_ringRotatePeriod;
            m_nextState = InventoryState::RLeft;
            if(m_ringTime >= m_ringRotatePeriod)
            {
                m_ringTime = 0.0f;
                m_ringAngle = 0.0f;
                m_nextState = InventoryState::Idle;
                m_currentState = InventoryState::Idle;
                m_itemsOffset--;
                if(m_itemsOffset < 0)
                {
                    m_itemsOffset = m_currentItemsCount - 1;
                }
            }
            restoreItemAngle(time);
            break;

        case InventoryState::RRight:
            m_ringTime += time;
            m_ringAngle = -m_ringAngleStep * m_ringTime / m_ringRotatePeriod;
            m_nextState = InventoryState::RRight;
            if(m_ringTime >= m_ringRotatePeriod)
            {
                m_ringTime = 0.0f;
                m_ringAngle = 0.0f;
                m_nextState = InventoryState::Idle;
                m_currentState = InventoryState::Idle;
                m_itemsOffset++;
                if(m_itemsOffset >= m_currentItemsCount)
                {
                    m_itemsOffset = 0;
                }
            }
            restoreItemAngle(time);
            break;

        case InventoryState::Idle:
            m_ringTime = 0.0f;
            switch(m_nextState)
            {
                default:
                case InventoryState::Idle:
                    m_itemTime += time;
                    m_itemAngle = 360.0f * m_itemTime / m_itemRotatePeriod;
                    if(m_itemTime >= m_itemRotatePeriod)
                    {
                        m_itemTime = 0.0f;
                        m_itemAngle = 0.0f;
                    }
                    mLabel_ItemName.show = true;
                    mLabel_Title.show = true;
                    break;

                case InventoryState::Closed:
                    audio::send(engine_lua.getGlobalSound(audio::TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                    mLabel_ItemName.show = false;
                    mLabel_Title.show = false;
                    m_currentState = m_nextState;
                    break;

                case InventoryState::RLeft:
                case InventoryState::RRight:
                    audio::send(TR_AUDIO_SOUND_MENUROTATE);
                    mLabel_ItemName.show = false;
                    m_currentState = m_nextState;
                    m_itemTime = 0.0f;
                    break;

                case InventoryState::Up:
                    m_nextItemsCount = this->getItemsTypeCount(nextItemType(m_currentItemsType));
                    if(m_nextItemsCount > 0)
                    {
                        //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        m_currentState = m_nextState;
                        m_ringTime = 0.0f;
                    }
                    else
                    {
                        m_nextState = InventoryState::Idle;
                    }
                    mLabel_ItemName.show = false;
                    mLabel_Title.show = false;
                    break;

                case InventoryState::Down:
                    m_nextItemsCount = this->getItemsTypeCount(previousItemType(m_currentItemsType));
                    if(m_nextItemsCount > 0)
                    {
                        //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        m_currentState = m_nextState;
                        m_ringTime = 0.0f;
                    }
                    else
                    {
                        m_nextState = InventoryState::Idle;
                    }
                    mLabel_ItemName.show = false;
                    mLabel_Title.show = false;
                    break;
            };
            break;

        case InventoryState::Disabled:
            if(m_nextState == InventoryState::Open)
            {
                if(setItemsType(m_currentItemsType) != MenuItemType::Invalid)
                {
                    audio::send(engine_lua.getGlobalSound(audio::TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    m_currentState = InventoryState::Open;
                    m_ringAngle = 180.0f;
                    m_ringVerticalAngle = 180.0f;
                }
            }
            break;

        case InventoryState::Up:
            m_currentState = InventoryState::Up;
            m_nextState = InventoryState::Up;
            m_ringTime += time;
            if(m_ringTime < m_ringRotatePeriod)
            {
                restoreItemAngle(time);
                m_ringRadius = m_baseRingRadius * (m_ringRotatePeriod - m_ringTime) / m_ringRotatePeriod;
                m_verticalOffset = -m_baseRingRadius * m_ringTime / m_ringRotatePeriod;
                m_ringAngle += 180.0f * time / m_ringRotatePeriod;
            }
            else if(m_ringTime < 2.0f * m_ringRotatePeriod)
            {
                if(m_ringTime - time <= m_ringRotatePeriod)
                {
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    m_ringRadius = 0.0f;
                    m_verticalOffset = m_baseRingRadius;
                    m_ringAngleStep = 360.0f / m_nextItemsCount;
                    m_ringAngle = 180.0f;
                    m_currentItemsType = nextItemType(m_currentItemsType);
                    m_currentItemsCount = m_nextItemsCount;
                    m_itemsOffset = 0;
                    setTitle(m_currentItemsType);
                }
                m_ringRadius = m_baseRingRadius * (m_ringTime - m_ringRotatePeriod) / m_ringRotatePeriod;
                m_verticalOffset -= m_baseRingRadius * time / m_ringRotatePeriod;
                m_ringAngle -= 180.0f * time / m_ringRotatePeriod;
            }
            else
            {
                m_nextState = InventoryState::Idle;
                m_currentState = InventoryState::Idle;
                m_ringAngle = 0.0f;
                m_verticalOffset = 0.0f;
            }
            break;

        case InventoryState::Down:
            m_currentState = InventoryState::Down;
            m_nextState = InventoryState::Down;
            m_ringTime += time;
            if(m_ringTime < m_ringRotatePeriod)
            {
                restoreItemAngle(time);
                m_ringRadius = m_baseRingRadius * (m_ringRotatePeriod - m_ringTime) / m_ringRotatePeriod;
                m_verticalOffset = m_baseRingRadius * m_ringTime / m_ringRotatePeriod;
                m_ringAngle += 180.0f * time / m_ringRotatePeriod;
            }
            else if(m_ringTime < 2.0f * m_ringRotatePeriod)
            {
                if(m_ringTime - time <= m_ringRotatePeriod)
                {
                    //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
                    m_ringRadius = 0.0f;
                    m_verticalOffset = -m_baseRingRadius;
                    m_ringAngleStep = 360.0f / m_nextItemsCount;
                    m_ringAngle = 180.0;
                    m_currentItemsType = previousItemType(m_currentItemsType);
                    m_currentItemsCount = m_nextItemsCount;
                    m_itemsOffset = 0;
                    setTitle(m_currentItemsType);
                }
                m_ringRadius = m_baseRingRadius * (m_ringTime - m_ringRotatePeriod) / m_ringRotatePeriod;
                m_verticalOffset += m_baseRingRadius * time / m_ringRotatePeriod;
                m_ringAngle -= 180.0f * time / m_ringRotatePeriod;
            }
            else
            {
                m_nextState = InventoryState::Idle;
                m_currentState = InventoryState::Idle;
                m_ringAngle = 0.0f;
                m_verticalOffset = 0.0f;
            }
            break;

        case InventoryState::Open:
            m_ringTime += time;
            m_ringRadius = m_baseRingRadius * m_ringTime / m_ringRotatePeriod;
            m_ringAngle -= 180.0f * time / m_ringRotatePeriod;
            m_ringVerticalAngle -= 180.0f * time / m_ringRotatePeriod;
            if(m_ringTime >= m_ringRotatePeriod)
            {
                m_currentState = InventoryState::Idle;
                m_nextState = InventoryState::Idle;
                m_ringVerticalAngle = 0.0f;

                m_ringRadius = m_baseRingRadius;
                m_ringTime = 0.0f;
                m_ringAngle = 0.0f;
                m_verticalOffset = 0.0f;
                setTitle(MenuItemType::Supply);
            }
            break;

        case InventoryState::Closed:
            m_ringTime += time;
            m_ringRadius = m_baseRingRadius * (m_ringRotatePeriod - m_ringTime) / m_ringRotatePeriod;
            m_ringAngle += 180.0f * time / m_ringRotatePeriod;
            m_ringVerticalAngle += 180.0f * time / m_ringRotatePeriod;
            if(m_ringTime >= m_ringRotatePeriod)
            {
                m_currentState = InventoryState::Disabled;
                m_nextState = InventoryState::Disabled;
                m_ringVerticalAngle = 180.0f;
                m_ringTime = 0.0f;
                mLabel_Title.show = false;
                m_ringRadius = m_baseRingRadius;
                m_currentItemsType = MenuItemType::Supply;
            }
            break;
    }
}

void InventoryManager::render()
{
    if((m_currentState != InventoryState::Disabled) && (m_inventory != nullptr) && !m_inventory->empty() && (gui::fontManager != nullptr))
    {
        int num = 0;
        for(InventoryNode& i : *m_inventory)
        {
            auto bi = engine::engine_world.getBaseItemByID(i.id);
            if(!bi || bi->type != m_currentItemsType)
            {
                continue;
            }

            btTransform matrix;
            matrix.setIdentity();
            util::Mat4_Translate(matrix, 0.0f, 0.0f, - m_baseRingRadius * 2.0f);
            //Mat4_RotateX(matrix, 25.0);
            util::Mat4_RotateX(matrix, 25.0f + m_ringVerticalAngle);
            btScalar ang = m_ringAngleStep * (-m_itemsOffset + num) + m_ringAngle;
            util::Mat4_RotateY(matrix, ang);
            util::Mat4_Translate(matrix, 0.0f, m_verticalOffset, m_ringRadius);
            util::Mat4_RotateX(matrix, -90.0f);
            util::Mat4_RotateZ(matrix, 90.0f);
            if(num == m_itemsOffset)
            {
                if(bi->name[0])
                {
                    mLabel_ItemName.text = bi->name;

                    if(i.count > 1)
                    {
                        char counter[32];
                        engine_lua.getString(STR_GEN_MASK_INVHEADER, 32, counter);
                        char tmp[gui::LineDefaultSize];
                        snprintf(tmp, gui::LineDefaultSize, static_cast<const char*>(counter), bi->name, i.count);
                        mLabel_ItemName.text = tmp;
                    }
                }
                util::Mat4_RotateZ(matrix, 90.0f + m_itemAngle - ang);
                gui::Item_Frame(bi->bf.get(), 0.0f);                            // here will be time != 0 for using items animation
            }
            else
            {
                util::Mat4_RotateZ(matrix, 90.0f - ang);
                gui::Item_Frame(bi->bf.get(), 0.0f);
            }
            util::Mat4_Translate(matrix, -0.5f * bi->bf->centre[0], -0.5f * bi->bf->centre[1], -0.5f * bi->bf->centre[2]);
            util::Mat4_Scale(matrix, 0.7f, 0.7f, 0.7f);
            gui::renderItem(bi->bf.get(), 0.0f, matrix);

            num++;
        }
    }
}
