
#include <cmath>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

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
    m_buffer = (GLfloat*)malloc(2 * 6 * m_max_lines * sizeof(GLfloat));
    vec3_set_zero(m_color);
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
        uint32_t new_buffer_size = m_max_lines * 12 * 2;
        GLfloat *new_buffer = (GLfloat*)malloc(new_buffer_size * sizeof(GLfloat));
        if(new_buffer != NULL)
        {
            free(m_buffer);
            m_buffer = new_buffer;
            m_max_lines *= 2;
        }
        m_need_realloc = false;
    }
    if(m_gl_vbo == 0)
    {
        qglGenBuffersARB(1, &m_gl_vbo);
    }
    m_lines = 0;
}

void CRenderDebugDrawer::Render()
{
    if((m_lines > 0) && (m_gl_vbo != 0))
    {
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, m_gl_vbo);
        qglBufferDataARB(GL_ARRAY_BUFFER_ARB, m_lines * 12 * sizeof(GLfloat), m_buffer, GL_STREAM_DRAW);
        qglVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)0);
        qglColorPointer(3, GL_FLOAT, 6 * sizeof(GLfloat),  (void*)(3 * sizeof(GLfloat)));
        qglDrawArrays(GL_LINES, 0, 2 * m_lines);
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

    vec3_set_zero(m_color);
    m_lines = 0;
}

void CRenderDebugDrawer::DrawAxis(float r, float transform[16])
{
    GLfloat *v0, *v;

    if(m_lines + 3 >= m_max_lines)
    {
        m_need_realloc = true;
        return;
    }

    v0 = v = m_buffer + 3 * 4 * m_lines;
    m_lines += 3;
    vec3_copy(v0, transform + 12);

    // OX
    v += 3;
    v[0] = 1.0;
    v[1] = 0.0;
    v[2] = 0.0;
    v += 3;
    vec3_add_mul(v, v0, transform + 0, r);
    v += 3;
    v[0] = 1.0;
    v[1] = 0.0;
    v[2] = 0.0;
    v += 3;

    // OY
    vec3_copy(v, v0);
    v += 3;
    v[0] = 0.0;
    v[1] = 1.0;
    v[2] = 0.0;
    v += 3;
    vec3_add_mul(v, v0, transform + 4, r);
    v += 3;
    v[0] = 0.0;
    v[1] = 1.0;
    v[2] = 0.0;
    v += 3;

    // OZ
    vec3_copy(v, v0);
    v += 3;
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 1.0;
    v += 3;
    vec3_add_mul(v, v0, transform + 8, r);
    v += 3;
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 1.0;
}

