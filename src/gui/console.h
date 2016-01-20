#pragma once

#include "gl_font.h"
#include "gui.h"

#include <cstdint>

#include <boost/optional.hpp>

#define CON_MIN_LOG 16
#define CON_MAX_LOG 128

#define CON_MIN_LINES 64
#define CON_MAX_LINES 256

#define CON_MIN_LINE_SIZE 80
#define CON_MAX_LINE_SIZE 256

#define CON_MIN_LINE_INTERVAL 0.5
#define CON_MAX_LINE_INTERVAL 4.0

namespace gui
{

struct Console
{
private:
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

    FontTexture *m_font = nullptr;                       // Texture font renderer

    glm::vec4 m_backgroundColor;

    //! Current log position plus one
    //! @note It's off-by-one, because line 0 is a virtual empty line.
    size_t m_historyPos = 0;
    size_t m_historySize = 1000;
    //! The most recent entry is at the front.
    std::vector<std::string> m_historyLines;

    std::list<Line> m_lines;
    size_t m_visibleLines = 40;
    size_t m_bufferSize = 0;

    uint16_t m_lineSize = CON_MAX_LINE_SIZE;                  // Console line size
    int16_t m_lineHeight;                // Height, including spacing

    float m_spacing = CON_MIN_LINE_INTERVAL;                    // Line spacing

    int16_t m_cursorPos;                 // Current cursor position, in symbols
    int16_t m_cursorX;                   // Cursor position in pixels
    int16_t m_cursorY;
    util::Duration m_blinkTime;                // Current cursor draw time
    util::Duration m_blinkPeriod;
    bool m_showCursor;                // Cursor visibility flag

    bool inited = false;                     // Ready-to-use flag
    bool m_isVisible;                       // Visibility flag

    std::string m_editingLine;

    std::vector<std::string> m_completionItems;

    Console();

public:
    void init();

    static Console& instance()
    {
        static Console con_base;
        return con_base;
    }

    ~Console() = default;

    void initFonts();

    void initGlobals();

    void setLineInterval(float interval);

    void draw();

    void drawBackground();

    void drawCursor();

    void filter(const std::string& text);

    void edit(int key, const boost::optional<Uint16>& mod = boost::none);

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

    void setSpacing(float val)
    {
        if(val < CON_MIN_LINE_INTERVAL || val > CON_MAX_LINE_INTERVAL)
            return;
        m_spacing = val;
    }

    void setVisible(bool val)
    {
        m_isVisible = val;
    }

    void setShowCursorPeriod(util::Duration val)
    {
        m_blinkPeriod = val;
    }

    void setLineSize(uint16_t val)
    {
        m_lineSize = val;
    }

    void setVisibleLines(size_t val)
    {
        m_visibleLines = val;
        setCursorY(static_cast<int16_t>(m_visibleLines * m_lineHeight));
    }

    size_t visibleLines() const
    {
        return m_visibleLines;
    }

    void setBufferSize(size_t val)
    {
        m_bufferSize = val;
    }

    void setHistorySize(size_t val)
    {
        m_historySize = val;
    }

    uint16_t lineSize() const
    {
        return m_lineSize;
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
