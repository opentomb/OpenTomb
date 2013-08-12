
#ifndef WORLD_H
#define WORLD_H

#include <SDL/SDL_opengl.h>
#include <stdint.h>
#include "bullet/LinearMath/btScalar.h"

#define OBJECT_STATIC_MESH 0x0001
#define OBJECT_ROOM_BASE 0x0002
#define OBJECT_ENTITY 0x0003
#define OBJECT_BULLET_MISC 0x0004

class btCollisionShape;
class btRigidBody;

struct room_s;
struct polygon_s;
struct camera_s;
struct portal_s;
struct render_s;
struct frustum_s;
struct base_mesh_s;
struct static_mesh_s;
struct entity_s;
struct skeletal_model_s;
struct render_object_list_s;

typedef struct room_box_s
{
    int32_t     x_min;
    int32_t     x_max;
    int32_t     y_min;
    int32_t     y_max;
    int32_t     true_floor;
    int32_t     overlap_index;
}room_box_t, *room_box_p;

typedef struct room_sector_s
{
    int32_t                     box_index;              
    int32_t                     floor;
    int32_t                     ceiling;   
    
    uint16_t                    fd_index;
    int8_t                      fd_kill;
    int8_t                      fd_secret;
    int16_t                     fd_end_level;

    struct room_sector_s        *sector_below;                                  // сектор снизу
    struct room_sector_s        *sector_above;                                  // сектор сверху
    struct room_s               *owner_room;                                    // комната с этой ячейкой

    int16_t                     index_x;
    int16_t                     index_y;
    btScalar                    pos_x;
    btScalar                    pos_y;
}room_sector_t, *room_sector_p;


typedef struct room_sprite_s
{
    struct sprite_s             *sprite;
    btScalar                    pos[3];
    int8_t                      was_rendered;
}room_sprite_t, *room_sprite_p;


typedef struct room_s
{
    uint32_t                    ID;                                             // room's ID
    uint32_t                    flags;                                          // room's type + water, wind info
    int8_t                      is_in_r_list;                                   // находится ли комната в рендер листе
    int8_t                      hide;                                           // do not render
    struct base_mesh_s         *mesh;                                           // room's base mesh
    
    uint32_t                    static_mesh_count;
    struct static_mesh_s       *static_mesh;
    uint32_t                    sprites_count;
    struct room_sprite_s       *sprites;
    
    struct engine_container_s  *containers;                                     // контейнеры с "перемещаемыми" оъектами
    
    btScalar                    bb_min[3];                                      // Ограничивающий объем
    btScalar                    bb_max[3];                                      // Ограничивающий объем
    btScalar                    transform[16];                                  // 
    
    uint16_t                    portal_count;                                   // количество порталов
    struct portal_s            *portals;                                        // соединительные порталы
       
    int8_t                      use_alternate;                                  // определяет - использовать ли альтернативную комнату
    struct room_s              *alternate_room;                                 // указатель на альтернативную комнату.
    
    uint32_t                    sectors_count;
    uint16_t                    sectors_x;
    uint16_t                    sectors_y;
    struct room_sector_s       *sectors;
    
    uint16_t                    active_frustums;                                // количество активных фрустумовдля данной комнаты
    struct frustum_s           *frustum;                                        // структура фрустума изначально содержится в комнате
    struct frustum_s           *last_frustum; 
    uint16_t                    max_path;                                       // максимальная удаленность комнаты от камеры по количеству пересеченных порталов
    
    uint16_t                    near_room_list_size;
    struct room_s               *near_room_list[64];
    btRigidBody                 *bt_body;
    
    struct engine_container_s   *self;
}room_t, *room_p;


typedef struct world_s
{
    char                       *name;
    uint32_t                    ID;
    uint32_t                    room_count;
    struct room_s              *rooms;
   
    uint32_t                    room_box_count;
    struct room_box_s          *room_boxes;
    
    uint32_t                    tex_count;                                      // количество текстур
    GLuint                     *textures;                                       // индексы текстур GL
    uint32_t                    special_tex_count;                              // количество спец текстур
    GLuint                     *special_textures;                               // индексы спец текстур GL
    
    uint32_t                    meshs_count;                                    // количество базовых мешей
    struct base_mesh_s         *meshes;                                         // массив базовых мешей
    
    uint32_t                    sprites_count;                                  // количество базовых спрайтов
    struct sprite_s            *sprites;                                        // базовые спрайты
    
    uint32_t                    skeletal_model_count;                           // количество базовых скелетных моделей
    struct skeletal_model_s    *skeletal_models;                                // массив базовых скелетных моделей
    
    struct entity_s            *Lara;                                           // это Лара. она не принадлежит ни одному руму.  
    struct skeletal_model_s    *sky_box;                                        // это глобальный скайбокс %)
    
    uint32_t                    entity_count;
    struct entity_s            *entity_list;                                    // список активных моделей мира
    
    uint32_t                    type; 
    
    uint16_t                   *floor_data;
    uint32_t                    floor_data_size;
    
    int16_t                    *anim_commands;
    uint32_t                    anim_commands_count;
}world_t, *world_p;

void World_Prepare(world_p world);
void World_Empty(world_p world);

void Room_Empty(room_p room);
void Room_AddEntity(room_p room, struct entity_s *entity);
int Room_RemoveEntity(room_p room, struct entity_s *entity);
void Room_AddToNearRoomsList(room_p room, room_p r);

room_p Room_CheckAlternate(room_p room);
room_sector_p Sector_CheckAlternate(room_sector_p sector);

int Room_IsPointIn(room_p room, btScalar dot[3]);
room_p Room_FindPos2d(world_p w, btScalar pos[3]);

room_p Room_FindPos(world_p w, btScalar pos[3]);
room_p Room_FindPosCogerrence(world_p w, btScalar pos[3], room_p room);
room_p Room_FindPosCogerrence2d(world_p w, btScalar pos[3], room_p room);
struct entity_s *Entity_GetByID(unsigned int ID);
room_p Room_GetByID(world_p w, unsigned int ID);
room_sector_p Room_GetSector(room_p room, btScalar pos[3]);

int Room_IsJoined(room_p r1, room_p r2);
int Room_IsOverlapped(room_p r0, room_p r1);
int Room_IsInNearRoomsList(room_p room, room_p r);

int World_AddEntity(world_p world, struct entity_s *entity);
struct sprite_s* World_FindSpriteByID(unsigned int ID, world_p world);
struct skeletal_model_s* World_FindModelByID(world_p w, uint32_t id);           // binary search the model by ID


#endif
