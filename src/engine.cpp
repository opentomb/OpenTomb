
#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <al.h>
#include <alc.h>
}

#include "core/system.h"
#include "core/gl_util.h"
#include "core/gl_font.h"
#include "core/console.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/gl_text.h"
#include "render/camera.h"
#include "render/render.h"
#include "script/script.h"
#include "physics/physics.h"
#include "fmv/tiny_codec.h"
#include "fmv/stream_codec.h"
#include "gui/gui.h"
#include "vt/vt_level.h"
#include "audio/audio.h"
#include "game.h"
#include "mesh.h"
#include "skeletal_model.h"
#include "entity.h"
#include "gameflow.h"
#include "room.h"
#include "world.h"
#include "resource.h"
#include "engine.h"
#include "controls.h"
#include "trigger.h"
#include "character_controller.h"
#include "render/bsp_tree.h"
#include "render/shader_manager.h"
#include "image.h"


static SDL_Window             *sdl_window     = NULL;
static SDL_Joystick           *sdl_joystick   = NULL;
static SDL_GameController     *sdl_controller = NULL;
static SDL_Haptic             *sdl_haptic     = NULL;
static SDL_GLContext           sdl_gl_context = 0;
static ALCdevice              *al_device      = NULL;
static ALCcontext             *al_context     = NULL;

static stream_codec_t           engine_video;

static char                     base_path[1024] = {0};
static volatile int             engine_done   = 0;
static int                      engine_set_zero_time = 0;
float time_scale = 1.0f;

engine_container_p      last_cont = NULL;
static float            ray_test_point[3] = {0.0f, 0.0f, 0.0f};
static ss_bone_frame_t  test_model = {0};
static int32_t          test_model_index = 0;

struct engine_control_state_s           control_states = {0};
struct control_settings_s               control_mapper = {0};
float                                   engine_frame_time = 0.0;

lua_State                              *engine_lua = NULL;
struct camera_s                         engine_camera;
struct camera_state_s                   engine_camera_state;


enum debug_view_state_e
{
    no_debug = 0,
    player_anim,
    sector_info,
    room_objects,
    ai_boxes,
    bsp_info,
    model_view,
    debug_states_count
};


engine_container_p Container_Create()
{
    engine_container_p ret;

    ret = (engine_container_p)malloc(sizeof(engine_container_t));
    ret->collision_group = COLLISION_GROUP_KINEMATIC;
    ret->collision_mask = COLLISION_MASK_ALL;
    ret->next = NULL;
    ret->object = NULL;
    ret->room = NULL;
    ret->sector = NULL;
    ret->object_type = 0;
    return ret;
}

void Container_Delete(engine_container_p cont)
{
    free(cont);
}


extern "C" int  Engine_ExecCmd(char *ch);

void Engine_Init_Pre();
void Engine_Init_Post();
void Engine_InitGL();
void Engine_InitAL();
void Engine_InitSDLVideo();
void Engine_InitSDLControls();
void Engine_InitDefaultGlobals();

void Engine_Display(float time);
void Engine_PollSDLEvents();
void Engine_Resize(int nominalW, int nominalH, int pixelsW, int pixelsH);

void TestModelApplyKey(int key);
void SetTestModel(int index);
void ShowModelView(float time);
void ShowDebugInfo();

void Engine_Start(int argc, char **argv)
{
    char *config_name = NULL;
    char *autoexec_name = NULL;

    Engine_InitDefaultGlobals();

    for(int i = 1; i < argc; ++i)
    {
        if(0 == strncmp(argv[i], "-config", 7))
        {
            if((i + 1 < argc) && (Sys_FileFound(argv[i + 1], 0)))
            {
                config_name = argv[i + 1];
            }
            ++i;
        }
        else if(0 == strncmp(argv[i], "-autoexec", 9))
        {
            if((i + 1 < argc) && (Sys_FileFound(argv[i + 1], 0)))
            {
                autoexec_name = argv[i + 1];
            }
            ++i;
        }
        else if(0 == strncmp(argv[i], "-base_path", 10))
        {
            if(i + 1 < argc)
            {
                strncpy(base_path, argv[i + 1], sizeof(base_path) - 1);
                if(base_path[0])
                {
                    char *ch = base_path;
                    for(; *ch; ++ch)
                    {
                        if(*ch == '\\')
                        {
                            *ch = '/';
                        }
                    }
                    if(*(ch - 1) != '/')
                    {
                        *ch = '/';
                        ++ch;
                        *ch = 0;
                    }
                }
            }
            ++i;
        }
        else
        {
            puts("usage:");
            puts("-config \"path_to_config_file\"");
            puts("-autoexec \"path_to_autoexec_file\"");
            puts("-base_path \"path_to_base_folder_location (contains data, resource, save and script folders)\"");
            exit(0);
        }
    }

    // Primary initialization.
    Engine_Init_Pre();

    Engine_LoadConfig(config_name ? config_name : "config.lua");

    // Init generic SDL interfaces.
    Engine_InitSDLControls();
    Engine_InitSDLVideo();
    Engine_InitAL();
    Audio_StreamExternalInit();

    // Additional OpenGL initialization.
    Engine_InitGL();
    renderer.DoShaders();

    // Secondary (deferred) initialization.
    Engine_Init_Post();

    // Make splash screen.
    Gui_LoadScreenAssignPic("resource/graphics/legal");

    // Initial window resize.
    Engine_Resize(screen_info.w, screen_info.h, screen_info.w, screen_info.h);

    // Clearing up memory for initial level loading.
    World_Prepare();

    // Setting up mouse.
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_WarpMouseInWindow(sdl_window, screen_info.w / 2, screen_info.h / 2);
    SDL_ShowCursor(0);

    luaL_dofile(engine_lua, autoexec_name ? autoexec_name : "autoexec.lua");
}


