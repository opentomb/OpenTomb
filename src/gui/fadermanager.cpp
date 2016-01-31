#include "fadermanager.h"

#include <boost/filesystem.hpp>
#include <boost/range/adaptors.hpp>

namespace gui
{
bool FaderManager::start(FaderType fader, FaderDir fade_direction)
{
    auto it = m_faders.find(fader);
    if(it == m_faders.end())
        return false;

    if(it->second.getStatus() == FaderStatus::Fading)
        return false;

    it->second.engage(fade_direction);
    return true;
}

bool FaderManager::stop(FaderType fader)
{
    auto it = m_faders.find(fader);
    if(it == m_faders.end())
        return false;

    if(it->second.getStatus() == FaderStatus::Idle)
        return false;

    it->second.cut();
    return true;
}

bool FaderManager::assignPicture(FaderType fader, const std::string& pic_name)
{
    boost::filesystem::path buf = pic_name;

    ///@STICK: we can write incorrect image file extension, but engine will try all supported formats
    if(!boost::filesystem::is_regular_file(buf))
    {
        buf.replace_extension(".bmp");
        if(!boost::filesystem::is_regular_file(buf))
        {
            buf.replace_extension(".jpg");
            if(!boost::filesystem::is_regular_file(buf))
            {
                buf.replace_extension(".png");
                if(!boost::filesystem::is_regular_file(buf))
                {
                    buf.replace_extension(".tga");
                    if(!boost::filesystem::is_regular_file(buf))
                    {
                        return false;
                    }
                }
            }
        }
    }

    auto it = m_faders.find(fader);
    if(it == m_faders.end())
        it = m_faders.insert(std::make_pair(fader, Fader(m_engine))).first;

    return it->second.setTexture(buf.string());
}

void FaderManager::setup(FaderType fader,
                         uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, loader::BlendingMode blending_mode,
                         util::Duration fadein_speed, util::Duration fadeout_speed)
{
    auto it = m_faders.find(fader);
    if(it == m_faders.end())
        it = m_faders.insert(std::make_pair(fader, Fader(m_engine))).first;
    it->second.setAlpha(alpha);
    it->second.setColor(R, G, B);
    it->second.setBlendingMode(blending_mode);
    it->second.setSpeed(fadein_speed, fadeout_speed);
}

FaderStatus FaderManager::getStatus(FaderType fader)
{
    auto it = m_faders.find(fader);
    if(it == m_faders.end())
        it = m_faders.insert(std::make_pair(fader, Fader(m_engine))).first;
    return it->second.getStatus();
}

FaderManager::FaderManager(engine::Engine* engine)
    : m_engine(engine)
{
    {
        Fader fader(m_engine);
        fader.setAlpha(255);
        fader.setColor(0, 0, 0);
        fader.setBlendingMode(loader::BlendingMode::Opaque);
        fader.setSpeed(util::MilliSeconds(500));
        fader.setScaleMode(FaderScale::Zoom);
        m_faders.insert(std::make_pair(FaderType::LoadScreen, std::move(fader)));
    }

    {
        Fader fader(m_engine);
        fader.setAlpha(255);
        fader.setColor(255, 180, 0);
        fader.setBlendingMode(loader::BlendingMode::Multiply);
        fader.setSpeed(util::MilliSeconds(10), util::MilliSeconds(800));
        m_faders.insert(std::make_pair(FaderType::Effect, std::move(fader)));
    }

    {
        Fader fader(m_engine);
        fader.setAlpha(255);
        fader.setColor(0, 0, 0);
        fader.setBlendingMode(loader::BlendingMode::Opaque);
        fader.setSpeed(util::MilliSeconds(500));
        fader.setScaleMode(FaderScale::Zoom);
        m_faders.insert(std::make_pair(FaderType::Black, std::move(fader)));
    }
}

FaderManager::~FaderManager()
{
    for(Fader& fader : m_faders | boost::adaptors::map_values)
    {
        fader.cut();
    }
}

void FaderManager::drawFaders()
{
    for(Fader& fader : m_faders | boost::adaptors::map_values)
    {
        fader.show();
    }
}

void FaderManager::showLoadScreenFader()
{
    auto it = m_faders.find(FaderType::LoadScreen);
    if(it == m_faders.end())
        it = m_faders.insert(std::make_pair(FaderType::LoadScreen, Fader(m_engine))).first;
    it->second.show();
}
} // namespace gui