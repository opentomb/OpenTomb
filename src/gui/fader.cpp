#include "fader.h"

#include "console.h"
#include "engine/engine.h"
#include "engine/system.h"
#include "strings.h"

#include <CImg.h>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

namespace gui
{
Fader::Fader() = default;

void Fader::setAlpha(uint8_t alpha)
{
    m_maxAlpha = static_cast<glm::float_t>(alpha) / 255;
}

void Fader::setScaleMode(FaderScale mode)
{
    m_textureScaleMode = mode;
}

void Fader::setColor(uint8_t R, uint8_t G, uint8_t B, FaderCorner corner)
{
    // Each corner of the fader could be colored independently, thus allowing
    // to create gradient faders. It is nifty yet not so useful feature, so
    // it is completely optional - if you won't specify corner, color will be
    // set for the whole fader.

    glm::vec4* dest = nullptr;

    switch(corner)
    {
        case FaderCorner::TopLeft:
            dest = &m_topLeftColor;
            break;

        case FaderCorner::TopRight:
            dest = &m_topRightColor;
            break;

        case FaderCorner::BottomLeft:
            dest = &m_bottomLeftColor;
            break;

        case FaderCorner::BottomRight:
            dest = &m_bottomRightColor;
            break;

        default:
            m_topRightColor[0] = static_cast<glm::float_t>(R) / 255;
            m_topRightColor[1] = static_cast<glm::float_t>(G) / 255;
            m_topRightColor[2] = static_cast<glm::float_t>(B) / 255;

            // Copy top right corner color to all other corners.

            m_topLeftColor = m_topRightColor;
            m_bottomRightColor = m_topRightColor;
            m_bottomLeftColor = m_topRightColor;
            return;
    }

    BOOST_ASSERT(dest != nullptr);

    (*dest)[0] = static_cast<glm::float_t>(R) / 255;
    (*dest)[1] = static_cast<glm::float_t>(G) / 255;
    (*dest)[2] = static_cast<glm::float_t>(B) / 255;
}

void Fader::setBlendingMode(loader::BlendingMode mode)
{
    m_blendingMode = mode;
}

void Fader::setSpeed(util::Duration fade_speed, util::Duration fade_speed_secondary)
{
    m_speed = fade_speed;
    m_speedSecondary = fade_speed_secondary;
}

void Fader::setDelay(util::Duration delay_msec)
{
    m_maxTime = delay_msec;
}

void Fader::setAspect()
{
    if(m_texture)
    {
        if(static_cast<float>(m_textureWidth) / static_cast<float>(engine::screen_info.w) >= static_cast<float>(m_textureHeight) / static_cast<float>(engine::screen_info.h))
        {
            m_textureWide = true;
            m_textureAspectRatio = static_cast<float>(m_textureHeight) / static_cast<float>(m_textureWidth);
        }
        else
        {
            m_textureWide = false;
            m_textureAspectRatio = static_cast<float>(m_textureWidth) / static_cast<float>(m_textureHeight);
        }
    }
}

bool Fader::setTexture(const std::string& texture_path)
{
    cimg_library::CImg<uint8_t> img;
    try
    {
        img.load(texture_path.c_str());
    }
    catch(cimg_library::CImgIOException& ex)
    {
        BOOST_LOG_TRIVIAL(warning) << "Failed to load image '" << texture_path << "': " << ex.what();
        Console::instance().warning(SYSWARN_IMG_NOT_LOADED_SDL, texture_path.c_str(), ex.what());
        return false;
    }

    // Get the color depth of the SDL surface
    GLint color_depth;
    GLenum texture_format;
    if(img.spectrum() == 4)        // Contains an alpha channel
    {
        texture_format = GL_RGBA;
        color_depth = GL_RGBA;
    }
    else if(img.spectrum() == 3)   // No alpha channel
    {
        texture_format = GL_RGB;
        color_depth = GL_RGB;
    }
    else
    {
        Console::instance().warning(SYSWARN_NOT_TRUECOLOR_IMG, texture_path.c_str());
        return false;
    }

    // Drop previously assigned texture, if it exists.
    dropTexture();

    // Set additional parameters
    m_textureWidth = img.width();
    m_textureHeight = img.height();
    // convert non-interleaved color layers to interleaved pixel color values (see https://www.codefull.org/2014/11/cimg-does-not-store-pixels-in-the-interleaved-format/)
    img.permute_axes("cxyz");

    // Have OpenGL generate a texture object handle for us
    glGenTextures(1, &m_texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // Set the texture's stretching properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Edit the texture object's image data using the information SDL_Surface gives us
    glTexImage2D(GL_TEXTURE_2D, 0, color_depth, m_textureWidth, m_textureHeight, 0,
                 texture_format, GL_UNSIGNED_BYTE, img.data());

    // Unbind the texture - is it really necessary?
    // glBindTexture(GL_TEXTURE_2D, 0);

    setAspect();

    Console::instance().notify(SYSNOTE_LOADED_FADER, texture_path.c_str());
    return true;
}

bool Fader::dropTexture()
{
    if(!m_texture)
        return false;

    /// if mTexture is incorrect then maybe trouble
    if(glIsTexture(m_texture))
    {
        glDeleteTextures(1, &m_texture);
    }
    m_texture = 0;
    return true;
}

void Fader::engage(FaderDir fade_dir)
{
    m_direction = fade_dir;
    m_active = true;
    m_complete = false;
    m_currentTime = util::Duration(0);

    if(m_direction == FaderDir::In)
    {
        m_currentAlpha = m_maxAlpha;      // Fade in: set alpha to maximum.
    }
    else
    {
        m_currentAlpha = 0.0;            // Fade out or timed: set alpha to zero.
    }
}

void Fader::cut()
{
    m_active = false;
    m_complete = false;
    m_currentAlpha = 0.0;
    m_currentTime = util::Duration(0);

    dropTexture();
}

void Fader::show()
{
    if(!m_active)
    {
        m_complete = true;
        return;                                 // If fader is not active, don't render it.
    }

    const auto alphaDelta = engine::Engine::instance.m_frameTime / m_speed;

    if(m_direction == FaderDir::In)          // Fade in case
    {
        if(m_currentAlpha > 0.0)                 // If alpha is more than zero, continue to fade.
        {
            m_currentAlpha -= alphaDelta;
        }
        else
        {
            m_complete = true;   // We've reached zero alpha, complete and disable fader.
            m_active = false;
            m_currentAlpha = 0.0;
            dropTexture();
        }
    }
    else if(m_direction == FaderDir::Out)  // Fade out case
    {
        if(m_currentAlpha < m_maxAlpha)   // If alpha is less than maximum, continue to fade.
        {
            m_currentAlpha += alphaDelta;
        }
        else
        {
            // We've reached maximum alpha, so complete fader but leave it active.
            // This is needed for engine to receive proper callback in case some events are
            // delayed to the next frame - e.g., level loading.

            m_complete = true;
            m_currentAlpha = m_maxAlpha;
        }
    }
    else    // Timed fader case
    {
        if(m_currentTime <= m_maxTime)
        {
            if(m_currentAlpha == m_maxAlpha)
            {
                m_currentTime += engine::Engine::instance.m_frameTime;
            }
            else if(m_currentAlpha < m_maxAlpha)
            {
                m_currentAlpha += alphaDelta;
            }
            else
            {
                m_currentAlpha = m_maxAlpha;
            }
        }
        else
        {
            if(m_currentAlpha > 0.0)
            {
                m_currentAlpha -= engine::Engine::instance.m_frameTime / m_speedSecondary;
            }
            else
            {
                m_complete = true;          // We've reached zero alpha, complete and disable fader.
                m_active = false;
                m_currentAlpha = 0.0;
                m_currentTime = util::Duration(0);
                dropTexture();
            }
        }
    }

    // Apply current alpha value to all vertices.

    m_topLeftColor[3] = m_currentAlpha;
    m_topRightColor[3] = m_currentAlpha;
    m_bottomLeftColor[3] = m_currentAlpha;
    m_bottomRightColor[3] = m_currentAlpha;

    // Draw the rectangle.
    // We draw it from the very top left corner to the end of the screen.

    if(m_texture)
    {
        // Texture is always modulated with alpha!
        glm::vec4 tex_color{ m_currentAlpha, m_currentAlpha, m_currentAlpha, m_currentAlpha };

        if(m_textureScaleMode == FaderScale::LetterBox)
        {
            if(m_textureWide)        // Texture is wider than the screen... Do letterbox.
            {
                // Draw lower letterbox.
                Gui::instance->drawRect(0.0,
                                        0.0,
                                        engine::screen_info.w,
                                        (engine::screen_info.h - engine::screen_info.w * m_textureAspectRatio) / 2,
                                        m_bottomLeftColor, m_bottomRightColor, m_bottomLeftColor, m_bottomRightColor,
                                        m_blendingMode);

                // Draw texture.
                Gui::instance->drawRect(0.0,
                                        (engine::screen_info.h - engine::screen_info.w * m_textureAspectRatio) / 2,
                                        engine::screen_info.w,
                                        engine::screen_info.w * m_textureAspectRatio,
                                        tex_color, tex_color, tex_color, tex_color,
                                        m_blendingMode,
                                        m_texture);

                // Draw upper letterbox.
                Gui::instance->drawRect(0.0,
                                        engine::screen_info.h - (engine::screen_info.h - engine::screen_info.w * m_textureAspectRatio) / 2,
                                        engine::screen_info.w,
                                        (engine::screen_info.h - engine::screen_info.w * m_textureAspectRatio) / 2,
                                        m_topLeftColor, m_topRightColor, m_topLeftColor, m_topRightColor,
                                        m_blendingMode);
            }
            else        // Texture is taller than the screen... Do pillarbox.
            {
                // Draw left pillarbox.
                Gui::instance->drawRect(0.0,
                                        0.0,
                                        (engine::screen_info.w - engine::screen_info.h / m_textureAspectRatio) / 2,
                                        engine::screen_info.h,
                                        m_topLeftColor, m_topLeftColor, m_bottomLeftColor, m_bottomLeftColor,
                                        m_blendingMode);

                // Draw texture.
                Gui::instance->drawRect((engine::screen_info.w - engine::screen_info.h / m_textureAspectRatio) / 2,
                                        0.0,
                                        engine::screen_info.h / m_textureAspectRatio,
                                        engine::screen_info.h,
                                        tex_color, tex_color, tex_color, tex_color,
                                        m_blendingMode,
                                        m_texture);

                // Draw right pillarbox.
                Gui::instance->drawRect(engine::screen_info.w - (engine::screen_info.w - engine::screen_info.h / m_textureAspectRatio) / 2,
                                        0.0,
                                        (engine::screen_info.w - engine::screen_info.h / m_textureAspectRatio) / 2,
                                        engine::screen_info.h,
                                        m_topRightColor, m_topRightColor, m_bottomRightColor, m_bottomRightColor,
                                        m_blendingMode);
            }
        }
        else if(m_textureScaleMode == FaderScale::Zoom)
        {
            if(m_textureWide)    // Texture is wider than the screen - scale vertical.
            {
                Gui::instance->drawRect(-((engine::screen_info.h / m_textureAspectRatio - engine::screen_info.w) / 2),
                                        0.0,
                                        engine::screen_info.h / m_textureAspectRatio,
                                        engine::screen_info.h,
                                        tex_color, tex_color, tex_color, tex_color,
                                        m_blendingMode,
                                        m_texture);
            }
            else                // Texture is taller than the screen - scale horizontal.
            {
                Gui::instance->drawRect(0.0,
                                        -((engine::screen_info.w / m_textureAspectRatio - engine::screen_info.h) / 2),
                                        engine::screen_info.w,
                                        engine::screen_info.w / m_textureAspectRatio,
                                        tex_color, tex_color, tex_color, tex_color,
                                        m_blendingMode,
                                        m_texture);
            }
        }
        else    // Simple stretch!
        {
            Gui::instance->drawRect(0.0,
                                    0.0,
                                    engine::screen_info.w,
                                    engine::screen_info.h,
                                    tex_color, tex_color, tex_color, tex_color,
                                    m_blendingMode,
                                    m_texture);
        }
    }
    else    // No texture, simply draw colored rect.
    {
        Gui::instance->drawRect(0.0, 0.0, engine::screen_info.w, engine::screen_info.h,
                                m_topLeftColor, m_topRightColor, m_bottomLeftColor, m_bottomRightColor,
                                m_blendingMode);
    }   // end if(mTexture)
}

FaderStatus Fader::getStatus() const
{
    if(m_complete)
    {
        return FaderStatus::Complete;
    }
    else if(m_active)
    {
        return FaderStatus::Fading;
    }
    else
    {
        return FaderStatus::Idle;
    }
}
} // namespace gui