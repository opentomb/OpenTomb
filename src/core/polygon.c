
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "system.h"
#include "vmath.h"
#include "polygon.h"


__inline void ApplyAnimTextureTransformation(GLfloat *uv_out, const GLfloat *uv_in, const struct tex_frame_s *tf)
{
    uv_out[0] = tf->mat[0+0*2] * uv_in[0] + tf->mat[0+1*2] * uv_in[1] + tf->move[0];
    uv_out[1] = tf->mat[1+0*2] * uv_in[0] + tf->mat[1+1*2] * uv_in[1] + tf->move[1] - tf->current_uvrotate;
}

/*
 * POLYGONS
 */

polygon_p Polygon_CreateArray(unsigned int pcount)
{
    return (polygon_p)calloc(pcount, sizeof(polygon_t));
}

void Polygon_Resize(polygon_p p, unsigned int count)
{
    p->vertices = (vertex_p)realloc(p->vertices, count*sizeof(vertex_t));
    p->vertex_count = count;
}

void Polygon_Clear(polygon_p p)
{
    if(p)
    {
        if(p->vertices)
        {
            free(p->vertices);
            p->vertices = NULL;
        }
        p->vertex_count = 0;
    }
}


int Polygon_IsBroken(polygon_p p)
{
    btScalar dif[3];
    vertex_p next_v, curr_v;

    if(p->vertex_count < 3)
    {
        return 1;
    }

    dif[0] = vec3_sqabs(p->plane);
    if(dif[0] < 0.999 || dif[0] > 1.001)
    {
        return 1;
    }

    next_v = p->vertices;
    curr_v = p->vertices + p->vertex_count - 1;
    for(uint16_t i=0;i<p->vertex_count;i++)
    {
        vec3_sub(dif, next_v->position, curr_v->position);
        if(vec3_sqabs(dif) < 0.0001)
        {
            return 1;
        }

        curr_v = next_v;
        next_v ++;
    }

    return 0;
}


/*
 * We made independent copy of src;
 */
void Polygon_Copy(polygon_p dst, polygon_p src)
{
    if(dst->vertex_count != src->vertex_count)
    {
        Polygon_Resize(dst, src->vertex_count);
    }

    dst->anim_id = src->anim_id;
    dst->frame_offset = src->frame_offset;
    dst->double_side  = src->double_side;
    dst->tex_index = src->tex_index;
    dst->transparency = src->transparency;

    vec4_copy(dst->plane, src->plane);

    for(uint16_t i=0;i<src->vertex_count;i++)
    {
        dst->vertices[i] = src->vertices[i];
    }
}


void Polygon_FindNormale(polygon_p p)
{
    btScalar v1[3], v2[3];

    vec3_sub(v1, p->vertices[1].position, p->vertices[0].position);
    vec3_sub(v2, p->vertices[2].position, p->vertices[1].position);
    vec3_cross(p->plane, v1, v2);
    p->plane[3] = -vec3_abs(p->plane);
    vec3_norm_plane(p->plane, p->vertices[0].position, p->plane[3]);
}


void Polygon_MoveSelf(polygon_p p, btScalar move[3])
{
    vertex_p v;

    v = p->vertices;
    for(uint16_t i=0;i<p->vertex_count;i++,v++)
    {
        vec3_add_to(v->position, move);
    }

    p->plane[3] = -vec3_dot(p->plane, p->vertices[0].position);
}


void Polygon_Move(polygon_p ret, polygon_p src, btScalar move[3])
{
    vertex_p ret_v, src_v;

    ret_v = ret->vertices;
    src_v = src->vertices;
    for(uint16_t i=0;i<src->vertex_count;i++,ret_v++,src_v++)
    {
        vec3_add(ret_v->position, src_v->position, move);
    }

    vec3_copy(ret->plane, src->plane);
    ret->plane[3] = -vec3_dot(ret->plane, ret->vertices[0].position);
}


void Polygon_TransformSelf(polygon_p p, btScalar tr[16])
{
    btScalar v[3];
    vertex_p vp;

    Mat4_vec3_rot_macro(v, tr, p->plane);
    vec3_copy(p->plane, v);
    vp = p->vertices;
    for(uint16_t i=0;i<p->vertex_count;i++,vp++)
    {
        Mat4_vec3_mul_macro(v, tr, vp->position);
        vec3_copy(vp->position, v);
        Mat4_vec3_rot_macro(v, tr, vp->normal);
        vec3_copy(vp->normal, v);
    }

    p->plane[3] = -vec3_dot(p->plane, p->vertices[0].position);
}


