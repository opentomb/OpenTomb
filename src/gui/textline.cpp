#include "textline.h"

#include "engine/system.h"
#include "render/shader_manager.h"

#include <algorithm>

namespace gui
{

namespace
{
std::list<const TextLine*> gui_base_lines;
std::list<TextLine> gui_temp_lines;
} // anonymous namespace

void addLine(const TextLine* line)
{
    gui_base_lines.push_back(line);
}

void deleteLine(const TextLine *line)
{
    gui_base_lines.erase(std::find(gui_base_lines.begin(), gui_base_lines.end(), line));
}

void moveLine(TextLine *line)
{
    line->absXoffset = line->X * engine::screen_info.scale_factor;
    line->absYoffset = line->Y * engine::screen_info.scale_factor;
}

void renderStringLine(const TextLine *l)
{
    GLfloat real_x = 0.0, real_y = 0.0;

    if(fontManager == nullptr)
    {
        return;
    }

    FontTexture* gl_font = fontManager->GetFont(l->font_id);
    FontStyleData* style = fontManager->GetFontStyle(l->style_id);

    if((gl_font == nullptr) || (style == nullptr) || (!l->show) || (style->hidden))
    {
        return;
    }

    glf_get_string_bb(gl_font, l->text.c_str(), -1, l->rect + 0, l->rect + 1, l->rect + 2, l->rect + 3);

    switch(l->Xanchor)
    {
        case HorizontalAnchor::Left:
            real_x = l->absXoffset;   // Used with center and right alignments.
            break;
        case HorizontalAnchor::Right:
            real_x = static_cast<float>(engine::screen_info.w) - (l->rect[2] - l->rect[0]) - l->absXoffset;
            break;
        case HorizontalAnchor::Center:
            real_x = (engine::screen_info.w / 2.0f) - ((l->rect[2] - l->rect[0]) / 2.0f) + l->absXoffset;  // Absolute center.
            break;
    }

    switch(l->Yanchor)
    {
        case VerticalAnchor::Bottom:
            real_y += l->absYoffset;
            break;
        case VerticalAnchor::Top:
            real_y = static_cast<float>(engine::screen_info.h) - (l->rect[3] - l->rect[1]) - l->absYoffset;
            break;
        case VerticalAnchor::Center:
            real_y = (engine::screen_info.h / 2.0f) + (l->rect[3] - l->rect[1]) - l->absYoffset;          // Consider the baseline.
            break;
    }

    // missing texture_coord pointer... GL_TEXTURE_COORD_ARRAY state are enabled here!
    /*if(style->rect)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        GLfloat x0 = l->rect[0] + real_x - style->rect_border * screen_info.w_unit;
        GLfloat y0 = l->rect[1] + real_y - style->rect_border * screen_info.h_unit;
        GLfloat x1 = l->rect[2] + real_x + style->rect_border * screen_info.w_unit;
        GLfloat y1 = l->rect[3] + real_y + style->rect_border * screen_info.h_unit;

        GLfloat rectCoords[8];
        rectCoords[0] = x0; rectCoords[1] = y0;
        rectCoords[2] = x1; rectCoords[3] = y0;
        rectCoords[4] = x1; rectCoords[5] = y1;
        rectCoords[6] = x0; rectCoords[7] = y1;
        color(style->rect_color);
        glVertexPointer(2, GL_FLOAT, 0, rectCoords);
        glDrawArrays(GL_POLYGON, 0, 4);
    }*/

    if(style->shadowed)
    {
        gl_font->gl_font_color[0] = 0.0f;
        gl_font->gl_font_color[1] = 0.0f;
        gl_font->gl_font_color[2] = 0.0f;
        gl_font->gl_font_color[3] = style->color[3] * FontShadowTransparency;// Derive alpha from base color.
        glf_render_str(gl_font,
                       (real_x + FontShadowHorizontalShift),
                       (real_y + FontShadowVerticalShift),
                       l->text.c_str());
    }

    std::copy(style->real_color + 0, style->real_color + 4, gl_font->gl_font_color);
    glf_render_str(gl_font, real_x, real_y, l->text.c_str());
}

/**
 * For simple temporary lines rendering.
 * Really all strings will be rendered in Gui_Render() function.
 */
TextLine *drawText(GLfloat x, GLfloat y, const char *fmt, ...)
{
    if(!fontManager)
        return nullptr;

    va_list argptr;
    gui_temp_lines.emplace_back();
    TextLine* l = &gui_temp_lines.back();

    l->font_id = FontType::Secondary;
    l->style_id = FontStyle::Generic;

    va_start(argptr, fmt);
    char tmpStr[LineDefaultSize];
    vsnprintf(tmpStr, LineDefaultSize, fmt, argptr);
    l->text = tmpStr;
    va_end(argptr);

    l->X = x;
    l->Y = y;
    l->Xanchor = HorizontalAnchor::Left;
    l->Yanchor = VerticalAnchor::Bottom;

    l->absXoffset = l->X * engine::screen_info.scale_factor;
    l->absYoffset = l->Y * engine::screen_info.scale_factor;

    l->show = true;
    return l;
}

void resizeTextLines()
{
    for(const TextLine* l : gui_base_lines)
    {
        l->absXoffset = l->X * engine::screen_info.scale_factor;
        l->absYoffset = l->Y * engine::screen_info.scale_factor;
    }

    for(const TextLine& l : gui_temp_lines)
    {
        l.absXoffset = l.X * engine::screen_info.scale_factor;
        l.absYoffset = l.Y * engine::screen_info.scale_factor;
    }
}

void renderStrings()
{
    if(fontManager == nullptr)
        return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render::TextShaderDescription *shader = render::renderer.shaderManager()->getTextShader();
    glUseProgram(shader->program);
    GLfloat screenSize[2] = {
        static_cast<GLfloat>(engine::screen_info.w),
        static_cast<GLfloat>(engine::screen_info.h)
    };
    glUniform2fv(shader->screenSize, 1, screenSize);
    glUniform1i(shader->sampler, 0);

    for(const TextLine* l : gui_base_lines)
    {
        renderStringLine(l);
    }

    for(const TextLine& l : gui_temp_lines)
    {
        if(l.show)
        {
            renderStringLine(&l);
        }
    }

    gui_temp_lines.clear();
}

} // namespace gui
