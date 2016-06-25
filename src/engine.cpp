
#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <AL/al.h>
#include <AL/alc.h>
}

#include "core/system.h"
#include "core/gl_util.h"
#include "core/gl_font.h"
#include "core/console.h"
#include "core/redblack.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/gl_text.h"
#include "render/camera.h"
#include "render/render.h"
#include "vt/vt_level.h"
#include "game.h"
#include "audio.h"
#include "mesh.h"
#include "skeletal_model.h"
#include "gui.h"
#include "entity.h"
#include "gameflow.h"
#include "room.h"
#include "world.h"
#include "resource.h"
#include "script.h"
#include "engine.h"
#include "physics.h"
#include "controls.h"
#include "trigger.h"
#include "character_controller.h"
#include "render/bsp_tree.h"


static SDL_Window             *sdl_window     = NULL;
static SDL_Joystick           *sdl_joystick   = NULL;
static SDL_GameController     *sdl_controller = NULL;
static SDL_Haptic             *sdl_haptic     = NULL;
static SDL_GLContext           sdl_gl_context = 0;
static ALCdevice              *al_device      = NULL;
static ALCcontext             *al_context     = NULL;

static volatile int             engine_done   = 0;
static int                      engine_set_sero_time = 0;
float time_scale = 1.0f;

engine_container_p      last_cont = NULL;
static float            ray_test_point[3] = {0.0f, 0.0f, 0.0f};

struct engine_control_state_s           control_states = {0};
struct control_settings_s               control_mapper = {0};
float                                   engine_frame_time = 0.0;

lua_State                              *engine_lua = NULL;
struct camera_s                         engine_camera;
struct camera_state_s                   engine_camera_state;


engine_container_p Container_Create()
{
    engine_container_p ret;

    ret = (engine_container_p)malloc(sizeof(engine_container_t));
    ret->next = NULL;
    ret->object = NULL;
    ret->object_type = 0;
    return ret;
}


void Engine_Init_Pre();
void Engine_Init_Post();
void Engine_InitGL();
void Engine_InitAL();
void Engine_InitSDLImage();
void Engine_InitSDLVideo();
void Engine_InitSDLControls();
void Engine_InitDefaultGlobals();

void Engine_Display();
void Engine_PollSDLEvents();
void Engine_Resize(int nominalW, int nominalH, int pixelsW, int pixelsH);

void ShowDebugInfo();

void Engine_Start(const char *config_name)
{
    Engine_InitDefaultGlobals();
    Engine_LoadConfig(config_name);

    // Primary initialization.
    Engine_Init_Pre();

    // Init generic SDL interfaces.
    Engine_InitSDLControls();
    Engine_InitSDLVideo();
    Engine_InitAL();

    Engine_InitSDLImage();

    // Additional OpenGL initialization.
    Engine_InitGL();
    renderer.DoShaders();

    // Secondary (deferred) initialization.
    Engine_Init_Post();

    // Make splash screen.
    Gui_LoadScreenAssignPic("resource/graphics/legal.png");

    // Initial window resize.
    Engine_Resize(screen_info.w, screen_info.h, screen_info.w, screen_info.h);

    // Clearing up memory for initial level loading.
    World_Prepare();

    // Setting up mouse.
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_WarpMouseInWindow(sdl_window, screen_info.w/2, screen_info.h/2);
    SDL_ShowCursor(0);

    luaL_dofile(engine_lua, "autoexec.lua");
}


void Engine_Shutdown(int val)
{
    renderer.ResetWorld(NULL, 0, NULL, 0);
    World_Clear();

    if(engine_lua)
    {
        lua_close(engine_lua);
        engine_lua = NULL;
    }

    Physics_Destroy();
    Gui_Destroy();
    Con_Destroy();
    GLText_Destroy();
    Sys_Destroy();

    /* no more renderings */
    SDL_GL_DeleteContext(sdl_gl_context);
    sdl_gl_context = 0;
    SDL_DestroyWindow(sdl_window);
    sdl_window = NULL;

    if(sdl_joystick)
    {
        SDL_JoystickClose(sdl_joystick);
        sdl_joystick = NULL;
    }

    if(sdl_controller)
    {
        SDL_GameControllerClose(sdl_controller);
        sdl_controller = NULL;
    }

    if(sdl_haptic)
    {
        SDL_HapticClose(sdl_haptic);
        sdl_haptic = NULL;
    }

    if(al_context)  // T4Larson <t4larson@gmail.com>: fixed
    {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(al_context);
        al_context = NULL;
    }

    if(al_device)
    {
        alcCloseDevice(al_device);
        al_device = NULL;
    }

    Sys_Destroy();
    IMG_Quit();
    SDL_Quit();

    exit(val);
}


void Engine_SetDone()
{
    engine_done = 1;
}


void Engine_InitDefaultGlobals()
{
    Sys_InitGlobals();
    Con_InitGlobals();
    Controls_InitGlobals();
    Game_InitGlobals();
    Audio_InitGlobals();
}

