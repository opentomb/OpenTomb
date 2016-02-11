#include "console.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>

#include "engine/engine.h"
#include "engine/system.h"
#include "gl_font.h"
#include "gui.h"
#include "render/shader_manager.h"
#include "script/script.h"

#include <boost/log/trivial.hpp>

using namespace gui;

Console::Console(engine::Engine* engine, boost::property_tree::ptree& config)
    : m_engine(engine)
{
    m_backgroundColor[0] = util::getSetting(config, "backgroundColor.r", 0.0f);
    m_backgroundColor[1] = util::getSetting(config, "backgroundColor.g", 0.0f);
    m_backgroundColor[2] = util::getSetting(config, "backgroundColor.b", 0.0f);
    m_backgroundColor[3] = util::getSetting(config, "backgroundColor.a", 0.8f);

    setShowCursorPeriod(util::MilliSeconds( util::getSetting(config, "blinkPeriod", 500) ));

    setSpacing(util::getSetting(config, "spacing", 0.5f));

    setLineLength(util::getSetting(config, "lineLength", 72));

    setVisibleLines(util::getSetting(config, "visibleLines", 40));

    setHistorySize(util::getSetting(config, "historySize", 16));

    setBufferSize(util::getSetting(config, "bufferSize", 128));

    setVisible(util::getSetting(config, "show", false));
}

void Console::init()
{
    // log size check
    if(m_historyLines.size() > HistorySizeMax)
        m_historyLines.resize(HistorySizeMax);

    // spacing check
    if(m_spacing < SpacingMin)
        m_spacing = SpacingMin;
    else if(m_spacing > SpacingMax)
        m_spacing = SpacingMax;

    // linesize check
    if(m_lineLength < LineLengthMin)
        m_lineLength = LineLengthMin;
    else if(m_lineLength > LineLengthMax)
        m_lineLength = LineLengthMax;

    inited = true;
}

void Console::initFonts()
{
    m_font = m_engine->m_gui.m_fontManager.getFont(FontType::Console);
    setSpacing(m_spacing);
}

void Console::draw()
{
    if(!inited || !m_isVisible)
        return;

    drawBackground();
    drawCursor();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render::TextShaderDescription* shader = m_engine->renderer.shaderManager()->getTextShader();
    glUseProgram(shader->program);
    glUniform1i(shader->sampler, 0);
    GLfloat screenSize[2] = {
        static_cast<GLfloat>(m_engine->m_screenInfo.w),
        static_cast<GLfloat>(m_engine->m_screenInfo.h)
    };
    glUniform2fv(shader->screenSize, 1, screenSize);

    const int x = 8;
    int y = m_cursorY + m_lineHeight;
    size_t n = 0;
    for(const Line& line : m_lines)
    {
        const glm::vec4& col = m_engine->m_gui.m_fontManager.getFontStyle(line.styleId)->real_color;
        y += m_lineHeight;
        m_font->gl_font_color = col;
        glf_render_str(m_font, static_cast<GLfloat>(x), static_cast<GLfloat>(y), line.text.c_str());
        ++n;
        if(n >= m_visibleLines)
            break;
    }
    const glm::vec4& col = m_engine->m_gui.m_fontManager.getFontStyle(FontStyle::ConsoleInfo)->real_color;
    m_font->gl_font_color = col;
    glf_render_str(m_font, static_cast<GLfloat>(x), static_cast<GLfloat>(m_cursorY) + m_lineHeight, m_editingLine.c_str());
}

void Console::drawBackground()
{
    /*
         * Draw console background to see the text
         */
    m_engine->m_gui.drawRect(0.0, m_cursorY + m_lineHeight - 8, m_engine->m_screenInfo.w, m_engine->m_screenInfo.h, m_backgroundColor, m_backgroundColor, m_backgroundColor, m_backgroundColor, loader::BlendingMode::Screen);

    /*
         * Draw finalise line
         */
    glm::vec4 white{ 1, 1, 1, 0.7f };
    m_engine->m_gui.drawRect(0.0, m_cursorY + m_lineHeight - 8, m_engine->m_screenInfo.w, 2, white, white, white, white, loader::BlendingMode::Screen);
}

