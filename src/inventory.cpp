#include "inventory.h"

#include "audio/audio.h"
#include "gui/console.h"
#include "gui/gui.h"
#include "script/script.h"
#include "strings.h"
#include "util/vmath.h"
#include "engine/engine.h"
#include "world/entity.h"

#include <glm/gtc/matrix_transform.hpp>

#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>

/*
 * GUI RENDEDR CLASS
 */
InventoryManager::InventoryManager(engine::Engine* engine)
    : m_engine(engine)
{
    m_labelTitle.position = { 0, 30 };
    m_labelTitle.Xanchor = gui::HorizontalAnchor::Center;
    m_labelTitle.Yanchor = gui::VerticalAnchor::Top;

    m_labelTitle.fontType = gui::FontType::Primary;
    m_labelTitle.fontStyle = gui::FontStyle::MenuTitle;
    m_labelTitle.show = false;

    m_labelItemName.position = { 0, 50 };
    m_labelItemName.Xanchor = gui::HorizontalAnchor::Center;
    m_labelItemName.Yanchor = gui::VerticalAnchor::Bottom;

    m_labelItemName.fontType = gui::FontType::Primary;
    m_labelItemName.fontStyle = gui::FontStyle::MenuContent;
    m_labelItemName.show = false;

    m_engine->m_gui.m_textlineManager.add(&m_labelItemName);
    m_engine->m_gui.m_textlineManager.add(&m_labelTitle);
}

InventoryManager::~InventoryManager()
{
    m_currentState = InventoryState::Disabled;
    m_nextState = InventoryState::Disabled;

    m_labelItemName.show = false;
    m_engine->m_gui.m_textlineManager.erase(&m_labelItemName);

    m_labelTitle.show = false;
    m_engine->m_gui.m_textlineManager.erase(&m_labelTitle);
}