// First stage of initialization.
void Engine_Init_Pre()
{
    /* Console must be initialized previously! some functions uses CON_AddLine before GL initialization!
     * Rendering activation may be done later. */

    Sys_Init();
    GLText_Init();
    Con_Init();
    Con_SetExecFunction(Engine_ExecCmd);
    Script_LuaInit();

    Script_CallVoidFunc(engine_lua, "loadscript_pre", true);

    Gameflow_Init();
    Cam_Init(&engine_camera);
    engine_camera_state.state = CAMERA_STATE_NORMAL;
    engine_camera_state.flyby = NULL;
    engine_camera_state.sink = NULL;
    engine_camera_state.shake_value = 0.0f;
    engine_camera_state.time = 0.0f;

    Physics_Init();
}

// Second stage of initialization.
void Engine_Init_Post()
{
    Script_CallVoidFunc(engine_lua, "loadscript_post", true);

    Con_InitFont();
    Gui_Init();

    Con_AddLine("Engine inited!", FONTSTYLE_CONSOLE_EVENT);
}


void Engine_InitGL()
{
    InitGLExtFuncs();
    qglClearColor(0.0, 0.0, 0.0, 1.0);

    qglEnable(GL_DEPTH_TEST);
    qglDepthFunc(GL_LEQUAL);

    if(renderer.settings.antialias)
    {
        qglEnable(GL_MULTISAMPLE);
    }
    else
    {
       qglDisable(GL_MULTISAMPLE);
    }

    // Default state: Vertex array and color array are enabled, all others disabled.. Drawable
    // items can rely on Vertex array to be enabled (but pointer can be
    // anything). They have to enable other arrays based on their need and then
    // return to default state
    qglEnableClientState(GL_VERTEX_ARRAY);
    qglEnableClientState(GL_COLOR_ARRAY);

    // function use anyway.
    qglAlphaFunc(GL_GEQUAL, 0.5);
}


void Engine_InitAL()
{
    ALCint paramList[] = {
        ALC_STEREO_SOURCES,  TR_AUDIO_STREAM_NUMSOURCES,
        ALC_MONO_SOURCES,   (TR_AUDIO_MAX_CHANNELS - TR_AUDIO_STREAM_NUMSOURCES),
        ALC_FREQUENCY,       44100, 0};

    Con_Printf("Audio driver: %s", SDL_GetCurrentAudioDriver());

    al_device = alcOpenDevice(NULL);
    if (!al_device)
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "InitAL: No AL audio devices!");
        return;
    }

    al_context = alcCreateContext(al_device, paramList);
    if(!alcMakeContextCurrent(al_context))
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "InitAL: AL context is not current!");
        return;
    }

    alSpeedOfSound(330.0 * 512.0);
    alDopplerVelocity(330.0 * 510.0);
    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
}


void Engine_InitSDLImage()
{
    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int init  = IMG_Init(flags);

    if((init & flags) != flags)
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "SDL_Image error: failed to initialize JPG and/or PNG support.");
    }
}


void Engine_InitSDLVideo()
{
    Uint32 video_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;
    PFNGLGETSTRINGPROC lglGetString = NULL;

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
    if(renderer.settings.antialias)
    {
        GLint maxSamples = 0;
        PFNGLGETIINTEGERVPROC lglGetIntegerv = NULL;
        /* I do not know why, but settings of this temporary window (zero position / size) are applied to the main window, ignoring screen settings */
        sdl_window     = SDL_CreateWindow(NULL, screen_info.x, screen_info.y, screen_info.w, screen_info.h, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        sdl_gl_context = SDL_GL_CreateContext(sdl_window);
        SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);

        lglGetIntegerv = (PFNGLGETIINTEGERVPROC)SDL_GL_GetProcAddress("glGetIntegerv");
        lglGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        maxSamples = (maxSamples > 16)?(16):(maxSamples);                       // Fix for faulty GL max. sample number.

        if(renderer.settings.antialias_samples > maxSamples)
        {
            renderer.settings.antialias_samples = maxSamples;                   // Limit to max.
            if(maxSamples == 0)
            {
                renderer.settings.antialias = 0;
                Sys_DebugLog(SYS_LOG_FILENAME, "InitSDLVideo: can't use antialiasing");
            }
            else
            {
                Sys_DebugLog(SYS_LOG_FILENAME, "InitSDLVideo: wrong AA sample number, using %d", maxSamples);
            }
        }

        SDL_GL_DeleteContext(sdl_gl_context);
        SDL_DestroyWindow(sdl_window);

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, renderer.settings.antialias);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, renderer.settings.antialias_samples);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, renderer.settings.z_depth);
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

    lglGetString = (PFNGLGETSTRINGPROC)SDL_GL_GetProcAddress("glGetString");
    Con_AddLine((const char*)lglGetString(GL_VENDOR), FONTSTYLE_CONSOLE_INFO);
    Con_AddLine((const char*)lglGetString(GL_RENDERER), FONTSTYLE_CONSOLE_INFO);
    Con_Printf("OpenGL version %s", lglGetString(GL_VERSION));
    Con_AddLine((const char*)lglGetString(GL_SHADING_LANGUAGE_VERSION), FONTSTYLE_CONSOLE_INFO);
}


