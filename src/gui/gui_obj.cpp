
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "../core/gl_util.h"
#include "../core/gl_font.h"
#include "../core/system.h"
#include "../core/console.h"
#include "../core/vmath.h"
#include "gui_obj.h"


static int Gui_CheckObjectsRects(gui_object_p parent, gui_object_p obj)
{
    if(parent->x > obj->x + obj->w || parent->x + parent->w < obj->x ||
       parent->y > obj->y + obj->h || parent->y + parent->h < obj->y)
    {
        return 0;
    }

    if(obj->x >= parent->x && obj->x + obj->w <= parent->x + parent->w &&
       obj->y >= parent->y && obj->y + obj->h <= parent->y + parent->h)
    {
        return 2;
    }

    return 1;
}


gui_object_p Gui_CreateObject()
{
    gui_object_p ret = (gui_object_p)calloc(1, sizeof(gui_object_t));
    ret->text = NULL;
    ret->line_height = 1.0f;
    return ret;
}

void Gui_DeleteObject(gui_object_p obj)
{
    if(obj)
    {
        if(obj->text)
        {
            obj->flags.draw_label = 0x00;
            obj->text_size = 0;
            free(obj->text);
            obj->text = NULL;
        }
        free(obj);
    }
}

void Gui_DeleteObjects(gui_object_p root)
{
    for(gui_object_p obj = root->childs; obj; )
    {
        gui_object_p next_obj = obj->next;
        Gui_DeleteObjects(obj);
        obj = next_obj;
    }
    Gui_DeleteObject(root);
}

gui_object_p Gui_CreateChildObject(gui_object_p root)
{
    gui_object_p ret = NULL;
    if(root)
    {
        gui_object_p *ins = &root->childs;
        ret = Gui_CreateObject();
        ret->parent = root;
        for(; *ins; ins = &((*ins)->next))
        {
            ret->prev = *ins;
        }
        *ins = ret;
    }

    return ret;
}

void Gui_DeleteChildObject(gui_object_p obj)
{
    if(obj && obj->parent)
    {
        gui_object_p *ins = &obj->parent->childs;
        for(; *ins; ins = &((*ins)->next))
        {
            if(*ins == obj)
            {
                if(obj->prev)
                {
                    obj->prev->next = obj->next;
                }
                *ins = obj->next;
                Gui_DeleteObject(obj);
                break;
            }
        }
    }
}

void Gui_SetObjectLabel(gui_object_p obj, const char *text, uint16_t font_id, uint16_t style_id)
{
    obj->font_id = font_id;
    obj->style_id = style_id;

    if(!text && obj->text)
    {
        obj->text[0] = 0;
        return;
    }

    if(text)
    {
        size_t len = strlen(text) + 1;
        if(obj->text_size < len)
        {
            char *old_ptr = obj->text;
            obj->text_size = len + 8 - (len % 8);
            obj->text = (char*)malloc(obj->text_size * sizeof(char));
            free(old_ptr);
        }
        strncpy(obj->text, text, len);
    }
}


#define to_float_cl(fv, cv) {(fv)[0] = (float)(cv)[0] / 255.0f; \
(fv)[1] = (float)(cv)[1] / 255.0f; \
(fv)[2] = (float)(cv)[2] / 255.0f; \
(fv)[3] = (float)(cv)[3] / 255.0f; }

static void Gui_DrawBackgroundInternal(gui_object_p root)
{
    GLfloat x0 = root->x + root->border_width;
    GLfloat y0 = root->y + root->border_width;
    GLfloat x1 = root->x + root->w - root->border_width;
    GLfloat y1 = root->y + root->h - root->border_width;
    GLfloat *v, backgroundArray[32];

    v = backgroundArray;
    *v++ = x0; *v++ = y0;
    to_float_cl(v, root->color_background);
    v += 4;
    *v++ = 0.0; *v++ = 0.0;

    *v++ = x1; *v++ = y0;
    to_float_cl(v, root->color_background);
    v += 4;
    *v++ = 0.0; *v++ = 0.0;

    *v++ = x1; *v++ = y1;
    to_float_cl(v, root->color_background);
    v += 4;
    *v++ = 0.0; *v++ = 0.0;

    *v++ = x0; *v++ = y1;
    to_float_cl(v, root->color_background);
    v += 4;
    *v++ = 0.0; *v++ = 0.0;

    qglVertexPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), backgroundArray);
    qglColorPointer(4, GL_FLOAT, 8 * sizeof(GLfloat), backgroundArray + 2);
    qglTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), backgroundArray + 6);
    qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

