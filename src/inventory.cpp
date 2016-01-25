#include "inventory.h"

#include "audio/audio.h"
#include "gui/console.h"
#include "gui/gui.h"
#include "script/script.h"
#include "strings.h"
#include "util/vmath.h"
#include "engine/engine.h"

#include <glm/gtc/matrix_transform.hpp>

#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>

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

    m_ringRotatePeriod = util::MilliSeconds(500);
    m_ringTime = util::Duration(0);
    m_ringAngle = 0.0f;
    m_ringVerticalAngle = 0.0f;
    m_ringAngleStep = 0.0f;
    m_baseRingRadius = 600.0f;
    m_ringRadius = 600.0f;
    m_verticalOffset = 0.0f;

    m_itemRotatePeriod = util::Seconds(4);
    m_itemTime = util::Duration(0);
    m_itemAngle = 0.0f;

    m_inventory.clear();

    mLabel_Title.position = { 0, 30 };
    mLabel_Title.Xanchor = gui::HorizontalAnchor::Center;
    mLabel_Title.Yanchor = gui::VerticalAnchor::Top;

    mLabel_Title.fontType = gui::FontType::Primary;
    mLabel_Title.fontStyle = gui::FontStyle::MenuTitle;
    mLabel_Title.show = false;

    mLabel_ItemName.position = { 0, 50 };
    mLabel_ItemName.Xanchor = gui::HorizontalAnchor::Center;
    mLabel_ItemName.Yanchor = gui::VerticalAnchor::Bottom;

    mLabel_ItemName.fontType = gui::FontType::Primary;
    mLabel_ItemName.fontStyle = gui::FontStyle::MenuContent;
    mLabel_ItemName.show = false;

    gui::TextLineManager::instance->add(&mLabel_ItemName);
    gui::TextLineManager::instance->add(&mLabel_Title);
}

InventoryManager::~InventoryManager()
{
    m_currentState = InventoryState::Disabled;
    m_nextState = InventoryState::Disabled;

    mLabel_ItemName.show = false;
    gui::TextLineManager::instance->erase(&mLabel_ItemName);

    mLabel_Title.show = false;
    gui::TextLineManager::instance->erase(&mLabel_Title);
}

int InventoryManager::getItemsTypeCount(MenuItemType type) const
{
    int ret = 0;
    for(world::ObjectId id : m_inventory | boost::adaptors::map_keys)
    {
        auto bi = engine::Engine::instance.m_world.getBaseItemByID(id);
        if(bi && bi->type == type)
        {
            ret++;
        }
    }
    return ret;
}

void InventoryManager::restoreItemAngle()
{
    if(m_itemAngle > 0.0f)
    {
        if(m_itemAngle <= 180.0f)
        {
            m_itemAngle -= 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
            if(m_itemAngle < 0.0f)
            {
                m_itemAngle = 0.0f;
            }
        }
        else
        {
            m_itemAngle += 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
            if(m_itemAngle >= 360.0f)
            {
                m_itemAngle = 0.0f;
            }
        }
    }
}

void InventoryManager::disable()
{
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

    mLabel_Title.text = engine_lua.getString(string_index);
}

