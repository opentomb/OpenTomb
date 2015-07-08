#include <ctime>
#include <cstdio>
#include <cstring>

#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_video.h>

#if !defined(__MACOSX__)
#include <SDL2/SDL_image.h>
#endif

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_haptic.h>

#include <lua.hpp>

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "vt/vt_level.h"

#include "obb.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "main_SDL.h"
#include "gl_util.h"
#include "script.h"
#include "console.h"
#include "system.h"
#include "common.h"
#include "camera.h"
#include "polygon.h"
#include "portal.h"
#include "engine.h"
#include "controls.h"
#include "world.h"
#include "render.h"
#include "vmath.h"
#include "gui.h"
#include "mesh.h"
#include "game.h"
#include "resource.h"
#include "entity.h"
#include "audio.h"
#include "gameflow.h"
//#include "string.h"

#if defined(__MACOSX__)
#include "FindConfigFile.h"
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <lua.hpp>
#include "LuaState.h"

#define NO_AUDIO        0

SDL_Window             *sdl_window     = NULL;
SDL_Joystick           *sdl_joystick   = NULL;
SDL_GameController     *sdl_controller = NULL;
SDL_Haptic             *sdl_haptic     = NULL;
SDL_GLContext           sdl_gl_context = 0;
ALCdevice              *al_device      = NULL;
ALCcontext             *al_context     = NULL;

int done = 0;
btScalar time_scale = 1.0;

btVector3 light_position = {255.0, 255.0, 8.0};
GLfloat cast_ray[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

std::shared_ptr<EngineContainer> last_cont = nullptr;

// BULLET IS PERFECT PHYSICS LIBRARY!!!
/*
 * 1) console
 *      - add notify functions
 * 2) LUA enngine global script:
 *      - add base functions for entity manipulation, I.E.: health, collision callbacks,
 *        spawn, new, delete, inventory manipulation.
 * 3) Skeletal models functionality:
 *      - add multianimation system: weapon animations (not a car / byke case: it ia two entities
 *        with the same coordinates / orientation);
 *      - add head, torso rotation for actor->look_at (additional and final multianimation);
 *      - add mesh replace system (I.E. Lara's head wit emotion / speek);
 *      - fix animation interpolation in animations switch case;
 * 6) Menu (create own menu)
 *      - settings
 *      - map loading list
 *      - saves loading list
 * 10) OpenGL
 *      - shaders
 *      - reflections
 *      - shadows
 *      - particles
 *      - GL and renderer optimisations
 * 40) Physics / gameplay
 *      - optimize and fix character controller, bug fixes: permanent task
 *      - weapons
 * 41) scripts module
 *      - cutscenes playing
 *      - enemies AI
 *      - end level -> next level
 * 42) sound
 *      - click removal;
 *      - add ADPCM and CDAUDIO.WAD soundtrack support;
 */

// =======================================================================
// MAIN
// =======================================================================


void Engine_InitGL()
{
    glewInit();
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    if(renderer.settings().antialias)
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }

    // Default state for Alpha func: >= 0.5. That's what all users of alpha
    // function use anyway.
    glAlphaFunc(GL_GEQUAL, 0.5);
}