void Engine_Shutdown(int val)
{
    renderer.ResetWorld(NULL, 0, NULL, 0);
    SSBoneFrame_Clear(&test_model);
    World_Clear();

    stream_codec_clear(&engine_video);

    if(engine_lua)
    {
        lua_close(engine_lua);
        engine_lua = NULL;
    }

    Audio_StreamExternalDeinit();
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
    SDL_Quit();

    exit(val);
}


const char *Engine_GetBasePath()
{
    return base_path;
}


void Engine_SetDone()
{
    stream_codec_stop(&engine_video, 0);
    Audio_StreamExternalStop();
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
    stream_codec_init(&engine_video);

    Sys_Init();
    GLText_Init();
    Con_Init();
    Gameflow_Init();
    Con_SetExecFunction(Engine_ExecCmd);
    Script_LuaInit();

    Script_CallVoidFunc(engine_lua, "loadscript_pre", true);

    Cam_Init(&engine_camera);
    engine_camera_state.state = CAMERA_STATE_NORMAL;
    engine_camera_state.target_id = ENTITY_ID_NONE;
    engine_camera_state.flyby = NULL;
    engine_camera_state.sink = NULL;
    engine_camera_state.shake_value = 0.0f;
    engine_camera_state.time = 0.0f;
    Mat4_E_macro(engine_camera_state.cutscene_tr);
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


void Engine_InitSDLVideo()
{
    Uint32 video_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;
    PFNGLGETSTRINGPROC lglGetString = NULL;

    if(screen_info.fullscreen)
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
    if(filename && Sys_FileFound(filename, 0))
    {
        lua_State *lua = luaL_newstate();
        if(lua != NULL)
        {
            luaL_openlibs(lua);
            lua_register(lua, "bind", lua_BindKey);                             // get and set key bindings
            lua_pushstring(lua, Engine_GetBasePath());
            lua_setglobal(lua, "base_path");
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
        Sys_Warn("Could not find config file");
    }
}


void Engine_SaveConfig(const char *filename)
{

}


void Engine_Display(float time)
{
    if(!engine_done)
    {
        qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//| GL_ACCUM_BUFFER_BIT);

        Cam_Apply(&engine_camera);
        Cam_RecalcClipPlanes(&engine_camera);
        // GL_VERTEX_ARRAY | GL_COLOR_ARRAY

        screen_info.debug_view_state %= debug_states_count;
        if(screen_info.debug_view_state)
        {
            ShowDebugInfo();
        }

        qglPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT); ///@PUSH <- GL_VERTEX_ARRAY | GL_COLOR_ARRAY
        qglEnableClientState(GL_NORMAL_ARRAY);
        qglEnableClientState(GL_TEXTURE_COORD_ARRAY);

        qglFrontFace(GL_CW);

        if(screen_info.debug_view_state != debug_view_state_e::model_view)
        {
            renderer.GenWorldList(&engine_camera);
            renderer.DrawList();
        }
        else
        {
            /*qglPolygonMode(GL_FRONT, GL_FILL);
            qglDisable(GL_CULL_FACE);*/
            qglDisable(GL_BLEND);
            qglEnable(GL_ALPHA_TEST);
            ShowModelView(time);
        }
        Gui_SwitchGLMode(1);
        qglEnable(GL_ALPHA_TEST);

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
    const float scale_coeff = 1024.0f;
    screen_info.w = nominalW;
    screen_info.h = nominalH;

    screen_info.scale_factor = (screen_info.w < screen_info.h) ? (screen_info.h / scale_coeff) : (screen_info.w / scale_coeff);

    GLText_UpdateResize(screen_info.w, screen_info.h, screen_info.scale_factor);
    Con_UpdateResize();
    Gui_UpdateResize();

    Cam_SetFovAspect(&engine_camera, screen_info.fov, (float)nominalW / (float)nominalH);
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
                        control_states.look_axis_x = event.motion.xrel * control_mapper.mouse_sensitivity_x;
                        control_states.look_axis_y = event.motion.yrel * control_mapper.mouse_sensitivity_y;
                    }

                    if((event.motion.x < ((screen_info.w / 2) - (screen_info.w / 4))) ||
                       (event.motion.x > ((screen_info.w / 2) + (screen_info.w / 4))) ||
                       (event.motion.y < ((screen_info.h / 2) - (screen_info.h / 4))) ||
                       (event.motion.y > ((screen_info.h / 2) + (screen_info.h / 4))))
                    {
                        SDL_WarpMouseInWindow(sdl_window, screen_info.w / 2, screen_info.h / 2);
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
                if((event.key.keysym.scancode == SDL_SCANCODE_F4) &&
                   (event.key.state == SDL_PRESSED) &&
                   (event.key.keysym.mod & KMOD_ALT))
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
                    Controls_Key(event.key.keysym.scancode, event.key.state);
                    // DEBUG KEYBOARD COMMANDS
                    Controls_DebugKeys(event.key.keysym.scancode, event.key.state);
                    if((screen_info.debug_view_state == debug_view_state_e::model_view) && event.key.state)
                    {
                        TestModelApplyKey(event.key.keysym.scancode);
                    }
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

        if(engine_set_zero_time)
        {
            engine_set_zero_time = 0;
            time = 0.0f;
        }
        else if(time > 1.0f / 30.0f)
        {
            time = 1.0f / 30.0f;
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

        Sys_ResetTempMem();
        Engine_PollSDLEvents();

        gl_text_line_p fps = GLText_OutTextXY(10.0f, 10.0f, fps_str);
        if(fps)
        {
            fps->x_align    = GLTEXT_ALIGN_RIGHT;
            fps->y_align    = GLTEXT_ALIGN_BOTTOM;
            fps->font_id    = FONT_PRIMARY;
            fps->style_id   = FONTSTYLE_MENU_TITLE;
        }

        int codec_end_state = stream_codec_check_end(&engine_video);
        if(codec_end_state == 1)
        {
            Audio_StreamExternalStop();
        }

        if(codec_end_state >= 0)
        {
            if(screen_info.debug_view_state != debug_view_state_e::model_view)
            {
                Game_Frame(time);
                Gameflow_ProcessCommands();
            }
            Audio_Update(time);
            Engine_Display(time);
        }
        else
        {
            stream_codec_audio_lock(&engine_video);
            if(engine_video.codec.audio.buff && (engine_video.codec.audio.buff_offset >= Audio_StreamExternalBufferOffset()))
            {
                Audio_StreamExternalUpdateBuffer(engine_video.codec.audio.buff, engine_video.codec.audio.buff_size,
                    engine_video.codec.audio.bits_per_sample, engine_video.codec.audio.channels, engine_video.codec.audio.sample_rate);
            }
            if(Audio_StreamExternalBufferIsNeedUpdate())
            {
                engine_video.update_audio = 1;
            }
            stream_codec_audio_unlock(&engine_video);
            Audio_StreamExternalPlay();

            stream_codec_video_lock(&engine_video);
            if(engine_video.codec.video.rgba)
            {
                Gui_SetScreenTexture(engine_video.codec.video.rgba, engine_video.codec.video.width, engine_video.codec.video.height, 32);
            }
            stream_codec_video_unlock(&engine_video);
            Gui_DrawLoadScreen(-1);

            if(control_states.gui_inventory)
            {
                stream_codec_stop(&engine_video, 0);
            }
        }
    }
}


void TestModelApplyKey(int key)
{
    switch(key)
    {
        case SDL_SCANCODE_LEFTBRACKET:
            test_model_index--;
            SetTestModel(test_model_index);
            break;

        case SDL_SCANCODE_RIGHTBRACKET:
            test_model_index++;
            SetTestModel(test_model_index);
            break;

        case SDL_SCANCODE_O:
            if(test_model.animations.current_animation > 0)
            {
                Anim_SetAnimation(&test_model.animations, test_model.animations.current_animation - 1, 0);
            }
            break;

        case SDL_SCANCODE_P:
            Anim_SetAnimation(&test_model.animations, test_model.animations.current_animation + 1, 0);
            break;

        default:
            break;
    }
}


void SetTestModel(int index)
{
    skeletal_model_p sm;
    uint32_t sm_count;
    World_GetSkeletalModelsInfo(&sm, &sm_count);

    index = (index >= 0) ? (index) : (sm_count - 1);
    index = (index >= sm_count) ? (0) : (index);

    if(sm_count > 0)
    {
        test_model_index = index;
        SSBoneFrame_Clear(&test_model);
        SSBoneFrame_CreateFromModel(&test_model, sm + index);
    }
}


void ShowModelView(float time)
{
    static float tr[16];
    static float test_model_angles[3] = {45.0f, 45.0f, 0.0f};
    static float test_model_dist = 1024.0f;
    static float test_model_z_offset = 256.0f;
    uint32_t sm_count;
    skeletal_model_p sm = NULL;

    World_GetSkeletalModelsInfo(&sm, &sm_count);
    if((test_model_index >= 0) && (test_model_index < sm_count))
    {
        sm += test_model_index;
    }

    if(sm && (test_model.animations.model == sm))
    {
        float subModelView[16], subModelViewProjection[16];
        float *cam_pos = engine_camera.gl_transform + 12;
        animation_frame_p af = sm->animations + test_model.animations.current_animation;
        const int current_light_number = 0;
        const lit_shader_description *shader = renderer.shaderManager->getEntityShader(current_light_number);

        if(control_states.look_right || control_states.move_right)
        {
            test_model_angles[0] += time * 256.0f;
        }
        if(control_states.look_left || control_states.move_left)
        {
            test_model_angles[0] -= time * 256.0f;
        }
        if(control_states.look_up)
        {
            test_model_angles[1] += time * 256.0f;
        }
        if(control_states.look_down)
        {
            test_model_angles[1] -= time * 256.0f;
        }
        if(control_states.move_forward && (test_model_dist >= 8.0f))
        {
            test_model_dist -= time * 512.0f;
        }
        if(control_states.move_backward)
        {
            test_model_dist += time * 512.0f;
        }
        if(control_states.move_up)
        {
            test_model_z_offset += time * 512.0f;
        }
        if(control_states.move_down)
        {
            test_model_z_offset -= time * 512.0f;
        }

        test_model.transform = tr;
        Mat4_E_macro(tr);
        Mat4_E_macro(engine_camera.gl_transform);
        engine_camera.ang[0] = test_model_angles[0];
        engine_camera.ang[1] = test_model_angles[1] + 90.0f;
        engine_camera.ang[2] = test_model_angles[2];
        Mat4_SetAnglesZXY(engine_camera.gl_transform, engine_camera.ang);
        cam_pos[0] = -engine_camera.gl_transform[8 + 0] * test_model_dist;
        cam_pos[1] = -engine_camera.gl_transform[8 + 1] * test_model_dist;
        cam_pos[2] = -engine_camera.gl_transform[8 + 2] * test_model_dist + test_model_z_offset;
        Cam_Apply(&engine_camera);

        test_model.animations.frame_time += time;
        test_model.animations.current_frame = test_model.animations.frame_time / test_model.animations.period;
        if(test_model.animations.current_frame >= af->frames_count)
        {
            test_model.animations.frame_time = 0.0f;
            test_model.animations.current_frame = 0;
        }
        test_model.animations.next_frame = test_model.animations.current_frame;
        SSBoneFrame_Update(&test_model, 0.0f);

        Mat4_Mat4_mul(subModelView, engine_camera.gl_view_mat, tr);
        Mat4_Mat4_mul(subModelViewProjection, engine_camera.gl_view_proj_mat, tr);
        qglUseProgramObjectARB(shader->program);

        {
            GLfloat ambient_component[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            qglUniform4fvARB(shader->light_ambient, 1, ambient_component);
        }
        renderer.DrawSkeletalModel(shader, &test_model, subModelView, subModelViewProjection);
        renderer.debugDrawer->DrawAxis(4096.0f, tr);

        for(int i = 0; i < test_model.bone_tag_count; ++i)
        {
            ss_bone_tag_p bf = test_model.bone_tags + i;
            Mat4_vec3_mul_macro(tr, bf->full_transform, bf->mesh_base->centre);
            renderer.OutTextXYZ(tr[0], tr[1], tr[2], "%d", i);
        }

        {
            const float dy = -18.0f * screen_info.scale_factor;
            float y = (float)screen_info.h + dy;

            GLText_OutTextXY(30.0f, y += dy, "MODEL[%d]; state: %d", (int)sm->id, (int)af->state_id);
            GLText_OutTextXY(30.0f, y += dy, "anim: %d of %d", (int)test_model.animations.current_animation, (int)sm->animation_count);
            GLText_OutTextXY(30.0f, y += dy, "frame: %d of %d, %d", (int)test_model.animations.current_frame, (int)af->max_frame, (int)af->frames_count);
            GLText_OutTextXY(30.0f, y += dy, "next a: %d,next f: %d", (int)af->next_anim->id, (int)af->next_frame);

            for(animation_command_p cmd = af->commands; cmd; cmd = cmd->next)
            {
                GLText_OutTextXY(30.0f, y += dy, "command[%d]: {%.1f,  %.1f,  %.1f}", (int)cmd->id, cmd->data[0], cmd->data[1], cmd->data[2]);
            }

            y = (float)screen_info.h + dy;
            for(uint16_t i = 0; i < af->state_change_count; ++i)
            {
                state_change_p stc = af->state_change + i;
                GLText_OutTextXY(screen_info.w - 350, y += dy, "state change[%d]:", (int)stc->id);
                for(uint16_t j = 0; j < stc->anim_dispatch_count; ++j)
                {
                    anim_dispatch_p disp = stc->anim_dispatch + j;
                    GLText_OutTextXY(screen_info.w - 320, y += dy, "frame_int(%d, %d), to (%d, %d):", (int)disp->frame_low, (int)disp->frame_high, (int)disp->next_anim, (int)disp->next_frame);
                }
            }
        }
    }
    else
    {
        SetTestModel(test_model_index);
    }
}


void ShowDebugInfo()
{
    float y = (float)screen_info.h;
    const float dy = -18.0f * screen_info.scale_factor;

    if(last_cont && (screen_info.debug_view_state != debug_view_state_e::model_view))
    {
        GLText_OutTextXY(30.0f, y += dy, "VIEW: Selected object");
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
                    room_p room = (room_p)last_cont->object;
                    room_sector_p rs = Room_GetSectorRaw(room, ray_test_point);
                    if(rs != NULL)
                    {
                        renderer.debugDrawer->SetColor(0.0, 1.0, 0.0);
                        renderer.debugDrawer->DrawSectorDebugLines(rs);
                        GLText_OutTextXY(30.0f, y += dy, "cont_room: (id = %d, sx = %d, sy = %d)", room->id, rs->index_x, rs->index_y);
                        GLText_OutTextXY(30.0f, y += dy, "room_below = %d, room_above = %d", (rs->room_below) ? (rs->room_below->id) : (-1), (rs->room_above) ? (rs->room_above->id) : (-1));
                        if(rs->trigger)
                        {
                            char trig_type[64];
                            char trig_func[64];
                            char trig_mask[16];
                            Trigger_TrigTypeToStr(trig_type, 64, rs->trigger->sub_function);
                            Trigger_TrigMaskToStr(trig_mask, rs->trigger->mask);
                            GLText_OutTextXY(30.0f, y += dy, "trig(sub = %s, val = 0x%X, mask = 0b%s, timer = %d)", trig_type, rs->trigger->function_value, trig_mask, rs->trigger->timer);
                            for(trigger_command_p cmd = rs->trigger->commands; cmd; cmd = cmd->next)
                            {
                                entity_p trig_obj = World_GetEntityByID(cmd->operands);
                                if(trig_obj)
                                {
                                    renderer.debugDrawer->SetColor(0.0, 0.0, 1.0);
                                    renderer.debugDrawer->DrawBBox(trig_obj->bf->bb_min, trig_obj->bf->bb_max, trig_obj->transform);
                                    Trigger_TrigMaskToStr(trig_mask, trig_obj->trigger_layout);
                                    gl_text_line_p text = renderer.OutTextXYZ(trig_obj->transform[12 + 0], trig_obj->transform[12 + 1], trig_obj->transform[12 + 2], "(id = 0x%X, layout = 0b%s)", trig_obj->id, trig_mask);
                                    if(text)
                                    {
                                        text->x_align = GLTEXT_ALIGN_CENTER;
                                    }
                                }
                                Trigger_TrigCmdToStr(trig_func, 64, cmd->function);
                                if(cmd->function == TR_FD_TRIGFUNC_SET_CAMERA)
                                {
                                    GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X, cam_id = 0x%X, cam_move = %d, cam_timer = %d)", trig_func, cmd->operands, cmd->camera.index, cmd->camera.move, cmd->camera.timer);
                                }
                                else
                                {
                                    GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X)", trig_func, cmd->operands);
                                }
                            }
                        }
                    }
                }
                break;
        }
    }

    switch(screen_info.debug_view_state)
    {
        case debug_view_state_e::player_anim:
            {
                GLText_OutTextXY(30.0f, y += dy, "VIEW: Lara anim");
                entity_p ent = World_GetPlayer();
                if(ent && ent->character)
                {
                    animation_frame_p anim = ent->bf->animations.model->animations + ent->bf->animations.current_animation;
                    GLText_OutTextXY(30.0f, y += dy, "curr_st = %03d, next_st = %03d", anim->state_id, ent->bf->animations.next_state);
                    GLText_OutTextXY(30.0f, y += dy, "curr_anim = %03d, curr_frame = %03d, next_anim = %03d, next_frame = %03d", ent->bf->animations.current_animation, ent->bf->animations.current_frame, ent->bf->animations.next_animation, ent->bf->animations.next_frame);
                    GLText_OutTextXY(30.0f, y += dy, "anim_next_anim = %03d, anim_next_frame = %03d", anim->next_anim->id, anim->next_frame);
                    GLText_OutTextXY(30.0f, y += dy, "posX = %f, posY = %f, posZ = %f", ent->transform[12], ent->transform[13], ent->transform[14]);
                }
            }
            break;

        case debug_view_state_e::sector_info:
            {
                entity_p ent = World_GetPlayer();
                GLText_OutTextXY(30.0f, y += dy, "VIEW: Sector info");
                if(engine_camera.current_room)
                {
                    GLText_OutTextXY(30.0f, y += dy, "cam_room = (id = %d)", engine_camera.current_room->id);
                }
                if(ent && ent->self->room)
                {
                    GLText_OutTextXY(30.0f, y += dy, "char_pos = (%.1f, %.1f, %.1f)", ent->transform[12 + 0], ent->transform[12 + 1], ent->transform[12 + 2]);
                    room_p room = ent->self->room;
                    room_sector_p rs = Room_GetSectorRaw(room, ent->transform + 12);
                    if(rs != NULL)
                    {
                        renderer.debugDrawer->SetColor(0.0f, 1.0f, 0.0f);
                        renderer.debugDrawer->DrawSectorDebugLines(rs);
                        GLText_OutTextXY(30.0f, y += dy, "room = (id = %d, sx = %d, sy = %d)", room->id, rs->index_x, rs->index_y);
                        GLText_OutTextXY(30.0f, y += dy, "room_below = %d, room_above = %d", (rs->room_below) ? (rs->room_below->id) : (-1), (rs->room_above) ? (rs->room_above->id) : (-1));
                        for(int i = 0; i < room->content->overlapped_room_list_size; ++i)
                        {
                            GLText_OutTextXY(30.0f, y += dy, "overlapped room (id = %d)", room->content->overlapped_room_list[i]->id);
                        }
                        for(int i = 0; i < room->content->near_room_list_size; ++i)
                        {
                            GLText_OutTextXY(30.0f, y += dy, "near room (id = %d)", room->content->near_room_list[i]->id);
                        }
                        if(rs->trigger)
                        {
                            char trig_type[64];
                            char trig_func[64];
                            char trig_mask[16];
                            Trigger_TrigTypeToStr(trig_type, 64, rs->trigger->sub_function);
                            Trigger_TrigMaskToStr(trig_mask, rs->trigger->mask);
                            GLText_OutTextXY(30.0f, y += dy, "trig(sub = %s, val = 0x%X, mask = 0b%s, timer = %d)", trig_type, rs->trigger->function_value, trig_mask, rs->trigger->timer);
                            for(trigger_command_p cmd = rs->trigger->commands; cmd; cmd = cmd->next)
                            {
                                entity_p trig_obj = World_GetEntityByID(cmd->operands);
                                if(trig_obj)
                                {
                                    renderer.debugDrawer->SetColor(0.0f, 0.0f, 1.0f);
                                    renderer.debugDrawer->DrawBBox(trig_obj->bf->bb_min, trig_obj->bf->bb_max, trig_obj->transform);
                                    Trigger_TrigMaskToStr(trig_mask, trig_obj->trigger_layout);
                                    gl_text_line_p text = renderer.OutTextXYZ(trig_obj->transform[12 + 0], trig_obj->transform[12 + 1], trig_obj->transform[12 + 2], "(id = 0x%X, layout = 0b%s)", trig_obj->id, trig_mask);
                                    if(text)
                                    {
                                        text->x_align = GLTEXT_ALIGN_CENTER;
                                    }
                                }
                                Trigger_TrigCmdToStr(trig_func, 64, cmd->function);
                                if(cmd->function == TR_FD_TRIGFUNC_SET_CAMERA)
                                {
                                    GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X, cam_id = 0x%X, cam_move = %d, cam_timer = %d)", trig_func, cmd->operands, cmd->camera.index, cmd->camera.move, cmd->camera.timer);
                                }
                                else
                                {
                                    GLText_OutTextXY(30.0f, y += dy, "   cmd(func = %s, op = 0x%X)", trig_func, cmd->operands);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case debug_view_state_e::room_objects:
            {
                entity_p ent = World_GetPlayer();
                GLText_OutTextXY(30.0f, y += dy, "VIEW: Room objects");
                if(ent && ent->self->room)
                {
                    room_p r = ent->self->room;
                    for(engine_container_p cont = r->containers; cont; cont = cont->next)
                    {
                        if(cont->object_type == OBJECT_ENTITY)
                        {
                            entity_p e = (entity_p)cont->object;
                            gl_text_line_p text = renderer.OutTextXYZ(e->transform[12 + 0], e->transform[12 + 1], e->transform[12 + 2], "(entity[0x%X])", e->id);
                            if(text)
                            {
                                text->x_align = GLTEXT_ALIGN_CENTER;
                            }
                        }
                    }

                    for(uint32_t i = 0; i < r->content->static_mesh_count; ++i)
                    {
                        static_mesh_p sm = r->content->static_mesh + i;
                        gl_text_line_p text = renderer.OutTextXYZ(sm->pos[0], sm->pos[1], sm->pos[2], "(static[0x%X])", sm->object_id);
                        if(text)
                        {
                            text->x_align = GLTEXT_ALIGN_CENTER;
                        }
                    }
                }
            }
            break;

        case debug_view_state_e::ai_boxes:
            {
                entity_p ent = World_GetPlayer();
                GLText_OutTextXY(30.0f, y += dy, "VIEW: AI boxes");
                if(ent && ent->self->sector && ent->self->sector->box)
                {
                    room_box_p box = ent->self->sector->box;
                    GLText_OutTextXY(30.0f, y += dy, "box = %d, floor = %d", (int)box->id, (int)box->bb_min[2]);
                    GLText_OutTextXY(30.0f, y += dy, "blockable = %d, blocked = %d", (int)box->is_blockable, (int)box->is_blocked);
                    GLText_OutTextXY(30.0f, y += dy, "fly = %d", (int)box->zone.FlyZone_Normal);
                    GLText_OutTextXY(30.0f, y += dy, "zones = %d, %d, %d, %d", (int)box->zone.GroundZone1_Normal, (int)box->zone.GroundZone2_Normal, (int)box->zone.GroundZone3_Normal, (int)box->zone.GroundZone4_Normal);
                    for(box_overlap_p ov = box->overlaps; ov; ov++)
                    {
                        GLText_OutTextXY(30.0f, y += dy, "overlap = %d", (int)ov->box);
                        if(ov->end)
                        {
                            break;
                        }
                    }

                    float tr[16];
                    Mat4_E_macro(tr);
                    renderer.debugDrawer->DrawBBox(box->bb_min, box->bb_max, tr);
                    if(ent->character && last_cont && (last_cont->object_type == OBJECT_ENTITY))
                    {
                        entity_p foe = (entity_p)last_cont->object;
                        if(foe->character && foe->self->sector)
                        {
                            Character_UpdatePath(foe, ent->self->sector);
                            if(foe->character->path_dist > 0)
                            {
                                renderer.debugDrawer->SetColor(0.0f, 0.0f, 0.0f);
                                for(int i = 0; i < foe->character->path_dist; ++i)
                                {
                                    renderer.debugDrawer->DrawBBox(foe->character->path[i]->bb_min, foe->character->path[i]->bb_max, tr);
                                }

                                GLfloat red[3] = {1.0f, 0.0f, 0.0f};
                                GLfloat from[3], to[3];
                                vec3_copy(from, foe->self->sector->pos);
                                from[2] = foe->transform[12 + 2] + TR_METERING_STEP;
                                for(int i = 1; i < foe->character->path_dist; ++i)
                                {
                                    Room_GetOverlapCenter(foe->character->path[i], foe->character->path[i - 1], to);
                                    renderer.debugDrawer->DrawLine(from, to, red, red);
                                    vec3_copy(from, to);
                                }
                                vec3_copy(to, ent->self->sector->pos);
                                to[2] = ent->transform[12 + 2] + TR_METERING_STEP;
                                renderer.debugDrawer->DrawLine(from, to, red, red);
                            }
                        }
                    }
                }
            }
            break;

        case debug_view_state_e::bsp_info:
            GLText_OutTextXY(30.0f, y += dy, "VIEW: BSP tree info");
            if(renderer.dynamicBSP)
            {
                GLText_OutTextXY(30.0f, y += dy, "input polygons = %07d", renderer.dynamicBSP->GetInputPolygonsCount());
                GLText_OutTextXY(30.0f, y += dy, "added polygons = %07d", renderer.dynamicBSP->GetAddedPolygonsCount());
            }
            break;

        case debug_view_state_e::model_view:
            GLText_OutTextXY(30.0f, y += dy, "VIEW: MODELS ANIM (use o, p, [, ], w, s, space, v and arrows)");
            break;
    };
}


/*
 * MISC ENGINE FUNCTIONALITY
 */

void Engine_TakeScreenShot()
{
    static int screenshot_cnt = 0;
    GLint ViewPort[4];
    char fname[128];
    GLubyte *pixels;
    uint32_t str_size;

    qglGetIntegerv(GL_VIEWPORT, ViewPort);
    snprintf(fname, 128, "screen_%.5d.png", screenshot_cnt);
    str_size = ViewPort[2] * 4;
    pixels = (GLubyte*)malloc(str_size * ViewPort[3]);
    qglReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    Image_Save(fname, IMAGE_FORMAT_PNG, (uint8_t*)pixels, ViewPort[2], ViewPort[3], 32);

    free(pixels);
    screenshot_cnt++;
}


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


void Engine_GetLevelScriptNameLocal(const char *level_path, int game_version, char *name, uint32_t buf_size)
{
    char level_name[LEVEL_NAME_MAX_LEN];
    Engine_GetLevelName(level_name, level_path);

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
    strncat(name, ".lua", buf_size);
}


bool Engine_LoadPCLevel(const char *name)
{
    int trv = VT_Level::get_PC_level_version(name);
    if(trv != TR_UNKNOWN)
    {
        World_Open(name, trv);

        char buf[LEVEL_NAME_MAX_LEN] = {0x00};
        Engine_GetLevelName(buf, name);

        Con_Notify("loaded PC level");
        Con_Notify("version = %d, map = \"%s\"", trv, buf);
        return true;
    }
    return false;
}


int Engine_LoadMap(const char *name)
{
    size_t map_len = strlen(name);
    size_t base_len = strlen(base_path);
    size_t buf_len = map_len + base_len + 1;
    char *map_name_buf = (char*)Sys_GetTempMem(buf_len);

    strncpy(map_name_buf, base_path, buf_len);
    strncat(map_name_buf, name, buf_len);

    if(!Sys_FileFound(map_name_buf, 0))
    {
        Con_Warning("file not found: \"%s\"", map_name_buf);
        return 0;
    }

    Game_StopFlyBy();
    engine_camera.current_room = NULL;
    engine_camera_state.state = CAMERA_STATE_NORMAL;
    engine_camera_state.time = 0.0f;
    engine_camera_state.sink = NULL;
    engine_camera_state.target_id = ENTITY_ID_NONE;
    Mat4_E_macro(engine_camera_state.cutscene_tr);
    renderer.ResetWorld(NULL, 0, NULL, 0);
    Audio_EndStreams();
    Gui_DrawLoadScreen(0);

    Gui_DrawLoadScreen(100);
    // Here we can place different platform-specific level loading routines.
    bool is_success_load = false;
    switch(VT_Level::get_level_format(map_name_buf))
    {
        case LEVEL_FORMAT_PC:
            is_success_load = Engine_LoadPCLevel(map_name_buf);
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
    Sys_ReturnTempMem(buf_len);

    if(is_success_load)
    {
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
        engine_set_zero_time = 1;
    }

    return is_success_load;
}


int  Engine_PlayVideo(const char *name)
{
    if(engine_video.state == VIDEO_STATE_STOPPED)
    {
        Audio_StreamExternalStop();
        Audio_StopStreams(-1);
        Con_SetShown(0);
        return stream_codec_play_rpl(&engine_video, name);
    }
    return 0;
}


int  Engine_IsVideoPlayed()
{
    return (engine_video.state != VIDEO_STATE_STOPPED);
}


extern "C" int Engine_ExecCmd(char *ch)
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
            Con_AddLine("r_crosshair - switch crosshair visibility\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cam_distance - camera distance to actor\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("r_wireframe, r_portals, r_frustums, r_room_boxes, r_boxes, r_normals, r_skip_room, r_flyby, r_cinematics, r_triggers, r_ai_boxes, r_cameras - render modes\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("playsound(id) - play specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("stopsound(id) - stop specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("Watch out for case sensitive commands!\0", FONTSTYLE_CONSOLE_WARNING);
        }
        else if(!strcmp(token, "goto"))
        {
            control_states.free_look = 1;
            engine_camera.gl_transform[12 + 0] = SC_ParseFloat(&ch);
            engine_camera.gl_transform[12 + 1] = SC_ParseFloat(&ch);
            engine_camera.gl_transform[12 + 2] = SC_ParseFloat(&ch);
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
        else if(!strcmp(token, "r_cameras"))
        {
            renderer.r_flags ^= R_DRAW_CAMERAS;
            return 1;
        }
        else if(!strcmp(token, "r_cinematics"))
        {
            renderer.r_flags ^= R_DRAW_CINEMATICS;
            return 1;
        }
        else if(!strcmp(token, "r_triggers"))
        {
            renderer.r_flags ^= R_DRAW_TRIGGERS;
            return 1;
        }
        else if(!strcmp(token, "r_ai_boxes"))
        {
            renderer.r_flags ^= R_DRAW_AI_BOXES;
            return 1;
        }
        else if(!strcmp(token, "r_crosshair"))
        {
            screen_info.crosshair = !screen_info.crosshair;
            return 1;
        }
        else if(!strcmp(token, "room_info"))
        {
            room_p r = engine_camera.current_room;
            if(r)
            {
                room_sector_p sect = Room_GetSectorXYZ(r, engine_camera.gl_transform + 12);
                Con_Printf("ID = %d, x_sect = %d, y_sect = %d", r->id, r->sectors_x, r->sectors_y);
                if(sect)
                {
                    Con_Printf("sect(%d, %d), inpenitrable = %d, r_up = %d, r_down = %d", sect->index_x, sect->index_y,
                               (int)(sect->ceiling == TR_METERING_WALLHEIGHT || sect->floor == TR_METERING_WALLHEIGHT), (int)(sect->room_above != NULL), (int)(sect->room_below != NULL));
                    for(uint32_t i = 0; i < r->content->static_mesh_count; i++)
                    {
                        Con_Printf("static[%d].object_id = %d", i, r->content->static_mesh[i].object_id);
                    }
                    for(engine_container_p cont = r->containers; cont; cont=cont->next)
                    {
                        if(cont->object_type == OBJECT_ENTITY)
                        {
                            entity_p e = (entity_p)cont->object;
                            Con_Printf("cont[entity](%d, %d, %d).object_id = %d", (int)e->transform[12 + 0], (int)e->transform[12 + 1], (int)e->transform[12 + 2], e->id);
                        }
                    }
                }
            }
            return 1;
        }
        else if(!strcmp(token, "xxx"))
        {
            //stream_codec_play_rpl(&engine_video, "data/tr2/fmv/CRASH.RPL");
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
                Con_AddText("Not available =(", FONTSTYLE_CONSOLE_WARNING);
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