int InventoryManager::getItemsTypeCount(MenuItemType type) const
{
    int ret = 0;
    for(world::ObjectId id : m_inventory | boost::adaptors::map_keys)
    {
        auto bi = m_engine->m_world.getBaseItemByID(id);
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
            m_itemAngle -= 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
            if(m_itemAngle < 0.0f)
            {
                m_itemAngle = 0.0f;
            }
        }
        else
        {
            m_itemAngle += 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
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

    m_labelTitle.text = m_engine->m_scriptEngine.getString(string_index);
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
            if(auto bi = m_engine->m_world.getBaseItemByID(id))
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
            m_ringTime += m_engine->getFrameTime();
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
            m_ringTime += m_engine->getFrameTime();
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
                    m_itemTime += m_engine->getFrameTime();
                    m_itemAngle = 360.0f * m_itemTime / m_itemRotatePeriod;
                    if(m_itemTime >= m_itemRotatePeriod)
                    {
                        m_itemTime = util::Duration(0);
                        m_itemAngle = 0.0f;
                    }
                    m_labelItemName.show = true;
                    m_labelTitle.show = true;
                    break;

                case InventoryState::Closed:
                    m_engine->m_world.m_audioEngine.send(m_engine->m_scriptEngine.getGlobalSound(audio::GlobalSoundId::MenuClose));
                    m_labelItemName.show = false;
                    m_labelTitle.show = false;
                    m_currentState = m_nextState;
                    break;

                case InventoryState::RLeft:
                case InventoryState::RRight:
                    m_engine->m_world.m_audioEngine.send(audio::SoundMenuRotate);
                    m_labelItemName.show = false;
                    m_currentState = m_nextState;
                    m_itemTime = util::Duration(0);
                    break;

                case InventoryState::Up:
                    m_nextItemsCount = this->getItemsTypeCount(nextItemType(m_currentItemsType));
                    if(m_nextItemsCount > 0)
                    {
                        //Audio_Send(lua_GetGlobalSound(m_engine->engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        m_currentState = m_nextState;
                        m_ringTime = util::Duration(0);
                    }
                    else
                    {
                        m_nextState = InventoryState::Idle;
                    }
                    m_labelItemName.show = false;
                    m_labelTitle.show = false;
                    break;

                case InventoryState::Down:
                    m_nextItemsCount = this->getItemsTypeCount(previousItemType(m_currentItemsType));
                    if(m_nextItemsCount > 0)
                    {
                        //Audio_Send(lua_GetGlobalSound(m_engine->engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
                        m_currentState = m_nextState;
                        m_ringTime = util::Duration(0);
                    }
                    else
                    {
                        m_nextState = InventoryState::Idle;
                    }
                    m_labelItemName.show = false;
                    m_labelTitle.show = false;
                    break;
            };
            break;

        case InventoryState::Disabled:
            if(m_nextState == InventoryState::Open)
            {
                if(setItemsType(m_currentItemsType) != MenuItemType::Invalid)
                {
                    m_engine->m_world.m_audioEngine.send(m_engine->m_scriptEngine.getGlobalSound(audio::GlobalSoundId::MenuOpen));
                    m_currentState = InventoryState::Open;
                    m_ringAngle = 180.0f;
                    m_ringVerticalAngle = 180.0f;
                }
            }
            break;

        case InventoryState::Up:
            m_currentState = InventoryState::Up;
            m_nextState = InventoryState::Up;
            m_ringTime += m_engine->getFrameTime();
            if(m_ringTime < m_ringRotatePeriod)
            {
                restoreItemAngle();
                m_ringRadius = m_baseRingRadius * (m_ringRotatePeriod - m_ringTime) / m_ringRotatePeriod;
                m_verticalOffset = -m_baseRingRadius * m_ringTime / m_ringRotatePeriod;
                m_ringAngle += 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
            }
            else if(m_ringTime < 2.0f * m_ringRotatePeriod)
            {
                if(m_ringTime - m_engine->getFrameTime() <= m_ringRotatePeriod)
                {
                    //Audio_Send(lua_GetGlobalSound(m_engine->engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
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
                m_verticalOffset -= m_baseRingRadius * m_engine->getFrameTime() / m_ringRotatePeriod;
                m_ringAngle -= 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
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
            m_ringTime += m_engine->getFrameTime();
            if(m_ringTime < m_ringRotatePeriod)
            {
                restoreItemAngle();
                m_ringRadius = m_baseRingRadius * (m_ringRotatePeriod - m_ringTime) / m_ringRotatePeriod;
                m_verticalOffset = m_baseRingRadius * m_ringTime / m_ringRotatePeriod;
                m_ringAngle += 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
            }
            else if(m_ringTime < 2.0f * m_ringRotatePeriod)
            {
                if(m_ringTime - m_engine->getFrameTime() <= m_ringRotatePeriod)
                {
                    //Audio_Send(lua_GetGlobalSound(m_engine->engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
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
                m_verticalOffset += m_baseRingRadius * m_engine->getFrameTime() / m_ringRotatePeriod;
                m_ringAngle -= 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
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
            m_ringTime += m_engine->getFrameTime();
            m_ringRadius = m_baseRingRadius * m_ringTime / m_ringRotatePeriod;
            m_ringAngle -= 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
            m_ringVerticalAngle -= 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
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
            m_ringTime += m_engine->getFrameTime();
            m_ringRadius = m_baseRingRadius * (m_ringRotatePeriod - m_ringTime) / m_ringRotatePeriod;
            m_ringAngle += 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
            m_ringVerticalAngle += 180.0f * m_engine->getFrameTime() / m_ringRotatePeriod;
            if(m_ringTime >= m_ringRotatePeriod)
            {
                m_currentState = InventoryState::Disabled;
                m_nextState = InventoryState::Disabled;
                m_ringVerticalAngle = 180.0f;
                m_ringTime = util::Duration(0);
                m_labelTitle.show = false;
                m_ringRadius = m_baseRingRadius;
                m_currentItemsType = MenuItemType::Supply;
            }
            break;
    }
}

void InventoryManager::render()
{
    if(m_currentState == InventoryState::Disabled || m_inventory.empty())
        return;

    int num = 0;
    for(const auto& i : m_inventory)
    {
        auto bi = m_engine->m_world.getBaseItemByID(i.first);
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
                m_labelItemName.text = bi->name;

                if(i.second.count > 1)
                {
                    m_labelItemName.text = (boost::format(m_engine->m_scriptEngine.getString(STR_GEN_MASK_INVHEADER)) % bi->name % i.second.count).str();
                }
            }
            matrix = glm::rotate(matrix, glm::radians(90.0f + m_itemAngle - ang), { 0,0,1 });
            bi->getSkeleton().itemFrame(util::Duration(0));                            // here will be time != 0 for using items animation
        }
        else
        {
            matrix = glm::rotate(matrix, glm::radians(90.0f - ang), { 0,0,1 });
            bi->getSkeleton().itemFrame(util::Duration(0));
        }
        matrix = glm::translate(matrix, -0.5f * bi->getSkeleton().getBoundingBox().getCenter());
        matrix = glm::scale(matrix, { 0.7f, 0.7f, 0.7f });
        render::renderItem(bi->getSkeleton(), 0.0f, matrix, m_engine->m_gui.m_guiProjectionMatrix);

        num++;
    }
}

void InventoryManager::print() const
{
    for(const auto& i : m_inventory)
    {
        m_engine->m_gui.getConsole().printf("item[id = %d]: count = %d", i.first, i.second.count);
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