void Console::drawCursor()
{
    GLint y = m_cursorY;

    if(m_blinkPeriod.count() > 0)
    {
        m_blinkTime += m_engine->getFrameTime();
        if(m_blinkTime > m_blinkPeriod)
        {
            m_blinkTime = util::Duration(0);
            m_showCursor = !m_showCursor;
        }
    }

    if(m_showCursor)
    {
        glm::vec4 white{ 1, 1, 1, 0.7f };
        m_engine->m_gui.drawRect(m_cursorX,
                                y + m_lineHeight * 0.9f,
                                1,
                                m_lineHeight * 0.8f,
                                white, white, white, white,
                                loader::BlendingMode::Screen);
    }
}

void Console::filter(const std::string& text)
{
    for(char c : text)
    {
        edit(c);
    }
}

namespace
{
std::string toLower(std::string str)
{
    for(char& c : str)
        c = std::tolower(c);
    return str;
}

bool startsWithLowercase(const std::string& haystack, const std::string& needle)
{
    return toLower(haystack.substr(0, needle.length())) == toLower(needle);
}
}

void Console::edit(int key, const boost::optional<uint16_t>& mod)
{
    if(key == SDLK_UNKNOWN || key == SDLK_BACKQUOTE || key == SDLK_BACKSLASH || !inited)
    {
        return;
    }

    if(key == SDLK_RETURN)
    {
        addLog(m_editingLine);
        addLine(std::string("> ") + m_editingLine, FontStyle::ConsoleInfo);
        m_engine->execCmd(m_editingLine.c_str());
        m_editingLine.clear();
        m_cursorPos = 0;
        m_cursorX = 8 + 1;
        return;
    }

    m_blinkTime = util::Duration(0);
    m_showCursor = true;

    const auto oldLength = utf8_strlen(m_editingLine.c_str()); // int16_t is absolutly enough

    switch(key)
    {
        case SDLK_UP:
        case SDLK_DOWN:
            if(m_historyLines.empty())
                break;
            m_engine->m_audioEngine.send(m_engine->m_scriptEngine.getGlobalSound(audio::GlobalSoundId::MenuPage));
            if(key == SDLK_UP && m_historyPos < m_historyLines.size())
                ++m_historyPos;
            else if(key == SDLK_DOWN && m_historyPos > 0)
                --m_historyPos;
            if(m_historyPos > 0)
                m_editingLine = m_historyLines[m_historyPos - 1];
            else
                m_editingLine.clear();
            m_cursorPos = utf8_strlen(m_editingLine.c_str());
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
            if(m_cursorPos > 0)
            {
                m_editingLine.erase(m_cursorPos - 1, 1);
                m_cursorPos--;
            }
            break;

        case SDLK_DELETE:
            if(m_cursorPos < oldLength)
            {
                m_editingLine.erase(m_cursorPos, 1);
            }
            break;

        case SDLK_TAB:
        {
            std::string needle = m_editingLine.substr(0, m_cursorPos);
            // find auto-completion terms, case-insensitive
            std::vector<std::string> found;
            std::copy_if(m_completionItems.begin(), m_completionItems.end(), std::back_inserter(found),
                         [needle](const std::string& completion)
            {
                return startsWithLowercase(completion, needle);
            });
            if(found.empty())
            {
                // no completion, do nothing
            }
            else if(found.size() == 1)
            {
                // if we have only one term found, use it!
                m_editingLine.erase(0, found[0].length());
                m_editingLine.insert(0, found[0]);
                m_cursorPos = found[0].length();
            }
            else
            {
                // else we must find the common completion string
                for(std::string& term : found)
                {
                    // cut off the needle part
                    term.erase(0, needle.length());
                }
                // now find a common start
                std::string common = found[0];
                for(size_t i = 1; !common.empty() && i < found.size(); ++i)
                {
                    // cut off from the end that's not common with current
                    for(size_t j = 0; j < std::min(common.length(), found[i].length()); ++j)
                    {
                        if(std::tolower(common[j]) != std::tolower(found[i][j]))
                        {
                            common.erase(j);
                            break;
                        }
                    }
                }
                if(common.empty())
                {
                    // nothing common, print possible completions
                    addLine("Possible completions:", FontStyle::ConsoleInfo);
                    for(const std::string& term : found)
                        addLine(std::string("* ") + needle + term, FontStyle::ConsoleInfo);
                }
                else
                {
                    m_editingLine.insert(m_cursorPos, common);
                    m_cursorPos += common.length();
                }
            }
        }
        break;

        default:
            if(key == SDLK_v && mod && (*mod & KMOD_CTRL))
            {
                if(char* clipboard = SDL_GetClipboardText())
                {
                    const auto textLength = utf8_strlen(clipboard);
                    if(oldLength < m_lineLength - textLength)
                    {
                        m_editingLine.insert(m_cursorPos, clipboard);
                        m_cursorPos += textLength;
                    }
                    SDL_free(clipboard);
                }
            }
            else if(!mod && oldLength + 1 < m_lineLength && key >= SDLK_SPACE)
            {
                m_editingLine.insert(m_editingLine.begin() + m_cursorPos, char(key));
                m_cursorPos++;
            }
            break;
    }

    calcCursorPosition();
}

