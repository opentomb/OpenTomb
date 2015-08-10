#include "engine.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_haptic.h>

#if !defined(__MACOSX__)
#include <SDL2/SDL_image.h>
#endif

#if defined(__MACOSX__)
#include "mac/FindConfigFile.h"
#endif

#include "LuaState.h"
#include "vt/level.h"

#include "gl_util.h"
#include "polygon.h"
#include "vmath.h"
#include "controls.h"
#include "console.h"
#include "system.h"
#include "common.h"
#include "script.h"
#include "render.h"
#include "game.h"
#include "world.h"
#include "camera.h"
#include "mesh.h"
#include "entity.h"
#include "resource.h"
#include "gui.h"
#include "audio.h"
#include "character_controller.h"
#include "gameflow.h"
#include "strings.h"


SDL_Window             *sdl_window = nullptr;
SDL_Joystick           *sdl_joystick = nullptr;
SDL_GameController     *sdl_controller = nullptr;
SDL_Haptic             *sdl_haptic = nullptr;
SDL_GLContext           sdl_gl_context = nullptr;
ALCdevice              *al_device = nullptr;
ALCcontext             *al_context = nullptr;

EngineControlState control_states{};
ControlSettings    control_mapper{};
AudioSettings      audio_settings{};

btScalar           engine_frame_time = 0.0;

Camera             engine_camera;
World              engine_world;

static btScalar   *frame_vertex_buffer = nullptr;
static size_t      frame_vertex_buffer_size = 0;
static size_t      frame_vertex_buffer_size_left = 0;

script::MainEngine engine_lua;

btDefaultCollisionConfiguration     *bt_engine_collisionConfiguration = nullptr;
btCollisionDispatcher               *bt_engine_dispatcher = nullptr;
btGhostPairCallback                 *bt_engine_ghostPairCallback = nullptr;
btBroadphaseInterface               *bt_engine_overlappingPairCache = nullptr;
btSequentialImpulseConstraintSolver *bt_engine_solver = nullptr;
btDiscreteDynamicsWorld             *bt_engine_dynamicsWorld = nullptr;
btOverlapFilterCallback             *bt_engine_filterCallback = nullptr;

RenderDebugDrawer                    debugDrawer;

// Debug globals.

btVector3 light_position = { 255.0, 255.0, 8.0 };
GLfloat cast_ray[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

EngineContainer* last_cont = nullptr;

void Engine_InitGL()
{
    glewExperimental = GL_TRUE;
    glewInit();

    // GLEW sometimes causes an OpenGL error for no apparent reason. Retrieve and
    // discard it so it doesn't clog up later logging.

    glGetError();

    glClearColor(0.0, 0.0, 0.0, 1.0);

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
}

void Engine_InitSDLControls()
{
    Uint32 init_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS; // These flags are used in any case.

    if(control_mapper.use_joy == 1)
    {
        init_flags |= SDL_INIT_GAMECONTROLLER;  // Update init flags for joystick.

        if(control_mapper.joy_rumble)
        {
            init_flags |= SDL_INIT_HAPTIC;      // Update init flags for force feedback.
        }

        SDL_Init(init_flags);

        int NumJoysticks = SDL_NumJoysticks();

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

    ///@TODO: is it really needed for correct work?

    if(SDL_GL_LoadLibrary(nullptr) < 0)
    {
        Sys_Error("Could not init OpenGL driver");
    }

    if(renderer.settings().use_gl3)
    {
        /* Request opengl 3.2 context. */
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    }

    // Create temporary SDL window and GL context for checking capabilities.

    sdl_window = SDL_CreateWindow(nullptr, screen_info.x, screen_info.y, screen_info.w, screen_info.h, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    sdl_gl_context = SDL_GL_CreateContext(sdl_window);

    if(!sdl_gl_context)
        Sys_Error("Can't create OpenGL context - shutting down. Try to disable use_gl3 option in config.");


    assert(sdl_gl_context);
    SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);

    // Check for correct number of antialias samples.

    if(renderer.settings().antialias)
    {
        GLint maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        maxSamples = (maxSamples > 16) ? (16) : (maxSamples);   // Fix for faulty GL max. sample number.

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

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, renderer.settings().antialias);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, renderer.settings().antialias_samples);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }

    // Remove temporary GL context and SDL window.

    SDL_GL_DeleteContext(sdl_gl_context);
    SDL_DestroyWindow(sdl_window);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, renderer.settings().z_depth);