void Engine_InitSDLControls()
{
    int    NumJoysticks;
    Uint32 init_flags    = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS;   // These flags are used in any case.

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
            Sys_DebugLog(SYS_LOG_FILENAME, "Error: there is no joystick #%d present.", control_mapper.joy_number);
            return;
        }

        if(SDL_IsGameController(control_mapper.joy_number))                     // If joystick has mapping (e.g. X360 controller)
        {
            SDL_GameControllerEventState(SDL_ENABLE);                           // Use GameController API
            sdl_controller = SDL_GameControllerOpen(control_mapper.joy_number);

            if(!sdl_controller)
            {
                Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't open game controller #%d.", control_mapper.joy_number);
                SDL_GameControllerEventState(SDL_DISABLE);                      // If controller init failed, close state.
                control_mapper.use_joy = 0;
            }
            else if(control_mapper.joy_rumble)                                  // Create force feedback interface.
            {
                sdl_haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(sdl_controller));
                if(!sdl_haptic)
                {
                    Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't initialize haptic from game controller #%d.", control_mapper.joy_number);
                }
            }
        }
        else
        {
            SDL_JoystickEventState(SDL_ENABLE);                                 // If joystick isn't mapped, use generic API.
            sdl_joystick = SDL_JoystickOpen(control_mapper.joy_number);

            if(!sdl_joystick)
            {
                Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't open joystick #%d.", control_mapper.joy_number);
                SDL_JoystickEventState(SDL_DISABLE);                            // If joystick init failed, close state.
                control_mapper.use_joy = 0;
            }
            else if(control_mapper.joy_rumble)                                  // Create force feedback interface.
            {
                sdl_haptic = SDL_HapticOpenFromJoystick(sdl_joystick);
                if(!sdl_haptic)
                {
                    Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't initialize haptic from joystick #%d.", control_mapper.joy_number);
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


void Engine_LoadConfig(const char *filename)
{
    if((filename != NULL) && Sys_FileFound(filename, 0))
    {
        lua_State *lua = luaL_newstate();
        if(lua != NULL)
        {
            luaL_openlibs(lua);
            lua_register(lua, "bind", lua_BindKey);                             // get and set key bindings
            luaL_dofile(lua, filename);

            Script_ParseScreen(lua, &screen_info);
            Script_ParseRender(lua, &renderer.settings);
            Script_ParseAudio(lua, &audio_settings);
            Script_ParseConsole(lua);
            Script_ParseControls(lua, &control_mapper);
            lua_close(lua);
        }
    }
    else
    {
        Sys_Warn("Could not find \"%s\"", filename);
    }
}


void Engine_SaveConfig(const char *filename)
{

}


void Engine_Display()
{
    if(!engine_done)
    {
        qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//| GL_ACCUM_BUFFER_BIT);

        Cam_Apply(&engine_camera);
        Cam_RecalcClipPlanes(&engine_camera);
        // GL_VERTEX_ARRAY | GL_COLOR_ARRAY

        screen_info.show_debuginfo %= 4;
        if(screen_info.show_debuginfo)
        {
            ShowDebugInfo();
        }

        qglPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT); ///@PUSH <- GL_VERTEX_ARRAY | GL_COLOR_ARRAY
        qglEnableClientState(GL_NORMAL_ARRAY);
        qglEnableClientState(GL_TEXTURE_COORD_ARRAY);

        qglFrontFace(GL_CW);

        renderer.GenWorldList(&engine_camera);
        renderer.DrawList();

        Gui_SwitchGLMode(1);
        qglEnable(GL_ALPHA_TEST);

        Gui_DrawNotifier();
        qglPopClientAttrib();        ///@POP -> GL_VERTEX_ARRAY | GL_COLOR_ARRAY
        Gui_Render();
        Gui_SwitchGLMode(0);

        renderer.DrawListDebugLines();

        SDL_GL_SwapWindow(sdl_window);
    }
}


void Engine_GLSwapWindow()
{
    SDL_GL_SwapWindow(sdl_window);
}


void Engine_Resize(int nominalW, int nominalH, int pixelsW, int pixelsH)
{
    screen_info.w = nominalW;
    screen_info.h = nominalH;

    screen_info.w_unit = (float)nominalW / SYS_SCREEN_METERING_RESOLUTION;
    screen_info.h_unit = (float)nominalH / SYS_SCREEN_METERING_RESOLUTION;
    screen_info.scale_factor = (screen_info.w < screen_info.h) ? (screen_info.h_unit) : (screen_info.w_unit);

    GLText_UpdateResize(screen_info.scale_factor);
    Con_UpdateResize();
    Gui_UpdateResize();

    Cam_SetFovAspect(&engine_camera, screen_info.fov, (float)nominalW/(float)nominalH);
    Cam_RecalcClipPlanes(&engine_camera);

    qglViewport(0, 0, pixelsW, pixelsH);
}


void Engine_PollSDLEvents()
{
    SDL_Event event;
    static int mouse_setup = 0;
    const float color[3] = {1.0f, 0.0f, 0.0f};
    static float from[3], to[3];

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
                if(!Con_IsShown() && control_states.mouse_look != 0 &&
                    ((event.motion.x != (screen_info.w / 2)) ||
                     (event.motion.y != (screen_info.h / 2))))
                {
                    if(mouse_setup)                                             // it is not perfect way, but cursor
                    {                                                           // every engine start is in one place
                        control_states.look_axis_x = event.motion.xrel * control_mapper.mouse_sensitivity * 0.01;
                        control_states.look_axis_y = event.motion.yrel * control_mapper.mouse_sensitivity * 0.01;
                    }

                    if((event.motion.x < ((screen_info.w / 2) - (screen_info.w / 4))) ||
                       (event.motion.x > ((screen_info.w / 2) + (screen_info.w / 4))) ||
                       (event.motion.y < ((screen_info.h / 2) - (screen_info.h / 4))) ||
                       (event.motion.y > ((screen_info.h / 2) + (screen_info.h / 4))))
                    {
                        SDL_WarpMouseInWindow(sdl_window, screen_info.w/2, screen_info.h/2);
                    }
                }
                mouse_setup = 1;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == 1) //LM = 1, MM = 2, RM = 3
                {
                    Controls_PrimaryMouseDown(from, to);
                }
                else if(event.button.button == 3)
                {
                    Controls_SecondaryMouseDown(&last_cont, ray_test_point);
                    if(last_cont && last_cont->object_type == OBJECT_ENTITY)
                    {
                        entity_p player = World_GetPlayer();
                        if(player && player->character)
                        {
                            Character_SetTarget(player, ((entity_p)last_cont->object)->id);
                        }
                    }
                    else
                    {
                        entity_p player = World_GetPlayer();
                        if(player && player->character)
                        {
                            Character_SetTarget(player, ENTITY_ID_NONE);
                        }
                    }
                }
                break;

            // Controller events are only invoked when joystick is initialized as
            // game controller, otherwise, generic joystick event will be used.
            case SDL_CONTROLLERAXISMOTION:
                Controls_WrapGameControllerAxis(event.caxis.axis, event.caxis.value);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                Controls_WrapGameControllerKey(event.cbutton.button, event.cbutton.state);
                break;

            // Joystick events are still invoked, even if joystick is initialized as game
            // controller - that's why we need sdl_joystick checking - to filter out
            // duplicate event calls.

            case SDL_JOYAXISMOTION:
                if(sdl_joystick)
                {
                    Controls_JoyAxis(event.jaxis.axis, event.jaxis.value);
                }
                break;

            case SDL_JOYHATMOTION:
                if(sdl_joystick)
                {
                    Controls_JoyHat(event.jhat.value);
                }
                break;

            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                // NOTE: Joystick button numbers are passed with added JOY_BUTTON_MASK (1000).
                if(sdl_joystick)
                {
                    Controls_Key((event.jbutton.button + JOY_BUTTON_MASK), event.jbutton.state);
                }
                break;

            case SDL_TEXTINPUT:
            case SDL_TEXTEDITING:
                if(Con_IsShown() && event.key.state)
                {
                    Con_Filter(event.text.text);
                    return;
                }
                break;

            case SDL_KEYUP:
            case SDL_KEYDOWN:
                if( (event.key.keysym.sym == SDLK_F4) &&
                    (event.key.state == SDL_PRESSED)  &&
                    (event.key.keysym.mod & KMOD_ALT) )
                {
                    Engine_SetDone();
                    break;
                }

                if(Con_IsShown() && event.key.state)
                {
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_RETURN:
                        case SDLK_UP:
                        case SDLK_DOWN:
                        case SDLK_LEFT:
                        case SDLK_RIGHT:
                        case SDLK_HOME:
                        case SDLK_END:
                        case SDLK_BACKSPACE:
                        case SDLK_DELETE:
                            Con_Edit(event.key.keysym.sym);
                            break;

                        default:
                            break;
                    }
                    return;
                }
                else
                {
                    Controls_Key(event.key.keysym.sym, event.key.state);
                    // DEBUG KEYBOARD COMMANDS
                    Controls_DebugKeys(event.key.keysym.sym, event.key.state);
                }
                break;

            case SDL_QUIT:
                Engine_SetDone();
                break;

            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    Engine_Resize(event.window.data1, event.window.data2, event.window.data1, event.window.data2);
                }
                break;

            default:
                break;
        }
    }
    renderer.debugDrawer->DrawLine(from, to, color, color);
}


