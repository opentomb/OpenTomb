#pragma once

#include "gl_font.h"

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
    FontType                   index;
    uint16_t                    size;
    std::shared_ptr<FontTexture> gl_font;
};

// Font style is different to font itself - whereas engine can have
// only three fonts, there could be unlimited amount of font styles.
// Font style management is done via font manager.
struct FontStyleData
{
    FontStyle                  index;          // Unique index which is used to identify style.

    GLfloat                     color[4];
    GLfloat                     real_color[4];
    GLfloat                     rect_color[4];
    GLfloat                     rect_border;

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

    bool             AddFont(const FontType index,
                             const uint32_t size,
                             const char* path);
    bool             RemoveFont(const FontType index);
    FontTexture*     GetFont(const FontType index);

    bool             AddFontStyle(const FontStyle index,
                                  const GLfloat R, const GLfloat G, const GLfloat B, const GLfloat A,
                                  const bool shadow, const bool fading,
                                  const bool rect, const GLfloat rect_border,
                                  const GLfloat rect_R, const GLfloat rect_G, const GLfloat rect_B, const GLfloat rect_A,
                                  const bool hide);
    bool             RemoveFontStyle(const FontStyle index);
    FontStyleData*  GetFontStyle(const FontStyle index);

    uint32_t getFontCount() const
    {
        return static_cast<uint32_t>(m_fonts.size());
    }
    uint32_t getFontStyleCount() const
    {
        return static_cast<uint32_t>(m_styles.size());
    }

    void             Update(); // Do fading routine here, etc. Put into Gui_Update, maybe...
    void             Resize(); // Resize fonts on window resize event.

private:
    Font*            GetFontAddress(const FontType index);

    GLfloat          m_fadeValue = 0; // Multiplier used with font RGB values to animate fade.
    bool             m_fadeDirection = true;

    std::list<FontStyleData> m_styles;

    std::list<Font>  m_fonts;

    FT_Library       m_fontLibrary = nullptr;  // GLF font library unit.
};

} //namespace gui