#if STENCIL_FRUSTUM
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

    sdl_window = SDL_CreateWindow("OpenTomb", screen_info.x, screen_info.y, screen_info.w, screen_info.h, video_flags);
    sdl_gl_context = SDL_GL_CreateContext(sdl_window);
    SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);

    ConsoleInfo::instance().addLine(reinterpret_cast<const char*>(glGetString(GL_VENDOR)), FONTSTYLE_CONSOLE_INFO);
    ConsoleInfo::instance().addLine(reinterpret_cast<const char*>(glGetString(GL_RENDERER)), FONTSTYLE_CONSOLE_INFO);
    std::string version = "OpenGL version ";
    version += reinterpret_cast<const char*>(glGetString(GL_VERSION));
    ConsoleInfo::instance().addLine(version, FONTSTYLE_CONSOLE_INFO);
    ConsoleInfo::instance().addLine(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)), FONTSTYLE_CONSOLE_INFO);
}

#if !defined(__MACOSX__)
void Engine_InitSDLImage()
{
    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int init = IMG_Init(flags);

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
        ALC_FREQUENCY,       44100, 0 };

    Sys_DebugLog(LOG_FILENAME, "Probing OpenAL devices...");

    const char *devlist = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);

    if(!devlist)
    {
        Sys_DebugLog(LOG_FILENAME, "InitAL: No AL audio devices!");
        return;
    }

    while(*devlist)
    {
        Sys_DebugLog(LOG_FILENAME, " Device: %s", devlist);
        ALCdevice* dev = alcOpenDevice(devlist);

        if(audio_settings.use_effects)
        {
            if(alcIsExtensionPresent(dev, ALC_EXT_EFX_NAME) == ALC_TRUE)
            {
                Sys_DebugLog(LOG_FILENAME, " EFX supported!");
                al_device = dev;
                al_context = alcCreateContext(al_device, paramList);
                // fails e.g. with Rapture3D, where EFX is supported
                if(al_context)
                {
                    break;
                }
            }
            alcCloseDevice(dev);
            devlist += std::strlen(devlist) + 1;
        }
        else
        {
            al_device = dev;
            al_context = alcCreateContext(al_device, paramList);
            break;
        }
    }

    if(!al_context)
    {
        Sys_DebugLog(LOG_FILENAME, " Failed to create OpenAL context.");
        alcCloseDevice(al_device);
        al_device = nullptr;
        return;
    }

    alcMakeContextCurrent(al_context);

    Audio_LoadALExtFunctions(al_device);

    std::string driver = "OpenAL library: ";
    driver += alcGetString(al_device, ALC_DEVICE_SPECIFIER);
    ConsoleInfo::instance().addLine(driver, FONTSTYLE_CONSOLE_INFO);

    alSpeedOfSound(330.0 * 512.0);
    alDopplerVelocity(330.0 * 510.0);
    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
#endif
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

    ConsoleInfo::instance().notify(SYSNOTE_ENGINE_INITED);

    // Clearing up memory for initial level loading.
    engine_world.prepare();

#ifdef NDEBUG
    // Setting up mouse.
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_WarpMouseInWindow(sdl_window, screen_info.w / 2, screen_info.h / 2);
    SDL_ShowCursor(0);
#endif

    // Make splash screen.
    Gui_FadeAssignPic(FADER_LOADSCREEN, "resource/graphics/legal.png");
    Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_OUT);

    engine_lua.doFile("autoexec.lua");
}