void Engine_JoyRumble(float power, int time)
{
    // JoyRumble is a simple wrapper for SDL's haptic rumble play.
    if(sdl_haptic)
    {
        SDL_HapticRumblePlay(sdl_haptic, power, time);
    }
}


void Engine_MainLoop()
{
    float time = 0.0f;
    float newtime = 0.0f;
    float oldtime = Sys_FloatTime();
    float time_cycl = 0.0f;

    const int max_cycles = 64;
    int cycles = 0;
    char fps_str[32] = "0.0";

    while(!engine_done)
    {
        newtime = Sys_FloatTime();
        time = newtime - oldtime;
        oldtime = newtime;
        time *= time_scale;

        if(engine_set_sero_time)
        {
            engine_set_sero_time = 0;
            time = 0.0f;
        }

        engine_frame_time = time;

        if(cycles < max_cycles)
        {
            cycles++;
            time_cycl += time;
        }
        else
        {
            screen_info.fps = ((float)max_cycles / time_cycl);
            snprintf(fps_str, 32, "%.1f", screen_info.fps);
            cycles = 0;
            time_cycl = 0.0f;
        }

        gl_text_line_p fps = GLText_OutTextXY(10.0f, 10.0f, fps_str);
        fps->x_align    = GLTEXT_ALIGN_RIGHT;
        fps->y_align    = GLTEXT_ALIGN_BOTTOM;
        fps->font_id    = FONT_PRIMARY;
        fps->style_id   = FONTSTYLE_MENU_TITLE;

        Sys_ResetTempMem();
        Engine_PollSDLEvents();
        Game_Frame(time);
        Gameflow_Do();

        Audio_Update(time);
        Engine_Display();
    }
}


