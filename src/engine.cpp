
#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
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
#include "render/shader_manager.h"
#include "script/script.h"
#include "physics/physics.h"
#include "fmv/tiny_codec.h"
#include "fmv/stream_codec.h"
#include "gui/gui.h"
#include "gui/gui_inventory.h"
#include "vt/vt_level.h"
#include "audio/audio.h"
#include "audio/audio_stream.h"
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
#include "image.h"
#include "core/utf8_32.h"


static SDL_Window              *sdl_window     = NULL;
static SDL_Joystick            *sdl_joystick   = NULL;
static SDL_GameController      *sdl_controller = NULL;
static SDL_Haptic              *sdl_haptic     = NULL;
static SDL_GLContext            sdl_gl_context = 0;

static stream_codec_t           engine_video;

static char                     base_path[1024] = {0};
static char                     config_name[1024] = "config.lua";
static volatile int             engine_done   = 0;
static int                      g_menu_mode = 0x00;
static int                      engine_set_zero_time = 0;
float                           time_scale = 1.0f;
float                           engine_frame_time = 0.0;

lua_State                      *engine_lua = NULL;
struct camera_s                 engine_camera;
struct camera_state_s           engine_camera_state;
static void (*g_text_handler)(int cmd, uint32_t key, void *data) = NULL;
static void *g_text_handler_data;


extern "C" int  Engine_ExecCmd(char *ch);

void Engine_Init_Pre();
void Engine_Init_Post();
void Engine_LoadConfig(const char *filename);
void Engine_InitGL();
void Engine_InitSDLVideo();
void Engine_InitSDLSubsystems();
void Engine_InitDefaultGlobals();

void Engine_Display(float time);
void Engine_PollSDLEvents();
void Engine_Resize(int nominalW, int nominalH, int pixelsW, int pixelsH);

void ClearTestModel();
void TestModelApplyKey(int key);
void Test_SecondaryMouseDown();
void SetTestModel(int index);
void ShowModelView(float time);
void ShowDebugInfo();

