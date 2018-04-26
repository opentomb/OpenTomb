
#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <stdint.h>

#include "core/base_types.h"

#define LEVEL_NAME_MAX_LEN                      (64)
#define MAX_ENGINE_PATH                         (1024)


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
