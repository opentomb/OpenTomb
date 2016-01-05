#include "fader.h"

#include "console.h"
#include "engine/engine.h"
#include "engine/system.h"
#include "strings.h"

#include <SDL2/SDL_surface.h>
#include <CImg.h>

#include <boost/filesystem.hpp>

#include <map>

namespace gui
{
namespace
{
std::map<FaderType, Fader> faderType;
} // anonymous namespace

Fader::Fader()
{
    SetColor(0, 0, 0);
    SetBlendingMode(loader::BlendingMode::Opaque);
    SetAlpha(255);
    SetSpeed(util::MilliSeconds(500));
    SetDelay(util::Duration(0));

    mActive = false;
    mComplete = true;  // All faders must be initialized as complete to receive proper start-up callbacks.
    mDirection = FaderDir::In;

    mTexture = 0;
}

void Fader::SetAlpha(uint8_t alpha)
{
    mMaxAlpha = static_cast<float>(alpha) / 255;
}

void Fader::SetScaleMode(FaderScale mode)
{
    mTextureScaleMode = mode;
}

void Fader::SetColor(uint8_t R, uint8_t G, uint8_t B, FaderCorner corner)
{
    // Each corner of the fader could be colored independently, thus allowing
    // to create gradient faders. It is nifty yet not so useful feature, so
    // it is completely optional - if you won't specify corner, color will be
    // set for the whole fader.

    switch(corner)
    {
        case FaderCorner::TopLeft:
            mTopLeftColor[0] = static_cast<GLfloat>(R) / 255;
            mTopLeftColor[1] = static_cast<GLfloat>(G) / 255;
            mTopLeftColor[2] = static_cast<GLfloat>(B) / 255;
            break;

        case FaderCorner::TopRight:
            mTopRightColor[0] = static_cast<GLfloat>(R) / 255;
            mTopRightColor[1] = static_cast<GLfloat>(G) / 255;
            mTopRightColor[2] = static_cast<GLfloat>(B) / 255;
            break;

        case FaderCorner::BottomLeft:
            mBottomLeftColor[0] = static_cast<GLfloat>(R) / 255;
            mBottomLeftColor[1] = static_cast<GLfloat>(G) / 255;
            mBottomLeftColor[2] = static_cast<GLfloat>(B) / 255;
            break;

        case FaderCorner::BottomRight:
            mBottomRightColor[0] = static_cast<GLfloat>(R) / 255;
            mBottomRightColor[1] = static_cast<GLfloat>(G) / 255;
            mBottomRightColor[2] = static_cast<GLfloat>(B) / 255;
            break;

        default:
            mTopRightColor[0] = static_cast<GLfloat>(R) / 255;
            mTopRightColor[1] = static_cast<GLfloat>(G) / 255;
            mTopRightColor[2] = static_cast<GLfloat>(B) / 255;

            // Copy top right corner color to all other corners.

            memcpy(mTopLeftColor, mTopRightColor, sizeof(GLfloat) * 4);
            memcpy(mBottomRightColor, mTopRightColor, sizeof(GLfloat) * 4);
            memcpy(mBottomLeftColor, mTopRightColor, sizeof(GLfloat) * 4);
            break;
    }
}

void Fader::SetBlendingMode(loader::BlendingMode mode)
{
    mBlendingMode = mode;
}

void Fader::SetSpeed(util::Duration fade_speed, util::Duration fade_speed_secondary)
{
    mSpeed = fade_speed;
    mSpeedSecondary = fade_speed_secondary;
}

void Fader::SetDelay(util::Duration delay_msec)
{
    mMaxTime = delay_msec;
}

void Fader::SetAspect()
{
    if(mTexture)
    {
        if(static_cast<float>(mTextureWidth) / static_cast<float>(engine::screen_info.w) >= static_cast<float>(mTextureHeight) / static_cast<float>(engine::screen_info.h))
        {
            mTextureWide = true;
            mTextureAspectRatio = static_cast<float>(mTextureHeight) / static_cast<float>(mTextureWidth);
        }
        else
        {
            mTextureWide = false;
            mTextureAspectRatio = static_cast<float>(mTextureWidth) / static_cast<float>(mTextureHeight);
        }
    }
}

bool Fader::SetTexture(const std::string& texture_path)
{
    cimg_library::CImg<uint8_t> surface;
    try
    {
        surface.load(texture_path.c_str());
    }
    catch(cimg_library::CImgIOException& ex)
    {
        Console::instance().warning(SYSWARN_IMG_NOT_LOADED_SDL, texture_path.c_str(), SDL_GetError());
        return false;
    }

    // Get the color depth of the SDL surface
    GLint color_depth;
    GLenum texture_format;
    if(surface.spectrum() == 4)        // Contains an alpha channel
    {
        texture_format = GL_RGBA;
        color_depth = GL_RGBA;
    }
    else if(surface.spectrum() == 3)   // No alpha channel
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
    DropTexture();

    // Have OpenGL generate a texture object handle for us
    glGenTextures(1, &mTexture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // Set the texture's stretching properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Edit the texture object's image data using the information SDL_Surface gives us
    glTexImage2D(GL_TEXTURE_2D, 0, color_depth, surface.width(), surface.height(), 0,
                    texture_format, GL_UNSIGNED_BYTE, surface.data());

    // Unbind the texture - is it really necessary?
    // glBindTexture(GL_TEXTURE_2D, 0);

    // Set additional parameters
    mTextureWidth = surface.width();
    mTextureHeight = surface.height();

    SetAspect();

    Console::instance().notify(SYSNOTE_LOADED_FADER, texture_path.c_str());
    return true;
}

bool Fader::DropTexture()
{
    if(!mTexture)
        return false;

    /// if mTexture is incorrect then maybe trouble
    if(glIsTexture(mTexture))
    {
        glDeleteTextures(1, &mTexture);
    }
    mTexture = 0;
    return true;
}

void Fader::Engage(FaderDir fade_dir)
{
    mDirection = fade_dir;
    mActive = true;
    mComplete = false;
    mCurrentTime = util::Duration(0);

    if(mDirection == FaderDir::In)
    {
        mCurrentAlpha = mMaxAlpha;      // Fade in: set alpha to maximum.
    }
    else
    {
        mCurrentAlpha = 0.0;            // Fade out or timed: set alpha to zero.
    }
}

void Fader::Cut()
{
    mActive = false;
    mComplete = false;
    mCurrentAlpha = 0.0;
    mCurrentTime = util::Duration(0);

    DropTexture();
}

void Fader::Show()
{
    if(!mActive)
    {
        mComplete = true;
        return;                                 // If fader is not active, don't render it.
    }

    const auto alphaDelta = engine::engine_frame_time / mSpeed;

    if(mDirection == FaderDir::In)          // Fade in case
    {
        if(mCurrentAlpha > 0.0)                 // If alpha is more than zero, continue to fade.
        {
            mCurrentAlpha -= alphaDelta;
        }
        else
        {
            mComplete = true;   // We've reached zero alpha, complete and disable fader.
            mActive = false;
            mCurrentAlpha = 0.0;
            DropTexture();
        }
    }
    else if(mDirection == FaderDir::Out)  // Fade out case
    {
        if(mCurrentAlpha < mMaxAlpha)   // If alpha is less than maximum, continue to fade.
        {
            mCurrentAlpha += alphaDelta;
        }
        else
        {
            // We've reached maximum alpha, so complete fader but leave it active.
            // This is needed for engine to receive proper callback in case some events are
            // delayed to the next frame - e.g., level loading.

            mComplete = true;
            mCurrentAlpha = mMaxAlpha;
        }
    }
    else    // Timed fader case
    {
        if(mCurrentTime <= mMaxTime)
        {
            if(mCurrentAlpha == mMaxAlpha)
            {
                mCurrentTime += engine::engine_frame_time;
            }
            else if(mCurrentAlpha < mMaxAlpha)
            {
                mCurrentAlpha += alphaDelta;
            }
            else
            {
                mCurrentAlpha = mMaxAlpha;
            }
        }
        else
        {
            if(mCurrentAlpha > 0.0)
            {
                mCurrentAlpha -= engine::engine_frame_time / mSpeedSecondary;
            }
            else
            {
                mComplete = true;          // We've reached zero alpha, complete and disable fader.
                mActive = false;
                mCurrentAlpha = 0.0;
                mCurrentTime = util::Duration(0);
                DropTexture();
            }
        }
    }

    // Apply current alpha value to all vertices.

    mTopLeftColor[3] = mCurrentAlpha;
    mTopRightColor[3] = mCurrentAlpha;
    mBottomLeftColor[3] = mCurrentAlpha;
    mBottomRightColor[3] = mCurrentAlpha;

    // Draw the rectangle.
    // We draw it from the very top left corner to the end of the screen.

    if(mTexture)
    {
        // Texture is always modulated with alpha!
        GLfloat tex_color[4] = { mCurrentAlpha, mCurrentAlpha, mCurrentAlpha, mCurrentAlpha };

        if(mTextureScaleMode == FaderScale::LetterBox)
        {
            if(mTextureWide)        // Texture is wider than the screen... Do letterbox.
            {
                // Draw lower letterbox.
                drawRect(0.0,
                             0.0,
                             engine::screen_info.w,
                             (engine::screen_info.h - engine::screen_info.w * mTextureAspectRatio) / 2,
                             mBottomLeftColor, mBottomRightColor, mBottomLeftColor, mBottomRightColor,
                             mBlendingMode);

                // Draw texture.
                drawRect(0.0,
                             (engine::screen_info.h - engine::screen_info.w * mTextureAspectRatio) / 2,
                             engine::screen_info.w,
                             engine::screen_info.w * mTextureAspectRatio,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);

                // Draw upper letterbox.
                drawRect(0.0,
                             engine::screen_info.h - (engine::screen_info.h - engine::screen_info.w * mTextureAspectRatio) / 2,
                             engine::screen_info.w,
                             (engine::screen_info.h - engine::screen_info.w * mTextureAspectRatio) / 2,
                             mTopLeftColor, mTopRightColor, mTopLeftColor, mTopRightColor,
                             mBlendingMode);
            }
            else        // Texture is taller than the screen... Do pillarbox.
            {
                // Draw left pillarbox.
                drawRect(0.0,
                             0.0,
                             (engine::screen_info.w - engine::screen_info.h / mTextureAspectRatio) / 2,
                             engine::screen_info.h,
                             mTopLeftColor, mTopLeftColor, mBottomLeftColor, mBottomLeftColor,
                             mBlendingMode);

                // Draw texture.
                drawRect((engine::screen_info.w - engine::screen_info.h / mTextureAspectRatio) / 2,
                             0.0,
                             engine::screen_info.h / mTextureAspectRatio,
                             engine::screen_info.h,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);

                // Draw right pillarbox.
                drawRect(engine::screen_info.w - (engine::screen_info.w - engine::screen_info.h / mTextureAspectRatio) / 2,
                             0.0,
                             (engine::screen_info.w - engine::screen_info.h / mTextureAspectRatio) / 2,
                             engine::screen_info.h,
                             mTopRightColor, mTopRightColor, mBottomRightColor, mBottomRightColor,
                             mBlendingMode);
            }
        }
        else if(mTextureScaleMode == FaderScale::Zoom)
        {
            if(mTextureWide)    // Texture is wider than the screen - scale vertical.
            {
                drawRect(-((engine::screen_info.h / mTextureAspectRatio - engine::screen_info.w) / 2),
                             0.0,
                             engine::screen_info.h / mTextureAspectRatio,
                             engine::screen_info.h,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);
            }
            else                // Texture is taller than the screen - scale horizontal.
            {
                drawRect(0.0,
                             -((engine::screen_info.w / mTextureAspectRatio - engine::screen_info.h) / 2),
                             engine::screen_info.w,
                             engine::screen_info.w / mTextureAspectRatio,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);
            }
        }
        else    // Simple stretch!
        {
            drawRect(0.0,
                         0.0,
                         engine::screen_info.w,
                         engine::screen_info.h,
                         tex_color, tex_color, tex_color, tex_color,
                         mBlendingMode,
                         mTexture);
        }
    }
    else    // No texture, simply draw colored rect.
    {
        drawRect(0.0, 0.0, engine::screen_info.w, engine::screen_info.h,
                     mTopLeftColor, mTopRightColor, mBottomLeftColor, mBottomRightColor,
                     mBlendingMode);
    }   // end if(mTexture)
}

FaderStatus Fader::getStatus() const
{
    if(mComplete)
    {
        return FaderStatus::Complete;
    }
    else if(mActive)
    {
        return FaderStatus::Fading;
    }
    else
    {
        return FaderStatus::Idle;
    }
}

bool fadeStart(FaderType fader, FaderDir fade_direction)
{
    // If fader exists, and is not active, we engage it.

    if(fader < FaderType::Sentinel && faderType[fader].getStatus() != FaderStatus::Fading)
    {
        faderType[fader].Engage(fade_direction);
        return true;
    }
    else
    {
        return false;
    }
}

bool fadeStop(FaderType fader)
{
    if(fader < FaderType::Sentinel && faderType[fader].getStatus() != FaderStatus::Idle)
    {
        faderType[fader].Cut();
        return true;
    }
    else
    {
        return false;
    }
}

bool fadeAssignPic(FaderType fader, const std::string& pic_name)
{
    if(fader < FaderType::Effect || fader >= FaderType::Sentinel)
        return false;

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

    return faderType[fader].SetTexture(buf.string());
}

void fadeSetup(FaderType fader,
                   uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, loader::BlendingMode blending_mode,
                   util::Duration fadein_speed, util::Duration fadeout_speed)
{
    if(fader >= FaderType::Sentinel) return;

    faderType[fader].SetAlpha(alpha);
    faderType[fader].SetColor(R, G, B);
    faderType[fader].SetBlendingMode(blending_mode);
    faderType[fader].SetSpeed(fadein_speed, fadeout_speed);
}

FaderStatus getFaderStatus(FaderType fader)
{
    if(fader >= FaderType::Effect && fader < FaderType::Sentinel)
    {
        return faderType[fader].getStatus();
    }
    else
    {
        return FaderStatus::Invalid;
    }
}

void initFaders()
{
    {
        const auto i = FaderType::LoadScreen;
        faderType[i].SetAlpha(255);
        faderType[i].SetColor(0, 0, 0);
        faderType[i].SetBlendingMode(loader::BlendingMode::Opaque);
        faderType[i].SetSpeed(util::MilliSeconds(500));
        faderType[i].SetScaleMode(FaderScale::Zoom);
    }

    {
        const auto i = FaderType::Effect;
        faderType[i].SetAlpha(255);
        faderType[i].SetColor(255, 180, 0);
        faderType[i].SetBlendingMode(loader::BlendingMode::Multiply);
        faderType[i].SetSpeed(util::MilliSeconds(10), util::MilliSeconds(800));
    }

    {
        const auto i = FaderType::Black;
        faderType[i].SetAlpha(255);
        faderType[i].SetColor(0, 0, 0);
        faderType[i].SetBlendingMode(loader::BlendingMode::Opaque);
        faderType[i].SetSpeed(util::MilliSeconds(500));
        faderType[i].SetScaleMode(FaderScale::Zoom);
    }
}

void destroyFaders()
{
    for(auto& fader : faderType)
    {
        fader.second.Cut();
    }
}

void drawFaders()
{
    for(auto& i : faderType)
    {
        i.second.Show();
    }
}

void showLoadScreenFader()
{
    faderType[FaderType::LoadScreen].Show();
}

} // namespace gui
