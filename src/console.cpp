
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#if defined(__MACOSX__)
#include <OpenGL/glext.h>
#endif
#include <SDL2/SDL_keycode.h>

#include "gl_font.h"
#include "console.h"
#include "system.h"
#include "engine.h"
#include "script.h"
#include "shader_manager.h"
#include "gui.h"
#include "vmath.h"

void ConsoleInfo::init()
{
    // log size check
    if(m_historyLines.size() > CON_MAX_LOG)
        m_historyLines.resize(CON_MAX_LOG);

    // spacing check
    if(m_spacing < CON_MIN_LINE_INTERVAL)
        m_spacing = CON_MIN_LINE_INTERVAL;
    else if(m_spacing > CON_MAX_LINE_INTERVAL)
        m_spacing = CON_MAX_LINE_INTERVAL;

    // linesize check
    if(m_lineSize < CON_MIN_LINE_SIZE)
        m_lineSize = CON_MIN_LINE_SIZE;
    else if(m_lineSize > CON_MAX_LINE_SIZE)
        m_lineSize = CON_MAX_LINE_SIZE;

    inited = true;
}

void ConsoleInfo::initFonts() {
    m_font = FontManager->GetFont(FONT_CONSOLE);
    setLineInterval(m_spacing);
}

void ConsoleInfo::initGlobals() {
    m_backgroundColor[0] = 1.0;
    m_backgroundColor[1] = 0.9;
    m_backgroundColor[2] = 0.7;
    m_backgroundColor[3] = 0.4;

    m_spacing         = CON_MIN_LINE_INTERVAL;
    m_lineSize       = CON_MIN_LINE_SIZE;

    m_blinkPeriod = 0.5;
}

void ConsoleInfo::setLineInterval(float interval) {
    if(!inited || !FontManager ||
            (interval < CON_MIN_LINE_INTERVAL) || (interval > CON_MAX_LINE_INTERVAL))
    {
        return; // nothing to do
    }

    inited = false;
    m_spacing = interval;
    // font->font_size has absolute size (after scaling)
    m_lineHeight = (1.0 + m_spacing) * m_font->font_size;
    m_cursorX = 8 + 1;
    m_cursorY = screen_info.h - m_lineHeight * m_lines.size();
    if(m_cursorY < 8)
    {
        m_cursorY = 8;
    }
    inited = true;
}

void ConsoleInfo::draw() {
    if(!FontManager || !inited || !m_isVisible)
        return;

    drawBackground();
    drawCursor();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::shared_ptr<TextShaderDescription> shader = renderer.shaderManager()->getTextShader();
    glUseProgramObjectARB(shader->program);
    glUniform1iARB(shader->sampler, 0);
    GLfloat screenSize[2] = {
        static_cast<GLfloat>(screen_info.w),
        static_cast<GLfloat>(screen_info.h)
    };
    glUniform2fvARB(shader->screenSize, 2, screenSize);

    int x = 8;
    int y = m_cursorY;
    size_t n = 0;
    for(const Line& line : m_lines)
    {
        GLfloat *col = FontManager->GetFontStyle(line.styleId)->real_color;
        y += m_lineHeight;
        std::copy(col, col+4, m_font->gl_font_color);
        glf_render_str(m_font, (GLfloat)x, (GLfloat)y, line.text.c_str());
        ++n;
        if( n >= m_visibleLines )
            break;
    }
}

void ConsoleInfo::drawBackground() {
    /*
         * Draw console background to see the text
         */
    Gui_DrawRect(0.0, m_cursorY + m_lineHeight - 8, screen_info.w, screen_info.h, m_backgroundColor, m_backgroundColor, m_backgroundColor, m_backgroundColor, BM_SCREEN);

    /*
         * Draw finalise line
         */
    GLfloat white[4] = { 1.0f, 1.0f, 1.0f, 0.7 };
    Gui_DrawRect(0.0, m_cursorY + m_lineHeight - 8, screen_info.w, 2, white, white, white, white, BM_SCREEN);
}

void ConsoleInfo::drawCursor() {
    GLint y = m_cursorY;

    if(m_blinkPeriod)
    {
        m_blinkTime += engine_frame_time;
        if(m_blinkTime > m_blinkPeriod)
        {
            m_blinkTime = 0.0;
            m_showCursor = !m_showCursor;
        }
    }

    if(m_showCursor)
    {
        GLfloat white[4] = { 1.0f, 1.0f, 1.0f, 0.7 };
        Gui_DrawRect(m_cursorX,
                     y + m_lineHeight * 0.9,
                     1,
                     m_lineHeight * 0.8,
                     white, white, white, white,
                     BM_SCREEN);
    }
}