void Engine_InitSDLControls()
{
    int    NumJoysticks;
    Uint32 init_flags    = SDL_INIT_VIDEO | SDL_INIT_EVENTS;                    // These flags are used in any case.

    if(control_mapper.use_joy == 1)
    {
        init_flags |= SDL_INIT_GAMECONTROLLER;                                  // Update init flags for joystick.

        if(control_mapper.joy_rumble)
        {
            init_flags |= SDL_INIT_HAPTIC;                                      // Update init flags for force feedback.
        }

        SDL_Init(init_flags);

        NumJoysticks = SDL_NumJoysticks();
        if((NumJoysticks < 1) || ((NumJoysticks - 1) < control_mapper.joy_number))
        {
            Sys_DebugLog(LOG_FILENAME, "Error: there is no joystick #%d present.", control_mapper.joy_number);
            return;
        }

        if(SDL_IsGameController(control_mapper.joy_number))                     // If joystick has mapping (e.g. X360 controller)
        {
            SDL_GameControllerEventState(SDL_ENABLE);                           // Use GameController API
            sdl_controller = SDL_GameControllerOpen(control_mapper.joy_number);

            if(!sdl_controller)
            {
                Sys_DebugLog(LOG_FILENAME, "Error: can't open game controller #%d.", control_mapper.joy_number);
                SDL_GameControllerEventState(SDL_DISABLE);                      // If controller init failed, close state.
                control_mapper.use_joy = 0;
            }
            else if(control_mapper.joy_rumble)                                  // Create force feedback interface.
            {
                    sdl_haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(sdl_controller));
                    if(!sdl_haptic)
                    {
                        Sys_DebugLog(LOG_FILENAME, "Error: can't initialize haptic from game controller #%d.", control_mapper.joy_number);
                    }
            }
        }
        else
        {
            SDL_JoystickEventState(SDL_ENABLE);                                 // If joystick isn't mapped, use generic API.
            sdl_joystick = SDL_JoystickOpen(control_mapper.joy_number);

            if(!sdl_joystick)
            {
                Sys_DebugLog(LOG_FILENAME, "Error: can't open joystick #%d.", control_mapper.joy_number);
                SDL_JoystickEventState(SDL_DISABLE);                            // If joystick init failed, close state.
                control_mapper.use_joy = 0;
            }
            else if(control_mapper.joy_rumble)                                  // Create force feedback interface.
            {
                sdl_haptic = SDL_HapticOpenFromJoystick(sdl_joystick);
                if(!sdl_haptic)
                {
                    Sys_DebugLog(LOG_FILENAME, "Error: can't initialize haptic from joystick #%d.", control_mapper.joy_number);
                }
            }
        }

        if(sdl_haptic)                                                          // To check if force feedback is working or not.
        {
            SDL_HapticRumbleInit(sdl_haptic);
            SDL_HapticRumblePlay(sdl_haptic, 1.0, 300);
        }
    }
    else
    {
        SDL_Init(init_flags);
    }
}

void Engine_InitSDLVideo()
{
    Uint32 video_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;

    if(screen_info.FS_flag)
    {
        video_flags |= SDL_WINDOW_FULLSCREEN;
    }
    else
    {
        video_flags |= (SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    }

    ///@TODO: is it really needede for correct work?
    if(SDL_GL_LoadLibrary(NULL) < 0)
    {
        Sys_Error("Could not init OpenGL driver");
    }

    // Check for correct number of antialias samples.
    if(renderer.settings().antialias)
    {
        /* I do not know why, but settings of this temporary window (zero position / size) are applied to the main window, ignoring screen settings */
        sdl_window     = SDL_CreateWindow(NULL, screen_info.x, screen_info.y, screen_info.w, screen_info.h, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        sdl_gl_context = SDL_GL_CreateContext(sdl_window);
        SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);

        GLint maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        maxSamples = (maxSamples > 16)?(16):(maxSamples);   // Fix for faulty GL max. sample number.

        if(renderer.settings().antialias_samples > maxSamples)
        {
            if(maxSamples == 0)
            {
                renderer.settings().antialias = 0;
                renderer.settings().antialias_samples = 0;
                Sys_DebugLog(LOG_FILENAME, "InitSDLVideo: can't use antialiasing");
            }
            else
            {
                renderer.settings().antialias_samples = maxSamples;   // Limit to max.
                Sys_DebugLog(LOG_FILENAME, "InitSDLVideo: wrong AA sample number, using %d", maxSamples);
            }
        }

        SDL_GL_DeleteContext(sdl_gl_context);
        SDL_DestroyWindow(sdl_window);

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, renderer.settings().antialias);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, renderer.settings().antialias_samples);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, renderer.settings().z_depth);
#if STENCIL_FRUSTUM
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif
    // set the opengl context version
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    sdl_window = SDL_CreateWindow("OpenTomb", screen_info.x, screen_info.y, screen_info.w, screen_info.h, video_flags);
    sdl_gl_context = SDL_GL_CreateContext(sdl_window);
    SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);

    ConsoleInfo::instance().addLine((const char*)glGetString(GL_VENDOR), FONTSTYLE_CONSOLE_INFO);
    ConsoleInfo::instance().addLine((const char*)glGetString(GL_RENDERER), FONTSTYLE_CONSOLE_INFO);
    std::string version = "OpenGL version ";
    version += (const char*)glGetString(GL_VERSION);
    ConsoleInfo::instance().addLine(version, FONTSTYLE_CONSOLE_INFO);
    ConsoleInfo::instance().addLine((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION), FONTSTYLE_CONSOLE_INFO);
}

