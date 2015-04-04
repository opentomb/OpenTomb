
#ifndef SYS_DEF_H
#define SYS_DEF_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>

#include <sys/time.h>
#include <stdio.h>
#include <string.h>

void Engine_Display();

void Engine_InitGL();
void Engine_InitSDLControls();
void Engine_InitSDLVideo();
void Engine_InitSDLImage();
void Engine_InitAL();

// Nominal values are used e.g. to set the size for the console.
// pixel values are used for glViewport. Both will be the same on
// normal displays, but on retina displays or similar, pixels will be twice nominal (or more).

void Engine_Resize(int nominalW, int nominalH, int pixelsW, int pixelsH);

void Engine_PollSDLInput();
void Engine_PrimaryMouseDown();
void Engine_SecondaryMouseDown();
void DebugKeys(int button, int state);

// Debug draw routines.

void ShowDebugInfo();
void SkeletalModelTestDraw();

#endif