void CRenderDebugDrawer::DrawFrustum(struct frustum_s *f)
{
    if(f != NULL)
    {
        GLfloat *v, *v0;
        float *fv = f->vertex;

        if(m_lines + f->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        v = v0 = m_buffer + 3 * 4 * m_lines;
        m_lines += f->vertex_count;

        for(uint16_t i = 0; i < f->vertex_count - 1; i++, fv += 3)
        {
            vec3_copy(v, fv);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;

            vec3_copy(v, fv + 3);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;
        }

        vec3_copy(v, fv);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
        vec3_copy(v, v0);
        v += 3;
        vec3_copy(v, m_color);
    }
}

void CRenderDebugDrawer::DrawPortal(struct portal_s *p)
{
    if(p != NULL)
    {
        GLfloat *v, *v0;
        float *pv = p->vertex;

        if(m_lines + p->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        v = v0 = m_buffer + 3 * 4 * m_lines;
        m_lines += p->vertex_count;

        for(uint16_t i = 0; i < p->vertex_count - 1; i++, pv += 3)
        {
            vec3_copy(v, pv);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;

            vec3_copy(v, pv + 3);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;
        }

        vec3_copy(v, pv);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
        vec3_copy(v, v0);
        v += 3;
        vec3_copy(v, m_color);
    }
}

void CRenderDebugDrawer::DrawBBox(float bb_min[3], float bb_max[3], float *transform)
{
    if(m_lines + 12 < m_max_lines)
    {
        float t[3], v0[3], v1[3], v2[3], v3[3], v4[3], v5[3], v6[3], v7[3];
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

        DrawLine(v0, v1, m_color, m_color);
        DrawLine(v1, v2, m_color, m_color);
        DrawLine(v2, v3, m_color, m_color);
        DrawLine(v3, v0, m_color, m_color);

        DrawLine(v4, v5, m_color, m_color);
        DrawLine(v5, v6, m_color, m_color);
        DrawLine(v6, v7, m_color, m_color);
        DrawLine(v7, v4, m_color, m_color);

        DrawLine(v0, v4, m_color, m_color);
        DrawLine(v1, v5, m_color, m_color);
        DrawLine(v2, v6, m_color, m_color);
        DrawLine(v3, v7, m_color, m_color);
    }
    else
    {
        m_need_realloc = true;
    }
}

void CRenderDebugDrawer::DrawOBB(struct obb_s *obb)
{
    GLfloat *v, *v0;
    polygon_p p = obb->polygons;

    if(m_lines + 12 >= m_max_lines)
    {
        m_need_realloc = true;
        return;
    }

    v = v0 = m_buffer + 3 * 4 * m_lines;
    m_lines += 12;

    vec3_copy(v, p->vertices[0].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[0].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    vec3_copy(v, p->vertices[1].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[3].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    vec3_copy(v, p->vertices[2].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[2].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    vec3_copy(v, p->vertices[3].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[1].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    for(uint16_t i = 0; i < 2; i++, p++)
    {
        vertex_p pv = p->vertices;
        v0 = v;
        for(uint16_t j = 0; j < p->vertex_count - 1; j++, pv++)
        {
            vec3_copy(v, pv->position);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;

            vec3_copy(v, (pv+1)->position);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;
        }

        vec3_copy(v, pv->position);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
        vec3_copy(v, v0);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
    }
}

void CRenderDebugDrawer::DrawMeshDebugLines(struct base_mesh_s *mesh, float transform[16], const float *overrideVertices, const float *overrideNormals)
{
    if((!m_need_realloc) && (m_drawFlags & R_DRAW_NORMALS))
    {
        GLfloat *v = m_buffer + 3 * 4 * m_lines;
        float n[3];

        if(m_lines + mesh->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        this->SetColor(0.8, 0.0, 0.9);
        m_lines += mesh->vertex_count;
        if(overrideVertices)
        {
            float *ov = (float*)overrideVertices;
            float *on = (float*)overrideNormals;
            for(uint32_t i = 0; i < mesh->vertex_count; i++, ov += 3, on += 3, v += 12)
            {
                Mat4_vec3_mul_macro(v, transform, ov);
                Mat4_vec3_rot_macro(n, transform, on);

                v[6 + 0] = v[0] + n[0] * 128.0;
                v[6 + 1] = v[1] + n[1] * 128.0;
                v[6 + 2] = v[2] + n[2] * 128.0;
                vec3_copy(v+3, m_color);
                vec3_copy(v+9, m_color);
            }
        }
        else
        {
            vertex_p mv = mesh->vertices;
            for (uint32_t i = 0; i < mesh->vertex_count; i++,mv++,v+=12)
            {
                Mat4_vec3_mul_macro(v, transform, mv->position);
                Mat4_vec3_rot_macro(n, transform, mv->normal);

                v[6 + 0] = v[0] + n[0] * 128.0;
                v[6 + 1] = v[1] + n[1] * 128.0;
                v[6 + 2] = v[2] + n[2] * 128.0;
                vec3_copy(v+3, m_color);
                vec3_copy(v+9, m_color);
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
        this->SetColor(0.0, 0.0, 1.0);
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
        this->SetColor(0.0, 0.1, 0.9);
        this->DrawBBox(room->bb_min, room->bb_max, NULL);
        /*for(uint32_t s = 0; s < room->sectors_count; s++)
        {
            drawSectorDebugLines(room->sectors + s);
        }*/
    }

    if(m_drawFlags & R_DRAW_PORTALS)
    {
        this->SetColor(0.0, 0.0, 0.0);
        for(uint16_t i = 0; i < room->content->portals_count; i++)
        {
            this->DrawPortal(room->content->portals+i);
        }
    }

    if(m_drawFlags & R_DRAW_FRUSTUMS)
    {
        this->SetColor(1.0, 0.0, 0.0);
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
        this->SetColor(1.0, 0.0, 1.0);
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
                this->SetColor(0.0, 1.0, 0.1);
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
    GLfloat *v;

    if(m_lines < m_max_lines - 1)
    {
        v = m_buffer + 3 * 4 * m_lines;
        m_lines++;

        vec3_copy(v, from);
        v += 3;
        vec3_copy(v, color_from);
        v += 3;
        vec3_copy(v, to);
        v += 3;
        vec3_copy(v, color_to);
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
        GLfloat *v = m_buffer + 3 * 4 * m_lines;

        m_lines += 2;
        vec3_add_mul(to, pointOnB, normalOnB, distance);

        vec3_copy(v, pointOnB);
        v += 3;
        vec3_copy(v, color);
        v += 3;

        vec3_copy(v, to);
        v += 3;
        vec3_copy(v, color);
    }
    else
    {
        m_need_realloc = true;
    }
}