void Engine_Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//| GL_ACCUM_BUFFER_BIT);

    engine_camera.apply();
    engine_camera.recalcClipPlanes();
    // GL_VERTEX_ARRAY | GL_COLOR_ARRAY
    if(screen_info.show_debuginfo)
    {
        Engine_ShowDebugInfo();
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

void Engine_Resize(int nominalW, int nominalH, int pixelsW, int pixelsH)
{
    screen_info.w = nominalW;
    screen_info.h = nominalH;

    screen_info.w_unit = static_cast<float>(nominalW) / GUI_SCREEN_METERING_RESOLUTION;
    screen_info.h_unit = static_cast<float>(nominalH) / GUI_SCREEN_METERING_RESOLUTION;
    screen_info.scale_factor = (screen_info.w < screen_info.h) ? (screen_info.h_unit) : (screen_info.w_unit);

    Gui_Resize();

    engine_camera.setFovAspect(screen_info.fov, static_cast<btScalar>(nominalW) / static_cast<btScalar>(nominalH));
    engine_camera.recalcClipPlanes();

    glViewport(0, 0, pixelsW, pixelsH);
}

extern gui_text_line_t system_fps;

namespace
{
    int fpsCycles = 0;
    btScalar fpsTime = 0;

    void fpsCycle(btScalar time)
    {
        if(fpsCycles < 20)
        {
            fpsCycles++;
            fpsTime += time;
        }
        else
        {
            screen_info.fps = (20.0f / fpsTime);
            snprintf(system_fps.text, system_fps.text_size, "%.1f", screen_info.fps);
            fpsCycles = 0;
            fpsTime = 0.0;
        }
    }
}

void Engine_Frame(btScalar time)
{
    if(time > 0.1)
    {
        time = 0.1f;
    }

    engine_frame_time = time;
    fpsCycle(time);

    Game_Frame(time);
    Gameflow_Do();
}

void Engine_ShowDebugInfo()
{
    GLfloat color_array[] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

    light_position = engine_camera.getPosition();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(2.0);
    glVertexPointer(3, GL_FLOAT, 0, cast_ray);
    glColorPointer(3, GL_FLOAT, 0, color_array);
    glDrawArrays(GL_LINES, 0, 2);

    if(std::shared_ptr<Character> ent = engine_world.character)
    {
        /*height_info_p fc = &ent->character->height_info
        txt = Gui_OutTextXY(20.0 / screen_info.w, 80.0 / screen_info.w, "Z_min = %d, Z_max = %d, W = %d", (int)fc->floor_point[2], (int)fc->ceiling_point[2], (int)fc->water_level);
        */
        Gui_OutTextXY(30.0, 30.0, "last_anim = %03d, curr_anim = %03d, next_anim = %03d, last_st = %03d, next_st = %03d, speed=%f frame=%d",
                      ent->m_bf.animations.last_animation,
                      ent->m_bf.animations.current_animation,
                      ent->m_bf.animations.next_animation,
                      ent->m_bf.animations.last_state,
                      ent->m_bf.animations.next_state,
                      engine_world.character->m_currentSpeed,
                      ent->m_bf.animations.current_frame
                      );
        //Gui_OutTextXY(30.0, 30.0, "curr_anim = %03d, next_anim = %03d, curr_frame = %03d, next_frame = %03d", ent->bf.animations.current_animation, ent->bf.animations.next_animation, ent->bf.animations.current_frame, ent->bf.animations.next_frame);
        Gui_OutTextXY(20, 8, "posX = %f, posY = %f, posZ = %f", ent->m_transform.getOrigin()[0], ent->m_transform.getOrigin()[1], ent->m_transform.getOrigin()[2]);
    }

    if(last_cont != nullptr)
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

    if(engine_camera.m_currentRoom != nullptr)
    {
        RoomSector* rs = engine_camera.m_currentRoom->getSectorRaw(engine_camera.getPosition());
        if(rs != nullptr)
        {
            Gui_OutTextXY(30.0, 90.0, "room = (id = %d, sx = %d, sy = %d)", engine_camera.m_currentRoom->id, rs->index_x, rs->index_y);
            Gui_OutTextXY(30.0, 120.0, "room_below = %d, room_above = %d", (rs->sector_below != nullptr) ? (rs->sector_below->owner_room->id) : (-1), (rs->sector_above != nullptr) ? (rs->sector_above->owner_room->id) : (-1));
        }
    }
    Gui_OutTextXY(30.0, 150.0, "cam_pos = (%.1f, %.1f, %.1f)", engine_camera.getPosition()[0], engine_camera.getPosition()[1], engine_camera.getPosition()[2]);
}

/**
 * overlapping room collision filter
 */
void Engine_RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
    EngineContainer* c0, *c1;

    c0 = static_cast<EngineContainer*>(static_cast<btCollisionObject*>(collisionPair.m_pProxy0->m_clientObject)->getUserPointer());
    Room* r0 = c0 ? c0->room : nullptr;
    c1 = static_cast<EngineContainer*>(static_cast<btCollisionObject*>(collisionPair.m_pProxy1->m_clientObject)->getUserPointer());
    Room* r1 = c1 ? c1->room : nullptr;

    if(c1 && c1 == c0)
    {
        if(static_cast<btCollisionObject*>(collisionPair.m_pProxy0->m_clientObject)->isStaticOrKinematicObject() ||
           static_cast<btCollisionObject*>(collisionPair.m_pProxy1->m_clientObject)->isStaticOrKinematicObject())
        {
            return;                                                             // No self interaction
        }
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
        return;
    }

    if(!r0 && !r1)
    {
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);// Both are out of rooms
        return;
    }

    if(r0 && r1)
    {
        if(r0->isInNearRoomsList(*r1))
        {
            dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
            return;
        }
        else
        {
            return;
        }
    }
}

