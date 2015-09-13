#include "fader.h"

#include "console.h"
#include "engine/system.h"

#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>

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
    SetSpeed(500);
    SetDelay(0);

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

void Fader::SetSpeed(uint16_t fade_speed, uint16_t fade_speed_secondary)
{
    mSpeed = 1000.0f / fade_speed;
    mSpeedSecondary = 1000.0f / fade_speed_secondary;
}

void Fader::SetDelay(uint32_t delay_msec)
{
    mMaxTime = delay_msec / 1000.0f;
}

void Fader::SetAspect()
{
    if(mTexture)
    {
        if((static_cast<float>(mTextureWidth) / static_cast<float>(engine::screen_info.w)) >= (static_cast<float>(mTextureHeight) / static_cast<float>(engine::screen_info.h)))
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

bool Fader::SetTexture(const char *texture_path)
{
#ifdef __APPLE_CC__
    // Load the texture file using ImageIO
    CGDataProviderRef provider = CGDataProviderCreateWithFilename(texture_path);
    CFDictionaryRef empty = CFDictionaryCreate(kCFAllocatorDefault, nullptr, nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CGImageSourceRef source = CGImageSourceCreateWithDataProvider(provider, empty);
    CGDataProviderRelease(provider);
    CFRelease(empty);

    // Check whether loading succeeded
    CGImageSourceStatus status = CGImageSourceGetStatus(source);
    if(status != kCGImageStatusComplete)
    {
        CFRelease(source);
        Console::instance().warning(SYSWARN_IMAGE_NOT_LOADED, texture_path, status);
        return false;
    }

    // Get the image
    CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, nullptr);
    CFRelease(source);
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);

    // Prepare the data to write to
    uint8_t *data = new uint8_t[width * height * 4];

    // Write image to bytes. This is done by drawing it into an off-screen image context using our data as the backing store
    CGColorSpaceRef deviceRgb = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(data, width, height, 8, width * 4, deviceRgb, kCGImageAlphaPremultipliedFirst);
    CGColorSpaceRelease(deviceRgb);
    assert(context);

    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);

    CGContextRelease(context);
    CGImageRelease(image);

    // Drop previously assigned texture, if it exists.
    DropTexture();

    // Have OpenGL generate a texture object handle for us
    glGenTextures(1, &mTexture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // Set the texture's stretching properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load texture. The weird format works out to ARGB8 in the end
    // (on little-endian systems), which is what we specified above and what
    // OpenGL prefers internally.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)width, (GLuint)height, 0,
                 GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, data);

    // Cleanup
    delete[] data;

    // Setup the additional required information
    mTextureWidth = width;
    mTextureHeight = height;

    SetAspect();

    Console::instance().notify(SYSNOTE_LOADED_FADER, texture_path);
    return true;
#else
    SDL_Surface *surface = IMG_Load(texture_path);
    GLenum       texture_format;
    GLint        color_depth;

    if(surface != nullptr)
    {
        // Get the color depth of the SDL surface
        color_depth = surface->format->BytesPerPixel;

        if(color_depth == 4)        // Contains an alpha channel
        {
            if(surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGBA;
            else
                texture_format = GL_BGRA;

            color_depth = GL_RGBA;
        }
        else if(color_depth == 3)   // No alpha channel
        {
            if(surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGB;
            else
                texture_format = GL_BGR;

            color_depth = GL_RGB;
        }
        else
        {
            Console::instance().warning(SYSWARN_NOT_TRUECOLOR_IMG, texture_path);
            SDL_FreeSurface(surface);
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
        glTexImage2D(GL_TEXTURE_2D, 0, color_depth, surface->w, surface->h, 0,
                     texture_format, GL_UNSIGNED_BYTE, surface->pixels);
    }
    else
    {
        Console::instance().warning(SYSWARN_IMG_NOT_LOADED_SDL, texture_path, SDL_GetError());
        return false;
    }

    // Unbind the texture - is it really necessary?
    // glBindTexture(GL_TEXTURE_2D, 0);

    // Free the SDL_Surface only if it was successfully created
    if(surface)
    {
        // Set additional parameters
        mTextureWidth = surface->w;
        mTextureHeight = surface->h;

        SetAspect();

        Console::instance().notify(SYSNOTE_LOADED_FADER, texture_path);
        SDL_FreeSurface(surface);
        return true;
    }
    else
    {
        /// if mTexture == 0 then trouble
        if(glIsTexture(mTexture))
        {
            glDeleteTextures(1, &mTexture);
        }
        mTexture = 0;
        return false;
    }
#endif
}

bool Fader::DropTexture()
{
    if(mTexture)
    {
        /// if mTexture is incorrect then maybe trouble
        if(glIsTexture(mTexture))
        {
            glDeleteTextures(1, &mTexture);
        }
        mTexture = 0;
        return true;
    }
    else
    {
        return false;
    }
}

void Fader::Engage(FaderDir fade_dir)
{
    mDirection = fade_dir;
    mActive = true;
    mComplete = false;
    mCurrentTime = 0.0;

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
    mCurrentTime = 0.0;

    DropTexture();
}

void Fader::Show()
{
    if(!mActive)
    {
        mComplete = true;
        return;                                 // If fader is not active, don't render it.
    }

    if(mDirection == FaderDir::In)          // Fade in case
    {
        if(mCurrentAlpha > 0.0)                 // If alpha is more than zero, continue to fade.
        {
            mCurrentAlpha -= engine::engine_frame_time * mSpeed;
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
            mCurrentAlpha += engine::engine_frame_time * mSpeed;
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
                mCurrentAlpha += engine::engine_frame_time * mSpeed;
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
                mCurrentAlpha -= engine::engine_frame_time * mSpeedSecondary;
            }
            else
            {
                mComplete = true;          // We've reached zero alpha, complete and disable fader.
                mActive = false;
                mCurrentAlpha = 0.0;
                mCurrentTime = 0.0;
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
                             (engine::screen_info.h - (engine::screen_info.w * mTextureAspectRatio)) / 2,
                             mBottomLeftColor, mBottomRightColor, mBottomLeftColor, mBottomRightColor,
                             mBlendingMode);

                // Draw texture.
                drawRect(0.0,
                             (engine::screen_info.h - (engine::screen_info.w * mTextureAspectRatio)) / 2,
                             engine::screen_info.w,
                             engine::screen_info.w * mTextureAspectRatio,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);

                // Draw upper letterbox.
                drawRect(0.0,
                             engine::screen_info.h - (engine::screen_info.h - (engine::screen_info.w * mTextureAspectRatio)) / 2,
                             engine::screen_info.w,
                             (engine::screen_info.h - (engine::screen_info.w * mTextureAspectRatio)) / 2,
                             mTopLeftColor, mTopRightColor, mTopLeftColor, mTopRightColor,
                             mBlendingMode);
            }
            else        // Texture is taller than the screen... Do pillarbox.
            {
                // Draw left pillarbox.
                drawRect(0.0,
                             0.0,
                             (engine::screen_info.w - (engine::screen_info.h / mTextureAspectRatio)) / 2,
                             engine::screen_info.h,
                             mTopLeftColor, mTopLeftColor, mBottomLeftColor, mBottomLeftColor,
                             mBlendingMode);

                // Draw texture.
                drawRect((engine::screen_info.w - (engine::screen_info.h / mTextureAspectRatio)) / 2,
                             0.0,
                             engine::screen_info.h / mTextureAspectRatio,
                             engine::screen_info.h,
                             tex_color, tex_color, tex_color, tex_color,
                             mBlendingMode,
                             mTexture);

                // Draw right pillarbox.
                drawRect(engine::screen_info.w - (engine::screen_info.w - (engine::screen_info.h / mTextureAspectRatio)) / 2,
                             0.0,
                             (engine::screen_info.w - (engine::screen_info.h / mTextureAspectRatio)) / 2,
                             engine::screen_info.h,
                             mTopRightColor, mTopRightColor, mBottomRightColor, mBottomRightColor,
                             mBlendingMode);
            }
        }
        else if(mTextureScaleMode == FaderScale::Zoom)
        {
            if(mTextureWide)    // Texture is wider than the screen - scale vertical.
            {
                drawRect(-(((engine::screen_info.h / mTextureAspectRatio) - engine::screen_info.w) / 2),
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
                             -(((engine::screen_info.w / mTextureAspectRatio) - engine::screen_info.h) / 2),
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

FaderStatus Fader::getStatus()
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

    if((fader < FaderType::Sentinel) && (faderType[fader].getStatus() != FaderStatus::Fading))
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
    if((fader < FaderType::Sentinel) && (faderType[fader].getStatus() != FaderStatus::Idle))
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
    if((fader >= FaderType::Effect) && (fader < FaderType::Sentinel))
    {
        char buf[MAX_ENGINE_PATH];

        ///@STICK: we can write incorrect image file extension, but engine will try all supported formats
        strncpy(buf, pic_name.c_str(), MAX_ENGINE_PATH);
        if(!engine::fileExists(buf, false))
        {
            size_t ext_len = 0;

            for(; ext_len + 1 < pic_name.length(); ext_len++)
            {
                if(buf[pic_name.length() - ext_len - 1] == '.')
                {
                    break;
                }
            }

            if(ext_len + 1 == pic_name.length())
            {
                return false;
            }

            buf[pic_name.length() - ext_len + 0] = 'b';
            buf[pic_name.length() - ext_len + 1] = 'm';
            buf[pic_name.length() - ext_len + 2] = 'p';
            buf[pic_name.length() - ext_len + 3] = 0;
            if(!engine::fileExists(buf, false))
            {
                buf[pic_name.length() - ext_len + 0] = 'j';
                buf[pic_name.length() - ext_len + 1] = 'p';
                buf[pic_name.length() - ext_len + 2] = 'g';
                if(!engine::fileExists(buf, false))
                {
                    buf[pic_name.length() - ext_len + 0] = 'p';
                    buf[pic_name.length() - ext_len + 1] = 'n';
                    buf[pic_name.length() - ext_len + 2] = 'g';
                    if(!engine::fileExists(buf, false))
                    {
                        buf[pic_name.length() - ext_len + 0] = 't';
                        buf[pic_name.length() - ext_len + 1] = 'g';
                        buf[pic_name.length() - ext_len + 2] = 'a';
                        if(!engine::fileExists(buf, false))
                        {
                            return false;
                        }
                    }
                }
            }
        }

        return faderType[fader].SetTexture(buf);
    }

    return false;
}

void fadeSetup(FaderType fader,
                   uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, loader::BlendingMode blending_mode,
                   uint16_t fadein_speed, uint16_t fadeout_speed)
{
    if(fader >= FaderType::Sentinel) return;

    faderType[fader].SetAlpha(alpha);
    faderType[fader].SetColor(R, G, B);
    faderType[fader].SetBlendingMode(blending_mode);
    faderType[fader].SetSpeed(fadein_speed, fadeout_speed);
}

FaderStatus getFaderStatus(FaderType fader)
{
    if((fader >= FaderType::Effect) && (fader < FaderType::Sentinel))
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
        faderType[i].SetSpeed(500);
        faderType[i].SetScaleMode(FaderScale::Zoom);
    }

    {
        const auto i = FaderType::Effect;
        faderType[i].SetAlpha(255);
        faderType[i].SetColor(255, 180, 0);
        faderType[i].SetBlendingMode(loader::BlendingMode::Multiply);
        faderType[i].SetSpeed(10, 800);
    }

    {
        const auto i = FaderType::Black;
        faderType[i].SetAlpha(255);
        faderType[i].SetColor(0, 0, 0);
        faderType[i].SetBlendingMode(loader::BlendingMode::Opaque);
        faderType[i].SetSpeed(500);
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