void Console::calcCursorPosition()
{
    if(m_font)
    {
        m_cursorX = static_cast<int16_t>(8 + 1 + glf_get_string_len(m_font, m_editingLine.c_str(), m_cursorPos));
    }
}

void Console::addLog(const std::string& text)
{
    if(inited && !text.empty())
    {
        m_historyLines.insert(m_historyLines.begin(), text);
        if(m_historyLines.size() > m_bufferSize)
            m_historyLines.resize(m_bufferSize);
        m_historyPos = 0;
    }
}

void Console::addLine(const std::string& text, FontStyle style)
{
    if(inited && !text.empty())
    {
        BOOST_LOG_TRIVIAL(info) << "CON: " << text;
        m_lines.emplace_front(text, style);
        m_historyPos = 0;
    }
}

void Console::addText(const std::string& text, FontStyle style)
{
    size_t position = 0;
    while(position != std::string::npos)
    {
        size_t end = text.find_first_of("\r\n", position);
        if(end != position + 1)
            addLine(text.substr(position, end - position - 1), style);
        if(end == std::string::npos)
            break;
        position = end + 1;
    }
}

void Console::printf(const char* fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, 4096, fmt, argptr);
    buf[4096 - 1] = 0;
    va_end(argptr);
    addLine(buf, FontStyle::ConsoleNotify);
}

void Console::warning(int warn_string_index, ...)
{
    va_list argptr;
    char buf[4096];
    char fmt[256];

    m_engine->m_scriptEngine.getSysNotify(warn_string_index, 256, fmt);

    va_start(argptr, warn_string_index);
    vsnprintf(buf, 4096, static_cast<const char*>(fmt), argptr);
    buf[4096 - 1] = 0;
    va_end(argptr);
    addLine(buf, FontStyle::ConsoleWarning);
}

void Console::notify(int notify_string_index, ...)
{
    va_list argptr;
    char buf[4096];
    char fmt[256];

    m_engine->m_scriptEngine.getSysNotify(notify_string_index, 256, fmt);

    va_start(argptr, notify_string_index);
    vsnprintf(buf, 4096, static_cast<const char*>(fmt), argptr);
    buf[4096 - 1] = 0;
    va_end(argptr);
    addLine(buf, FontStyle::ConsoleNotify);
}

void Console::clean()
{
    m_lines.clear();
}

void Console::setSpacing(float val)
{
    if(val < SpacingMin || val > SpacingMax)
        return;

    m_spacing = val;

    if(!inited)
    {
        return; // nothing to do
    }

    inited = false;
    // font->font_size has absolute size (after scaling)
    m_lineHeight = static_cast<int16_t>((1 + m_spacing) * m_font->font_size);
    m_cursorX = 8 + 1;
    m_cursorY = static_cast<int16_t>(m_engine->m_screenInfo.h - m_lineHeight * m_visibleLines);
    if(m_cursorY < 8)
    {
        m_cursorY = 8;
    }
    inited = true;
}
