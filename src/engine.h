
#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <stdint.h>

#define LEVEL_NAME_MAX_LEN                      (64)
#define MAX_ENGINE_PATH                         (1024)

#define OBJECT_STATIC_MESH                      (0x0001)
#define OBJECT_ROOM_BASE                        (0x0002)
#define OBJECT_ENTITY                           (0x0003)
#define OBJECT_HAIR                             (0x0004)
#define OBJECT_BULLET_MISC                      (0x7FFF)

#define COLLISION_SHAPE_BOX                     0x0001
#define COLLISION_SHAPE_BOX_BASE                0x0002     // use mesh box collision
#define COLLISION_SHAPE_SPHERE                  0x0003
#define COLLISION_SHAPE_TRIMESH                 0x0004     // for static objects and room's!
#define COLLISION_SHAPE_TRIMESH_CONVEX          0x0005     // for dynamic objects
#define COLLISION_SHAPE_SINGLE_BOX              0x0006     // use single box collision
#define COLLISION_SHAPE_SINGLE_SPHERE           0x0007

#define COLLISION_NONE                          (0x0000)
#define COLLISION_MASK_ALL                      (0x7FFF)        // bullet uses signed short int for these flags!

#define COLLISION_GROUP_ALL                     (0x7FFF)
#define COLLISION_GROUP_STATIC_ROOM             (0x0001)        // room mesh
#define COLLISION_GROUP_STATIC_OBLECT           (0x0002)        // room static object
#define COLLISION_GROUP_KINEMATIC               (0x0004)        // doors, blocks, static animated entityes
//#define COLLISION_GROUP_GHOST                   (0x0008)        // probe objects
#define COLLISION_GROUP_TRIGGERS                (0x0010)        // probe objects
#define COLLISION_GROUP_CHARACTERS              (0x0020)        // Lara, enemies, friends, creatures
#define COLLISION_GROUP_VEHICLE                 (0x0040)        // car, moto, bike
#define COLLISION_GROUP_BULLETS                 (0x0080)        // bullets, rockets, grenades, arrows...
#define COLLISION_GROUP_DYNAMICS                (0x0100)        // test balls, warious
#define COLLISION_GROUP_DYNAMICS_NI             (0x0200)        // test balls, warious


#define COLLISION_FILTER_CHARACTER              (COLLISION_GROUP_STATIC_ROOM | COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_KINEMATIC | \
                                                 COLLISION_GROUP_CHARACTERS | COLLISION_GROUP_VEHICLE | COLLISION_GROUP_DYNAMICS)

#define COLLISION_FILTER_HEIGHT_TEST            (COLLISION_GROUP_STATIC_ROOM | COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_KINEMATIC | COLLISION_GROUP_VEHICLE)

typedef struct engine_container_s
{
    uint16_t                     object_type;
    uint16_t                     collision_shape;
    int16_t                      collision_group;
    int16_t                      collision_mask;
    void                        *object;
    struct room_s               *room;
    struct room_sector_s        *sector;
    struct engine_container_s   *next;
}engine_container_t, *engine_container_p;

typedef struct engine_control_state_s
{
    float       free_look_speed;
    float       cam_distance;
    float       cam_angles[3];
    float       look_axis_x;                       // Unified look axis data.
    float       look_axis_y;

    int8_t      free_look;
    int8_t      mouse_look;
    int8_t      noclip;
    int8_t      move_forward;                      // Directional movement keys.
    int8_t      move_backward;
    int8_t      move_left;
    int8_t      move_right;
    int8_t      move_up;                           // These are not typically used.
    int8_t      move_down;

    int8_t      look_up;                           // Look (camera) keys.
    int8_t      look_down;
    int8_t      look_left;
    int8_t      look_right;
    int8_t      look_roll_left;
    int8_t      look_roll_right;

    int8_t      do_jump;                              // Eventual actions.
    int8_t      do_draw_weapon;
    int8_t      do_roll;

    int8_t      state_action;                         // Conditional actions.
    int8_t      state_walk;
    int8_t      state_sprint;
    int8_t      state_crouch;
    int8_t      state_look;

    int8_t      use_flare;                            // Use item hotkeys.
    int8_t      use_big_medi;
    int8_t      use_small_medi;

    int8_t      use_prev_weapon;                      // Weapon hotkeys.
    int8_t      use_next_weapon;
    int8_t      use_weapon1;
    int8_t      use_weapon2;
    int8_t      use_weapon3;
    int8_t      use_weapon4;
    int8_t      use_weapon5;
    int8_t      use_weapon6;
    int8_t      use_weapon7;
    int8_t      use_weapon8;


    int8_t      gui_pause;                         // GUI keys - not sure if it must be here.
    int8_t      gui_inventory;

}engine_control_state_t, *engine_control_state_p;


extern float                                 engine_frame_time;
extern struct camera_s                       engine_camera;
extern struct camera_state_s                 engine_camera_state;

engine_container_p Container_Create();
void Container_Delete(engine_container_p cont);

void Engine_Start(int argc, char **argv);
void Engine_Shutdown(int val) __attribute__((noreturn));
const char *Engine_GetBasePath();
void Engine_SetDone();
void Engine_LoadConfig(const char *filename);
void Engine_SaveConfig(const char *filename);
void Engine_JoyRumble(float power, int time);

void Engine_GLSwapWindow();
void Engine_MainLoop();

// PC-specific level loader routines.

bool Engine_LoadPCLevel(const char *name);

// General level loading routines.

void Engine_TakeScreenShot();
void Engine_GetLevelName(char *name, const char *path);
void Engine_GetLevelScriptNameLocal(const char *level_path, int game_version, char *name, uint32_t buf_size);
int  Engine_LoadMap(const char *name);
int  Engine_PlayVideo(const char *name);
int  Engine_IsVideoPlayed();

#endif
