#include "fontmanager.h"

#include "engine/engine.h"
#include "engine/system.h"
#include "gui.h"

namespace gui
{

FontManager::FontManager()
{
    FT_Init_FreeType(&m_fontLibrary);
}

FontManager::~FontManager()
{
    // must be freed before releasing the library
    m_styles.clear();
    m_fonts.clear();
    FT_Done_FreeType(m_fontLibrary);
    m_fontLibrary = nullptr;
}

FontTexture *FontManager::GetFont(const FontType index)
{
    for(const Font& current_font : m_fonts)
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
    for(Font& current_font : m_fonts)
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
    for(FontStyleData& current_style : m_styles)
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
        if(m_fonts.size() >= MaxFonts)
        {
            return false;
        }

        m_fonts.emplace_front();
        desired_font = &m_fonts.front();
        desired_font->size = static_cast<uint16_t>(size);
        desired_font->index = index;
    }

    desired_font->gl_font = glf_create_font(m_fontLibrary, path, size);

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
        if(m_styles.size() >= static_cast<int>(FontStyle::Sentinel))
        {
            return false;
        }

        m_styles.emplace_front();
        desired_style = &m_styles.front();
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
    if(m_fonts.empty())
    {
        return false;
    }

    for(auto it = m_fonts.begin(); it != m_fonts.end(); ++it)
    {
        if(it->index == index)
        {
            m_fonts.erase(it);
            return true;
        }
    }

    return false;
}

bool FontManager::RemoveFontStyle(const FontStyle index)
{
    if(m_styles.empty())
    {
        return false;
    }

    for(auto it = m_styles.begin(); it != m_styles.end(); ++it)
    {
        if(it->index == index)
        {
            m_styles.erase(it);
            return true;
        }
    }

    return false;
}

void FontManager::Update()
{
    const auto fontFadeDelta = util::toSeconds(engine::engine_frame_time) * FontFadeSpeed;

    if(m_fadeDirection)
    {
        m_fadeValue += fontFadeDelta;

        if(m_fadeValue >= 1.0)
        {
            m_fadeValue = 1.0;
            m_fadeDirection = false;
        }
    }
    else
    {
        m_fadeValue -= fontFadeDelta;

        if(m_fadeValue <= FontFadeMin)
        {
            m_fadeValue = FontFadeMin;
            m_fadeDirection = true;
        }
    }

    for(FontStyleData& current_style : m_styles)
    {
        if(current_style.fading)
        {
            current_style.real_color[0] = current_style.color[0] * m_fadeValue;
            current_style.real_color[1] = current_style.color[1] * m_fadeValue;
            current_style.real_color[2] = current_style.color[2] * m_fadeValue;
        }
        else
        {
            std::copy_n( current_style.color, 3, current_style.real_color );
        }
    }
}

void FontManager::Resize()
{
    for(Font& current_font : m_fonts)
    {
        glf_resize(current_font.gl_font.get(), static_cast<uint16_t>(static_cast<float>(current_font.size) * engine::screen_info.scale_factor));
    }
}

} // namespace gui