/**
 * update current room of bullet object
 */
void Engine_InternalTickCallback(btDynamicsWorld *world, btScalar /*timeStep*/)
{
    for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        assert(i >= 0 && i < bt_engine_dynamicsWorld->getCollisionObjectArray().size());
        btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if(body && !body->isStaticObject() && body->getMotionState())
        {
            btTransform trans;
            body->getMotionState()->getWorldTransform(trans);
            EngineContainer* cont = static_cast<EngineContainer*>(body->getUserPointer());
            if(cont && (cont->object_type == OBJECT_BULLET_MISC))
            {
                cont->room = Room_FindPosCogerrence(trans.getOrigin(), cont->room);
            }
        }
    }
}

void Engine_InitDefaultGlobals()
{
    ConsoleInfo::instance().initGlobals();
    Controls_InitGlobals();
    Game_InitGlobals();
    renderer.initGlobals();
    Audio_InitGlobals();
}

// First stage of initialization.

void Engine_Init_Pre()
{
    /* Console must be initialized previously! some functions uses ConsoleInfo::instance().addLine before GL initialization!
     * Rendering activation may be done later. */

    Gui_InitFontManager();
    ConsoleInfo::instance().init();

    engine_lua["loadscript_pre"]();

    Gameflow_Init();

    frame_vertex_buffer = static_cast<btScalar*>(malloc(sizeof(btScalar) * INIT_FRAME_VERTEX_BUFFER_SIZE));
    frame_vertex_buffer_size = INIT_FRAME_VERTEX_BUFFER_SIZE;
    frame_vertex_buffer_size_left = frame_vertex_buffer_size;

    ConsoleInfo::instance().setCompletionItems(engine_lua.getGlobals());

    Com_Init();
    renderer.init();
    renderer.setCamera(&engine_camera);

    Engine_InitBullet();
}

// Second stage of initialization.

void Engine_Init_Post()
{
    engine_lua["loadscript_post"]();

    ConsoleInfo::instance().initFonts();

    Gui_Init();
    Sys_Init();
}

// Bullet Physics initialization.

void Engine_InitBullet()
{
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    bt_engine_collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    bt_engine_dispatcher = new btCollisionDispatcher(bt_engine_collisionConfiguration);
    bt_engine_dispatcher->setNearCallback(Engine_RoomNearCallback);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    bt_engine_overlappingPairCache = new btDbvtBroadphase();
    bt_engine_ghostPairCallback = new btGhostPairCallback();
    bt_engine_overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(bt_engine_ghostPairCallback);

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    bt_engine_solver = new btSequentialImpulseConstraintSolver;

    bt_engine_dynamicsWorld = new btDiscreteDynamicsWorld(bt_engine_dispatcher, bt_engine_overlappingPairCache, bt_engine_solver, bt_engine_collisionConfiguration);
    bt_engine_dynamicsWorld->setInternalTickCallback(Engine_InternalTickCallback);
    bt_engine_dynamicsWorld->setGravity(btVector3(0, 0, -4500.0));

    debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints);
    bt_engine_dynamicsWorld->setDebugDrawer(&debugDrawer);
    //bt_engine_dynamicsWorld->getPairCache()->setInternalGhostPairCallback(bt_engine_filterCallback);
}