#if !defined(__MACOSX__)
void Engine_InitSDLImage()
{
    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int init  = IMG_Init(flags);

    if((init & flags) != flags)
    {
        Sys_DebugLog(LOG_FILENAME, "SDL_Image error: failed to initialize JPG and/or PNG support.");
    }
}
#endif

void Engine_InitAL()
{
#if !NO_AUDIO
    ALCint paramList[] = {
        ALC_STEREO_SOURCES,  TR_AUDIO_STREAM_NUMSOURCES,
        ALC_MONO_SOURCES,   (TR_AUDIO_MAX_CHANNELS - TR_AUDIO_STREAM_NUMSOURCES),
        ALC_FREQUENCY,       44100, 0};

    //const char *drv = SDL_GetCurrentAudioDriver();

    al_device = alcOpenDevice(NULL);
    if (!al_device)
    {
        Sys_DebugLog(LOG_FILENAME, "InitAL: No AL audio devices!");
        return;
    }

    al_context = alcCreateContext(al_device, paramList);
    if(!alcMakeContextCurrent(al_context))
    {
        Sys_DebugLog(LOG_FILENAME, "InitAL: AL context is not current!");
        return;
    }

    alSpeedOfSound(330.0 * 512.0);
    alDopplerVelocity(330.0 * 510.0);
    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
#endif
}

int main(int /*argc*/, char** /*argv*/)
{
    btScalar time, newtime;
    static btScalar oldtime = 0.0;

    Engine_Start();

    // Entering main loop.
    while(!done)
    {
        newtime = Sys_FloatTime();
        time = newtime - oldtime;
        oldtime = newtime;
        Engine_Frame(time * time_scale);
    }

    // Main loop interrupted; shutting down.
    Engine_Shutdown(EXIT_SUCCESS);
    return(EXIT_SUCCESS);
}

void Engine_Start()
{
#if defined(__MACOSX__)
    FindConfigFile();
#endif

    // Set defaults parameters and load config file.
    Engine_InitConfig("config.lua");

    // Primary initialization.
    Engine_Init_Pre();

    // Init generic SDL interfaces.
    Engine_InitSDLControls();
    Engine_InitSDLVideo();

#if !defined(__MACOSX__)
    Engine_InitSDLImage();
#endif

    // Additional OpenGL initialization.
    Engine_InitGL();
    renderer.doShaders();

    // Secondary (deferred) initialization.
    Engine_Init_Post();

    // Initial window resize.
    Engine_Resize(screen_info.w, screen_info.h, screen_info.w, screen_info.h);

    // OpenAL initialization.
    Engine_InitAL();

    // Clearing up memory for initial level loading.
    engine_world.prepare();

#ifdef NDEBUG
    // Setting up mouse.
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_WarpMouseInWindow(sdl_window, screen_info.w/2, screen_info.h/2);
    SDL_ShowCursor(0);
#endif

    // Make splash screen.
    Gui_FadeAssignPic(FADER_LOADSCREEN, "resource/graphics/legal.png");
    Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_OUT);

    luaL_dofile(engine_lua.getState(), "autoexec.lua");
}


void Engine_Display()
{
    if(!done)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//| GL_ACCUM_BUFFER_BIT);

        engine_camera.apply();
        engine_camera.recalcClipPlanes();
        // GL_VERTEX_ARRAY | GL_COLOR_ARRAY
        if(screen_info.show_debuginfo)
        {
            ShowDebugInfo();
        }

        glFrontFace(GL_CW);

        renderer.genWorldList();
        renderer.drawList();

        //glDisable(GL_CULL_FACE);
        //Render_DrawAxis(10000.0);
        /*if(engine_world.character)
        {
            glPushMatrix();
            glTranslatef(engine_world.character->transform[12], engine_world.character->transform[13], engine_world.character->transform[14]);
            Render_DrawAxis(1000.0);
            glPopMatrix();
        }*/

        Gui_SwitchGLMode(1);
        {
            glEnable(GL_ALPHA_TEST);

            Gui_DrawNotifier();
            if(engine_world.character && main_inventory_manager)
            {
                Gui_DrawInventory();
            }
        }

        Gui_Render();
        Gui_SwitchGLMode(0);

        renderer.drawListDebugLines();

        SDL_GL_SwapWindow(sdl_window);
    }
}

