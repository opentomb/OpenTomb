#pragma once

#include <cstdint>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "gui.h"
#include "gl_font.h"

#define CON_MIN_LOG 16
#define CON_MAX_LOG 128

#define CON_MIN_LINES 64
#define CON_MAX_LINES 256

#define CON_MIN_LINE_SIZE 80
#define CON_MAX_LINE_SIZE 256

#define CON_MIN_LINE_INTERVAL 0.5
#define CON_MAX_LINE_INTERVAL 4.0

struct ConsoleInfo
{
private:
    struct Line {
        std::string text{};
        font_Style styleId = FONTSTYLE_GENERIC;

        Line() = default;
        Line(const std::string& t, font_Style s = FONTSTYLE_GENERIC)
            : text(t)
            , styleId(s)
        {
        }
    };

    gl_tex_font_s *m_font = nullptr;                       // Texture font renderer
    
    GLfloat m_backgroundColor[4];

    size_t m_historyPos = 0;                    // Current log position
    size_t m_historySize;
    std::vector<std::string> m_historyLines;
    
    std::list<Line> m_lines;
    size_t m_visibleLines;
    size_t m_bufferSize;
    
    uint16_t m_lineSize = CON_MAX_LINE_SIZE;                  // Console line size
    int16_t m_lineHeight;                // Height, including spacing
    
    float m_spacing = CON_MIN_LINE_INTERVAL;                    // Line spacing
    
    int16_t m_cursorPos;                 // Current cursor position, in symbols
    int16_t m_cursorX;                   // Cursor position in pixels
    int16_t m_cursorY;
    float m_blinkTime;                // Current cursor draw time
    float m_blinkPeriod;
    int8_t m_showCursor;                // Cursor visibility flag

    bool inited = false;                     // Ready-to-use flag
    bool m_isVisible;                       // Visibility flag

    std::string& currentLine() {
        return m_lines.front().text;
    }

    ConsoleInfo() = default;

public:
    void init();

    static ConsoleInfo& instance() {
        static ConsoleInfo con_base;
        return con_base;
    }

    ~ConsoleInfo() = default;

    void initFonts();

    void initGlobals();

    void setLineInterval(float interval);

    void draw();

    void drawBackground();

    void drawCursor();

    void filter(const std::string& text);

    void edit(int key);

    void calcCursorPosition();

    void addLog(const std::string& text);

    void addLine(const std::string& text, font_Style style);

    void addText(const std::string& text, font_Style style);

    void printf(const char *fmt, ...);

    void warning(int warn_string_index, ...);

    void notify(int notify_string_index, ...);

    void clean();

    bool isVisible() const {
        return m_isVisible;
    }

    void toggleVisibility() {
        m_isVisible = !m_isVisible;
    }

    int spacing() const {
        return m_spacing;
    }

    void setSpacing(float val) {
        m_spacing = val;
    }

    void setVisible(bool val) {
        m_isVisible = val;
    }

    void setShowCursorPeriod(float val) {
        m_blinkPeriod = val;
    }

    void setLineSize(uint16_t val) {
        m_lineSize = val;
    }

    void setVisibleLines(size_t val) {
        m_visibleLines = val;
    }

    size_t visibleLines() const {
        return m_visibleLines;
    }

    void setBufferSize(size_t val) {
        m_bufferSize = val;
    }

    void setHistorySize(size_t val) {
        m_historySize = val;
    }

    uint16_t lineSize() const {
        return m_lineSize;
    }

    int16_t lineHeight() const {
        return m_lineHeight;
    }

    void setCursorY(int16_t y) {
        m_cursorY = y;
    }

    void setBackgroundColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
        m_backgroundColor[0] = r;
        m_backgroundColor[1] = g;
        m_backgroundColor[2] = b;
        m_backgroundColor[3] = a;
    }
};
