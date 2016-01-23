#include "fadermanager.h"

#include <CImg.h>

#include <boost/filesystem.hpp>
#include <boost/range/adaptors.hpp>

namespace gui
{

bool FaderManager::start(FaderType fader, FaderDir fade_direction)
{
    // If fader exists, and is not active, we engage it.

    if(m_faders[fader].getStatus() != FaderStatus::Fading)
    {
        m_faders[fader].engage(fade_direction);
        return true;
    }
    else
    {
        return false;
    }
}

bool FaderManager::stop(FaderType fader)
{
    if(m_faders[fader].getStatus() != FaderStatus::Idle)
    {
        m_faders[fader].cut();
        return true;
    }
    else
    {
        return false;
    }
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

    return m_faders[fader].setTexture(buf.string());
}

void FaderManager::setup(FaderType fader,
                   uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, loader::BlendingMode blending_mode,
                   util::Duration fadein_speed, util::Duration fadeout_speed)
{
    m_faders[fader].setAlpha(alpha);
    m_faders[fader].setColor(R, G, B);
    m_faders[fader].setBlendingMode(blending_mode);
    m_faders[fader].setSpeed(fadein_speed, fadeout_speed);
}

FaderStatus FaderManager::getStatus(FaderType fader)
{
    return m_faders[fader].getStatus();
}

FaderManager::FaderManager()
{
    {
        Fader& fader = m_faders[FaderType::LoadScreen];
        fader.setAlpha(255);
        fader.setColor(0, 0, 0);
        fader.setBlendingMode(loader::BlendingMode::Opaque);
        fader.setSpeed(util::MilliSeconds(500));
        fader.setScaleMode(FaderScale::Zoom);
    }

    {
        Fader& fader = m_faders[FaderType::Effect];
        fader.setAlpha(255);
        fader.setColor(255, 180, 0);
        fader.setBlendingMode(loader::BlendingMode::Multiply);
        fader.setSpeed(util::MilliSeconds(10), util::MilliSeconds(800));
    }

    {
        Fader& fader = m_faders[FaderType::Black];
        fader.setAlpha(255);
        fader.setColor(0, 0, 0);
        fader.setBlendingMode(loader::BlendingMode::Opaque);
        fader.setSpeed(util::MilliSeconds(500));
        fader.setScaleMode(FaderScale::Zoom);
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
    m_faders[FaderType::LoadScreen].show();
}

} // namespace gui