void ShowDebugInfo()
{
    float y = (float)screen_info.h;
    const float dy = -18.0f * screen_info.scale_factor;

    if(last_cont)
    {
        switch(last_cont->object_type)
        {
            case OBJECT_ENTITY:
                GLText_OutTextXY(30.0f, y += dy, "cont_entity: id = %d, model = %d, room = %d", ((entity_p)last_cont->object)->id, ((entity_p)last_cont->object)->bf->animations.model->id, (last_cont->room) ? (last_cont->room->id) : (-1));
                break;

            case OBJECT_STATIC_MESH:
                GLText_OutTextXY(30.0f, y += dy, "cont_static: id = %d, room = %d", ((static_mesh_p)last_cont->object)->object_id, (last_cont->room) ? (last_cont->room->id) : (-1));
                break;

            case OBJECT_ROOM_BASE:
                {
                    room_sector_p rs = Room_GetSectorRaw((room_p)last_cont->object, ray_test_point);
                    if(rs != NULL)
                    {
                        renderer.debugDrawer->SetColor(0.0, 1.0, 0.0);
                        renderer.debugDrawer->DrawSectorDebugLines(rs);
                        GLText_OutTextXY(30.0f, y += dy, "cont_room: (id = %d, sx = %d, sy = %d)", rs->owner_room->id, rs->index_x, rs->index_y);
                        GLText_OutTextXY(30.0f, y += dy, "room_below = %d, room_above = %d", (rs->sector_below != NULL) ? (rs->sector_below->owner_room->id) : (-1), (rs->sector_above != NULL) ? (rs->sector_above->owner_room->id) : (-1));
                        if(rs->trigger)
                        {
                            char trig_type[64];
                            char trig_func[64];
                            Trigger_TrigTypeToStr(trig_type, 64, rs->trigger->sub_function);
                            GLText_OutTextXY(30.0f, y += dy, "trig(sub = %s, val = 0x%X, mask = 0x%X)", trig_type, rs->trigger->function_value, rs->trigger->mask);
                            for(trigger_command_p cmd = rs->trigger->commands; cmd; cmd = cmd->next)
                            {
                                entity_p trig_obj = World_GetEntityByID(cmd->operands);
                                if(trig_obj)
                                {
                                    renderer.debugDrawer->SetColor(0.0, 0.0, 1.0);
                                    renderer.debugDrawer->DrawBBox(trig_obj->bf->bb_min, trig_obj->bf->bb_max, trig_obj->transform);
                                    renderer.OutTextXYZ(trig_obj->transform[12 + 0], trig_obj->transform[12 + 1], trig_obj->transform[12 + 2], "(id = 0x%X)", trig_obj->id);
                                }
                                Trigger_TrigCmdToStr(trig_func, 64, cmd->function);
                                GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X)", trig_func, cmd->operands);
                            }
                        }
                    }
                }
                break;
        }
    }

    switch(screen_info.show_debuginfo)
    {
        case 1:
            {
                entity_p ent = World_GetPlayer();
                if(ent && ent->character)
                {
                    GLText_OutTextXY(30.0f, y += dy, "last_anim = %03d, curr_anim = %03d, next_anim = %03d, last_st = %03d, next_st = %03d", ent->bf->animations.last_animation, ent->bf->animations.current_animation, ent->bf->animations.next_animation, ent->bf->animations.last_state, ent->bf->animations.next_state);
                    GLText_OutTextXY(30.0f, y += dy, "curr_anim = %03d, next_anim = %03d, curr_frame = %03d, next_frame = %03d", ent->bf->animations.current_animation, ent->bf->animations.next_animation, ent->bf->animations.current_frame, ent->bf->animations.next_frame);
                    GLText_OutTextXY(30.0f, y += dy, "posX = %f, posY = %f, posZ = %f", ent->transform[12], ent->transform[13], ent->transform[14]);
                }
            }
            break;

        case 2:
            {
                entity_p ent = World_GetPlayer();
                if(ent && ent->self->room)
                {
                    GLText_OutTextXY(30.0f, y += dy, "char_pos = (%.1f, %.1f, %.1f)", ent->transform[12 + 0], ent->transform[12 + 1], ent->transform[12 + 2]);
                    room_sector_p rs = Room_GetSectorRaw(ent->self->room, ent->transform + 12);
                    if(rs != NULL)
                    {
                        renderer.debugDrawer->SetColor(0.0, 1.0, 0.0);
                        renderer.debugDrawer->DrawSectorDebugLines(rs);
                        GLText_OutTextXY(30.0f, y += dy, "room = (id = %d, sx = %d, sy = %d)", rs->owner_room->id, rs->index_x, rs->index_y);
                        GLText_OutTextXY(30.0f, y += dy, "room_below = %d, room_above = %d", (rs->sector_below != NULL) ? (rs->sector_below->owner_room->id) : (-1), (rs->sector_above != NULL) ? (rs->sector_above->owner_room->id) : (-1));
                        if(rs->trigger)
                        {
                            char trig_type[64];
                            char trig_func[64];
                            Trigger_TrigTypeToStr(trig_type, 64, rs->trigger->sub_function);
                            GLText_OutTextXY(30.0f, y += dy, "trig(sub = %s, val = 0x%X, mask = 0x%X)", trig_type, rs->trigger->function_value, rs->trigger->mask);
                            for(trigger_command_p cmd = rs->trigger->commands; cmd; cmd = cmd->next)
                            {
                                entity_p trig_obj = World_GetEntityByID(cmd->operands);
                                if(trig_obj)
                                {
                                    renderer.debugDrawer->SetColor(0.0, 0.0, 1.0);
                                    renderer.debugDrawer->DrawBBox(trig_obj->bf->bb_min, trig_obj->bf->bb_max, trig_obj->transform);
                                    renderer.OutTextXYZ(trig_obj->transform[12 + 0], trig_obj->transform[12 + 1], trig_obj->transform[12 + 2], "(id = 0x%X)", trig_obj->id);
                                }
                                Trigger_TrigCmdToStr(trig_func, 64, cmd->function);
                                GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X)", trig_func, cmd->operands);
                            }
                        }
                    }
                }
            }
            break;

        case 3:
            if(renderer.dynamicBSP)
            {
                GLText_OutTextXY(30.0f, y += dy, "input polygons = %07d", renderer.dynamicBSP->GetInputPolygonsCount());
                GLText_OutTextXY(30.0f, y += dy, "added polygons = %07d", renderer.dynamicBSP->GetAddedPolygonsCount());
            }
            break;
    };
}