static void Gui_DrawBorderInternal(gui_object_p root)
{
    GLfloat *v, borderArray[80];
    GLfloat x0 = root->x + root->border_width;
    GLfloat y0 = root->y + root->border_width;
    GLfloat x1 = root->x + root->w - root->border_width;
    GLfloat y1 = root->y + root->h - root->border_width;

    v = borderArray;
   *v++ = x0; *v++ = y1;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = root->x; *v++ = root->y + root->h;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = x0; *v++ = y0;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = root->x; *v++ = root->y;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = x1; *v++ = y0;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = root->x + root->w; *v++ = root->y;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = x1; *v++ = y1;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = root->x + root->w; *v++ = root->y + root->h;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = x0; *v++ = y1;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

   *v++ = root->x; *v++ = root->y + root->h;
    to_float_cl(v, root->color_border);
    v += 4;
   *v++ = 0.0; *v++ = 0.0;

    qglVertexPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), borderArray);
    qglColorPointer(4, GL_FLOAT, 8 * sizeof(GLfloat), borderArray + 2);
    qglTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), borderArray + 6);
    qglDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
}

static void Gui_DrawLabelInternal(gui_object_p root)
{
    gl_tex_font_p gl_font = NULL;
    gl_fontstyle_p style = NULL;
    
    if((gl_font = GLText_GetFont(root->font_id)) && (style = GLText_GetFontStyle(root->style_id)))
    {
        GLfloat real_x = 0.0f, real_y = 0.0f;
        int32_t x0, y0, x1, y1;
        GLfloat shadow_color[4];
        int32_t w_pt = ((root->w - 2 * root->border_width) * 64.0f + 0.5f);
        GLfloat ascender = glf_get_ascender(gl_font) / 64.0f;
        GLfloat descender = glf_get_descender(gl_font) / 64.0f;
        GLfloat dy = root->line_height * (ascender - descender);
        int n_lines = 1;
        char *begin = root->text;
        char *end = begin;
        
        shadow_color[0] = 0.0f;
        shadow_color[1] = 0.0f;
        shadow_color[2] = 0.0f;
        shadow_color[3] = (float)style->font_color[3] * GUI_FONT_SHADOW_TRANSPARENCY;

        if(root->flags.word_wrap)
        {
            int n_sym = 0;
            n_lines = 0;
            for(char *ch = glf_get_string_for_width(gl_font, root->text, w_pt, &n_sym); *begin; ch = glf_get_string_for_width(gl_font, ch, w_pt, &n_sym))
            {
                ++n_lines;
                begin = ch;
            }
            begin = root->text;
            x1 = x0 + w_pt;
            y1 = y0 + n_lines * dy * 64.0f;
        }
        
        real_y = root->y - root->border_width - descender;
        switch(root->v_align)
        {
            case GLTEXT_ALIGN_TOP:
                real_y = root->y - root->border_width - ascender - dy * (n_lines - 1);
                break;
            case GLTEXT_ALIGN_CENTER:
                real_y = root->y - descender + (root->h - dy * n_lines) / 2;
                break;
        }

        for(int line = n_lines - 1; line >= 0; --line)
        {
            int n_sym = -1;
            if(n_lines > 1)
            {
                end = glf_get_string_for_width(gl_font, begin, w_pt, &n_sym);
            }
            
            glf_get_string_bb(gl_font, begin, n_sym, &x0, &y0, &x1, &y1);
            
            real_x = root->x + root->border_width - x0 / 64.0f;
            switch(root->h_align)
            {
                case GLTEXT_ALIGN_RIGHT:
                    real_x = root->x + root->w - root->border_width - x1 / 64.0f;
                    break;
                case GLTEXT_ALIGN_CENTER:
                    real_x = root->x + root->w / 2 - (x1 + x0) / 128.0f;
                    break;
            }
        
            if(style->shadowed)
            {
                vec4_copy(gl_font->gl_font_color, shadow_color);
                glf_render_str(gl_font,
                               (real_x + GUI_FONT_SHADOW_HORIZONTAL_SHIFT),
                               (real_y + line * dy + GUI_FONT_SHADOW_VERTICAL_SHIFT),
                               begin, n_sym);
            }
            vec4_copy(gl_font->gl_font_color, style->font_color);
            glf_render_str(gl_font, real_x, real_y + line * dy, begin, n_sym);
            begin = end;
        }
    }
}

