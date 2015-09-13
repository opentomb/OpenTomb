#include "fontmanager.h"

#include "engine/system.h"
#include "gui.h"

namespace gui
{

FontManager::FontManager()
{
    this->font_library = nullptr;
    FT_Init_FreeType(&this->font_library);

    this->mFadeValue = 0.0;
    this->mFadeDirection = true;
}

FontManager::~FontManager()
{
    // must be freed before releasing the library
    styles.clear();
    fonts.clear();
    FT_Done_FreeType(this->font_library);
    this->font_library = nullptr;
}

FontTexture *FontManager::GetFont(const FontType index)
{
    for(const Font& current_font : this->fonts)
    {
        if(current_font.index == index)
        {
            return current_font.gl_font.get();
        }
    }

    return nullptr;
}

Font *FontManager::GetFontAddress(const FontType index)
{
    for(Font& current_font : this->fonts)
    {
        if(current_font.index == index)
        {
            return &current_font;
        }
    }

    return nullptr;
}

FontStyleData *FontManager::GetFontStyle(const FontStyle index)
{
    for(FontStyleData& current_style : this->styles)
    {
        if(current_style.index == index)
        {
            return &current_style;
        }
    }

    return nullptr;
}

bool FontManager::AddFont(const FontType index, const uint32_t size, const char* path)
{
    if((size < MinFontSize) || (size > MaxFontSize))
    {
        return false;
    }

    Font* desired_font = GetFontAddress(index);

    if(desired_font == nullptr)
    {
        if(this->fonts.size() >= MaxFonts)
        {
            return false;
        }

        this->fonts.emplace_front();
        desired_font = &this->fonts.front();
        desired_font->size = static_cast<uint16_t>(size);
        desired_font->index = index;
    }

    desired_font->gl_font = glf_create_font(this->font_library, path, size);

    return true;
}

bool FontManager::AddFontStyle(const FontStyle index,
                                   const GLfloat R, const GLfloat G, const GLfloat B, const GLfloat A,
                                   const bool shadow, const bool fading,
                                   const bool rect, const GLfloat rect_border,
                                   const GLfloat rect_R, const GLfloat rect_G, const GLfloat rect_B, const GLfloat rect_A,
                                   const bool hide)
{
    FontStyleData* desired_style = GetFontStyle(index);

    if(desired_style == nullptr)
    {
        if(this->styles.size() >= static_cast<int>(FontStyle::Sentinel))
        {
            return false;
        }

        this->styles.emplace_front();
        desired_style = &this->styles.front();
        desired_style->index = index;
    }

    desired_style->rect_border = rect_border;
    desired_style->rect_color[0] = rect_R;
    desired_style->rect_color[1] = rect_G;
    desired_style->rect_color[2] = rect_B;
    desired_style->rect_color[3] = rect_A;

    desired_style->color[0] = R;
    desired_style->color[1] = G;
    desired_style->color[2] = B;
    desired_style->color[3] = A;

    memcpy(desired_style->real_color, desired_style->color, sizeof(GLfloat) * 4);

    desired_style->fading = fading;
    desired_style->shadowed = shadow;
    desired_style->rect = rect;
    desired_style->hidden = hide;

    return true;
}

bool FontManager::RemoveFont(const FontType index)
{
    if(this->fonts.empty())
    {
        return false;
    }

    for(auto it = this->fonts.begin(); it != this->fonts.end(); ++it)
    {
        if(it->index == index)
        {
            this->fonts.erase(it);
            return true;
        }
    }

    return false;
}

bool FontManager::RemoveFontStyle(const FontStyle index)
{
    if(this->styles.empty())
    {
        return false;
    }

    for(auto it = this->styles.begin(); it != this->styles.end(); ++it)
    {
        if(it->index == index)
        {
            this->styles.erase(it);
            return true;
        }
    }

    return false;
}

void FontManager::Update()
{
    if(this->mFadeDirection)
    {
        this->mFadeValue += engine::engine_frame_time * FontFadeSpeed;

        if(this->mFadeValue >= 1.0)
        {
            this->mFadeValue = 1.0;
            this->mFadeDirection = false;
        }
    }
    else
    {
        this->mFadeValue -= engine::engine_frame_time * FontFadeSpeed;

        if(this->mFadeValue <= FontFadeMin)
        {
            this->mFadeValue = FontFadeMin;
            this->mFadeDirection = true;
        }
    }

    for(FontStyleData& current_style : this->styles)
    {
        if(current_style.fading)
        {
            current_style.real_color[0] = current_style.color[0] * this->mFadeValue;
            current_style.real_color[1] = current_style.color[1] * this->mFadeValue;
            current_style.real_color[2] = current_style.color[2] * this->mFadeValue;
        }
        else
        {
            std::copy_n( current_style.color, 3, current_style.real_color );
        }
    }
}

void FontManager::Resize()
{
    for(Font& current_font : this->fonts)
    {
        glf_resize(current_font.gl_font.get(), static_cast<uint16_t>(static_cast<float>(current_font.size) * engine::screen_info.scale_factor));
    }
}

} // namespace gui