/*
 * MISC ENGINE FUNCTIONALITY
 */

void Engine_GetLevelName(char *name, const char *path)
{
    int i, len, start, ext;

    if(!path || (path[0] == 0x00))
    {
        name[0] = 0x00;
        return;
    }

    ext = len = strlen(path);
    start = 0;

    for(i = len; i >= 0; i--)
    {
        if(path[i] == '.')
        {
            ext = i;
        }
        if(path[i] == '\\' || path[i] == '/')
        {
            start = i + 1;
            break;
        }
    }

    for(i = start; (i < ext) && (i + 1 < LEVEL_NAME_MAX_LEN + start); i++)
    {
        name[i-start] = path[i];
    }
    name[i-start] = 0;
}


void Engine_GetLevelScriptName(int game_version, char *name, const char *postfix, uint32_t buf_size)
{
    char level_name[LEVEL_NAME_MAX_LEN];
    Engine_GetLevelName(level_name, gameflow_manager.CurrentLevelPath);

    name[0] = 0;

    strncat(name, "scripts/level/", buf_size);

    if(game_version < TR_II)
    {
        strncat(name, "tr1/", buf_size);
    }
    else if(game_version < TR_III)
    {
        strncat(name, "tr2/", buf_size);
    }
    else if(game_version < TR_IV)
    {
        strncat(name, "tr3/", buf_size);
    }
    else if(game_version < TR_V)
    {
        strncat(name, "tr4/", buf_size);
    }
    else
    {
        strncat(name, "tr5/", buf_size);
    }

    for(char *ch = level_name; *ch; ch++)
    {
        *ch = toupper(*ch);
    }

    strncat(name, level_name, buf_size);
    if(postfix)
    {
        strncat(name, postfix, buf_size);
    }
    strncat(name, ".lua", buf_size);
}