void Polygon_Transform(polygon_p ret, polygon_p src, btScalar tr[16])
{
    vertex_p ret_v, src_v;

    Mat4_vec3_rot_macro(ret->plane, tr, src->plane);
    ret_v = ret->vertices;
    src_v = src->vertices;
    for(uint16_t i=0;i<src->vertex_count;i++,ret_v++,src_v++)
    {
        Mat4_vec3_mul_macro(ret_v->position, tr, src_v->position);
        Mat4_vec3_rot_macro(ret_v->normal, tr, src_v->normal);
    }

    ret->plane[3] = -vec3_dot(ret->plane, ret->vertices[0].position);
}


int Polygon_RayIntersect(polygon_p p, btScalar dir[3], btScalar dot[3], btScalar *t)
{
    btScalar tt, u, v, E1[3], E2[3], P[3], Q[3], T[3];
    vertex_p vp;

    u = vec3_dot(p->plane, dir);
    if(fabs(u) < 0.001 /*|| vec3_plane_dist(p->plane, dot) < -0.001*/)          // FIXME: magick
    {
        return 0;                                                               // plane is parallel to the ray - no intersection
    }
    *t = - vec3_plane_dist(p->plane, dot);
    *t /= u;

    vp = p->vertices;                                                           // current polygon pointer
    vec3_sub(T, dot, vp[0].position);

    vec3_sub(E2, vp[1].position, vp[0].position)
    for(uint16_t i=0;i<p->vertex_count-2;i++,vp++)
    {
        vec3_copy(E1, E2)                                                       // PREV
        vec3_sub(E2, vp[2].position, p->vertices[0].position)                   // NEXT

        vec3_cross(P, dir, E2)
        vec3_cross(Q, T, E1)

        tt = vec3_dot(P, E1);
        u = vec3_dot(P, T);
        u /= tt;
        v = vec3_dot(Q, dir);
        v /= tt;
        tt = 1.0 - u - v;
        if((u <= 1.0) && (u >= 0.0) && (v <= 1.0) && (v >= 0.0) && (tt <= 1.0) && (tt >= 0.0))
        {
            return 1;
        }
    }
    return 0;
}