void Engine_Start(int argc, char **argv)
{
    char *autoexec_name = NULL;

    Engine_InitDefaultGlobals();

    for(int i = 1; i < argc; ++i)
    {
        if(0 == strncmp(argv[i], "-config", 7))
        {
            if((i + 1 < argc) && (Sys_FileFound(argv[i + 1], 0)))
            {
                strncpy(config_name, argv[i + 1], sizeof(config_name));
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
    Engine_InitSDLSubsystems();
    Engine_InitSDLVideo();

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
    Audio_CoreInit();

    luaL_dofile(engine_lua, autoexec_name ? autoexec_name : "autoexec.lua");
}


void Engine_Shutdown(int val)
{
    char path[1024];
    size_t path_base_len = sizeof(path) - 1;
    strncpy(path, Engine_GetBasePath(), path_base_len);
    path[path_base_len] = 0;
    strncat(path, config_name, path_base_len - strlen(path));
    Script_ExportConfig(path);

    stream_codec_stop(&engine_video, 0);
    StreamTrack_Stop(Audio_GetStreamExternal());

    renderer.ResetWorld(NULL, 0, NULL, 0);
    ClearTestModel();
    World_Clear();

    stream_codec_clear(&engine_video);

    if(engine_lua)
    {
        lua_close(engine_lua);
        engine_lua = NULL;
    }

    Gameflow_Destroy();
    Physics_Destroy();
    Gui_Destroy();
    Con_Destroy();
    GLText_Destroy();
    glf_destroy();
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

    Audio_CoreDeinit();

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
    glf_init();
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

    if(SDL_GL_LoadLibrary(NULL) < 0)
    {
        Sys_Error("Could not init OpenGL driver: %s", SDL_GetError());
    }

    if(renderer.settings.antialias)
    {
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
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // set the opengl context version
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    sdl_window = SDL_CreateWindow("OpenTomb", screen_info.x, screen_info.y, screen_info.w, screen_info.h, video_flags);
    if(!sdl_window)
    {
        Sys_Error("Could not create SDL window: %s", SDL_GetError());
    }
    sdl_gl_context = SDL_GL_CreateContext(sdl_window);
    if(!sdl_gl_context)
    {
        Sys_Error("Could not create GL context: %s", SDL_GetError());
    }

    lglGetString = (PFNGLGETSTRINGPROC)SDL_GL_GetProcAddress("glGetString");
    Con_AddLine((const char*)lglGetString(GL_VENDOR), FONTSTYLE_CONSOLE_INFO);
    Con_AddLine((const char*)lglGetString(GL_RENDERER), FONTSTYLE_CONSOLE_INFO);
    Con_Printf("OpenGL version %s", lglGetString(GL_VERSION));
    Con_AddLine((const char*)lglGetString(GL_SHADING_LANGUAGE_VERSION), FONTSTYLE_CONSOLE_INFO);
}


void Engine_InitSDLSubsystems()
{
    int    NumJoysticks;
    Uint32 init_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;                       // These flags are used in any case.

    if(control_settings.use_joy == 1)
    {
        init_flags |= SDL_INIT_GAMECONTROLLER;                                  // Update init flags for joystick.

        if(control_settings.joy_rumble)
        {
            init_flags |= SDL_INIT_HAPTIC;                                      // Update init flags for force feedback.
        }

        SDL_Init(init_flags);

        NumJoysticks = SDL_NumJoysticks();
        if((NumJoysticks < 1) || ((NumJoysticks - 1) < control_settings.joy_number))
        {
            Sys_DebugLog(SYS_LOG_FILENAME, "Error: there is no joystick #%d present.", control_settings.joy_number);
            return;
        }

        if(SDL_IsGameController(control_settings.joy_number))                   // If joystick has mapping (e.g. X360 controller)
        {
            SDL_GameControllerEventState(SDL_ENABLE);                           // Use GameController API
            sdl_controller = SDL_GameControllerOpen(control_settings.joy_number);

            if(!sdl_controller)
            {
                Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't open game controller #%d.", control_settings.joy_number);
                SDL_GameControllerEventState(SDL_DISABLE);                      // If controller init failed, close state.
                control_settings.use_joy = 0;
            }
            else if(control_settings.joy_rumble)                                // Create force feedback interface.
            {
                sdl_haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(sdl_controller));
                if(!sdl_haptic)
                {
                    Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't initialize haptic from game controller #%d.", control_settings.joy_number);
                }
            }
        }
        else
        {
            SDL_JoystickEventState(SDL_ENABLE);                                 // If joystick isn't mapped, use generic API.
            sdl_joystick = SDL_JoystickOpen(control_settings.joy_number);

            if(!sdl_joystick)
            {
                Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't open joystick #%d.", control_settings.joy_number);
                SDL_JoystickEventState(SDL_DISABLE);                            // If joystick init failed, close state.
                control_settings.use_joy = 0;
            }
            else if(control_settings.joy_rumble)                                // Create force feedback interface.
            {
                sdl_haptic = SDL_HapticOpenFromJoystick(sdl_joystick);
                if(!sdl_haptic)
                {
                    Sys_DebugLog(SYS_LOG_FILENAME, "Error: can't initialize haptic from joystick #%d.", control_settings.joy_number);
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
        if(lua)
        {
            console_params_t cp = { 0 };
            luaL_openlibs(lua);
            lua_register(lua, "bind", lua_Bind);                                // get and set key bindings
            lua_pushstring(lua, Engine_GetBasePath());
            lua_setglobal(lua, "base_path");
            Script_LuaRegisterConfigFuncs(lua);
            luaL_dofile(lua, filename);

            Script_ParseScreen(lua, &screen_info);
            Script_ParseRender(lua, &renderer.settings);
            Script_ParseAudio(lua, &audio_settings);
            Script_ParseControls(lua, &control_settings);

            if(0 < Script_ParseConsole(lua, &cp))
            {
                Con_SetParams(&cp);
            }
            lua_close(lua);
        }
    }
    else
    {
        Sys_Warn("Could not find config file");
    }
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

    GLText_UpdateResize(screen_info.scale_factor);
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
    
    for(int i = 0; i < ACT_LASTINDEX; i++)
    {
        control_states.actions[i].prev_state = control_states.actions[i].state;
    }
    control_states.last_key = 0;

    while(SDL_PollEvent(&event))
    {
        g_menu_mode = g_text_handler ? ((Gui_ConIsShown()) ? (0x01) : (0x02)) : 0x00;
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
                if(!g_menu_mode && (control_states.mouse_look != 0) &&
                    ((event.motion.x != (screen_info.w / 2)) ||
                     (event.motion.y != (screen_info.h / 2))))
                {
                    if(mouse_setup)                                             // it is not perfect way, but cursor
                    {                                                           // every engine start is in one place
                        control_states.look_axis_x = event.motion.xrel * control_settings.mouse_sensitivity_x;
                        control_states.look_axis_y = event.motion.yrel * control_settings.mouse_sensitivity_y;
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

            case SDL_MOUSEWHEEL:
                if(Gui_ConIsShown())
                {
                    Gui_ConScroll(event.wheel.y);
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == 1) //LM = 1, MM = 2, RM = 3
                {
                    Controls_PrimaryMouseDown(from, to);
                }
                else if(event.button.button == 3)
                {
                    Test_SecondaryMouseDown();
                }
                break;

            // Controller events are only invoked when joystick is initialized as
            // game controller, otherwise, generic joystick event will be used.
            case SDL_CONTROLLERAXISMOTION:
                Controls_WrapGameControllerAxis(event.caxis.axis, event.caxis.value, g_menu_mode);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                Controls_WrapGameControllerKey(event.cbutton.button, event.cbutton.state, g_menu_mode);
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
                    Controls_JoyHat(event.jhat.value, g_menu_mode);
                }
                break;

            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                // NOTE: Joystick button numbers are passed with added JOY_BUTTON_MASK (1000).
                if(sdl_joystick)
                {
                    Controls_Key((event.jbutton.button + JOY_BUTTON_MASK), event.jbutton.state, g_menu_mode);
                }
                break;

            case SDL_TEXTINPUT:
            case SDL_TEXTEDITING:
                if(g_text_handler && event.key.state)
                {
                    uint8_t *utf8 = (uint8_t*)event.text.text;
                    uint32_t utf32;
                    while(*utf8)
                    {
                        utf8 = utf8_to_utf32(utf8, &utf32);
                        g_text_handler(GUI_COMMAND_TEXT, utf32, g_text_handler_data);
                    }
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

                if(g_text_handler && event.key.state)
                {
                    int cmd = GUI_COMMAND_NONE;
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_RETURN:
                            cmd = GUI_COMMAND_ACTIVATE;
                            break;

                        case SDLK_ESCAPE:
                            cmd = GUI_COMMAND_CLOSE;
                            break;

                        case SDLK_LEFT:
                            cmd = GUI_COMMAND_LEFT;
                            break;

                        case SDLK_RIGHT:
                            cmd = GUI_COMMAND_RIGHT;
                            break;

                        case SDLK_UP:
                            cmd = GUI_COMMAND_UP;
                            break;

                        case SDLK_DOWN:
                            cmd = GUI_COMMAND_DOWN;
                            break;

                        case SDLK_HOME:
                            cmd = GUI_COMMAND_HOME;
                            break;

                        case SDLK_END:
                            cmd = GUI_COMMAND_END;
                            break;

                        case SDLK_BACKSPACE:
                            cmd = GUI_COMMAND_BACKSPACE;
                            break;

                        case SDLK_DELETE:
                            cmd = GUI_COMMAND_DELETE;
                            break;
                    };
                    g_text_handler(cmd, event.key.keysym.sym, g_text_handler_data);
                }
                else if(!g_text_handler)
                {
                    Controls_DebugKeys(event.key.keysym.scancode, event.key.state);
                    if((screen_info.debug_view_state == debug_view_state_e::model_view) && event.key.state)
                    {
                        TestModelApplyKey(event.key.keysym.scancode);
                    }
                }
                Controls_Key(event.key.keysym.scancode, event.key.state, g_menu_mode);
                break;

            case SDL_QUIT:
                Engine_SetDone();
                break;

            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    Engine_Resize(event.window.data1, event.window.data2, event.window.data1, event.window.data2);
                }
                else if(event.window.event == SDL_WINDOWEVENT_MOVED)
                {
                    screen_info.x = event.window.data1;
                    screen_info.y = event.window.data2;
                }
                break;

            default:
                break;
        }
        g_menu_mode = g_text_handler ? ((Gui_ConIsShown()) ? (0x01) : (0x02)) : 0x00;
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
    int64_t newtime = 0;
    int64_t sec_base_offset = Sys_MicroSecTime(0) / 1E6;
    int64_t oldtime = Sys_MicroSecTime(sec_base_offset);
    float time = 0.0f;
    float time_cycl = 0.0f;

    const int max_cycles = 64;
    int cycles = 0;
    char fps_str[32] = "0.0";

    while(!engine_done)
    {
        newtime = Sys_MicroSecTime(sec_base_offset);
        time = (newtime - oldtime) / 1E6;
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
            snprintf(fps_str, sizeof(fps_str), "%.1f", screen_info.fps);
            cycles = 0;
            time_cycl = 0.0f;
        }

        Sys_ResetTempMem();
        Engine_PollSDLEvents();

        if (renderer.settings.show_fps)
        {
            gl_text_line_p fps = GLText_OutTextXY(screen_info.w - 10.0f, 10.0f, fps_str);
            if (fps)
            {
                fps->x_align = GLTEXT_ALIGN_RIGHT;
                fps->y_align = GLTEXT_ALIGN_BOTTOM;
                fps->font_id = FONT_PRIMARY;
                fps->style_id = FONTSTYLE_MENU_TITLE;
            }
        }

        int codec_end_state = stream_codec_check_end(&engine_video);
        if(codec_end_state == 1)
        {
            StreamTrack_Stop(Audio_GetStreamExternal());
        }

        if(codec_end_state >= 0)
        {
            if(!g_menu_mode && (screen_info.debug_view_state != debug_view_state_e::model_view))
            {
                Game_Frame(time);
                Gameflow_ProcessCommands();
            }
            Audio_Update(time);
            Engine_Display(time);
        }
        else
        {
            stream_track_p s = Audio_GetStreamExternal();
            stream_codec_audio_lock(&engine_video);
            if(engine_video.codec.audio.buff && (engine_video.codec.audio.buff_offset >= s->buffer_offset))
            {
                s->current_volume = audio_settings.sound_volume;
                StreamTrack_UpdateBuffer(s, engine_video.codec.audio.buff, engine_video.codec.audio.buff_size,
                    engine_video.codec.audio.bits_per_sample, engine_video.codec.audio.channels, engine_video.codec.audio.sample_rate);
            }
            if(StreamTrack_IsNeedUpdateBuffer(s))
            {
                engine_video.update_audio = 1;
            }
            stream_codec_audio_unlock(&engine_video);
            StreamTrack_Play(s);

            stream_codec_video_lock(&engine_video);
            if(engine_video.codec.video.rgba)
            {
                Gui_SetScreenTexture(engine_video.codec.video.rgba, engine_video.codec.video.width, engine_video.codec.video.height, 32);
            }
            stream_codec_video_unlock(&engine_video);
            Gui_DrawLoadScreen(-1);

            if(control_states.actions[ACT_INVENTORY].state &&
              !control_states.actions[ACT_INVENTORY].prev_state)
            {
                stream_codec_stop(&engine_video, 0);
            }
        }
    }
}


/*
 * MISC ENGINE FUNCTIONALITY
 */

void Engine_SetTextInputHandler(void (*f)(int cmd, uint32_t key, void *data), void *data)
{
    g_text_handler = f;
    g_text_handler_data = data;
    if(f)
    {
        SDL_StartTextInput();
    }
    else
    {
        SDL_StopTextInput();
    }
}

void Engine_TakeScreenShot()
{
    GLint ViewPort[4];
    char fname[128];
    GLubyte *pixels;
    uint32_t str_size;

    time_t now = time(0);
    struct tm tstruct = *localtime(&now);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);

    qglGetIntegerv(GL_VIEWPORT, ViewPort);
    snprintf(fname, sizeof(fname), "screen_%s.png", buf);

    str_size = ViewPort[2] * 4;
    pixels = (GLubyte*)malloc(str_size * ViewPort[3]);
    qglReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    Image_Save(fname, IMAGE_FORMAT_PNG, (uint8_t*)pixels, ViewPort[2], ViewPort[3], 32);

    free(pixels);
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
        codec_init(&engine_video.codec, SDL_RWFromFile(name, "rb"));
        if(engine_video.codec.input)
        {
            if(0 == codec_open_rpl(&engine_video.codec))
            {
                stream_track_p s = Audio_GetStreamExternal();
                StreamTrack_Stop(s);
                Audio_StopStreams(-1);
                Gui_ConShow(0);

                s->current_volume = audio_settings.sound_volume;
                while(StreamTrack_IsNeedUpdateBuffer(s))
                {
                    codec_decode_audio(&engine_video.codec);
                    if(engine_video.codec.audio.buff && (engine_video.codec.audio.buff_offset >= s->buffer_offset))
                    {
                        StreamTrack_UpdateBuffer(s, engine_video.codec.audio.buff, engine_video.codec.audio.buff_size,
                            engine_video.codec.audio.bits_per_sample, engine_video.codec.audio.channels, engine_video.codec.audio.sample_rate);
                    }
                }

                return stream_codec_play(&engine_video);
            }
            codec_clear(&engine_video.codec);
        }
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
        ch = SC_ParseToken(ch, token, sizeof(token));
        if(!strcmp(token, "help"))
        {
            Con_AddLine("Available commands:\0", FONTSTYLE_CONSOLE_WARNING);
            Con_AddLine("help - show help info\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("loadMap(\"file_name\") - load level \"file_name\"\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("setgamef(game, level) - load level (ie: setgamef(2, 1) for TR2 level 1)\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("save, load - save and load game state in \"file_name\"\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("exit - close program\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cls - clean console\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("show_fps - switch show fps flag\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cvars - lua's table of cvar's, to see them type: show_table(cvars)\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("free_look - switch camera mode\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("r_crosshair - switch crosshair visibility\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cam_distance - camera distance to actor\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("r_wireframe, r_portals, r_frustums, r_room_boxes, r_boxes, r_normals, r_skip_room, r_flyby, r_cinematics, r_triggers, r_ai_boxes, r_cameras - render modes, r_path - show character path\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("playsound(id) - play specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("stopsound(id) - stop specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("Watch out for case sensitive commands!\0", FONTSTYLE_CONSOLE_WARNING);
        }
        else if(!strcmp(token, "goto"))
        {
            control_states.free_look = 1;
            engine_camera.transform.M4x4[12 + 0] = SC_ParseFloat(&ch);
            engine_camera.transform.M4x4[12 + 1] = SC_ParseFloat(&ch);
            engine_camera.transform.M4x4[12 + 2] = SC_ParseFloat(&ch);
            return 1;
        }
        else if(!strcmp(token, "save"))
        {
            ch = SC_ParseToken(ch, token, sizeof(token));
            if(NULL != ch)
            {
                Game_Save(token);
            }
            return 1;
        }
        else if(!strcmp(token, "load"))
        {
            ch = SC_ParseToken(ch, token, sizeof(token));
            if(NULL != ch)
            {
                Game_Load(token);
            }
            return 1;
        }
        else if(!strcmp(token, "exit"))
        {
            Engine_SetDone();
            return 1;
        }
        else if(!strcmp(token, "cls"))
        {
            Con_Clean();
            return 1;
        }
        else if(!strcmp(token, "show_fps"))
        {
            renderer.settings.show_fps = !renderer.settings.show_fps;
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
        else if(!strcmp(token, "r_path"))
        {
            renderer.r_flags ^= R_DRAW_AI_PATH;
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
                room_sector_p sect = Room_GetSectorXYZ(r, engine_camera.transform.M4x4 + 12);
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
                            Con_Printf("cont[entity](%d, %d, %d).object_id = %d", (int)e->transform.M4x4[12 + 0], (int)e->transform.M4x4[12 + 1], (int)e->transform.M4x4[12 + 2], e->id);
                        }
                    }
                }
            }
            return 1;
        }
        else if(!strcmp(token, "xxx"))
        {
            Con_SetLinesHistorySize(18);
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
            return 1;
        }
    }

    return 0;
}
