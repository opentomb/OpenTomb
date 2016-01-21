#pragma once

#include "gl_font.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <list>

namespace gui
{
// OpenTomb has three types of fonts - primary, secondary and console
// font. This should be enough for most of the cases. However, user
// can generate and use additional font types via script, but engine
// behaviour with extra font types is undefined.
enum class FontType
{
    Primary,
    Secondary,
    Console
};

// This is predefined enumeration of font styles, which can be extended
// with user-defined script functions.
///@TODO: add system message console style
enum class FontStyle
{
    ConsoleInfo,
    ConsoleWarning,
    ConsoleEvent,
    ConsoleNotify,
    MenuTitle,
    MenuHeading1,
    MenuHeading2,
    MenuItemActive,
    MenuItemInactive,
    MenuContent,
    StatsTitle,
    StatsContent,
    Notifier,
    SavegameList,
    Generic,
    Sentinel
};

// Font struct contains additional field for font type which is
// used to dynamically create or delete fonts.
struct Font
{
    FontType                     index;
    uint16_t                     size;
    std::shared_ptr<FontTexture> gl_font;
};

// Font style is different to font itself - whereas engine can have
// only three fonts, there could be unlimited amount of font styles.
// Font style management is done via font manager.
struct FontStyleData
{
    FontStyle                  index;          // Unique index which is used to identify style.

    glm::vec4 color;
    glm::vec4 real_color;
    glm::vec4 rect_color;
    glm::float_t rect_border;

    bool                        shadowed;
    bool                        rect;
    bool                        fading;         // TR4-like looped fading font effect.
    bool                        hidden;         // Used to bypass certain GUI lines easily.
};

// Font manager is a singleton class which is used to manage all in-game fonts
// and font styles. Every time you want to change font or style, font manager
// functions should be used.
class FontManager
{
public:
    FontManager();
    ~FontManager();

    bool             addFont(const FontType index,
                             const uint32_t size,
                             const char* path);
    bool             removeFont(const FontType index);
    FontTexture*     getFont(const FontType index);

    bool             addFontStyle(const FontStyle index,
                                  const glm::vec4& color,
                                  const bool shadow, const bool fading,
                                  const bool rect, const glm::float_t rect_border,
                                  const glm::vec4& rectCol,
                                  const bool hide);
    bool            removeFontStyle(const FontStyle index);
    FontStyleData*  getFontStyle(const FontStyle index);

    size_t getFontCount() const
    {
        return m_fonts.size();
    }

    size_t getFontStyleCount() const
    {
        return m_styles.size();
    }

    void             update(); // Do fading routine here, etc. Put into Gui_Update, maybe...
    void             resize(); // Resize fonts on window resize event.

private:
    Font*            getFontAddress(const FontType index);

    glm::float_t     m_fadeValue = 0; // Multiplier used with font RGB values to animate fade.
    bool             m_fadeDirection = true;

    std::list<FontStyleData> m_styles;

    std::list<Font>  m_fonts;

    FT_Library       m_fontLibrary = nullptr;  // GLF font library unit.
};

} //namespace gui