void Engine_DumpRoom(Room* r)
{
    if(r != nullptr)
    {
        Sys_DebugLog("room_dump.txt", "ROOM = %d, (%d x %d), bottom = %g, top = %g, pos(%g, %g)", r->id, r->sectors_x, r->sectors_y, r->bb_min[2], r->bb_max[2], r->transform.getOrigin()[0], r->transform.getOrigin()[1]);
        Sys_DebugLog("room_dump.txt", "flag = 0x%X, alt_room = %d, base_room = %d", r->flags, (r->alternate_room != nullptr) ? (r->alternate_room->id) : (-1), (r->base_room != nullptr) ? (r->base_room->id) : (-1));
        for(const RoomSector& rs : r->sectors)
        {
            Sys_DebugLog("room_dump.txt", "(%d,%d)\tfloor = %d, ceiling = %d, portal = %d", rs.index_x, rs.index_y, rs.floor, rs.ceiling, rs.portal_to_room);
        }
        for(auto sm : r->static_mesh)
        {
            Sys_DebugLog("room_dump.txt", "static_mesh = %d", sm->object_id);
        }
        for(const std::shared_ptr<EngineContainer>& cont : r->containers)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                Entity* ent = static_cast<Entity*>(cont->object);
                Sys_DebugLog("room_dump.txt", "entity: id = %d, model = %d", ent->id(), ent->m_bf.animations.model->id);
            }
        }
    }
}

void Engine_Destroy()
{
    renderer.empty();
    //ConsoleInfo::instance().destroy();
    Com_Destroy();
    Sys_Destroy();

    //delete dynamics world
    delete bt_engine_dynamicsWorld;

    //delete solver
    delete bt_engine_solver;

    //delete broadphase
    delete bt_engine_overlappingPairCache;

    //delete dispatcher
    delete bt_engine_dispatcher;

    delete bt_engine_collisionConfiguration;

    delete bt_engine_ghostPairCallback;

    Gui_Destroy();
}

void Engine_Shutdown(int val)
{
    engine_lua.clearTasks();
    renderer.empty();
    engine_world.empty();
    Engine_Destroy();

    /* no more renderings */
    SDL_GL_DeleteContext(sdl_gl_context);
    SDL_DestroyWindow(sdl_window);

    if(sdl_joystick)
    {
        SDL_JoystickClose(sdl_joystick);
    }

    if(sdl_controller)
    {
        SDL_GameControllerClose(sdl_controller);
    }

    if(sdl_haptic)
    {
        SDL_HapticClose(sdl_haptic);
    }

    if(al_context)  // T4Larson <t4larson@gmail.com>: fixed
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(al_context);
    }

    if(al_device)
    {
        alcCloseDevice(al_device);
    }

    /* free temporary memory */
    if(frame_vertex_buffer)
    {
        free(frame_vertex_buffer);
    }
    frame_vertex_buffer = nullptr;
    frame_vertex_buffer_size = 0;
    frame_vertex_buffer_size_left = 0;

#if !defined(__MACOSX__)
    IMG_Quit();
#endif
    SDL_Quit();

    exit(val);
}

bool Engine_FileFound(const std::string& name, bool Write)
{
    FILE *ff;

    if(Write)
    {
        ff = fopen(name.c_str(), "ab");
    }
    else
    {
        ff = fopen(name.c_str(), "rb");
    }

    if(!ff)
    {
        return false;
    }
    else
    {
        fclose(ff);
        return true;
    }
}

int Engine_GetLevelFormat(const std::string& /*name*/)
{
    // PLACEHOLDER: Currently, only PC levels are supported.

    return LEVEL_FORMAT_PC;
}

std::string Engine_GetLevelName(const std::string& path)
{
    if(path.empty())
    {
        return{};
    }

    size_t ext = path.find_last_of(".");
    assert(ext != std::string::npos);

    size_t start = path.find_last_of("\\/");
    if(start == std::string::npos)
        start = 0;
    else
        ++start;

    return path.substr(start, ext - start);
}

