
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "../utf8_32.h"
#include "../gl_util.h"
#include "../gl_font.h"
#include "../gl_text.h"
#include "../vmath.h"
#include "gui_obj.h"

static float g_frame_time = 0.0f;

void Gui_SetFrameTime(float time)
{
    g_frame_time = time;
}

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

static void Gui_HeightForWidth(gui_object_p obj)
{
    gl_tex_font_p gl_font = NULL;
    gui_object_text_p label = obj->label;
    if(label && (gl_font = GLText_GetFont(label->font_id)))
    {
        int32_t ascender = glf_get_ascender(gl_font) * label->line_height / 64.0f;
        int32_t descender = glf_get_descender(gl_font) * label->line_height / 64.0f;
        int32_t dy = ascender - descender;
        int32_t text_h = 0;
        
        if(obj->flags.word_wrap && label->text[0])
        {
            int32_t w_pt = obj->w - obj->margin_left - obj->margin_right;
            int n_sym = 0;
            char *begin = label->text;
            w_pt *= 64;
            for(char *ch = glf_get_string_for_width(gl_font, label->text, w_pt, &n_sym); *begin; ch = glf_get_string_for_width(gl_font, ch, w_pt, &n_sym))
            {
                text_h += dy;
                begin = ch;
            }
        }
        else
        {
            text_h = dy;
        }
        
        obj->h = text_h + obj->margin_top + obj->margin_bottom;
    }
}


gui_object_p Gui_CreateObject()
{
    gui_object_p ret = (gui_object_p)calloc(1, sizeof(gui_object_t));
    ret->label = NULL;
    return ret;
}

void Gui_DeleteObject(gui_object_p obj)
{
    if(obj)
    {
        if(obj->handlers.delete_user_data)
        {
            obj->handlers.delete_user_data(obj->data);
            obj->data = NULL;
        }
        if(obj->label)
        {
            obj->flags.draw_label = 0x00;
            if(obj->label->text && obj->label->text_size)
            {
                free(obj->label->text);
            }
            obj->label->text_size = 0;
            obj->label->text = NULL;
            free(obj->label);
            obj->label = NULL;
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
        
        vec4_copy(ret->color_border, root->color_border);
        vec4_copy(ret->color_background, root->color_background);
        ret->flags.v_self_align = root->flags.v_content_align;
        ret->flags.h_self_align = root->flags.h_content_align;
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
    if(!obj->label)
    {
        obj->label = (gui_object_text_p)calloc(1, sizeof(gui_object_text_t));
        obj->label->line_height = 1.50f;
    }
    obj->label->font_id = font_id;
    obj->label->style_id = style_id;

    if(!text && obj->label->text)
    {
        obj->label->text[0] = 0;
        return;
    }

    if(text)
    {
        size_t len = strlen(text) + 1;
        if(obj->label->text_size < len)
        {
            char *old_ptr = obj->label->text;
            size_t new_len = len + 8 - (len % 8);
            obj->label->text = (char*)calloc(new_len, sizeof(char));
            if(obj->label->text_size)
            {
                free(old_ptr);
            }
            obj->label->text_size = new_len;
        }
        strncpy(obj->label->text, text, len);
    }
}


void Gui_SetExternalObjectLabel(gui_object_p obj, const char *text, uint16_t font_id, uint16_t style_id)
{
    if(!obj->label)
    {
        obj->label = (gui_object_text_p)calloc(1, sizeof(gui_object_text_t));
        obj->label->line_height = 1.50f;
    }
    obj->label->font_id = font_id;
    obj->label->style_id = style_id;

    if(obj->label->text && obj->label->text_size)
    {
        free(obj->label->text);
        obj->label->text_size = 0;
    }

    obj->label->text = (char*)text;
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

static void Gui_DrawCursorInternal(GLint x, GLint y, GLint h, float *time)
{
    *time += g_frame_time;
    if(*time > 1.0f)
    {
        *time = 0.0f;
    }

    if(*time < 0.5f)
    {
        GLfloat line_w = 1.0f;
        GLfloat cursor_array[16];
        GLfloat *v = cursor_array;

       *v++ = (GLfloat)x;
       *v++ = (GLfloat)y - 0.1 * (GLfloat)h;
        v[0] = 1.0; v[1] = 1.0; v[2] = 1.0; v[3] = 0.7;             v += 4;
        v[0] = 0.0; v[1] = 0.0;                                     v += 2;
       *v++ = (GLfloat)x;
       *v++ = (GLfloat)y + 0.7 * (GLfloat)h;
        v[0] = 1.0; v[1]= 1.0; v[2] = 1.0; v[3] = 0.7;              v += 4;
        v[0] = 0.0; v[1] = 0.0;

        BindWhiteTexture();
        qglGetFloatv(GL_LINE_WIDTH, &line_w);
        qglLineWidth(1.0f);        
        qglVertexPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), cursor_array);
        qglColorPointer(4, GL_FLOAT, 8 * sizeof(GLfloat), cursor_array + 2);
        qglTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), cursor_array + 6);
        qglDrawArrays(GL_LINES, 0, 2);
        qglLineWidth(line_w);
    }
}