static void Gui_DrawObjectsInternal(gui_object_p root, int stencil)
{
    if(!root->flags.hide)
    {
        if(root->flags.draw_background)
        {
            Gui_DrawBackgroundInternal(root);
        }

        if(root->flags.draw_border && root->border_width)
        {
            Gui_DrawBorderInternal(root);
        }

        if(root->text && root->flags.draw_label)
        {
            Gui_DrawLabelInternal(root);
        }

        if(root->flags.clip_children && root->childs)
        {
            uint8_t a = root->color_background[3];
            root->color_background[3] = 0;
            qglStencilFunc(GL_EQUAL, stencil, 0xFF);
            qglStencilOp(GL_KEEP, GL_INCR, GL_INCR);
            Gui_DrawBackgroundInternal(root);
            qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            root->color_background[3] = a;
            ++stencil;
        }

        for(gui_object_p obj = root->childs; obj; obj = obj->next)
        {
            obj->x += root->x + root->content_dx;
            obj->y += root->y + root->content_dy;
            if(Gui_CheckObjectsRects(root, obj))
            {
                if(root->childs)
                {
                    qglStencilFunc(GL_EQUAL, stencil, 0xFF);
                }
                Gui_DrawObjectsInternal(obj, stencil);
            }
            obj->x -= root->x + root->content_dx;
            obj->y -= root->y + root->content_dy;
        }

        if(root->flags.clip_children && root->childs)
        {
            uint8_t a = root->color_background[3];
            root->color_background[3] = 0;
            qglStencilFunc(GL_GREATER, stencil - 1, 0xFF);
            qglStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
            Gui_DrawBackgroundInternal(root);
            root->color_background[3] = a;
            qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            qglStencilFunc(GL_ALWAYS, 0x00, 0xFF);
        }
    }
}

void Gui_DrawObjects(gui_object_p root)
{
    if(root)
    {
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        BindWhiteTexture();
        qglEnable(GL_STENCIL_TEST);
        qglClear(GL_STENCIL_BUFFER_BIT);
        qglStencilFunc(GL_ALWAYS, 0, 0xFF);
        qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        Gui_DrawObjectsInternal(root, 0);
        qglDisable(GL_STENCIL_TEST);
    }
}

void Gui_LayoutVertical(gui_object_p root, int16_t spacing, int16_t margin)
{
    gui_object_p prev = NULL;
    root->content_w = 0;
    root->content_h = 0;
    for(gui_object_p obj = root->childs; obj; obj = obj->next)
    {
        if(!obj->flags.hide)
        {
            obj->y = (prev) ? (prev->y - obj->h - spacing)
                            : (root->h - root->border_width - obj->h - margin);
            prev = obj;
            if(obj->w + obj->x > root->content_w)
            {
                root->content_w = obj->w + obj->x;
            }
        }
    }
    
    if(prev)
    {
        root->content_h = root->h - prev->y;
    }
}

void Gui_LayoutHorizontal(gui_object_p root, int16_t spacing, int16_t margin)
{
    gui_object_p prev = NULL;
    root->content_w = 0;
    root->content_h = 0;
    for(gui_object_p obj = root->childs; obj; obj = obj->next)
    {
        if(!obj->flags.hide)
        {
            obj->x = (prev) ? (prev->x + prev->w + spacing)
                            : (root->border_width + margin);
            prev = obj;
            if(obj->w + obj->x > root->content_w)
            {
                root->content_w = obj->w + obj->x;
            }
            if(root->h - obj->y > root->content_h)
            {
                root->content_h = root->h - obj->y;
            }
        }
    }
}