std::string Engine_GetLevelScriptName(loader::Game game_version, const std::string& postfix)
{
    std::string level_name = Engine_GetLevelName(gameflow_manager.CurrentLevelPath);

    std::string name = "scripts/level/";

    if(game_version < loader::Game::TR2)
    {
        name += "tr1/";
    }
    else if(game_version < loader::Game::TR3)
    {
        name += "tr2/";
    }
    else if(game_version < loader::Game::TR4)
    {
        name += "tr3/";
    }
    else if(game_version < loader::Game::TR5)
    {
        name += "tr4/";
    }
    else
    {
        name += "tr5/";
    }

    for(char& c : level_name)
    {
        c = std::toupper(c);
    }

    name += level_name;
    name += postfix;
    name += ".lua";
    return name;
}

bool Engine_LoadPCLevel(const std::string& name)
{
    std::unique_ptr<loader::Level> loader = loader::Level::createLoader(name, loader::Game::Unknown);
    if(!loader)
        return false;

    loader->load();

    TR_GenWorld(&engine_world, loader);

    std::string buf = Engine_GetLevelName(name);

    ConsoleInfo::instance().notify(SYSNOTE_LOADED_PC_LEVEL);
    ConsoleInfo::instance().notify(SYSNOTE_ENGINE_VERSION, static_cast<int>(loader->m_gameVersion), buf.c_str());
    ConsoleInfo::instance().notify(SYSNOTE_NUM_ROOMS, engine_world.rooms.size());

    return true;
}

int Engine_LoadMap(const std::string& name)
{
    if(!Engine_FileFound(name))
    {
        ConsoleInfo::instance().warning(SYSWARN_FILE_NOT_FOUND, name.c_str());
        return 0;
    }

    Gui_DrawLoadScreen(0);

    renderer.hideSkyBox();
    renderer.resetWorld();

    gameflow_manager.CurrentLevelPath = name;          // it is needed for "not in the game" levels or correct saves loading.

    Gui_DrawLoadScreen(50);

    engine_world.empty();
    engine_world.prepare();

    engine_lua.clean();

    Audio_Init();

    Gui_DrawLoadScreen(100);

    // Here we can place different platform-specific level loading routines.

    switch(Engine_GetLevelFormat(name))
    {
        case LEVEL_FORMAT_PC:
            if(Engine_LoadPCLevel(name) == false) return 0;
            break;

        case LEVEL_FORMAT_PSX:
            break;

        case LEVEL_FORMAT_DC:
            break;

        case LEVEL_FORMAT_OPENTOMB:
            break;

        default:
            break;
    }

    engine_world.id = 0;
    engine_world.name = nullptr;
    engine_world.type = 0;

    Game_Prepare();

    engine_lua.prepare();

    renderer.setWorld(&engine_world);

    Gui_DrawLoadScreen(1000);

    Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_IN);
    Gui_NotifierStop();

    return 1;
}

