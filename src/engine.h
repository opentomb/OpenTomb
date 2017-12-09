
#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <stdint.h>

#include "core/base_types.h"

#define LEVEL_NAME_MAX_LEN                      (64)
#define MAX_ENGINE_PATH                         (1024)


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

    int8_t      look;                              // Look (camera) keys.
    int8_t      look_up;
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

void Engine_Start(int argc, char **argv);
void Engine_Shutdown(int val) __attribute__((noreturn));
const char *Engine_GetBasePath();
void Engine_SetDone();
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
