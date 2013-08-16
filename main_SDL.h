
#ifndef SYS_DEF_H
#define SYS_DEF_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>

#include <sys/time.h>
#include <stdio.h>
#include <string.h>


void TestGenScene();
int KeyToChar(SDL_Event *event);

void Engine_PrepareOpenGL();
//void Engine_TerminateProgram();
void Engine_Display();
// nominal values are used e.g. to set the size for the console.
// pixel values are used for glViewport. Both will be the same on
// normal displays, but on retina displays or similar, pixels will be twice nominal (or more)
void Engine_Resize(int nominalW, int nominalH, int pixelsW, int pixelsH);
void Engine_PrimaryMouseDown();
void Engine_SecondaryMouseDown();
void DebugKeys(int button, int state);
//void Engine_KeyDown(SDLKey key);
//void Engine_KeyUp(SDLKey key);
//void Engine_SetShiftDown(bool isDown);
//void Engine_UpdateTime(double time);
//void Engine_UpdateGame(double time);

#endif