int Engine_ExecCmd(const char *ch)
{
    std::vector<char> token(ConsoleInfo::instance().lineSize());
    RoomSector* sect;
    FILE *f;

    while(ch != nullptr)
    {
        const char *pch = ch;

        ch = script::MainEngine::parse_token(ch, token.data());
        if(!strcmp(token.data(), "help"))
        {
            for(size_t i = SYSNOTE_COMMAND_HELP1; i <= SYSNOTE_COMMAND_HELP15; i++)
            {
                ConsoleInfo::instance().notify(i);
            }
        }
        else if(!strcmp(token.data(), "goto"))
        {
            control_states.free_look = true;
            const auto x = script::MainEngine::parseFloat(&ch);
            const auto y = script::MainEngine::parseFloat(&ch);
            const auto z = script::MainEngine::parseFloat(&ch);
            renderer.camera()->setPosition({ x, y, z });
            return 1;
        }
        else if(!strcmp(token.data(), "save"))
        {
            ch = script::MainEngine::parse_token(ch, token.data());
            if(NULL != ch)
            {
                Game_Save(token.data());
            }
            return 1;
        }
        else if(!strcmp(token.data(), "load"))
        {
            ch = script::MainEngine::parse_token(ch, token.data());
            if(NULL != ch)
            {
                Game_Load(token.data());
            }
            return 1;
        }
        else if(!strcmp(token.data(), "exit"))
        {
            Engine_Shutdown(0);
            return 1;
        }
        else if(!strcmp(token.data(), "cls"))
        {
            ConsoleInfo::instance().clean();
            return 1;
        }
        else if(!strcmp(token.data(), "spacing"))
        {
            ch = script::MainEngine::parse_token(ch, token.data());
            if(NULL == ch)
            {
                ConsoleInfo::instance().notify(SYSNOTE_CONSOLE_SPACING, ConsoleInfo::instance().spacing());
                return 1;
            }
            ConsoleInfo::instance().setLineInterval(atof(token.data()));
            return 1;
        }
        else if(!strcmp(token.data(), "showing_lines"))
        {
            ch = script::MainEngine::parse_token(ch, token.data());
            if(NULL == ch)
            {
                ConsoleInfo::instance().notify(SYSNOTE_CONSOLE_LINECOUNT, ConsoleInfo::instance().visibleLines());
                return 1;
            }
            else
            {
                const auto val = atoi(token.data());
                if((val >=2 ) && (val <= screen_info.h/ConsoleInfo::instance().lineHeight()))
                {
                    ConsoleInfo::instance().setVisibleLines(val);
                    ConsoleInfo::instance().setCursorY(screen_info.h - ConsoleInfo::instance().lineHeight() * ConsoleInfo::instance().visibleLines());
                }
                else
                {
                    ConsoleInfo::instance().warning(SYSWARN_INVALID_LINECOUNT);
                }
            }
            return 1;
        }
        else if(!strcmp(token.data(), "r_wireframe"))
        {
            renderer.toggleWireframe();
            return 1;
        }
        else if(!strcmp(token.data(), "r_points"))
        {
            renderer.toggleDrawPoints();
            return 1;
        }
        else if(!strcmp(token.data(), "r_coll"))
        {
            renderer.toggleDrawColl();
            return 1;
        }
        else if(!strcmp(token.data(), "r_normals"))
        {
            renderer.toggleDrawNormals();
            return 1;
        }
        else if(!strcmp(token.data(), "r_portals"))
        {
            renderer.toggleDrawPortals();
            return 1;
        }
        else if(!strcmp(token.data(), "r_frustums"))
        {
            renderer.toggleDrawFrustums();
            return 1;
        }
        else if(!strcmp(token.data(), "r_room_boxes"))
        {
            renderer.toggleDrawRoomBoxes();
            return 1;
        }
        else if(!strcmp(token.data(), "r_boxes"))
        {
            renderer.toggleDrawBoxes();
            return 1;
        }
        else if(!strcmp(token.data(), "r_axis"))
        {
            renderer.toggleDrawAxis();
            return 1;
        }
        else if(!strcmp(token.data(), "r_nullmeshes"))
        {
            renderer.toggleDrawNullMeshes();
            return 1;
        }
        else if(!strcmp(token.data(), "r_dummy_statics"))
        {
            renderer.toggleDrawDummyStatics();
            return 1;
        }
        else if(!strcmp(token.data(), "r_skip_room"))
        {
            renderer.toggleSkipRoom();
            return 1;
        }
        else if(!strcmp(token.data(), "room_info"))
        {
            if(Room* r = renderer.camera()->m_currentRoom)
            {
                sect = r->getSectorXYZ(renderer.camera()->getPosition());
                ConsoleInfo::instance().printf("ID = %d, x_sect = %d, y_sect = %d", r->id, r->sectors_x, r->sectors_y);
                if(sect)
                {
                    ConsoleInfo::instance().printf("sect(%d, %d), inpenitrable = %d, r_up = %d, r_down = %d", sect->index_x, sect->index_y,
                                                   static_cast<int>(sect->ceiling == TR_METERING_WALLHEIGHT || sect->floor == TR_METERING_WALLHEIGHT), static_cast<int>(sect->sector_above != nullptr), static_cast<int>(sect->sector_below != nullptr));
                    for(uint32_t i = 0; i < sect->owner_room->static_mesh.size(); i++)
                    {
                        ConsoleInfo::instance().printf("static[%d].object_id = %d", i, sect->owner_room->static_mesh[i]->object_id);
                    }
                    for(const std::shared_ptr<EngineContainer>& cont : sect->owner_room->containers)
                    {
                        if(cont->object_type == OBJECT_ENTITY)
                        {
                            Entity* e = static_cast<Entity*>(cont->object);
                            ConsoleInfo::instance().printf("cont[entity](%d, %d, %d).object_id = %d", static_cast<int>(e->m_transform.getOrigin()[0]), static_cast<int>(e->m_transform.getOrigin()[1]), static_cast<int>(e->m_transform.getOrigin()[2]), e->id());
                        }
                    }
                }
            }
            return 1;
        }
        else if(!strcmp(token.data(), "xxx"))
        {
            f = fopen("ascII.txt", "r");
            if(f)
            {
                long int size;
                char *buf;
                fseek(f, 0, SEEK_END);
                size = ftell(f);
                buf = static_cast<char*>(malloc((size + 1)*sizeof(char)));

                fseek(f, 0, SEEK_SET);
                fread(buf, size, sizeof(char), f);
                buf[size] = 0;
                fclose(f);
                ConsoleInfo::instance().clean();
                ConsoleInfo::instance().addText(buf, FONTSTYLE_CONSOLE_INFO);
                free(buf);
            }
            else
            {
                ConsoleInfo::instance().addText("Not avaliable =(", FONTSTYLE_CONSOLE_WARNING);
            }
            return 1;
        }
        else if(token[0])
        {
            ConsoleInfo::instance().addLine(pch, FONTSTYLE_CONSOLE_EVENT);
            try
            {
                engine_lua.doString(pch);
            }
            catch(lua::RuntimeError& error)
            {
                ConsoleInfo::instance().addLine(error.what(), FONTSTYLE_CONSOLE_WARNING);
            }
            catch(lua::LoadError& error)
            {
                ConsoleInfo::instance().addLine(error.what(), FONTSTYLE_CONSOLE_WARNING);
            }
            return 0;
        }
    }

    return 0;
}