int Polygon_IntersectPolygon(polygon_p p1, polygon_p p2)
{
    btScalar dist[3], dir[3], t, *result_buf, *result_v;
    vertex_p prev_v, curr_v;
    size_t buf_size;
    char cnt = 0;

    if(SPLIT_IN_BOTH != Polygon_SplitClassify(p1, p2->plane) || (SPLIT_IN_BOTH != Polygon_SplitClassify(p2, p1->plane)))
    {
        return 0;                                                               // quick check
    }

    buf_size = (p1->vertex_count + p2->vertex_count) * 3 * sizeof(btScalar);
    result_buf = (btScalar*)Sys_GetTempMem(buf_size);
    result_v = result_buf;

    /*
     * intersection of polygon p1 and plane p2
     */
    prev_v = p1->vertices + p1->vertex_count - 1;
    curr_v = p1->vertices;
    dist[0] = vec3_plane_dist(p2->plane, prev_v->position);
    for(uint16_t i=0;i<p1->vertex_count;i++)
    {
        dist[1] = vec3_plane_dist(p2->plane, curr_v->position);
        if(dist[1] > SPLIT_EPSILON)
        {
            if(dist[0] < -SPLIT_EPSILON)
            {
                vec3_sub(dir, curr_v->position, prev_v->position);
                vec3_ray_plane_intersect(prev_v->position, dir, p2->plane, result_v, t);
                result_v += 3;
                cnt++;
            }
        }
        else if(dist[1] < -SPLIT_EPSILON)
        {
            if(dist[0] > SPLIT_EPSILON)
            {
                vec3_sub(dir, curr_v->position, prev_v->position);
                vec3_ray_plane_intersect(prev_v->position, dir, p2->plane, result_v, t);
                result_v += 3;
                cnt++;
            }
        }
        else
        {
            vec3_copy(result_v, curr_v->position);
            result_v += 3;
            cnt++;
        }

        if(cnt >= 2)
        {
            break;
        }
        dist[0] = dist[1];
        prev_v = curr_v;
        curr_v ++;
    }

    /*
     * splitting p2 by p1 split plane
     */
    prev_v = p2->vertices + p2->vertex_count - 1;
    curr_v = p2->vertices;
    dist[0] = vec3_plane_dist(p1->plane, prev_v->position);
    for(uint16_t i=0;i<p2->vertex_count;i++)
    {
        dist[1] = vec3_plane_dist(p1->plane, curr_v->position);
        if(dist[1] > SPLIT_EPSILON)
        {
            if(dist[0] < -SPLIT_EPSILON)
            {
                vec3_sub(dir, curr_v->position, prev_v->position);
                vec3_ray_plane_intersect(prev_v->position, dir, p1->plane, result_v, t);
                result_v += 3;
                cnt++;
            }
        }
        else if(dist[1] < -SPLIT_EPSILON)
        {
            if(dist[0] > SPLIT_EPSILON)
            {
                vec3_sub(dir, curr_v->position, prev_v->position);
                vec3_ray_plane_intersect(prev_v->position, dir, p1->plane, result_v, t);
                result_v += 3;
                cnt++;
            }
        }
        else
        {
            vec3_copy(result_v, curr_v->position);
            result_v += 3;
            cnt++;
        }

        if(cnt >= 4)
        {
            break;
        }
        dist[0] = dist[1];
        prev_v = curr_v;
        curr_v ++;
    }

    vec3_cross(dir, p1->plane, p2->plane);                                      // vector of two planes intersection line
    t = ABS(dir[0]);
    dist[0] = ABS(dir[1]);
    dist[1] = ABS(dir[2]);
    int pn = PLANE_X;
    if(t < dist[0])
    {
        t = dist[0];
        pn = PLANE_Y;
    }
    if(t < dist[1])
    {
        pn = PLANE_Z;
    }

    switch(pn)
    {
        case PLANE_X:
            dist[0] = (result_buf[3+0] -  result_buf[0]) / dir[0];
            dist[1] = (result_buf[6+0] -  result_buf[0]) / dir[0];
            dist[2] = (result_buf[9+0] -  result_buf[0]) / dir[0];
            break;

        case PLANE_Y:
            dist[0] = (result_buf[3+1] -  result_buf[1]) / dir[1];
            dist[1] = (result_buf[6+1] -  result_buf[1]) / dir[1];
            dist[2] = (result_buf[9+1] -  result_buf[1]) / dir[1];
            break;

        case PLANE_Z:
            dist[0] = (result_buf[3+2] -  result_buf[2]) / dir[2];
            dist[1] = (result_buf[6+2] -  result_buf[2]) / dir[2];
            dist[2] = (result_buf[9+2] -  result_buf[2]) / dir[2];
            break;
    };

    Sys_ReturnTempMem(buf_size);

    if(dist[0] > 0)
    {
        return !((dist[1] < 0.0 && dist[2] < 0.0) || (dist[1] > dist[0] && dist[2] > dist[0]));
    }
    return !((dist[1] < dist[0] && dist[2] < dist[0]) || (dist[1] > 0.0 && dist[2] > 0.0));
}


int Polygon_SplitClassify(polygon_p p, btScalar n[4])
{
    int i;
    int positive = 0;
    int negative = 0;
    btScalar dist;
    vertex_p v;

    v = p->vertices;
    for (i=0;i<p->vertex_count;i++,v++)
    {
        dist = vec3_plane_dist(n, v->position);
        if (dist > SPLIT_EPSILON)
        {
            positive++;
        }
        else if (dist < -SPLIT_EPSILON)
        {
            negative++;
        }
    }

    if(positive > 0 && negative == 0)
    {
        return SPLIT_FRONT;
    }
    else if(positive == 0 && negative > 0)
    {
        return SPLIT_BACK;
    }
    else if (positive < 1 && negative < 1)
    {
        return SPLIT_IN_PLANE;
    }

    return SPLIT_IN_BOTH;
}

/*
 * animated textures coordinates splits too!
 */