MenuItemType InventoryManager::setItemsType(MenuItemType type)
{
    if(m_inventory.empty())
    {
        m_currentItemsType = type;
        return type;
    }

    int count = this->getItemsTypeCount(type);
    if(count == 0)
    {
        for(world::ObjectId id : m_inventory | boost::adaptors::map_keys)
        {
            if(auto bi = engine::Engine::instance.m_world.getBaseItemByID(id))
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
        m_ringTime = util::Duration();
        m_ringAngle = 0.0f;
        return type;
    }

    return MenuItemType::Invalid;
}

void InventoryManager::frame()
{
    if(m_inventory.empty())
    {
        m_currentState = InventoryState::Disabled;
        m_nextState = InventoryState::Disabled;
        return;
    }

    switch(m_currentState)
    {
        case InventoryState::Activate:
            break;

        case InventoryState::RLeft:
            m_ringTime += engine::Engine::instance.getFrameTime();
            m_ringAngle = m_ringAngleStep * m_ringTime / m_ringRotatePeriod;
            m_nextState = InventoryState::RLeft;
            if(m_ringTime >= m_ringRotatePeriod)
            {
                m_ringTime = util::Duration(0);
                m_ringAngle = 0.0f;
                m_nextState = InventoryState::Idle;
                m_currentState = InventoryState::Idle;
                m_itemsOffset--;
                if(m_itemsOffset < 0)
                {
                    m_itemsOffset = m_currentItemsCount - 1;
                }
            }
            restoreItemAngle();
            break;

        case InventoryState::RRight:
            m_ringTime += engine::Engine::instance.getFrameTime();
            m_ringAngle = -m_ringAngleStep * m_ringTime / m_ringRotatePeriod;
            m_nextState = InventoryState::RRight;
            if(m_ringTime >= m_ringRotatePeriod)
            {
                m_ringTime = util::Duration(0);
                m_ringAngle = 0.0f;
                m_nextState = InventoryState::Idle;
                m_currentState = InventoryState::Idle;
                m_itemsOffset++;
                if(m_itemsOffset >= m_currentItemsCount)
                {
                    m_itemsOffset = 0;
                }
            }
            restoreItemAngle();
            break;

        case InventoryState::Idle:
            m_ringTime = util::Duration(0);
            switch(m_nextState)
            {
                default:
                case InventoryState::Idle:
                    m_itemTime += engine::Engine::instance.getFrameTime();
                    m_itemAngle = 360.0f * m_itemTime / m_itemRotatePeriod;
                    if(m_itemTime >= m_itemRotatePeriod)
                    {
                        m_itemTime = util::Duration(0);
                        m_itemAngle = 0.0f;
                    }
                    mLabel_ItemName.show = true;
                    mLabel_Title.show = true;
                    break;

                case InventoryState::Closed:
                    engine::Engine::instance.m_world.m_audioEngine.send(engine_lua.getGlobalSound(audio::GlobalSoundId::MenuClose));
                    mLabel_ItemName.show = false;
                    mLabel_Title.show = false;
                    m_currentState = m_nextState;
                    break;

                case InventoryState::RLeft:
                case InventoryState::RRight:
                    engine::Engine::instance.m_world.m_audioEngine.send(audio::SoundMenuRotate);
                    mLabel_ItemName.show = false;
                    m_currentState = m_nextState;
                    m_itemTime = util::Duration(0);
                    break;

                case InventoryState::Up:
                    m_nextItemsCount = this->getItemsTypeCount(nextItemType(m_currentItemsType));
                    if(m_nextItemsCount > 0)
                    {
                        //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        m_currentState = m_nextState;
                        m_ringTime = util::Duration(0);
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
                        m_ringTime = util::Duration(0);
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
                    engine::Engine::instance.m_world.m_audioEngine.send(engine_lua.getGlobalSound(audio::GlobalSoundId::MenuOpen));
                    m_currentState = InventoryState::Open;
                    m_ringAngle = 180.0f;
                    m_ringVerticalAngle = 180.0f;
                }
            }
            break;

        case InventoryState::Up:
            m_currentState = InventoryState::Up;
            m_nextState = InventoryState::Up;
            m_ringTime += engine::Engine::instance.getFrameTime();
            if(m_ringTime < m_ringRotatePeriod)
            {
                restoreItemAngle();
                m_ringRadius = m_baseRingRadius * (m_ringRotatePeriod - m_ringTime) / m_ringRotatePeriod;
                m_verticalOffset = -m_baseRingRadius * m_ringTime / m_ringRotatePeriod;
                m_ringAngle += 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
            }
            else if(m_ringTime < 2.0f * m_ringRotatePeriod)
            {
                if(m_ringTime - engine::Engine::instance.getFrameTime() <= m_ringRotatePeriod)
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
                m_verticalOffset -= m_baseRingRadius * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
                m_ringAngle -= 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
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
            m_ringTime += engine::Engine::instance.getFrameTime();
            if(m_ringTime < m_ringRotatePeriod)
            {
                restoreItemAngle();
                m_ringRadius = m_baseRingRadius * (m_ringRotatePeriod - m_ringTime) / m_ringRotatePeriod;
                m_verticalOffset = m_baseRingRadius * m_ringTime / m_ringRotatePeriod;
                m_ringAngle += 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
            }
            else if(m_ringTime < 2.0f * m_ringRotatePeriod)
            {
                if(m_ringTime - engine::Engine::instance.getFrameTime() <= m_ringRotatePeriod)
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
                m_verticalOffset += m_baseRingRadius * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
                m_ringAngle -= 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
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
            m_ringTime += engine::Engine::instance.getFrameTime();
            m_ringRadius = m_baseRingRadius * m_ringTime / m_ringRotatePeriod;
            m_ringAngle -= 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
            m_ringVerticalAngle -= 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
            if(m_ringTime >= m_ringRotatePeriod)
            {
                m_currentState = InventoryState::Idle;
                m_nextState = InventoryState::Idle;
                m_ringVerticalAngle = 0.0f;

                m_ringRadius = m_baseRingRadius;
                m_ringTime = util::Duration(0);
                m_ringAngle = 0.0f;
                m_verticalOffset = 0.0f;
                setTitle(MenuItemType::Supply);
            }
            break;

        case InventoryState::Closed:
            m_ringTime += engine::Engine::instance.getFrameTime();
            m_ringRadius = m_baseRingRadius * (m_ringRotatePeriod - m_ringTime) / m_ringRotatePeriod;
            m_ringAngle += 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
            m_ringVerticalAngle += 180.0f * engine::Engine::instance.getFrameTime() / m_ringRotatePeriod;
            if(m_ringTime >= m_ringRotatePeriod)
            {
                m_currentState = InventoryState::Disabled;
                m_nextState = InventoryState::Disabled;
                m_ringVerticalAngle = 180.0f;
                m_ringTime = util::Duration(0);
                mLabel_Title.show = false;
                m_ringRadius = m_baseRingRadius;
                m_currentItemsType = MenuItemType::Supply;
            }
            break;
    }
}

void InventoryManager::render()
{
    if(m_currentState == InventoryState::Disabled || m_inventory.empty() || gui::FontManager::instance == nullptr)
        return;

    int num = 0;
    for(const auto& i : m_inventory)
    {
        auto bi = engine::Engine::instance.m_world.getBaseItemByID(i.first);
        if(!bi || bi->type != m_currentItemsType)
        {
            continue;
        }

        glm::mat4 matrix(1.0f);
        matrix = glm::translate(matrix, { 0, 0, -m_baseRingRadius * 2.0f });
        //Mat4_RotateX(matrix, 25.0);
        matrix = glm::rotate(matrix, glm::radians(25.0f), { 1,0,0 });
        glm::float_t ang = m_ringAngleStep * (-m_itemsOffset + num) + m_ringAngle;
        matrix = glm::rotate(matrix, glm::radians(ang), { 0,1,0 });
        matrix = glm::translate(matrix, { 0, m_verticalOffset, m_ringRadius });
        matrix = glm::rotate(matrix, -util::Rad90, { 1,0,0 });
        matrix = glm::rotate(matrix, util::Rad90, { 0,0,1 });
        if(num == m_itemsOffset)
        {
            if(bi->name[0])
            {
                mLabel_ItemName.text = bi->name;

                if(i.second.count > 1)
                {
                    mLabel_ItemName.text = (boost::format(engine_lua.getString(STR_GEN_MASK_INVHEADER)) % bi->name % i.second.count).str();
                }
            }
            matrix = glm::rotate(matrix, glm::radians(90.0f + m_itemAngle - ang), { 0,0,1 });
            bi->bf->itemFrame(util::Duration(0));                            // here will be time != 0 for using items animation
        }
        else
        {
            matrix = glm::rotate(matrix, glm::radians(90.0f - ang), { 0,0,1 });
            bi->bf->itemFrame(util::Duration(0));
        }
        matrix = glm::translate(matrix, -0.5f * bi->bf->getBoundingBox().getCenter());
        matrix = glm::scale(matrix, { 0.7f, 0.7f, 0.7f });
        render::renderItem(*bi->bf, 0.0f, matrix, gui::Gui::instance->guiProjectionMatrix);

        num++;
    }
}

void InventoryManager::print() const
{
    for(const auto& i : m_inventory)
    {
        gui::Console::instance().printf("item[id = %d]: count = %d", i.first, i.second.count);
    }
}

void InventoryManager::saveGame(std::ostream& f, world::ObjectId oid) const
{
    for(const auto& i : m_inventory)
    {
        f << boost::format("\naddItem(%d, %d, %d);")
             % oid
             % i.first
             % i.second.count;
    }
}