void Engine_InitConfig(const char *filename)
{
    Engine_InitDefaultGlobals();

    if((filename != nullptr) && Engine_FileFound(filename))
    {
        script::ScriptEngine state;
        state.registerC("bind", &script::MainEngine::bindKey);                             // get and set key bindings
        try
        {
            state.doFile(filename);
        }
        catch(lua::RuntimeError& error)
        {
            Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
            return;
        }
        catch(lua::LoadError& error)
        {
            Sys_DebugLog(LUA_LOG_FILENAME, "%s", error.what());
            return;
        }

        state.parseScreen(&screen_info);
        state.parseRender(&renderer.settings());
        state.parseAudio(&audio_settings);
        state.parseConsole(&ConsoleInfo::instance());
        state.parseControls(&control_mapper);
        state.parseSystem(&system_settings);
    }
    else
    {
        Sys_Warn("Could not find \"%s\"", filename);
    }
}

int engine_lua_fputs(const char *str, FILE* /*f*/)
{
    ConsoleInfo::instance().addText(str, FONTSTYLE_CONSOLE_NOTIFY);
    return static_cast<int>(strlen(str));
}

int engine_lua_fprintf(FILE *f, const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];
    int ret;

    // Create string
    va_start(argptr, fmt);
    ret = vsnprintf(buf, 4096, fmt, argptr);
    va_end(argptr);

    // Write it to target file
    fwrite(buf, 1, ret, f);

    // Write it to console, too (if it helps) und
    ConsoleInfo::instance().addText(buf, FONTSTYLE_CONSOLE_NOTIFY);

    return ret;
}

int engine_lua_printf(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];
    int ret;

    va_start(argptr, fmt);
    ret = vsnprintf(buf, 4096, fmt, argptr);
    va_end(argptr);

    ConsoleInfo::instance().addText(buf, FONTSTYLE_CONSOLE_NOTIFY);

    return ret;
}