bool Engine_LoadPCLevel(const char *name)
{
    int trv = VT_Level::get_PC_level_version(name);
    if(trv != TR_UNKNOWN)
    {
        VT_Level *tr_level = new VT_Level();
        tr_level->read_level(name, trv);
        tr_level->prepare_level();
        //tr_level->dump_textures();

        World_Open(tr_level);

        char buf[LEVEL_NAME_MAX_LEN] = {0x00};
        Engine_GetLevelName(buf, name);

        Con_Notify("loaded PC level");
        Con_Notify("version = %d, map = \"%s\"", trv, buf);
        Con_Notify("rooms count = %d", tr_level->rooms_count);

        delete tr_level;
        return true;
    }
    return false;
}


int Engine_LoadMap(const char *name)
{
    if(!Sys_FileFound(name, 0))
    {
        Con_Warning("file not found: \"%s\"", name);
        return 0;
    }

    Game_StopFlyBy();
    engine_camera.current_room = NULL;
    renderer.ResetWorld(NULL, 0, NULL, 0);
    Gui_DrawLoadScreen(0);

    // it is needed for "not in the game" levels or correct saves loading.
    strncpy(gameflow_manager.CurrentLevelPath, name, MAX_ENGINE_PATH);

    Gui_DrawLoadScreen(100);


    // Here we can place different platform-specific level loading routines.
    switch(VT_Level::get_level_format(name))
    {
        case LEVEL_FORMAT_PC:
            if(!Engine_LoadPCLevel(name))
            {
                return 0;
            }
            break;

        /*case LEVEL_FORMAT_PSX:
            return 0;
            break;

        case LEVEL_FORMAT_DC:
            return 0;
            break;

        case LEVEL_FORMAT_OPENTOMB:
            return 0;
            break;*/

        default:
            return 0;
    }

    Audio_Init();
    Game_Prepare();

    room_p rooms;
    uint32_t rooms_count;
    anim_seq_p seq;
    uint32_t seq_count;

    World_GetRoomInfo(&rooms, &rooms_count);
    World_GetAnimSeqInfo(&seq, &seq_count);
    renderer.ResetWorld(rooms, rooms_count, seq, seq_count);

    Gui_DrawLoadScreen(1000);
    Gui_NotifierStop();
    engine_set_sero_time = 1;

    return 1;
}