void Engine_Resize(int nominalW, int nominalH, int pixelsW, int pixelsH)
{
    screen_info.w = nominalW;
    screen_info.h = nominalH;

    screen_info.w_unit = (float)nominalW / GUI_SCREEN_METERING_RESOLUTION;
    screen_info.h_unit = (float)nominalH / GUI_SCREEN_METERING_RESOLUTION;
    screen_info.scale_factor = (screen_info.w < screen_info.h)?(screen_info.h_unit):(screen_info.w_unit);

    Gui_Resize();

    engine_camera.setFovAspect(screen_info.fov, (btScalar)nominalW/(btScalar)nominalH);
    engine_camera.recalcClipPlanes();

    glViewport(0, 0, pixelsW, pixelsH);
}

void Engine_Frame(btScalar time)
{
    static int cycles = 0;
    static btScalar time_cycl = 0.0;
    extern gui_text_line_t system_fps;
    if(time > 0.1)
    {
        time = 0.1;
    }

    engine_frame_time = time;
    if(cycles < 20)
    {
        cycles++;
        time_cycl += time;
    }
    else
    {
        screen_info.fps = (20.0 / time_cycl);
        snprintf(system_fps.text, system_fps.text_size, "%.1f", screen_info.fps);
        cycles = 0;
        time_cycl = 0.0;
    }

    Game_Frame(time);
    Gameflow_Do();

    Engine_Display();
}


void ShowDebugInfo()
{
    GLfloat color_array[] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};

    light_position = engine_camera.m_pos;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(2.0);
    glVertexPointer(3, GL_FLOAT, 0, cast_ray);
    glColorPointer(3, GL_FLOAT, 0, color_array);
    glDrawArrays(GL_LINES, 0, 2);

    std::shared_ptr<Character> ent = engine_world.character;
    if(ent)
    {
        /*height_info_p fc = &ent->character->height_info
        txt = Gui_OutTextXY(20.0 / screen_info.w, 80.0 / screen_info.w, "Z_min = %d, Z_max = %d, W = %d", (int)fc->floor_point[2], (int)fc->ceiling_point[2], (int)fc->water_level);
        */

        Gui_OutTextXY(30.0, 30.0, "last_anim = %03d, curr_anim = %03d, next_anim = %03d, last_st = %03d, next_st = %03d", ent->m_bf.animations.last_animation, ent->m_bf.animations.current_animation, ent->m_bf.animations.next_animation, ent->m_bf.animations.last_state, ent->m_bf.animations.next_state);
        //Gui_OutTextXY(30.0, 30.0, "curr_anim = %03d, next_anim = %03d, curr_frame = %03d, next_frame = %03d", ent->bf.animations.current_animation, ent->bf.animations.next_animation, ent->bf.animations.current_frame, ent->bf.animations.next_frame);
        //Gui_OutTextXY(NULL, 20, 8, "posX = %f, posY = %f, posZ = %f", engine_world.character->transform[12], engine_world.character->transform[13], engine_world.character->transform[14]);
    }

    if(last_cont != NULL)
    {
        switch(last_cont->object_type)
        {
            case OBJECT_ENTITY:
                Gui_OutTextXY(30.0, 60.0, "cont_entity: id = %d, model = %d", static_cast<Entity*>(last_cont->object)->id(), static_cast<Entity*>(last_cont->object)->m_bf.animations.model->id);
                break;

            case OBJECT_STATIC_MESH:
                Gui_OutTextXY(30.0, 60.0, "cont_static: id = %d", static_cast<StaticMesh*>(last_cont->object)->object_id);
                break;

            case OBJECT_ROOM_BASE:
                Gui_OutTextXY(30.0, 60.0, "cont_room: id = %d", static_cast<Room*>(last_cont->object)->id);
                break;
        }

    }

    if(engine_camera.m_currentRoom != NULL)
    {
        RoomSector* rs = engine_camera.m_currentRoom->getSectorRaw(engine_camera.m_pos);
        if(rs != NULL)
        {
            Gui_OutTextXY(30.0, 90.0, "room = (id = %d, sx = %d, sy = %d)", engine_camera.m_currentRoom->id, rs->index_x, rs->index_y);
            Gui_OutTextXY(30.0, 120.0, "room_below = %d, room_above = %d", (rs->sector_below != NULL)?(rs->sector_below->owner_room->id):(-1), (rs->sector_above != NULL)?(rs->sector_above->owner_room->id):(-1));
        }
    }
    Gui_OutTextXY(30.0, 150.0, "cam_pos = (%.1f, %.1f, %.1f)", engine_camera.m_pos[0], engine_camera.m_pos[1], engine_camera.m_pos[2]);
}
