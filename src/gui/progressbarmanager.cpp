#include "progressbarmanager.h"

#include "character_controller.h"
#include "world/character.h"

#include <boost/range/adaptors.hpp>

namespace gui
{
ProgressbarManager::ProgressbarManager(engine::Engine* engine)
    : m_engine(engine)
{
    BOOST_LOG_TRIVIAL(info) << "Initializing ProgressbarManager";

    {
        ProgressBar& bar = getProgressBar(BarType::Health);

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
        ProgressBar& bar = getProgressBar(BarType::Air);
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
        ProgressBar& bar = getProgressBar(BarType::Stamina);

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
        ProgressBar& bar = getProgressBar(BarType::Warmth);
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
        ProgressBar& bar = getProgressBar(BarType::Loading);
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

void ProgressbarManager::draw(const world::World& world)
{
    if(!world.m_character)
        return;

    if(world.m_character->getCurrentWeaponState() > world::WeaponState::HideToReady)
        getProgressBar(BarType::Health).m_forced = true;

    if(world.m_character->getParam(world::CharParameterId::PARAM_POISON) > 0.0)
        getProgressBar(BarType::Health).m_alternate = true;

    getProgressBar(BarType::Air).show(world.m_character->getParam(world::CharParameterId::PARAM_AIR));
    getProgressBar(BarType::Stamina).show(world.m_character->getParam(world::CharParameterId::PARAM_STAMINA));
    getProgressBar(BarType::Health).show(world.m_character->getParam(world::CharParameterId::PARAM_HEALTH));
    getProgressBar(BarType::Warmth).show(world.m_character->getParam(world::CharParameterId::PARAM_WARMTH));
}

void ProgressbarManager::showLoading(int value)
{
    getProgressBar(BarType::Loading).show(value);
}

void ProgressbarManager::resize()
{
    for(ProgressBar& bar : m_progressBars | boost::adaptors::map_values)
    {
        bar.resize();
    }
}
}