int Engine_ExecCmd(char *ch)
{
    char token[1024];

    while(ch != NULL)
    {
        char *pch = ch;
        ch = SC_ParseToken(ch, token);
        if(!strcmp(token, "help"))
        {
            Con_AddLine("Available commands:\0", FONTSTYLE_CONSOLE_WARNING);
            Con_AddLine("help - show help info\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("loadMap(\"file_name\") - load level \"file_name\"\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("save, load - save and load game state in \"file_name\"\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("exit - close program\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cls - clean console\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("show_fps - switch show fps flag\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("spacing - read and write spacing\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("showing_lines - read and write number of showing lines\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cvars - lua's table of cvar's, to see them type: show_table(cvars)\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("free_look - switch camera mode\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cam_distance - camera distance to actor\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("r_wireframe, r_portals, r_frustums, r_room_boxes, r_boxes, r_normals, r_skip_room, r_flyby, r_triggers - render modes\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("playsound(id) - play specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("stopsound(id) - stop specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("Watch out for case sensitive commands!\0", FONTSTYLE_CONSOLE_WARNING);
        }
        else if(!strcmp(token, "goto"))
        {
            control_states.free_look = 1;
            engine_camera.pos[0] = SC_ParseFloat(&ch);
            engine_camera.pos[1] = SC_ParseFloat(&ch);
            engine_camera.pos[2] = SC_ParseFloat(&ch);
            return 1;
        }
        else if(!strcmp(token, "save"))
        {
            ch = SC_ParseToken(ch, token);
            if(NULL != ch)
            {
                Game_Save(token);
            }
            return 1;
        }
        else if(!strcmp(token, "load"))
        {
            ch = SC_ParseToken(ch, token);
            if(NULL != ch)
            {
                Game_Load(token);
            }
            return 1;
        }
        else if(!strcmp(token, "exit"))
        {
            Engine_Shutdown(0);
            return 1;
        }
        else if(!strcmp(token, "cls"))
        {
            Con_Clean();
            return 1;
        }
        else if(!strcmp(token, "spacing"))
        {
            ch = SC_ParseToken(ch, token);
            if(NULL == ch)
            {
                Con_Notify("spacing = %d", Con_GetLineInterval());
                return 1;
            }
            Con_SetLineInterval(atof(token));
            return 1;
        }
        else if(!strcmp(token, "showing_lines"))
        {
            ch = SC_ParseToken(ch, token);
            if(NULL == ch)
            {
                Con_Notify("showing lines = %d", Con_GetShowingLines());
                return 1;
            }
            else
            {
                Con_SetShowingLines(atoi(token));
            }
            return 1;
        }
        else if(!strcmp(token, "r_wireframe"))
        {
            renderer.r_flags ^= R_DRAW_WIRE;
            return 1;
        }
        else if(!strcmp(token, "r_points"))
        {
            renderer.r_flags ^= R_DRAW_POINTS;
            return 1;
        }
        else if(!strcmp(token, "r_coll"))
        {
            renderer.r_flags ^= R_DRAW_COLL;
            return 1;
        }
        else if(!strcmp(token, "r_normals"))
        {
            renderer.r_flags ^= R_DRAW_NORMALS;
            return 1;
        }
        else if(!strcmp(token, "r_portals"))
        {
            renderer.r_flags ^= R_DRAW_PORTALS;
            return 1;
        }
        else if(!strcmp(token, "r_frustums"))
        {
            renderer.r_flags ^= R_DRAW_FRUSTUMS;
            return 1;
        }
        else if(!strcmp(token, "r_room_boxes"))
        {
            renderer.r_flags ^= R_DRAW_ROOMBOXES;
            return 1;
        }
        else if(!strcmp(token, "r_boxes"))
        {
            renderer.r_flags ^= R_DRAW_BOXES;
            return 1;
        }
        else if(!strcmp(token, "r_axis"))
        {
            renderer.r_flags ^= R_DRAW_AXIS;
            return 1;
        }
        else if(!strcmp(token, "r_nullmeshes"))
        {
            renderer.r_flags ^= R_DRAW_NULLMESHES;
            return 1;
        }
        else if(!strcmp(token, "r_dummy_statics"))
        {
            renderer.r_flags ^= R_DRAW_DUMMY_STATICS;
            return 1;
        }
        else if(!strcmp(token, "r_skip_room"))
        {
            renderer.r_flags ^= R_SKIP_ROOM;
            return 1;
        }
        else if(!strcmp(token, "r_flyby"))
        {
            renderer.r_flags ^= R_DRAW_FLYBY;
            return 1;
        }
        else if(!strcmp(token, "r_triggers"))
        {
            renderer.r_flags ^= R_DRAW_TRIGGERS;
            return 1;
        }
        else if(!strcmp(token, "room_info"))
        {
            room_p r = engine_camera.current_room;
            if(r)
            {
                room_sector_p sect = Room_GetSectorXYZ(r, engine_camera.pos);
                Con_Printf("ID = %d, x_sect = %d, y_sect = %d", r->id, r->sectors_x, r->sectors_y);
                if(sect)
                {
                    Con_Printf("sect(%d, %d), inpenitrable = %d, r_up = %d, r_down = %d", sect->index_x, sect->index_y,
                               (int)(sect->ceiling == TR_METERING_WALLHEIGHT || sect->floor == TR_METERING_WALLHEIGHT), (int)(sect->sector_above != NULL), (int)(sect->sector_below != NULL));
                    for(uint32_t i = 0; i < sect->owner_room->content->static_mesh_count; i++)
                    {
                        Con_Printf("static[%d].object_id = %d", i, sect->owner_room->content->static_mesh[i].object_id);
                    }
                    for(engine_container_p cont = sect->owner_room->content->containers; cont; cont=cont->next)
                    {
                        if(cont->object_type == OBJECT_ENTITY)
                        {
                            entity_p e = (entity_p)cont->object;
                            Con_Printf("cont[entity](%d, %d, %d).object_id = %d", (int)e->transform[12+0], (int)e->transform[12+1], (int)e->transform[12+2], e->id);
                        }
                    }
                }
            }
            return 1;
        }
        else if(!strcmp(token, "xxx"))
        {
            SDL_RWops *f = SDL_RWFromFile("ascII.txt", "r");
            if(f)
            {
                long int size;
                char *buf;
                SDL_RWseek(f, 0, RW_SEEK_END);
                size= SDL_RWtell(f);
                buf = (char*) malloc((size+1)*sizeof(char));

                SDL_RWseek(f, 0, RW_SEEK_SET);
                SDL_RWread(f, buf, sizeof(char), size);
                buf[size] = 0;
                SDL_RWclose(f);
                Con_Clean();
                Con_AddText(buf, FONTSTYLE_CONSOLE_INFO);
                free(buf);
            }
            else
            {
                Con_AddText("Not avaliable =(", FONTSTYLE_CONSOLE_WARNING);
            }
            return 1;
        }
        else if(token[0])
        {
            if(engine_lua)
            {
                Con_AddLine(pch, FONTSTYLE_GENERIC);
                if (luaL_dostring(engine_lua, pch) != LUA_OK)
                {
                    Con_AddLine(lua_tostring(engine_lua, -1), FONTSTYLE_CONSOLE_WARNING);
                    lua_pop(engine_lua, 1);
                }
            }
            else
            {
                char buf[1024];
                snprintf(buf, 1024, "Command \"%s\" not found", token);
                Con_AddLine(buf, FONTSTYLE_CONSOLE_WARNING);
            }
            return 0;
        }
    }

    return 0;
}
