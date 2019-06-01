
#include <cmath>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

#include "../core/gl_util.h"
#include "../core/gl_text.h"
#include "../core/system.h"
#include "../core/console.h"
#include "../core/vmath.h"
#include "../core/polygon.h"
#include "../core/obb.h"
#include "camera.h"
#include "render_debug.h"
#include "frustum.h"
#include "../room.h"
#include "../mesh.h"
#include "../skeletal_model.h"
#include "../entity.h"
#include "../engine.h"


#define DEBUG_DRAWER_DEFAULT_BUFFER_SIZE        (128 * 1024)
#define vec4_color_convert(res, x) {(res)[0] = (x)[0]*255; (res)[1] = (x)[1]*255; (res)[2] = (x)[2]*255; (res)[3] = 255;}
#define put_line(v0, v1, v) {\
    vec3_copy(v->pos, v0);\
    vec4_copy(v->rgba, m_rgba);\
    ++v;\
    vec3_copy(v->pos, v1);\
    vec4_copy(v->rgba, m_rgba);\
    ++v;}

/*
 * =============================================================================
 */

CRenderDebugDrawer::CRenderDebugDrawer():
m_drawFlags(0x00000000),
m_max_lines(DEBUG_DRAWER_DEFAULT_BUFFER_SIZE),
m_lines(0),
m_need_realloc(false),
m_gl_vbo(0),
m_buffer(NULL)
{
    m_buffer = (struct vertex_s*)malloc(2 * m_max_lines * sizeof(struct vertex_s));
    m_rgba[0] = m_rgba[1] = m_rgba[2] = 0;
    m_rgba[3] = 255;
}

CRenderDebugDrawer::~CRenderDebugDrawer()
{
    free(m_buffer);
    m_buffer = NULL;
    if(m_gl_vbo != 0)
    {
        qglDeleteBuffersARB(1, &m_gl_vbo);
        m_gl_vbo = 0;
    }
}

void CRenderDebugDrawer::Reset()
{
    if(m_need_realloc)
    {
        struct vertex_s *new_buffer = (struct vertex_s*)malloc(4 * m_max_lines * sizeof(struct vertex_s));
        if(new_buffer != NULL)
        {
            free(m_buffer);
            m_buffer = new_buffer;
            m_max_lines *= 2;
            m_need_realloc = false;
        }
    }
    if(m_gl_vbo == 0)
    {
        qglGenBuffersARB(1, &m_gl_vbo);
    }
    m_lines = 0;
}

void CRenderDebugDrawer::Render()
{
    static const uint32_t step = 1024 * 1024;
    
    if((m_lines > 0) && (m_gl_vbo != 0))
    {       
        uint32_t linesStart = 0;
        bool done = false;
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, m_gl_vbo);
        while (!done)
        {
            uint32_t linesToDrow = m_lines - linesStart;
            if (linesToDrow > step)
            {
                linesToDrow = step;
            }
            else
            {
                done = true;
            }
            qglBufferDataARB(GL_ARRAY_BUFFER_ARB, linesToDrow * 2 * sizeof(struct vertex_s), m_buffer + 2 * linesStart, GL_STREAM_DRAW);
            qglVertexPointer(3, GL_FLOAT, sizeof(struct vertex_s), (GLvoid*)offsetof(struct vertex_s, pos));
            qglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(struct vertex_s), (GLvoid*)offsetof(struct vertex_s, rgba));
            qglDrawArrays(GL_LINES, 0, 2 * linesToDrow);
            linesStart += step;
        }
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

    vec4_set_zero(m_rgba);
    m_lines = 0;
}

void CRenderDebugDrawer::DrawAxis(float r, float transform[16])
{
    struct vertex_s *v0, *v;

    if(m_lines + 3 >= m_max_lines)
    {
        m_need_realloc = true;
        return;
    }

    v0 = v = m_buffer + 2 * m_lines;
    m_lines += 3;

    // OX
    vec3_copy(v->pos, transform + 12);
    v->rgba[0] = 255;
    v->rgba[1] = 0;
    v->rgba[2] = 0;
    v->rgba[3] = 255;
    ++v;
    vec3_add_mul(v->pos, v0->pos, transform + 0, r);
    v->rgba[0] = 255;
    v->rgba[1] = 0;
    v->rgba[2] = 0;
    v->rgba[3] = 255;

    // OY
    ++v;
    *v = *v0;
    v->rgba[0] = 0;
    v->rgba[1] = 255;
    v->rgba[2] = 0;
    v->rgba[3] = 255;
    ++v;
    *v = *v0;
    vec3_add_mul(v->pos, v0->pos, transform + 4, r);
    v->rgba[0] = 0;
    v->rgba[1] = 255;
    v->rgba[2] = 0;
    v->rgba[3] = 255;
    
    // OZ
    ++v;
    *v = *v0;
    v->rgba[0] = 0;
    v->rgba[1] = 0;
    v->rgba[2] = 255;
    v->rgba[3] = 255;
    ++v;
    *v = *v0;
    vec3_add_mul(v->pos, v0->pos, transform + 8, r);
    v->rgba[0] = 0;
    v->rgba[1] = 0;
    v->rgba[2] = 255;
    v->rgba[3] = 255;
}