void ConsoleInfo::filter(const std::string &text) {
    for(char c : text) {
        edit(c);
    }
}

void ConsoleInfo::edit(int key) {
    if(key == SDLK_UNKNOWN || key == SDLK_BACKQUOTE || key == SDLK_BACKSLASH || !inited)
    {
        return;
    }

    if(key == SDLK_RETURN)
    {
        addLog(currentLine());
        if(!Engine_ExecCmd(currentLine().c_str()))
            currentLine() = std::string();
        m_cursorPos = 0;
        m_cursorX = 8 + 1;
        return;
    }

    m_blinkTime = 0.0;
    m_showCursor = 1;

    int16_t oldLength = utf8_strlen(currentLine().c_str());    // int16_t is absolutly enough

    switch(key)
    {
    case SDLK_UP:
        Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUPAGE));
        currentLine() = m_historyLines[m_historyPos];
        m_historyPos++;
        if(m_historyPos >= m_historyLines.size())
        {
            m_historyPos = 0;
        }
        m_cursorPos = utf8_strlen(currentLine().c_str());
        break;

    case SDLK_DOWN:
        Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUPAGE));
        currentLine() = m_historyLines[m_historyPos];
        if(m_historyPos == 0)
        {
            m_historyPos = m_lines.size() - 1;
        }
        else
        {
            m_historyPos--;
        }
        m_cursorPos = utf8_strlen(currentLine().c_str());
        break;

    case SDLK_LEFT:
        if(m_cursorPos > 0)
        {
            m_cursorPos--;
        }
        break;

    case SDLK_RIGHT:
        if(m_cursorPos < oldLength)
        {
            m_cursorPos++;
        }
        break;

    case SDLK_HOME:
        m_cursorPos = 0;
        break;

    case SDLK_END:
        m_cursorPos = oldLength;
        break;

    case SDLK_BACKSPACE:
        if(m_cursorPos > 0) {
            currentLine().erase(m_cursorPos-1);
            m_cursorPos--;
        }
        break;

    case SDLK_DELETE:
        if(m_cursorPos < oldLength) {
            currentLine().erase(m_cursorPos);
        }
        break;

    default:
        if((oldLength < m_lineSize-1) && (key >= SDLK_SPACE)) {
            currentLine().insert(currentLine().begin() + m_cursorPos, char(key));
            m_cursorPos++;
        }
        break;
    }

    calcCursorPosition();
}

void ConsoleInfo::calcCursorPosition() {
    if(m_font) {
        m_cursorX = 8 + 1 + glf_get_string_len(m_font, currentLine().c_str(), m_cursorPos);
    }
}

void ConsoleInfo::addLog(const std::string &text) {
    if(inited && !text.empty()) {
        m_historyLines.insert(m_historyLines.begin(), text);
        if( m_historyLines.size() > m_bufferSize)
            m_historyLines.resize(m_bufferSize);
        m_historyPos = 0;
    }
}

void ConsoleInfo::addLine(const std::string &text, font_Style style) {
    if(inited && !text.empty()) {
        m_lines.emplace_front(text, style);
        m_historyPos = 0;
    }
}

void ConsoleInfo::addText(const std::string &text, font_Style style) {
    size_t pos = 0;
    while(pos != std::string::npos) {
        size_t end = text.find_first_of("\r\n", pos);
        if( end != pos+1 )
            addLine(text.substr(pos, end-pos-1), style);
        pos = end+1;
    }
}

void ConsoleInfo::printf(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, 4096, fmt, argptr);
    buf[4096-1] = 0;
    va_end(argptr);
    addLine(buf, FONTSTYLE_CONSOLE_NOTIFY);
}

void ConsoleInfo::warning(int warn_string_index, ...)
{
    va_list argptr;
    char buf[4096];
    char fmt[256];

    lua_GetSysNotify(engine_lua, warn_string_index, 256, fmt);

    va_start(argptr, warn_string_index);
    vsnprintf(buf, 4096, (const char*)fmt, argptr);
    buf[4096-1] = 0;
    va_end(argptr);
    addLine(buf, FONTSTYLE_CONSOLE_WARNING);
}

void ConsoleInfo::notify(int notify_string_index, ...)
{
    va_list argptr;
    char buf[4096];
    char fmt[256];

    lua_GetSysNotify(engine_lua, notify_string_index, 256, fmt);

    va_start(argptr, notify_string_index);
    vsnprintf(buf, 4096, (const char*)fmt, argptr);
    buf[4096-1] = 0;
    va_end(argptr);
    addLine(buf, FONTSTYLE_CONSOLE_NOTIFY);
}

void ConsoleInfo::clean() {
    m_lines.clear();
}
