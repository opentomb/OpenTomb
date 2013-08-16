
#ifndef MESH_H
#define MESH_H

#define MESH_FULL_OPAQUE 0x00                                                   // Fully opaque object (all polygons are opaque: all t.flags < 0x02)
#define MESH_FULL_TRANSPERENCY 0x01                                             // Fully transparancy object (all polygons are transparancy: all t.flags >= 2)
#define MESH_PART_TRANSPERENCY 0x02                                             // Has transparancy and opaque polygons

#include <SDL2/SDL_opengl.h>
#include <stdint.h>
#include "bullet/LinearMath/btScalar.h"

class btCollisionShape;
class btRigidBody;
class btCollisionShape;

struct polygon_s;
struct room_s;
struct engine_container_s;
struct bounding_volume_s;
struct vertex_s;

/*
 * base mesh, uses everywhere
 */
typedef struct base_mesh_s
{
    uint32_t              ID;                                                   // mesh's ID
    uint32_t              transparancy_flags;                                   // transparancy flags
    
    uint32_t              transparancy_count;                                   // number of transparancy polygons
    uint32_t              poly_count;                                           // number of all mesh's polygons
    struct polygon_s     *polygons;                                             // polygons data
    
    uint32_t num_texture_pages;                                                 // face without structure wrapping
    uint32_t *element_count_per_texture;                                        //
    uint32_t *elements;                                                         //
    
    uint32_t              vertex_count;                                         // number of mesh's vertices
    struct vertex_s      *vertices;

    btScalar              centre[3];                                            // geometry centre of mesh
    btScalar              bb_min[3];                                            // AABB bounding volume
    btScalar              bb_max[3];                                            // AABB bounding volume
    btScalar              R;                                                    // radius of the bounbing sphere
    int8_t               *skin_map;                                             // vertices map for skin mesh

    GLuint                vbo_vertex_array;
    GLuint                vbo_index_array;
}base_mesh_t, *base_mesh_p;


/*
 * base sprite structure
 */
typedef struct sprite_s
{
    uint32_t            ID;                                                     // object's ID
    uint32_t            texture;                                                // texture number
    GLfloat             tex_coord[8];                                           // texture coordinates
    uint32_t            flag; 
    btScalar            left;                                                   // world sprite's gabarites
    btScalar            right;
    btScalar            top;
    btScalar            bottom;
}sprite_t, *sprite_p;

/*
 * room static mesh.
 */
typedef struct static_mesh_s
{
    uint32_t                    ID;                                             // ID модели
    uint8_t                     was_rendered;                                   // 0 - was not rendered, 1 - opaque, 2 - transparancy, 3 - full rendered
    uint8_t                     was_rendered_lines;
    uint8_t                     hide;                                           // disable static mesh rendering
    btScalar                    pos[3];                                         // model position
    btScalar                    rot[3];                                         // model angles
    btScalar                    transform[16];                                  // gl transformation matrix
    struct bounding_volume_s   *bv;
    struct engine_container_s  *self;
    
    struct base_mesh_s         *mesh;                                           // base model
    btRigidBody                *bt_body;
}static_mesh_t, *static_mesh_p;

/*
 * Это анимированные модели типа игрок, непись, креатура, двери, ловушки, прочая хрень
 */

/*
 * Структура костяной анимированной модели. Взято с openraider.
 * модель -> анимация -> фрейм -> кость
 * спасибо Terry 'Mongoose' Hendrix II
 */

/*
 * SMOOTCHED ANIMATIONS
 */
typedef struct ss_bone_tag_s
{
    base_mesh_p         mesh;                                                   // базовый меш
    base_mesh_p         mesh2;                                                  // базовый меш 2, для ТР4+
    btScalar            offset[3];                                              // смещение
 
    btScalar            qrotate[4];  
    btScalar            transform[16];                                          // матрица 4 на 4 трансформации, напрямую используется в OpenGL - стековая
    btScalar            full_transform[16];                                     // матрица 4 на 4 трансформации, напрямую используется в OpenGL - полная
    
    uint16_t            flag;                                                   // флаг стека // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
    uint16_t            overrided;                                              // флаг - модель заменена на дополнительную анимацию
}ss_bone_tag_t, *ss_bone_tag_p;

/*
 * базовый кадр анимационной скелетной модели
 */

typedef struct ss_bone_frame_s
{
    uint16_t                    bone_tag_count;                                 // количество костей
    struct ss_bone_tag_s       *bone_tags;                                      // массив костей
    btScalar                    pos[3];                                         // позиция (базовое смещение)
    btScalar                    bb_min[3];                                      // ограничивающий бокс
    btScalar                    bb_max[3];                                      // ограничивающий бокс
    btScalar                    centre[3];                                      // Центр ограничивающего бокса
}ss_bone_frame_t, *ss_bone_frame_p;

/*
 * ORIGINAL ANIMATIONS
 */

/*
 * базовый элемент анимационной скелетной модели (кость, bone)
 * изначально были только углы поворота. для снятия нагрузок со стека OpenGL
 * и уменьшения вычислений теперь хранится полная матрица преобразований
 * стек не нужен. необходимо для шкурной анимации.
 */
