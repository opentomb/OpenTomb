#pragma once

#include "fontmanager.h"
#include "gui.h"

#include <GL/glew.h>

#include <string>

namespace gui
{

struct TextLine
{
    std::string                 text;

    FontType                    font_id;
    FontStyle                   style_id;

    GLfloat                     X;
    HorizontalAnchor            Xanchor;
    mutable GLfloat             absXoffset;
    GLfloat                     Y;
    VerticalAnchor              Yanchor;
    mutable GLfloat             absYoffset;

    mutable GLfloat             rect[4];    //x0, yo, x1, y1

    bool                        show;
};

void addLine(const TextLine *line);
void deleteLine(const TextLine* line);
void moveLine(TextLine* line);
void renderStringLine(const TextLine* l);
void renderStrings();

/**
 * Draws text using a FontType::Secondary.
 */
TextLine* drawText(GLfloat x, GLfloat y, const char *fmt, ...);

void resizeTextLines();

} // namespace gui