void CRenderDebugDrawer::DrawFrustum(struct frustum_s *f)
{
    if(f != NULL)
    {
        struct vertex_s *v, *v0;
        float *fv = f->vertex;

        if(m_lines + f->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        v = v0 = m_buffer + 2 * m_lines;
        m_lines += f->vertex_count;

        for(uint16_t i = 0; i < f->vertex_count - 1; i++, fv += 3)
        {
            put_line(fv, fv + 3, v);
        }

        vec3_copy(v->pos, fv);
        vec4_copy(v->rgba, m_rgba);
        ++v;
        *v = *v0;
    }
}

void CRenderDebugDrawer::DrawPortal(struct portal_s *p)
{
    if(p != NULL)
    {
        struct vertex_s *v, *v0;
        float *pv = p->vertex;

        if(m_lines + p->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        v = v0 = m_buffer + 2 * m_lines;
        m_lines += p->vertex_count;

        for(uint16_t i = 0; i < p->vertex_count - 1; i++, pv += 3)
        {
            put_line(pv, pv + 3, v);
        }

        vec3_copy(v->pos, pv);
        vec4_copy(v->rgba, m_rgba);
        ++v;
        *v = *v0;
    }
}

void CRenderDebugDrawer::DrawBBox(float bb_min[3], float bb_max[3], float *transform)
{
    if(m_lines + 12 < m_max_lines)
    {
        float t[3], v0[3], v1[3], v2[3], v3[3], v4[3], v5[3], v6[3], v7[3];
        if (transform)
        {
            Mat4_vec3_mul_macro(v0, transform, bb_min);

            t[0] = bb_max[0]; t[1] = bb_min[1]; t[2] = bb_min[2];
            Mat4_vec3_mul_macro(v1, transform, t);

            t[0] = bb_max[0]; t[1] = bb_max[1]; t[2] = bb_min[2];
            Mat4_vec3_mul_macro(v2, transform, t);

            t[0] = bb_min[0]; t[1] = bb_max[1]; t[2] = bb_min[2];
            Mat4_vec3_mul_macro(v3, transform, t);

            t[0] = bb_min[0]; t[1] = bb_min[1]; t[2] = bb_max[2];
            Mat4_vec3_mul_macro(v4, transform, t);

            t[0] = bb_max[0]; t[1] = bb_min[1]; t[2] = bb_max[2];
            Mat4_vec3_mul_macro(v5, transform, t);

            Mat4_vec3_mul_macro(v6, transform, bb_max);

            t[0] = bb_min[0]; t[1] = bb_max[1]; t[2] = bb_max[2];
            Mat4_vec3_mul_macro(v7, transform, t);
        }
        else
        {
            vec3_copy(v0, bb_min);
            v0[0] = bb_min[0]; v0[1] = bb_min[1]; v0[2] = bb_min[2];
            v1[0] = bb_max[0]; v1[1] = bb_min[1]; v1[2] = bb_min[2];
            v2[0] = bb_max[0]; v2[1] = bb_max[1]; v2[2] = bb_min[2];
            v3[0] = bb_min[0]; v3[1] = bb_max[1]; v3[2] = bb_min[2];
            v4[0] = bb_min[0]; v4[1] = bb_min[1]; v4[2] = bb_max[2];
            v5[0] = bb_max[0]; v5[1] = bb_min[1]; v5[2] = bb_max[2];
            v6[0] = bb_max[0]; v6[1] = bb_max[1]; v6[2] = bb_max[2];
            v7[0] = bb_min[0]; v7[1] = bb_max[1]; v7[2] = bb_max[2];
        }
        
        struct vertex_s *v = m_buffer + 2 * m_lines;
        m_lines += 12;
        put_line(v0, v1, v);
        put_line(v1, v2, v);
        put_line(v2, v3, v);
        put_line(v3, v0, v);

        put_line(v4, v5, v);
        put_line(v5, v6, v);
        put_line(v6, v7, v);
        put_line(v7, v4, v);

        put_line(v0, v4, v);
        put_line(v1, v5, v);
        put_line(v2, v6, v);
        put_line(v3, v7, v);
    }
    else
    {
        m_need_realloc = true;
    }
}

void CRenderDebugDrawer::DrawOBB(struct obb_s *obb)
{
    struct vertex_s *v, *v0;
    polygon_p p = obb->polygons;

    if(m_lines + 12 >= m_max_lines)
    {
        m_need_realloc = true;
        return;
    }

    v = v0 = m_buffer + 2 * m_lines;
    m_lines += 12;

    put_line(p->vertices[0].position, (p+1)->vertices[0].position, v);
    put_line(p->vertices[1].position, (p+1)->vertices[3].position, v);
    put_line(p->vertices[2].position, (p+1)->vertices[2].position, v);
    put_line(p->vertices[3].position, (p+1)->vertices[1].position, v);

    for(uint16_t i = 0; i < 2; i++, p++)
    {
        vertex_p pv = p->vertices;
        v0 = v;
        for(uint16_t j = 0; j < p->vertex_count - 1; j++, pv++)
        {
            put_line(pv->position, (pv+1)->position, v);
        }

        vec3_copy(v->pos, pv->position);
        vec4_copy(v->rgba, m_rgba);
        ++v;
        *v = *v0;
        ++v;
    }
}

void CRenderDebugDrawer::DrawMeshDebugLines(struct base_mesh_s *mesh, float transform[16], const float *overrideVertices, const float *overrideNormals)
{
    if((!m_need_realloc) && (m_drawFlags & R_DRAW_NORMALS))
    {
        struct vertex_s *v = m_buffer + 2 * m_lines;
        float n[3];

        if(m_lines + mesh->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        this->SetColor(204, 0, 230, 255);
        m_lines += mesh->vertex_count;
        if(overrideVertices)
        {
            float *ov = (float*)overrideVertices;
            float *on = (float*)overrideNormals;
            for(uint32_t i = 0; i < mesh->vertex_count; i++, ov += 3, on += 3)
            {
                Mat4_vec3_mul_macro(v->pos, transform, ov);
                vec4_copy(v->rgba, m_rgba);
                
                Mat4_vec3_rot_macro(n, transform, on);
                v[1].pos[0] = v->pos[0] + n[0] * 128.0;
                v[1].pos[1] = v->pos[1] + n[1] * 128.0;
                v[1].pos[2] = v->pos[2] + n[2] * 128.0;
                vec4_copy(v[1].rgba, m_rgba);
                v += 2;
            }
        }
        else
        {
            vertex_p mv = mesh->vertices;
            for (uint32_t i = 0; i < mesh->vertex_count; i++,mv++)
            {
                Mat4_vec3_mul_macro(v->pos, transform, mv->position);
                vec4_copy(v->rgba, m_rgba);
                
                Mat4_vec3_rot_macro(n, transform, mv->normal);
                v[1].pos[0] = v->pos[0] + n[0] * 128.0;
                v[1].pos[1] = v->pos[1] + n[1] * 128.0;
                v[1].pos[2] = v->pos[2] + n[2] * 128.0;
                vec4_copy(v[1].rgba, m_rgba);
                v += 2;
            }
        }
    }
}

void CRenderDebugDrawer::DrawSkeletalModelDebugLines(struct ss_bone_frame_s *bframe, float transform[16])
{
    if((!m_need_realloc) && m_drawFlags & R_DRAW_NORMALS)
    {
        float tr[16];

        ss_bone_tag_p btag = bframe->bone_tags;
        for(uint16_t i = 0; i < bframe->bone_tag_count; i++, btag++)
        {
            Mat4_Mat4_mul(tr, transform, btag->current_transform);
            this->DrawMeshDebugLines(btag->mesh_base, tr, NULL, NULL);
        }
    }
}

void CRenderDebugDrawer::DrawEntityDebugLines(struct entity_s *entity)
{
    if(m_need_realloc || !(m_drawFlags & (R_DRAW_AXIS | R_DRAW_NORMALS | R_DRAW_BOXES)) ||
       !(entity->state_flags & ENTITY_STATE_VISIBLE) || (entity->bf->animations.model->hide && !(m_drawFlags & R_DRAW_NULLMESHES)))
    {
        return;
    }

    if(m_drawFlags & R_DRAW_BOXES)
    {
        this->SetColor(0, 0, 255, 255);
        this->DrawOBB(entity->obb);
    }

    if(m_drawFlags & R_DRAW_AXIS)
    {
        this->DrawAxis(1000.0, entity->transform.M4x4);
    }

    if(entity->bf->animations.model && entity->bf->animations.model->animations)
    {
        this->DrawSkeletalModelDebugLines(entity->bf, entity->transform.M4x4);
    }
}

void CRenderDebugDrawer::DrawSectorDebugLines(struct room_sector_s *rs)
{
    if(m_lines + 12 < m_max_lines)
    {
        float bb_min[3] = {(float)(rs->pos[0] - TR_METERING_SECTORSIZE / 2.0), (float)(rs->pos[1] - TR_METERING_SECTORSIZE / 2.0), (float)rs->floor};
        float bb_max[3] = {(float)(rs->pos[0] + TR_METERING_SECTORSIZE / 2.0), (float)(rs->pos[1] + TR_METERING_SECTORSIZE / 2.0), (float)rs->ceiling};

        this->DrawBBox(bb_min, bb_max, NULL);
    }
    else
    {
        m_need_realloc = true;
    }
}

void CRenderDebugDrawer::DrawRoomDebugLines(struct room_s *room, struct camera_s *cam)
{
    frustum_p frus;
    engine_container_p cont;
    entity_p ent;

    if(m_need_realloc)
    {
        return;
    }

    if(m_drawFlags & R_DRAW_ROOMBOXES)
    {
        this->SetColor(0, 0, 230, 255);
        this->DrawBBox(room->bb_min, room->bb_max, NULL);
        /*for(uint32_t s = 0; s < room->sectors_count; s++)
        {
            drawSectorDebugLines(room->sectors + s);
        }*/
    }

    if(m_drawFlags & R_DRAW_PORTALS)
    {
        this->SetColor(0, 0, 0, 255);
        for(uint16_t i = 0; i < room->content->portals_count; i++)
        {
            this->DrawPortal(room->content->portals+i);
        }
    }

    if(m_drawFlags & R_DRAW_FRUSTUMS)
    {
        this->SetColor(255, 0, 0, 255);
        for(frus = room->frustum; frus; frus = frus->next)
        {
            this->DrawFrustum(frus);
        }
    }

    if(!(m_drawFlags & R_SKIP_ROOM) && (room->content->mesh != NULL))
    {
        this->DrawMeshDebugLines(room->content->mesh, room->transform, NULL, NULL);
    }

    if(m_drawFlags & R_DRAW_TRIGGERS)
    {
        this->SetColor(255, 0, 255, 255);
        for(uint32_t i = 0; i < room->sectors_count; i++)
        {
            if(room->content->sectors[i].trigger)
            {
                this->DrawSectorDebugLines(room->content->sectors + i);
            }
        }
    }

    for(uint32_t i = 0; i < room->content->static_mesh_count; i++)
    {
        if(Frustum_IsOBBVisibleInFrustumList(room->content->static_mesh[i].obb, (room->frustum) ? (room->frustum) : (cam->frustum)) &&
           (!room->content->static_mesh[i].hide || (m_drawFlags & R_DRAW_DUMMY_STATICS)))
        {
            if(m_drawFlags & R_DRAW_BOXES)
            {
                this->SetColor(0, 255, 26, 255);
                this->DrawOBB(room->content->static_mesh[i].obb);
            }

            if(m_drawFlags & R_DRAW_AXIS)
            {
                this->DrawAxis(1000.0, room->content->static_mesh[i].transform);
            }

            this->DrawMeshDebugLines(room->content->static_mesh[i].mesh, room->content->static_mesh[i].transform, NULL, NULL);
        }
    }

    for(cont = room->containers; cont; cont = cont->next)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                ent = (entity_p)cont->object;
                if(Frustum_IsOBBVisibleInFrustumList(ent->obb, (room->frustum) ? (room->frustum) : (cam->frustum)))
                {
                    this->DrawEntityDebugLines(ent);
                }
                break;
        };
    }
}

void CRenderDebugDrawer::DrawLine(const float from[3], const float to[3], const float color_from[3], const float color_to[3])
{
    if(m_lines < m_max_lines - 1)
    {
        struct vertex_s *v = m_buffer + 2 * m_lines;
        m_lines++;

        vec3_copy(v->pos, from);
        vec4_color_convert(v->rgba, color_from);
        ++v;
        vec3_copy(v->pos, to);
        vec4_color_convert(v->rgba, color_to);
    }
    else
    {
        m_need_realloc = true;
    }
}

void CRenderDebugDrawer::DrawContactPoint(const float pointOnB[3], const float normalOnB[3], float distance, int lifeTime, const float color[3])
{
    if(m_lines + 2 < m_max_lines)
    {
        float to[3];
        struct vertex_s *v = m_buffer + 2 * m_lines;

        m_lines += 2;
        vec3_add_mul(to, pointOnB, normalOnB, distance);

        vec3_copy(v->pos, pointOnB);
        vec4_color_convert(v->rgba, color);
        ++v;

        vec3_copy(v->pos, to);
        vec4_color_convert(v->rgba, color);
    }
    else
    {
        m_need_realloc = true;
    }
}
