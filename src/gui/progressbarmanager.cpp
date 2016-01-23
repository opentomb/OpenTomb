#include "progressbarmanager.h"

#include "character_controller.h"
#include "world/character.h"

#include <boost/range/adaptors.hpp>

namespace gui
{

ProgressbarManager::ProgressbarManager()
{
    {
        ProgressBar& bar = m_progressBars[BarType::Health];

        bar.setSize(250, 15, 3);
        bar.setPosition(HorizontalAnchor::Left, 30, VerticalAnchor::Top, 30);
        bar.setColor(BarColorType::BaseMain, 255, 50, 50, 200);
        bar.setColor(BarColorType::BaseFade, 100, 255, 50, 200);
        bar.setColor(BarColorType::AltMain, 255, 180, 0, 255);
        bar.setColor(BarColorType::AltFade, 255, 255, 0, 255);
        bar.setColor(BarColorType::BackMain, 0, 0, 0, 160);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 130);
        bar.setColor(BarColorType::BorderMain, 200, 200, 200, 50);
        bar.setColor(BarColorType::BorderFade, 80, 80, 80, 100);
        bar.setValues(LARA_PARAM_HEALTH_MAX, LARA_PARAM_HEALTH_MAX / 3);
        bar.setBlink(util::MilliSeconds(300));
        bar.setExtrude(true, 100);
        bar.setAutoshow(true, util::MilliSeconds(2000), true, util::MilliSeconds(400));
    }
    {
        ProgressBar& bar = m_progressBars[BarType::Air];
        bar.m_invert = true;

        bar.setSize(250, 15, 3);
        bar.setPosition(HorizontalAnchor::Right, 30, VerticalAnchor::Top, 30);
        bar.setColor(BarColorType::BaseMain, 0, 50, 255, 200);
        bar.setColor(BarColorType::BaseFade, 190, 190, 255, 200);
        bar.setColor(BarColorType::BackMain, 0, 0, 0, 160);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 130);
        bar.setColor(BarColorType::BorderMain, 200, 200, 200, 50);
        bar.setColor(BarColorType::BorderFade, 80, 80, 80, 100);
        bar.setValues(LARA_PARAM_AIR_MAX, LARA_PARAM_AIR_MAX / 3);
        bar.setBlink(util::MilliSeconds(300));
        bar.setExtrude(true, 100);
        bar.setAutoshow(true, util::MilliSeconds(2000), true, util::MilliSeconds(400));
    }
    {
        ProgressBar& bar = m_progressBars[BarType::Stamina];

        bar.setSize(250, 15, 3);
        bar.setPosition(HorizontalAnchor::Left, 30, VerticalAnchor::Top, 55);
        bar.setColor(BarColorType::BaseMain, 255, 100, 50, 200);
        bar.setColor(BarColorType::BaseFade, 255, 200, 0, 200);
        bar.setColor(BarColorType::BackMain, 0, 0, 0, 160);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 130);
        bar.setColor(BarColorType::BorderMain, 110, 110, 110, 100);
        bar.setColor(BarColorType::BorderFade, 60, 60, 60, 180);
        bar.setValues(LARA_PARAM_STAMINA_MAX, 0);
        bar.setExtrude(true, 100);
        bar.setAutoshow(true, util::MilliSeconds(500), true, util::MilliSeconds(300));
    }
    {
        ProgressBar& bar = m_progressBars[BarType::Warmth];
        bar.m_invert = true;

        bar.setSize(250, 15, 3);
        bar.setPosition(HorizontalAnchor::Right, 30, VerticalAnchor::Top, 55);
        bar.setColor(BarColorType::BaseMain, 255, 0, 255, 255);
        bar.setColor(BarColorType::BaseFade, 190, 120, 255, 255);
        bar.setColor(BarColorType::BackMain, 0, 0, 0, 160);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 130);
        bar.setColor(BarColorType::BorderMain, 200, 200, 200, 50);
        bar.setColor(BarColorType::BorderFade, 80, 80, 80, 100);
        bar.setValues(LARA_PARAM_WARMTH_MAX, LARA_PARAM_WARMTH_MAX / 3);
        bar.setBlink(util::MilliSeconds(200));
        bar.setExtrude(true, 60);
        bar.setAutoshow(true, util::MilliSeconds(500), true, util::MilliSeconds(300));
    }
    {
        ProgressBar& bar = m_progressBars[BarType::Loading];
        bar.m_visible = true;

        bar.setSize(800, 25, 3);
        bar.setPosition(HorizontalAnchor::Center, 0, VerticalAnchor::Bottom, 40);
        bar.setColor(BarColorType::BaseMain, 255, 225, 127, 230);
        bar.setColor(BarColorType::BaseFade, 255, 187, 136, 230);
        bar.setColor(BarColorType::BackMain, 30, 30, 30, 100);
        bar.setColor(BarColorType::BackFade, 60, 60, 60, 100);
        bar.setColor(BarColorType::BorderMain, 200, 200, 200, 80);
        bar.setColor(BarColorType::BorderFade, 80, 80, 80, 80);
        bar.setValues(1000, 0);
        bar.setExtrude(true, 70);
        bar.setAutoshow(false, util::MilliSeconds(500), false, util::MilliSeconds(300));
    }
}

void ProgressbarManager::draw()
{
    if(!engine::engine_world.character)
        return;

    if(engine::engine_world.character->m_currentWeaponState > world::WeaponState::HideToReady)
        m_progressBars[BarType::Health].m_forced = true;

    if(engine::engine_world.character->getParam(world::PARAM_POISON) > 0.0)
        m_progressBars[BarType::Health].m_alternate = true;

    m_progressBars[BarType::Air].show(engine::engine_world.character->getParam(world::PARAM_AIR));
    m_progressBars[BarType::Stamina].show(engine::engine_world.character->getParam(world::PARAM_STAMINA));
    m_progressBars[BarType::Health].show(engine::engine_world.character->getParam(world::PARAM_HEALTH));
    m_progressBars[BarType::Warmth].show(engine::engine_world.character->getParam(world::PARAM_WARMTH));
}

void ProgressbarManager::showLoading(int value)
{
    m_progressBars[BarType::Loading].show(value);
}

void ProgressbarManager::resize()
{
    for(ProgressBar& bar : m_progressBars | boost::adaptors::map_values)
    {
        bar.resize();
    }
}

}
