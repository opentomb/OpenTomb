#include "fontmanager.h"

#include "engine/engine.h"
#include "engine/system.h"

namespace gui
{
FontManager::FontManager(engine::Engine* engine)
    : m_engine(engine)
{
    BOOST_LOG_TRIVIAL(info) << "Initializing FontManager";

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

FontTexture *FontManager::getFont(const FontType index)
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

Font *FontManager::getFontAddress(const FontType index)
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

FontStyleData *FontManager::getFontStyle(const FontStyle index)
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

bool FontManager::addFont(const FontType index, const uint32_t size, const char* path)
{
    if(size < MinFontSize || size > MaxFontSize)
    {
        return false;
    }

    Font* desired_font = getFontAddress(index);

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

bool FontManager::addFontStyle(const FontStyle index, const glm::vec4& color,
                               const bool shadow, const bool fading,
                               const bool rect, const glm::float_t rect_border,
                               const glm::vec4& rectCol,
                               const bool hide)
{
    FontStyleData* desired_style = getFontStyle(index);

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
    desired_style->rect_color = rectCol;

    desired_style->color = color;

    desired_style->real_color = desired_style->color;

    desired_style->fading = fading;
    desired_style->shadowed = shadow;
    desired_style->rect = rect;
    desired_style->hidden = hide;

    return true;
}

bool FontManager::removeFont(const FontType index)
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

bool FontManager::removeFontStyle(const FontStyle index)
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

void FontManager::update()
{
    const auto fontFadeDelta = m_engine->getFrameTimeSecs() * FontFadeSpeed;

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
        const auto alpha = current_style.real_color.a;
        if(current_style.fading)
        {
            current_style.real_color = current_style.color * m_fadeValue;
        }
        else
        {
            current_style.real_color = current_style.color;
        }
        current_style.real_color.a = alpha;
    }
}

void FontManager::resize()
{
    for(Font& current_font : m_fonts)
    {
        glf_resize(current_font.gl_font.get(), static_cast<uint16_t>(static_cast<float>(current_font.size) * m_engine->m_screenInfo.scale_factor));
    }
}
} // namespace gui