typedef struct bone_tag_s
{
    btScalar              offset[3];                                            // смещение
    btScalar              rotate[3];                                            // поворот кости   
    btScalar              qrotate[4];                                           // rotation quaternion
}bone_tag_t, *bone_tag_p;

/*
 * базовый кадр анимационной скелетной модели
 */

typedef struct bone_frame_s
{
    uint16_t            bone_tag_count;                                         // количество костей
    struct bone_tag_s  *bone_tags;                                              // массив костей
    btScalar            pos[3];                                                 // позиция (базовое смещение)
    btScalar            bb_min[3];                                              // ограничивающий бокс
    btScalar            bb_max[3];                                              // ограничивающий бокс
    btScalar            centre[3];                                              // Центр ограничивающего бокса
}bone_frame_t, *bone_frame_p;

/*
 * Базовый элемент дерева мешей
 */

typedef struct mesh_tree_tag_s                                                  
{
    base_mesh_p                 mesh;                                           // базовый меш
    base_mesh_p                 mesh2;                                          // базовый меш 2, для ТР4+
    btScalar                    offset[3];                                      // смещение
    uint16_t                    flag;                                           // флаг стека // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
    uint16_t                    overrided;                                      // флаг - модель заменена на дополнительную анимацию 
}mesh_tree_tag_t, *mesh_tree_tag_p;

/*
 * Структура переходов от одной анимации к другой
 */

typedef struct anim_dispath_s
{
    uint16_t    next_anim;                                                      // Следующая анимация
    uint16_t    next_frame;                                                     // Стартовый фрейм следующей анимации
    uint16_t    frame_low;                                                      // Нижняя граний номера фрейма с которого можно начинать переход к следующему
    uint16_t    frame_high;                                                     // Верхняя граний номера фрейма с которого можно начинать переход к следующему
} anim_dispath_t, *anim_dispath_p;

typedef struct state_change_s
{
    uint32_t                    ID;
    uint16_t                    anim_dispath_count;
    struct anim_dispath_s      *anim_dispath;
} state_change_t, *state_change_p;

/*
 * один фрейм анимацимаций модели
 */

typedef struct animation_frame_s                                                
{
    unsigned int                ID;
    unsigned char               frame_rate;
    int                         accel_hi;
    int                         accel_hi2;
    int                         accel_lo;
    int                         accel_lo2;
    int                         speed;
    int                         speed2;
    uint32_t                    anim_command;
    uint32_t                    num_anim_commands;
    unsigned int                state_id;
    int                         unknown;
    int                         unknown2;
    unsigned int                frames_count;                                   // количество фреймов анимации
    struct bone_frame_s        *frames;                                         // все анимации
    
    unsigned int                state_change_count;                             // количество смен анимаций
    struct state_change_s      *state_change;                                   // данные о сменах анимаций
    
    struct animation_frame_s   *next_anim;                                      // следующая анимация
    int                         next_frame;                                     // следующий фрейм
}animation_frame_t, *animation_frame_p;

/*
 * skeletal model with animations data.
 */

typedef struct skeletal_model_s
{
    uint32_t                    ID;                                             // ID
    uint16_t                    transparancy_flags;                             // флаги прозрачности
    uint16_t                    hide;                                           // do not render
    btScalar                    bbox_min[3];                                    // данные о размерах модели
    btScalar                    bbox_max[3];
    btScalar                    centre[3];                                      // центр модели
        
    uint16_t                    animation_count;                                // количество анимаций
    struct animation_frame_s   *animations;                                     // сами анимации
    
    uint16_t                    all_frames_count;                               // количество всех фреймов
    struct bone_frame_s        *all_bone_frames;                                // все фреймы модели
    struct bone_tag_s          *all_bone_tags;                                  // все повороты
    
    uint16_t                    mesh_count;                                     // количество мешей модели
    struct base_mesh_s         *mesh_offset;                                    // смещение от базовых мешей к мешам модели, дефолтный меш
    struct mesh_tree_tag_s     *mesh_tree;                                      // базовый скелет модели.
}skeletal_model_t, *skeletal_model_p; 


void BaseMesh_Clear(base_mesh_p mesh);
void BaseMesh_FindBB(base_mesh_p mesh);
void Mesh_GenVBO(struct base_mesh_s *mesh);

void Mesh_DefaultColor(struct base_mesh_s *mesh);
void Mesh_MullColors(struct base_mesh_s *mesh, float *cl_mult);

void SkeletalModel_Clear(skeletal_model_p model);
void SkeletalModel_FillRotations(skeletal_model_p model);
void SkeletonModelFillTransparancy(skeletal_model_p model);
void FillSkinnedMeshMap(skeletal_model_p model);

void BoneFrame_Copy(bone_frame_p dst, bone_frame_p src);
mesh_tree_tag_p SkeletonClone(mesh_tree_tag_p src, int tags_count);
void SkeletonCopyMeshes(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count);
void SkeletonCopyMeshes2(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count);


uint32_t Mesh_AddVertex(base_mesh_p mesh, struct vertex_s *vertex);
void Mesh_GenFaces(base_mesh_p mesh);

#endif
