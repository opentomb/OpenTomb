#include "textline.h"

#include "engine/system.h"
#include "render/shader_manager.h"
#include "render/render.h"
#include "engine/engine.h"

#include <glm/gtc/type_ptr.hpp>

namespace gui
{
void TextLine::move(float scaleFactor)
{
    offset = position * scaleFactor;
}

TextLineManager::TextLineManager(engine::Engine* engine)
    : m_engine(engine)
{
}

void TextLineManager::renderLine(const TextLine& line)
{
    glm::float_t real_x = 0.0, real_y = 0.0;

    FontTexture* gl_font = m_engine->m_gui.m_fontManager.getFont(line.fontType);
    FontStyleData* style = m_engine->m_gui.m_fontManager.getFontStyle(line.fontStyle);

    if(gl_font == nullptr || style == nullptr || !line.show || style->hidden)
    {
        return;
    }

    glf_get_string_bb(gl_font, line.text.c_str(), -1, line.topLeft, line.bottomRight);

    switch(line.Xanchor)
    {
        case HorizontalAnchor::Left:
            real_x = line.offset.x;   // Used with center and right alignments.
            break;
        case HorizontalAnchor::Right:
            real_x = static_cast<float>(m_engine->m_screenInfo.w) - (line.bottomRight.x - line.topLeft.x) - line.offset.x;
            break;
        case HorizontalAnchor::Center:
            real_x = m_engine->m_screenInfo.w / 2.0f - (line.bottomRight.x - line.topLeft.x) / 2.0f + line.offset.x;  // Absolute center.
            break;
    }

    switch(line.Yanchor)
    {
        case VerticalAnchor::Bottom:
            real_y += line.offset.y;
            break;
        case VerticalAnchor::Top:
            real_y = static_cast<float>(m_engine->m_screenInfo.h) - (line.bottomRight.y - line.topLeft.y) - line.offset.y;
            break;
        case VerticalAnchor::Center:
            real_y = m_engine->m_screenInfo.h / 2.0f + (line.bottomRight.y - line.topLeft.y) - line.offset.y;          // Consider the baseline.
            break;
    }

    if(style->shadowed)
    {
        gl_font->gl_font_color[0] = 0.0f;
        gl_font->gl_font_color[1] = 0.0f;
        gl_font->gl_font_color[2] = 0.0f;
        gl_font->gl_font_color[3] = style->color[3] * FontShadowTransparency;// Derive alpha from base color.
        glf_render_str(gl_font,
                       real_x + FontShadowHorizontalShift,
                       real_y + FontShadowVerticalShift,
                       line.text.c_str());
    }

    gl_font->gl_font_color = style->real_color;
    glf_render_str(gl_font, real_x, real_y, line.text.c_str());
}

/**
 * For simple temporary lines rendering.
 * Really all strings will be rendered in Gui_Render() function.
 */
TextLine* TextLineManager::drawText(glm::float_t x, glm::float_t y, const std::string& str)
{
    m_tempLines.emplace_back();
    TextLine* line = &m_tempLines.back();

    line->text = str;
    line->fontType = FontType::Secondary;
    line->fontStyle = FontStyle::Generic;

    line->position = { x, y };
    line->Xanchor = HorizontalAnchor::Left;
    line->Yanchor = VerticalAnchor::Bottom;

    line->offset = line->position * m_engine->m_screenInfo.scale_factor;

    line->show = true;
    return line;
}

void TextLineManager::resizeTextLines()
{
    for(const TextLine* l : m_baseLines)
    {
        l->offset = l->position * m_engine->m_screenInfo.scale_factor;
    }

    for(const TextLine& l : m_tempLines)
    {
        l.offset = l.position * m_engine->m_screenInfo.scale_factor;
    }
}

void TextLineManager::renderStrings()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render::TextShaderDescription *shader = m_engine->renderer.shaderManager()->getTextShader();
    glUseProgram(shader->program);
    glm::vec2 screenSize{
        static_cast<glm::float_t>(m_engine->m_screenInfo.w),
        static_cast<glm::float_t>(m_engine->m_screenInfo.h)
    };
    glUniform2fv(shader->screenSize, 1, glm::value_ptr(screenSize));
    glUniform1i(shader->sampler, 0);

    for(const TextLine* l : m_baseLines)
    {
        renderLine(*l);
    }

    for(const TextLine& l : m_tempLines)
    {
        if(l.show)
        {
            renderLine(l);
        }
    }

    m_tempLines.clear();
}
} // namespace gui