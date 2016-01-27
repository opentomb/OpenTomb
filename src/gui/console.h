#pragma once

#include "gl_font.h"
#include "fontmanager.h"
#include "util/helpers.h"

#include <cstdint>
#include <list>

#include <boost/optional.hpp>

namespace gui
{
class Console
{
private:
    static constexpr size_t HistorySizeMin = 16;
    static constexpr size_t HistorySizeMax = 128;

    static constexpr size_t VisibleLinesMin = 64;
    static constexpr size_t VisibleLinesMax = 256;

    static constexpr size_t LineLengthMin = 80;
    static constexpr size_t LineLengthMax = 256;

    static constexpr float SpacingMin = 0.5f;
    static constexpr float SpacingMax = 4.0f;

    struct Line
    {
        std::string text{};
        FontStyle styleId = FontStyle::Generic;

        Line() = default;
        explicit Line(const std::string& t, FontStyle s = FontStyle::Generic)
            : text(t)
            , styleId(s)
        {
        }
    };

    engine::Engine* m_engine;

    FontTexture *m_font = nullptr;                       // Texture font renderer

    glm::vec4 m_backgroundColor;

    //! Current log position plus one
    //! @note It's off-by-one, because line 0 is a virtual empty line.
    size_t m_historyPos = 0;
    size_t m_historySize = 1000;
    //! The most recent entry is at the front.
    std::vector<std::string> m_historyLines;

    std::list<Line> m_lines;
    size_t m_visibleLines = VisibleLinesMin;
    size_t m_bufferSize = 0;

    size_t m_lineLength = LineLengthMax;                  // Console line size
    int16_t m_lineHeight;                // Height, including spacing

    float m_spacing = SpacingMin;                    // Line spacing

    int16_t m_cursorPos = 0;                 // Current cursor position, in symbols
    int16_t m_cursorX;                   // Cursor position in pixels
    int16_t m_cursorY;
    util::Duration m_blinkTime;                // Current cursor draw time
    util::Duration m_blinkPeriod;
    bool m_showCursor;                // Cursor visibility flag

    bool inited = false;                     // Ready-to-use flag
    bool m_isVisible;                       // Visibility flag

    std::string m_editingLine;

    std::vector<std::string> m_completionItems;

public:
    explicit Console(engine::Engine* engine);

    void init();

    ~Console() = default;

    void initFonts();

    void initGlobals();

    void draw();

    void drawBackground();

    void drawCursor();

    void filter(const std::string& text);

    void edit(int key, const boost::optional<uint16_t>& mod = boost::none);

    void calcCursorPosition();

    void addLog(const std::string& text);

    void addLine(const std::string& text, FontStyle style);

    void addText(const std::string& text, FontStyle style);

    void printf(const char *fmt, ...);

    void warning(int warn_string_index, ...);

    void notify(int notify_string_index, ...);

    void clean();

    bool isVisible() const
    {
        return m_isVisible;
    }

    void toggleVisibility()
    {
        m_isVisible = !m_isVisible;
    }

    float spacing() const
    {
        return m_spacing;
    }

    void setSpacing(float val);

    void setVisible(bool val)
    {
        m_isVisible = val;
    }

    void setShowCursorPeriod(util::Duration val)
    {
        m_blinkPeriod = val;
    }

    void setLineLength(size_t val)
    {
        if(val < LineLengthMin || val > LineLengthMax)
            return;

        m_lineLength = val;
    }

    void setVisibleLines(size_t val)
    {
        if(val < VisibleLinesMin || val > VisibleLinesMax)
            return;

        m_visibleLines = val;
        setCursorY(static_cast<int16_t>(m_visibleLines * m_lineHeight));
    }

    size_t visibleLines() const
    {
        return m_visibleLines;
    }

    void setBufferSize(size_t val)
    {
        if(val < HistorySizeMin || val > HistorySizeMax)
            return;

        m_bufferSize = val;
    }

    void setHistorySize(size_t val)
    {
        if(val < HistorySizeMin || val > HistorySizeMax)
            return;

        m_historySize = val;
    }

    size_t lineLength() const
    {
        return m_lineLength;
    }

    int16_t lineHeight() const
    {
        return m_lineHeight;
    }

    void setCursorY(int16_t y)
    {
        m_cursorY = y;
    }

    void setBackgroundColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        m_backgroundColor[0] = r;
        m_backgroundColor[1] = g;
        m_backgroundColor[2] = b;
        m_backgroundColor[3] = a;
    }

    void setCompletionItems(const std::vector<std::string>& items)
    {
        m_completionItems = items;
    }
};
} // namespace gui