void Polygon_Split(polygon_p src, btScalar n[4], polygon_p front, polygon_p back)
{
    btScalar t, tmp, dir[3];
    vertex_t *curr_v, *prev_v, tv;
    btScalar dist[2];

    vec4_copy(front->plane, src->plane);
    front->anim_id = src->anim_id;
    front->frame_offset = src->frame_offset;
    front->double_side = src->double_side;
    front->tex_index = src->tex_index;
    front->transparency = src->transparency;

    vec4_copy(back->plane, src->plane);
    back->anim_id = src->anim_id;
    back->frame_offset = src->frame_offset;
    back->double_side = src->double_side;
    back->tex_index = src->tex_index;
    back->transparency = src->transparency;

    curr_v = src->vertices;
    prev_v = src->vertices + src->vertex_count - 1;
    
    dist[0] = vec3_plane_dist(n, prev_v->position);
    for(uint16_t i=0;i<src->vertex_count;i++)
    {
        dist[1] = vec3_plane_dist(n, curr_v->position);

        if(dist[1] > SPLIT_EPSILON)
        {
            if(dist[0] < -SPLIT_EPSILON)
            {
                vec3_sub(dir, curr_v->position, prev_v->position);
                vec3_ray_plane_intersect(prev_v->position, dir, n, tv.position, t);

                tv.normal[0] = prev_v->normal[0] + t * (curr_v->normal[0] - prev_v->normal[0]);
                tv.normal[1] = prev_v->normal[1] + t * (curr_v->normal[1] - prev_v->normal[1]);
                tv.normal[2] = prev_v->normal[2] + t * (curr_v->normal[2] - prev_v->normal[2]);
                vec3_norm(tv.normal, tmp);

                tv.color[0] = prev_v->color[0] + t * (curr_v->color[0] - prev_v->color[0]);
                tv.color[1] = prev_v->color[1] + t * (curr_v->color[1] - prev_v->color[1]);
                tv.color[2] = prev_v->color[2] + t * (curr_v->color[2] - prev_v->color[2]);
                tv.color[3] = prev_v->color[3] + t * (curr_v->color[3] - prev_v->color[3]);

                tv.tex_coord[0] = prev_v->tex_coord[0] + t * (curr_v->tex_coord[0] - prev_v->tex_coord[0]);
                tv.tex_coord[1] = prev_v->tex_coord[1] + t * (curr_v->tex_coord[1] - prev_v->tex_coord[1]);

                front->vertices[front->vertex_count++] = tv;
                back->vertices[back->vertex_count++] = tv;
            }
            front->vertices[front->vertex_count++] = *curr_v;
        }
        else if(dist[1] < -SPLIT_EPSILON)
        {
            if(dist[0] > SPLIT_EPSILON)
            {
                vec3_sub(dir, curr_v->position, prev_v->position);
                vec3_ray_plane_intersect(prev_v->position, dir, n, tv.position, t);

                tv.normal[0] = prev_v->normal[0] + t * (curr_v->normal[0] - prev_v->normal[0]);
                tv.normal[1] = prev_v->normal[1] + t * (curr_v->normal[1] - prev_v->normal[1]);
                tv.normal[2] = prev_v->normal[2] + t * (curr_v->normal[2] - prev_v->normal[2]);
                vec3_norm(tv.normal, tmp);

                tv.color[0] = prev_v->color[0] + t * (curr_v->color[0] - prev_v->color[0]);
                tv.color[1] = prev_v->color[1] + t * (curr_v->color[1] - prev_v->color[1]);
                tv.color[2] = prev_v->color[2] + t * (curr_v->color[2] - prev_v->color[2]);
                tv.color[3] = prev_v->color[3] + t * (curr_v->color[3] - prev_v->color[3]);

                tv.tex_coord[0] = prev_v->tex_coord[0] + t * (curr_v->tex_coord[0] - prev_v->tex_coord[0]);
                tv.tex_coord[1] = prev_v->tex_coord[1] + t * (curr_v->tex_coord[1] - prev_v->tex_coord[1]);

                front->vertices[front->vertex_count++] = tv;
                back->vertices[back->vertex_count++] = tv;
            }
            back->vertices[back->vertex_count++] = *curr_v;
        }
        else
        {
            front->vertices[front->vertex_count++] = *curr_v;
            back->vertices[back->vertex_count++] = *curr_v;
        }

        prev_v = curr_v;
        curr_v ++;
        dist[0] = dist[1];
    }
}


int Polygon_IsInsideBBox(polygon_p p, btScalar bb_min[3], btScalar bb_max[3])
{
    vertex_p v = p->vertices;

    for(uint16_t i=0;i<p->vertex_count;i++,v++)
    {
        if((v->position[0] < bb_min[0]) || (v->position[0] > bb_max[0]) ||
           (v->position[1] < bb_min[1]) || (v->position[1] > bb_max[1]) ||
           (v->position[2] < bb_min[2]) || (v->position[2] > bb_max[2]))
        {
            return 0;
        }
    }

    return 1;
}


int Polygon_IsInsideBQuad(polygon_p p, btScalar bb_min[3], btScalar bb_max[3])
{
    vertex_p v = p->vertices;

    for(uint16_t i=0;i<p->vertex_count;i++,v++)
    {
        if((v->position[0] < bb_min[0]) || (v->position[0] > bb_max[0]) ||
           (v->position[1] < bb_min[1]) || (v->position[1] > bb_max[1]))
        {
            return 0;
        }
    }

    return 1;
}