static void Gui_DrawLabelInternal(gui_object_p root)
{
    gl_tex_font_p gl_font = NULL;
    gl_fontstyle_p style = NULL;
    gui_object_text_p label = root->label;
            
    if((gl_font = GLText_GetFont(label->font_id)) && (style = GLText_GetFontStyle(label->style_id)))
    {
        GLfloat real_x = 0.0f, real_y = 0.0f;
        int32_t x0, y0, x1, y1;
        GLfloat shadow_color[4];
        int32_t awailable_w = root->w - root->margin_left - root->margin_right;
        int32_t w_pt = awailable_w * 64;
        int32_t ascender = label->line_height * glf_get_ascender(gl_font) / 64.0f;
        int32_t descender = label->line_height * glf_get_descender(gl_font) / 64.0f;
        int32_t dy = ascender - descender;
        int n_lines = 1;
        int total_chars = 0;
        char *begin = label->text;
        char *end = begin;

        shadow_color[0] = 0.0f;
        shadow_color[1] = 0.0f;
        shadow_color[2] = 0.0f;
        shadow_color[3] = (float)style->font_color[3] * GUI_FONT_SHADOW_TRANSPARENCY;
        
        if(root->flags.word_wrap && root->label->text[0])
        {
            int n_sym = 0;
            n_lines = 0;
            for(char *ch = glf_get_string_for_width(gl_font, label->text, w_pt, &n_sym); *begin; ch = glf_get_string_for_width(gl_font, ch, w_pt, &n_sym))
            {
                ++n_lines;
                begin = ch;
            }
            begin = label->text;
            x1 = w_pt;
            y1 = n_lines * dy * 64;
        }

        switch(root->flags.v_content_align)
        {
            case GUI_ALIGN_TOP:
                real_y = root->y + root->h - root->margin_top - ascender - dy * (n_lines - 1);
                break;
            case GUI_ALIGN_CENTER:
                real_y = root->y + (root->margin_bottom - root->margin_top) / 2 - descender + (root->h - dy * n_lines) / 2;
                break;
            default:
                real_y = root->y + root->margin_bottom - descender;
                break;
        }

        total_chars = 0;
        for(int line = n_lines - 1; line >= 0; --line)
        {
            int n_sym = 0;
            end = glf_get_string_for_width(gl_font, begin, w_pt, &n_sym);
            
            glf_get_string_bb(gl_font, begin, n_sym, &x0, &y0, &x1, &y1);

            switch(root->flags.h_content_align)
            {
                case GUI_ALIGN_RIGHT:
                    real_x = root->x + root->w - root->margin_right - x1 / 64.0f;
                    break;
                case GUI_ALIGN_CENTER:
                    real_x = root->x + root->margin_left + (awailable_w) / 2 - (x1 + x0) / 128.0f;
                    break;
                default:
                    real_x = root->x + root->margin_left - x0 / 64.0f;
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
            
            if(root->flags.edit_text && (label->cursor_pos <= total_chars + n_sym) && ((label->cursor_pos > total_chars) || (label->cursor_pos == 0)))
            {
                int cursor_x = glf_get_string_len(gl_font, begin, label->cursor_pos - total_chars) / 64;
                Gui_DrawCursorInternal(real_x + cursor_x, real_y + line * dy, dy, &label->cursor_time);
            }
            
            total_chars += n_sym;
            begin = end;
        }
    }
}

static void Gui_DrawObjectsInternal(gui_object_p root, int stencil)
{
    if(!root->flags.hide && (root->w > 0) && (root->h > 0))
    {
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        BindWhiteTexture();
        if(root->flags.draw_background)
        {
            Gui_DrawBackgroundInternal(root);
        }

        if(root->flags.draw_border && root->border_width)
        {
            Gui_DrawBorderInternal(root);
        }

        if(root->label && root->label->text && root->flags.draw_label)
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
        qglEnable(GL_STENCIL_TEST);
        qglClear(GL_STENCIL_BUFFER_BIT);
        qglStencilFunc(GL_ALWAYS, 0, 0xFF);
        qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        Gui_DrawObjectsInternal(root, 0);
        qglDisable(GL_STENCIL_TEST);
    }
}

void Gui_LayoutVertical(gui_object_p root)
{
    gui_object_p prev = NULL;
    int16_t free_h = root->h - root->margin_top - root->margin_bottom;
    int16_t weights_used = 0;
    int16_t weights_total = 0;
    int16_t height_used = 0;
    int16_t content_h = -root->spacing;
    int total_spacings = 0;
    
    for(gui_object_p obj = root->childs; obj; obj = obj->next)
    {
        if(!obj->flags.hide)
        {
            if(!obj->flags.fixed_w)
            {
                obj->x = root->margin_left;
                obj->w = root->w - root->margin_left - root->margin_right;
            }
            else if(obj->flags.h_self_align == GUI_ALIGN_RIGHT)
            {
                obj->x = root->w - root->margin_right - obj->w;
            }
            else if(obj->flags.h_self_align == GUI_ALIGN_CENTER)
            {
                obj->x = (root->margin_left + root->w - root->margin_right - obj->w) / 2;
            }
            else
            {
                obj->x = root->margin_left;
            }
            
            if(obj->flags.autoheight)
            {
                Gui_HeightForWidth(obj);
            }
            content_h += obj->h + root->spacing;
            
            if(root->flags.fit_inside)
            {
                ++total_spacings;
                free_h -= (obj->flags.fixed_h) ? (obj->h) : (0);
                weights_total += (obj->flags.fixed_h) ? (0) : (obj->weight_y);
            }
        }
    }
    
    if(total_spacings)
    {
        --total_spacings;
        free_h -= total_spacings * root->spacing;
    }
    
    weights_total = (weights_total) ? (weights_total) : (1);
    
    for(gui_object_p obj = root->childs; obj; obj = obj->next)
    {
        if(!obj->flags.hide)
        {
            if(!obj->flags.fixed_h && root->flags.fit_inside)
            {
                weights_used += obj->weight_y;
                obj->h = height_used;
                height_used = (int32_t)weights_used * (int32_t)free_h / (int32_t)weights_total;
                obj->h = height_used - obj->h;
            }
            
            if(prev)
            {
                obj->y = prev->y - obj->h - root->spacing;
            }
            else if(root->flags.v_content_align == GUI_ALIGN_BOTTOM)
            {
                obj->y = root->margin_bottom + content_h - obj->h;
            }
            else
            {
                obj->y = root->h - obj->h - root->margin_top;
            }
            prev = obj;
        }
    }
}

void Gui_LayoutHorizontal(gui_object_p root)
{
    gui_object_p prev = NULL;
    int16_t weights_used = 0;
    int16_t weights_total = 0;
    int16_t width_used = 0;
    int16_t free_w = root->w - root->margin_left - root->margin_right;
    int16_t content_w = -root->spacing;
    int total_spacings = 0;
    
    for(gui_object_p obj = root->childs; obj; obj = obj->next)
    {
        if(!obj->flags.hide)
        {
            if(!obj->flags.fixed_h)
            {
                obj->y = root->margin_bottom;
                obj->h = root->h - root->margin_bottom - root->margin_top;
            }
            else if(obj->flags.v_self_align == GUI_ALIGN_BOTTOM)
            {
                obj->y = root->margin_bottom;
            }
            else if(obj->flags.v_self_align == GUI_ALIGN_CENTER)
            {
                obj->y = (root->margin_bottom + root->h - root->margin_top - obj->h) / 2;
            }
            else
            {
                obj->y = root->h - root->margin_top - obj->h;
            }
            
            content_w += obj->w + root->spacing;
            
            if(root->flags.fit_inside)
            {
                ++total_spacings;
                free_w -= (obj->flags.fixed_w) ? (obj->w) : (0);
                weights_total += (obj->flags.fixed_w) ? (0) : (obj->weight_x);
            }
        }
    }
    
    if(total_spacings)
    {
        --total_spacings;
        free_w -= total_spacings * root->spacing;
    }
    
    weights_total = (weights_total) ? (weights_total) : (1);
    
    for(gui_object_p obj = root->childs; obj; obj = obj->next)
    {
        if(!obj->flags.hide)
        {
            if(!obj->flags.fixed_w && root->flags.fit_inside)
            {
                weights_used += obj->weight_x;
                obj->w = width_used;
                width_used = (int32_t)weights_used * (int32_t)free_w / (int32_t)weights_total;
                obj->w = width_used - obj->w;
            }
            
            if(prev)
            {
                obj->x = prev->x + prev->w + root->spacing;
            }
            else if(root->flags.v_content_align == GUI_ALIGN_RIGHT)
            {
                obj->x = root->w - root->margin_right + obj->w - content_w;
            }
            else
            {
                obj->x = root->margin_left;
            }

            prev = obj;
        }
    }
}

void Gui_LayoutObjects(gui_object_p root)
{
    if(root)
    {
        if(root->flags.layout == GUI_LAYOUT_VERTICAL)
        {
            Gui_LayoutVertical(root);
        }
        else if(root->flags.layout == GUI_LAYOUT_HORIZONTAL)
        {
            Gui_LayoutHorizontal(root);
        }
        for(gui_object_p obj = root->childs; obj; obj = obj->next)
        {
            Gui_LayoutObjects(obj);
        }
    }
}

void Gui_EnsureVisible(gui_object_p obj)
{
    if(obj && obj->parent)
    {
        gui_object_p cont = obj->parent;
        if((cont->flags.layout == GUI_LAYOUT_VERTICAL) || (cont->flags.layout == GUI_LAYOUT_NONE))
        {
            if(obj->y + obj->h + cont->content_dy > cont->h - cont->margin_top)
            {
                cont->content_dy = cont->h - obj->h - obj->y - cont->margin_top;
            }
            else if(obj->y + cont->content_dy < cont->margin_bottom)
            {
                cont->content_dy = cont->margin_bottom - obj->y;
            }
        }
        if((cont->flags.layout == GUI_LAYOUT_HORIZONTAL) || (cont->flags.layout == GUI_LAYOUT_NONE))
        {
            if(obj->x + cont->content_dx < cont->margin_left)
            {
                cont->content_dx = cont->margin_left - obj->x;
            }
            else if(obj->x + cont->content_dx + obj->w > cont->w - cont->margin_right)
            {
                cont->content_dx = cont->w - obj->w - obj->x - cont->margin_right;
            }
        }
    }
}


void Gui_ApplyEditCommands(gui_object_p edit, int cmd, uint32_t key)
{
    if(edit && edit->label && edit->label->text && edit->label->text_size)
    {
        uint32_t oldLength = utf8_strlen(edit->label->text);
        edit->label->cursor_time = 0.0f;
        switch(cmd)
        {
            case GUI_COMMAND_OPEN:
            case GUI_COMMAND_ACTIVATE:
            case GUI_COMMAND_CLOSE:
                if(edit->handlers.do_command)
                {
                    edit->handlers.do_command(edit, cmd);
                }
                break;

            case GUI_COMMAND_LEFT:
                if(edit->label->cursor_pos > 0)
                {
                    edit->label->cursor_pos--;
                }
                break;

            case GUI_COMMAND_RIGHT:
                if(edit->label->cursor_pos < oldLength)
                {
                    edit->label->cursor_pos++;
                }
                break;

            case GUI_COMMAND_HOME:
                edit->label->cursor_pos = 0;
                break;

            case GUI_COMMAND_END:
                edit->label->cursor_pos = oldLength;
                break;

            case GUI_COMMAND_BACKSPACE:
                if(edit->label->cursor_pos > 0)
                {
                    edit->label->cursor_pos--;
                    utf8_delete_char((uint8_t*)edit->label->text, edit->label->cursor_pos);
                }
                break;

            case GUI_COMMAND_DELETE:
                if((edit->label->cursor_pos < oldLength))
                {
                    utf8_delete_char((uint8_t*)edit->label->text, edit->label->cursor_pos);
                }
                break;
                
            case GUI_COMMAND_TEXT:
                if(oldLength + 8 >= edit->label->text_size)
                {
                    char *new_buff = (char*)calloc(edit->label->text_size * 2, sizeof(char));
                    if(new_buff)
                    {
                        memcpy(new_buff, edit->label->text, edit->label->text_size);
                        free(edit->label->text);
                        edit->label->text = new_buff;
                        edit->label->text_size *= 2;
                    }
                }
                if((oldLength + 8 < edit->label->text_size) && (key >= ' '))
                {
                    utf8_insert_char((uint8_t*)edit->label->text, key, edit->label->cursor_pos, edit->label->text_size);
                    ++oldLength;
                    ++edit->label->cursor_pos;
                }
                break;
        };
    }
}