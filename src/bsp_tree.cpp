
#include <stdint.h>
#include <SDL2/SDL_opengl.h>
#include "bullet/LinearMath/btScalar.h"
#include "polygon.h"
#include "bsp_tree.h"
#include "vmath.h"


bsp_node_p BSP_CreateNode()
{
    bsp_node_p ret = (bsp_node_p)malloc(sizeof(bsp_node_t));
    ret->front = NULL;
    ret->back = NULL;
    ret->polygons_front = NULL;
    ret->polygons_back = NULL;

    return ret;
}

void BSP_AddPolygon(struct bsp_node_s *root, struct polygon_s *p)
{
    if(Polygon_IsBroken(p))
    {
        return;
    }

    if(root->polygons_front == NULL)
    {
        // we though root->front == NULL and root->back == NULL
        root->polygons_front = (polygon_p)malloc(sizeof(polygon_t));
        root->polygons_front->next = NULL;
        root->polygons_front->vertex_count = 0;
        root->polygons_front->vertices = NULL;
        Polygon_Copy(root->polygons_front, p);
        vec4_copy(root->plane, p->plane);
        return;
    }

    int split_type = Polygon_SplitClassify(p, root->plane);

    switch(split_type)
    {
        case SPLIT_IN_PLANE:
            {
                polygon_p lp = (polygon_p)malloc(sizeof(polygon_t));
                lp->vertex_count = 0;
                lp->vertices = NULL;
                Polygon_Copy(lp, p);
                if(vec3_dot(p->plane, root->plane) > 0.9)
                {
                    lp->next = root->polygons_front;
                    root->polygons_front = lp;
                }
                else
                {
                    lp->next = root->polygons_back;
                    root->polygons_back = lp;
                }
            }
            break;

        case SPLIT_FRONT:
            if(root->front == NULL)
            {
                root->front = BSP_CreateNode();
            }
            BSP_AddPolygon(root->front, p);
            break;

        case SPLIT_BACK:
            if(root->back == NULL)
            {
                root->back = BSP_CreateNode();
            }
            BSP_AddPolygon(root->back, p);
            break;

        case SPLIT_IN_BOTH:
            {
                polygon_p front, back;
                front = (polygon_p)malloc(sizeof(polygon_t));
                front->vertex_count = 0;
                front->vertices = NULL;
                front->next = NULL;
                back = (polygon_p)malloc(sizeof(polygon_t));
                back->vertex_count = 0;
                back->vertices = NULL;
                back->next = NULL;
                Polygon_Split(p, root->plane, front, back);

                if(root->front == NULL)
                {
                    root->front = BSP_CreateNode();
                }
                BSP_AddPolygon(root->front, front);
                Polygon_Clear(front);
                free(front);
                if(root->back == NULL)
                {
                    root->back = BSP_CreateNode();
                }
                BSP_AddPolygon(root->back, back);
                Polygon_Clear(back);
                free(back);
            }
            break;
    };
}

void SBP_FreeTree(struct bsp_node_s *root)
{
    if(root->polygons_front != NULL)
    {
        polygon_p p = root->polygons_front;
        for(polygon_p next=p->next;p!=NULL;)
        {
            Polygon_Clear(p);
            free(p);
            p = next;
            if(p != NULL)
            {
                next = p->next;
            }
        }
        root->polygons_front = NULL;
    }
    if(root->polygons_back != NULL)
    {
        polygon_p p = root->polygons_back;
        for(polygon_p next=p->next;p!=NULL;)
        {
            Polygon_Clear(p);
            free(p);
            p = next;
            if(p != NULL)
            {
                next = p->next;
            }
        }
        root->polygons_back = NULL;
    }

    if(root->front != NULL)
    {
        SBP_FreeTree(root->front);
        root->front = NULL;
    }

    if(root->back != NULL)
    {
        SBP_FreeTree(root->back);
        root->back = NULL;
    }

    free(root);
}
