#include <time.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_audio.h>
#if !defined(__MACOSX__)
#include <SDL2/SDL_image.h>
#endif
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_haptic.h>

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/lstate.h"
}

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

extern "C" {
#include "al/AL/al.h"
#include "al/AL/alc.h"
#include "al/AL/alext.h"
}

#define SKELETAL_TEST   0
#define NO_AUDIO        0

SDL_Window             *sdl_window     = NULL;
SDL_Joystick           *sdl_joystick   = NULL;
SDL_GameController     *sdl_controller = NULL;
SDL_Haptic             *sdl_haptic     = NULL;
SDL_GLContext           sdl_gl_context = 0;
ALCdevice              *al_device      = NULL;
ALCcontext             *al_context     = NULL;

static int done = 0;
GLfloat light_position[] = {255.0, 255.0, 8.0, 0.0};
GLfloat cast_ray[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

static int model =  0;
static int mesh =   0;
static int paused = 0;
static int frame =  0;
static int anim =   0;
static int sprite = 0;

static btScalar time_scale = 1.0;

engine_container_p      last_cont = NULL;

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

void SkeletalModelTestDraw()
{
    bone_frame_p bframe;
    static btScalar time = 0.0;
    skeletal_model_p smodel;
    animation_frame_p af;
    anim_dispatch_p adsp;
    sprite_p bsprite;
    bone_tag_p btag;
    GLfloat tr[16];
    mesh_tree_tag_p mt;
    int i, j, y, stack;

    if((engine_world.skeletal_models == NULL) || (engine_world.meshes == NULL))
    {
        return;
    }

    if(frame < 0)
    {
        frame = 0;
    }

    if((uint32_t)model > engine_world.skeletal_model_count)
    {
        model = 0;
    }
    smodel = engine_world.skeletal_models + model;

    if((uint16_t)anim + 1 > smodel->animation_count)
    {
        anim = 0;
    }
    af = smodel->animations + anim;

    if(!paused)
    {
        time += engine_frame_time;
        if(time < 0.0)
        {
            time = 0.0;
        }
        if(time > 0.05)
        {
            frame ++;
            time = 0.0;
        }
    }

    sprite %= engine_world.sprites_count;
    bsprite = engine_world.sprites + sprite;

    frame %= smodel->animations[anim].frames_count;
    bframe = smodel->animations[anim].frames + frame;

    Gui_OutTextXY(screen_info.w-632, 120, "sprite ID = %d;  mesh ID = %d", bsprite->id, mesh);
    Gui_OutTextXY(screen_info.w-632, 96, "model ID = %d, anim = %d of %d, rate = %d, frame = %d of %d", smodel->id, anim, smodel->animation_count, smodel->animations[anim].original_frame_rate, frame, smodel->animations[anim].frames_count);
    Gui_OutTextXY(screen_info.w-632, 72, "next anim = %d, next frame = %d, num_state_changes = %d", (af->next_anim)?(af->next_anim->id):-1, af->next_frame, af->state_change_count);
    Gui_OutTextXY(screen_info.w-632, 48, "vx = %f, vy = %f, ax = %f, ay = %f", af->speed_x, af->speed_y, af->accel_x, af->accel_y);
    Gui_OutTextXY(screen_info.w-632, 24, "bb_min(%d, %d, %d), bb_max(%d, %d, %d)", (int)bframe->bb_min[0], (int)bframe->bb_min[1], (int)bframe->bb_min[2], (int)bframe->bb_max[0], (int)bframe->bb_max[1], (int)bframe->bb_max[2]);
    Gui_OutTextXY(screen_info.w-632, 4, "x0 = %d, y0 = %d, z0 = %d", (int)bframe->pos[0], (int)bframe->pos[1], (int)bframe->pos[2]);

    y = screen_info.h - 24;
    for(i=0;i<af->state_change_count;i++)
    {
        for(j=0;j<af->state_change[i].anim_dispatch_count;j++)
        {
            adsp = af->state_change[i].anim_dispatch + j;
            Gui_OutTextXY(8, y, "[%d, %d], id = %d next anim = %d, next frame = %d, interval = [%d, %d]",
                          i, j, af->state_change[i].id, adsp->next_anim, adsp->next_frame, adsp->frame_low, adsp->frame_high);
            y -= 24;
        }
    }

    /*
     * RENDER MODEL
     */
    glPushMatrix();
    btag = bframe->bone_tags;
    mt = smodel->mesh_tree;

    tr[15] = 1.0;
    vec3_add(tr+12, mt->offset, bframe->pos);
    Mat4_set_qrotation(tr, btag->qrotate);
    //glEnable(GL_TEXTURE_2D);
    glMultMatrixf(tr);
    Render_Mesh(mt->mesh_base, NULL, NULL);
    btag++;
    mt++;
    for(stack=0,i=1;i<bframe->bone_tag_count;i++,btag++,mt++)
    {
        tr[15] = 1.0;
        vec3_copy(tr+12, mt->offset);
        Mat4_set_qrotation(tr, btag->qrotate);
        if(mt->flag & 0x01)
        {
            if(stack > 0)                                                       // PARANOID GL STACK CHECK
            {
                glPopMatrix();
                stack--;
            }
        }
        if(mt->flag & 0x02)
        {
            glPushMatrix();
            stack++;
        }
        glMultMatrixf(tr);
        Render_Mesh(mt->mesh_base, NULL, NULL);
        //Render_BBox(tree_tag->mesh->bb_min, tree_tag->mesh->bb_max);
        if(mt->mesh_skin)
        {
            Render_SkinMesh(mt->mesh_skin, tr);
        }
    }

    for(i=0;i<stack;i++)                                                        // PARANOID: GL STACK CHECK AND CORRECTION
    {
        glPopMatrix();
    }
    stack = 0;
    glPopMatrix();

//    glPushAttrib(GL_ENABLE_BIT);
//    glEnable(GL_ALPHA_TEST);
//    glDisable(GL_CULL_FACE);
//    Render_Sprite(bsprite, 1024.0, 0.0, 0.0);
//    glPopAttrib();

    glPushMatrix();
    btScalar matrix[16];
    Mat4_E_macro(matrix);
    Mat4_Translate(matrix, -1024.0, 0.0, 0.0);
    Render_Mesh(engine_world.meshes + mesh, NULL, NULL);
    glPopMatrix();
}

void Engine_InitGL()
{
    InitGLExtFuncs();
#if SKELETAL_TEST
    glClearColor(0.3, 0.3, 0.3, 1.0);
#else
    glClearColor(0.0, 0.0, 0.0, 1.0);
#endif
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    if(renderer.settings.antialias)
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }

    // Default state: Vertex array and color array are enabled, all others disabled.. Drawable
    // items can rely on Vertex array to be enabled (but pointer can be
    // anything). They have to enable other arrays based on their need and then
    // return to default state
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Default state for Alpha func: >= 0.5. That's what all users of alpha
    // function use anyway.
    glAlphaFunc(GL_GEQUAL, 0.5);
}

void Engine_InitSDLControls()
{
    int    NumJoysticks;
    Uint32 init_flags    = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS;                    // These flags are used in any case.

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
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
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
    if(renderer.settings.antialias)
    {
        /* I do not know why, but settings of this temporary window (zero position / size) are applied to the main window, ignoring screen settings */
        sdl_window     = SDL_CreateWindow(NULL, screen_info.x, screen_info.y, screen_info.w, screen_info.h, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        sdl_gl_context = SDL_GL_CreateContext(sdl_window);
        SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);
        GLint maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        if ((maxSamples == 0) || (renderer.settings.antialias_samples > maxSamples))
        {
            renderer.settings.antialias_samples = maxSamples;   // Limit to max.
            Sys_DebugLog(LOG_FILENAME, "InitSDLVideo: wrong AA sample number, using %d", maxSamples);
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

    // set the opengl context version
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    sdl_window = SDL_CreateWindow("OpenTomb", screen_info.x, screen_info.y, screen_info.w, screen_info.h, video_flags);
    sdl_gl_context = SDL_GL_CreateContext(sdl_window);
    SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);
    SDL_SetWindowBrightness(sdl_window, renderer.settings.brightness);

    Con_AddLine((const char*)glGetString(GL_VENDOR), FONTSTYLE_CONSOLE_INFO);
    Con_AddLine((const char*)glGetString(GL_RENDERER), FONTSTYLE_CONSOLE_INFO);
    char buf[con_base.line_size];
    snprintf(buf, con_base.line_size, "OpenGL version %s", glGetString(GL_VERSION));
    Con_AddLine((const char*)buf, FONTSTYLE_CONSOLE_INFO);
    Con_AddLine((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION), FONTSTYLE_CONSOLE_INFO);
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

int main(int argc, char **argv)
{
    btScalar time, newtime;
    static btScalar oldtime = 0.0;

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
    Render_DoShaders();

    // Secondary (deferred) initialization.
    Engine_Init_Post();

    // Initial window resize.
    Engine_Resize(screen_info.w, screen_info.h, screen_info.w, screen_info.h);

    // OpenAL initialization.
    Engine_InitAL();

    // Clearing up memory for initial level loading.
    World_Prepare(&engine_world);

    // Setting up mouse.
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_WarpMouseInWindow(sdl_window, screen_info.w/2, screen_info.h/2);
    SDL_ShowCursor(0);

    // Make splash screen.
    Gui_FadeAssignPic(FADER_LOADSCREEN, "resource/graphics/legal.png");
    Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_OUT);

    luaL_dofile(engine_lua, "autoexec.lua");

#if SKELETAL_TEST
    control_states.free_look = 1;
#endif

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


void Engine_Display()
{
    if(!done)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//| GL_ACCUM_BUFFER_BIT);

        Cam_RecalcClipPlanes(&engine_camera);
        Cam_Apply(&engine_camera);

        // GL_VERTEX_ARRAY | GL_COLOR_ARRAY
        if(screen_info.show_debuginfo)
        {
            ShowDebugInfo();
        }

        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT); ///@PUSH <- GL_VERTEX_ARRAY | GL_COLOR_ARRAY
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glFrontFace(GL_CW);

        Render_GenWorldList();
        Render_DrawList();
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

        //glDisable(GL_CULL_FACE);
        //Render_DrawAxis(10000.0);
        /*if(engine_world.Character)
        {
            glPushMatrix();
            glTranslatef(engine_world.Character->transform[12], engine_world.Character->transform[13], engine_world.Character->transform[14]);
            Render_DrawAxis(1000.0);
            glPopMatrix();
        }*/

        Gui_SwitchGLMode(1);
        {
            glEnable(GL_ALPHA_TEST);

            Gui_DrawNotifier();
            if(engine_world.Character && engine_world.Character->character && main_inventory_manager)
            {
                Gui_DrawInventory();
            }
        }
        glPopClientAttrib();        ///@POP -> GL_VERTEX_ARRAY | GL_COLOR_ARRAY

        Gui_Render();
        Gui_SwitchGLMode(0);

        Render_DrawList_DebugLines();

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

    Cam_SetFovAspect(&engine_camera, screen_info.fov, (btScalar)nominalW/(btScalar)nominalH);
    Cam_RecalcClipPlanes(&engine_camera);

    glViewport(0, 0, pixelsW, pixelsH);
}


/*
 * MOUSE FUNCTIONS
 */
void Engine_PrimaryMouseDown()
{
    engine_container_p cont = Container_Create();
    btScalar dbgR = 128.0;
    btScalar *v = engine_camera.pos;
    btScalar *dir = engine_camera.view_dir;
    btScalar new_pos[3];
    btVector3 localInertia(0, 0, 0);
    btTransform startTransform;
    btCollisionShape *cshape;
    btRigidBody *body;

    cshape = new btSphereShape(dbgR);
    //cshape = new btCapsuleShapeZ(50.0, 100.0);
    startTransform.setIdentity();
    new_pos[0] = v[0];
    new_pos[1] = v[1];
    new_pos[2] = v[2];
    startTransform.setOrigin(btVector3(new_pos[0], new_pos[1], new_pos[2]));
    cshape->calculateLocalInertia(12.0, localInertia);
    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    body = new btRigidBody(12.0, motionState, cshape, localInertia);
    bt_engine_dynamicsWorld->addRigidBody(body);
    body->setLinearVelocity(btVector3(dir[0], dir[1], dir[2]) * 6000);
    cont->room = Room_FindPosCogerrence(new_pos, engine_camera.current_room);
    cont->object_type = OBJECT_BULLET_MISC;                     // bullet have to destroy this user pointer
    body->setUserPointer(cont);
    body->setCcdMotionThreshold(dbgR);                          // disable tunneling effect
    body->setCcdSweptSphereRadius(dbgR);
}


void Engine_SecondaryMouseDown()
{
    engine_container_t *c0;
    btVector3 from, to, place;
    engine_container_t cam_cont;

    vec3_copy(from.m_floats, engine_camera.pos);
    to = from + btVector3(engine_camera.view_dir[0], engine_camera.view_dir[1], engine_camera.view_dir[2]) * 32768.0;

    cam_cont.next = NULL;
    cam_cont.object = NULL;
    cam_cont.object_type = 0;
    cam_cont.room = engine_camera.current_room;

    bt_engine_ClosestRayResultCallback cbc(&cam_cont);
    //cbc.m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    bt_engine_dynamicsWorld->rayTest(from, to, cbc);
    if(cbc.hasHit())
    {
        place.setInterpolate3(from, to, cbc.m_closestHitFraction);
        vec3_copy(cast_ray, place.m_floats);
        cast_ray[3] = cast_ray[0] + 100.0 * cbc.m_hitNormalWorld.m_floats[0];
        cast_ray[4] = cast_ray[1] + 100.0 * cbc.m_hitNormalWorld.m_floats[1];
        cast_ray[5] = cast_ray[2] + 100.0 * cbc.m_hitNormalWorld.m_floats[2];

        if((c0 = (engine_container_p)cbc.m_collisionObject->getUserPointer()))
        {
            if(c0->object_type == OBJECT_BULLET_MISC)
            {
                btCollisionObject* obj = (btCollisionObject*)cbc.m_collisionObject;
                btRigidBody* body = btRigidBody::upcast(obj);
                if(body && body->getMotionState())
                {
                    delete body->getMotionState();
                }
                if(body && body->getCollisionShape())
                {
                    delete body->getCollisionShape();
                }

                body->setUserPointer(NULL);
                c0->room = NULL;
                free(c0);

                bt_engine_dynamicsWorld->removeCollisionObject(obj);
                delete obj;
            }
            else
            {
                last_cont = c0;
            }
        }
    }
}


void Engine_Frame(btScalar time)
{
    static int  cycles = 0;
    static btScalar time_cycl = 0.0;
    extern gui_text_line_t system_fps;
    if(time > 0.1)
    {
        time = 0.1;
    }

    ResetTempbtScalar();
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
        //Gui_StringAutoRect(&system_fps);
        cycles = 0;
        time_cycl = 0.0;
    }

    Engine_PollSDLInput();

#if SKELETAL_TEST
    Game_ApplyControls(NULL);
#else
    Game_Frame(time);
    Gameflow_Do();
#endif

    Engine_Display();
}


void ShowDebugInfo()
{
    entity_p ent;
    btTransform trans;
    GLfloat color_array[] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};

    vec3_copy(light_position, engine_camera.pos);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(2.0);
    glVertexPointer(3, GL_FLOAT, 0, cast_ray);
    glColorPointer(3, GL_FLOAT, 0, color_array);
    glDrawArrays(GL_LINES, 0, 2);

#if !SKELETAL_TEST

    /*//color3f(0.0, 0.0, 0.0);
    for(int j=bt_engine_dynamicsWorld->getNumCollisionObjects()-1; j>=0 ;j--)
    {
        btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            engine_container_p cont = ((engine_container_p)body->getUserPointer());
            if(!body->isStaticObject() && (!cont || (cont->object_type == OBJECT_BULLET_MISC && (!cont->room || cont->room->is_in_r_list))))
            {
                body->getMotionState()->getWorldTransform(trans);
                glPushMatrix();
                trans.getOpenGLMatrix(tr);
                glMultMatrixf(tr);
                //color3f(1.0, 1.0, 1.0);
                //Render_DrawAxis();
                glPopMatrix();
            }
        }
    }*/

#endif

#if !SKELETAL_TEST

    ent = engine_world.Character;
    if(ent && ent->character)
    {
        /*height_info_p fc = &ent->character->height_info
        txt = Gui_OutTextXY(20.0 / screen_info.w, 80.0 / screen_info.w, "Z_min = %d, Z_max = %d, W = %d", (int)fc->floor_point.m_floats[2], (int)fc->ceiling_point.m_floats[2], (int)fc->water_level);
        */

        Gui_OutTextXY(30.0, 30.0, "last_anim = %03d, curr_anim = %03d, next_anim = %03d, last_st = %03d, next_st = %03d", ent->bf.animations.last_animation, ent->bf.animations.current_animation, ent->bf.animations.next_animation, ent->bf.animations.last_state, ent->bf.animations.next_state);
        //Gui_OutTextXY(30.0, 30.0, "curr_anim = %03d, next_anim = %03d, curr_frame = %03d, next_frame = %03d", ent->bf.animations.current_animation, ent->bf.animations.next_animation, ent->bf.animations.current_frame, ent->bf.animations.next_frame);
        //Gui_OutTextXY(NULL, 20, 8, "posX = %f, posY = %f, posZ = %f", engine_world.Character->transform[12], engine_world.Character->transform[13], engine_world.Character->transform[14]);
    }

    if(last_cont != NULL)
    {
        switch(last_cont->object_type)
        {
            case OBJECT_ENTITY:
                Gui_OutTextXY(30.0, 60.0, "cont_entity: id = %d, model = %d", ((entity_p)last_cont->object)->id, ((entity_p)last_cont->object)->bf.animations.model->id);
                break;

            case OBJECT_STATIC_MESH:
                Gui_OutTextXY(30.0, 60.0, "cont_static: id = %d", ((static_mesh_p)last_cont->object)->object_id);
                break;

            case OBJECT_ROOM_BASE:
                Gui_OutTextXY(30.0, 60.0, "cont_room: id = %d", ((room_p)last_cont->object)->id);
                break;
        }

    }

    if(engine_camera.current_room != NULL)
    {
        room_sector_p rs = Room_GetSectorRaw(engine_camera.current_room, engine_camera.pos);
        if(rs != NULL)
        {
            Gui_OutTextXY(30.0, 90.0, "room = (id = %d, sx = %d, sy = %d)", engine_camera.current_room->id, rs->index_x, rs->index_y);
            Gui_OutTextXY(30.0, 120.0, "room_below = %d, room_above = %d", (rs->sector_below != NULL)?(rs->sector_below->owner_room->id):(-1), (rs->sector_above != NULL)?(rs->sector_above->owner_room->id):(-1));
        }
    }
    Gui_OutTextXY(30.0, 150.0, "cam_pos = (%.1f, %.1f, %.1f)", engine_camera.pos[0], engine_camera.pos[1], engine_camera.pos[2]);
#endif
}

void Engine_PollSDLInput()
{
    SDL_Event   event;
    static int mouse_setup = 0;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
                if(!con_base.show && control_states.mouse_look != 0 &&
                    ((event.motion.x != (screen_info.w/2)) ||
                     (event.motion.y != (screen_info.h/2))))
                {
                    if(mouse_setup)                                             // it is not perfect way, but cursor
                    {                                                           // every engine start is in one place
                        control_states.look_axis_x = event.motion.xrel * control_mapper.mouse_sensitivity * 0.01;
                        control_states.look_axis_y = event.motion.yrel * control_mapper.mouse_sensitivity * 0.01;
                    }

                    if((event.motion.x < ((screen_info.w/2)-(screen_info.w/4))) ||
                       (event.motion.x > ((screen_info.w/2)+(screen_info.w/4))) ||
                       (event.motion.y < ((screen_info.h/2)-(screen_info.h/4))) ||
                       (event.motion.y > ((screen_info.h/2)+(screen_info.h/4))))
                    {
                        SDL_WarpMouseInWindow(sdl_window, screen_info.w/2, screen_info.h/2);
                    }
                }
                mouse_setup = 1;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == 1) //LM = 1, MM = 2, RM = 3
                {
                    Engine_PrimaryMouseDown();
                }
                else if(event.button.button == 3)
                {
                    Engine_SecondaryMouseDown();
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
                    Controls_JoyAxis(event.jaxis.axis, event.jaxis.value);
                break;

            case SDL_JOYHATMOTION:
                if(sdl_joystick)
                    Controls_JoyHat(event.jhat.value);
                break;

            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                // NOTE: Joystick button numbers are passed with added JOY_BUTTON_MASK (1000).
                if(sdl_joystick)
                    Controls_Key((event.jbutton.button + JOY_BUTTON_MASK), event.jbutton.state);
                break;

            case SDL_TEXTINPUT:
            case SDL_TEXTEDITING:
                if(con_base.show && event.key.state)
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
                    done = 1;
                    break;
                }

                if(con_base.show && event.key.state)
                {
                    switch (event.key.keysym.sym)
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
                    DebugKeys(event.key.keysym.sym, event.key.state);
                }
                break;

            case SDL_QUIT:
                done = 1;
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
}

void DebugKeys(int button, int state)
{
    if(state)
    {
        switch(button)
        {
            case SDLK_RETURN:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_ACTIVATE);
                }
                break;

            case SDLK_g:
                if(time_scale == 1.0)
                {
                    time_scale = 0.033;
                }
                else
                {
                    time_scale = 1.0;
                }
                break;

            case SDLK_UP:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_UP);
                }
                break;

            case SDLK_DOWN:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_DOWN);
                }
                break;

            case SDLK_LEFT:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_R_LEFT);
                }
                break;

            case SDLK_RIGHT:
                if(main_inventory_manager)
                {
                    main_inventory_manager->send(gui_InventoryManager::INVENTORY_R_RIGHT);
                }
                break;

                /*models switching*/
            case SDLK_p:
                model++;
                if((uint32_t)model + 1 > engine_world.skeletal_model_count)
                {
                    model = 0;
                }
                frame = 0;
                anim = 0;
                break;

            case SDLK_o:
                model--;
                if(model < 0)
                {
                    model = engine_world.skeletal_model_count-1;
                }
                frame = 0;
                anim = 0;
                break;

            case SDLK_1:
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->current_weapon = 1;
                }
                break;

            case SDLK_2:
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->current_weapon = 2;
                }
                break;

            case SDLK_3:
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->current_weapon = 3;
                }
                break;

            case SDLK_4:
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->current_weapon = 4;
                }
                break;

            case SDLK_5:
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->current_weapon = 5;
                }
                break;

            case SDLK_6:
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->current_weapon = 6;
                }
                break;

            case SDLK_7:
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->current_weapon = 7;
                }
                break;

            case SDLK_8:
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->current_weapon = 8;
                }
                break;

            case SDLK_9:
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->current_weapon = 9;
                }
                break;

                /*rumble*/
            case SDLK_f:
                //Audio_Send(105);
                //Gui_FadeStart(FADER_EFFECT, GUI_FADER_DIR_TIMED);
                break;

                /*full health*/
            case SDLK_h:
                if(Character_ChangeParam(engine_world.Character, PARAM_HEALTH, LARA_PARAM_HEALTH_MAX))
                    Audio_Send(TR_AUDIO_SOUND_MEDIPACK);
                break;

                /*animations switching*/

            case SDLK_u:
                anim--;
                if(anim < 0)
                {
                    anim = engine_world.skeletal_models[model].animation_count-1;
                }
                break;

            case SDLK_i:
                if(engine_world.skeletal_model_count > 0)
                {
                    anim++;
                    if(anim > engine_world.skeletal_models[model].animation_count-1)
                    {
                        anim = 0;
                    }
                }
                break;

            case SDLK_t:
                mesh--;
                if(mesh < 0)
                {
                    mesh = engine_world.meshes_count-1;
                }
                break;

            case SDLK_y:
                screen_info.show_debuginfo = !screen_info.show_debuginfo;
                mesh++;
                if((uint32_t)mesh + 1 > engine_world.meshes_count)
                {
                    mesh = 0;
                }
                break;

            case SDLK_z:
                paused = !paused;
                if(engine_world.Character != NULL)
                {
                    engine_world.Character->character->resp.kill = 0;

                    if(engine_world.Character->move_type == MOVE_UNDERWATER)
                    {
                        Entity_SetAnimation(engine_world.Character, 103, 0, 0);
                        Character_SetParam(engine_world.Character, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
                        engine_world.Character->move_type = MOVE_ON_FLOOR;
                    }
                    else
                    {
                        Entity_SetAnimation(engine_world.Character, 108, 0, 0);
                        Character_SetParam(engine_world.Character, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
                        engine_world.Character->move_type = MOVE_UNDERWATER;
                    }
                    engine_world.Character->bf.animations.anim_flags = ANIM_NORMAL_CONTROL;
                }
                break;

            case SDLK_v:
                frame++;
                break;

            case SDLK_b:
                frame--;
                break;

            case SDLK_n:
                control_states.noclip = !control_states.noclip;
                sprite--;
                if(sprite < 0)
                {
                    sprite = engine_world.sprites_count - 1;
                }
                break;

            case SDLK_m:
                sprite++;
                if((uint32_t)sprite >= engine_world.sprites_count)
                {
                    sprite = 0;
                }
                break;

                /*
                 * alternate rooms testing: be carefull, something is wrong and engine may crash!
                 */
            case SDLK_r:
                if(!con_base.show)
                {
                    for(uint32_t i=0;i<engine_world.room_count;i++)
                    {
                        //if(engine_world.rooms[i].alternate_room != NULL)
                        {
                            Room_SwapToAlternate(engine_world.rooms + i);
                        }
                    }
                }
                break;

            case SDLK_e:
                if(!con_base.show)
                {
                    for(uint32_t i=0;i<engine_world.room_count;i++)
                    {
                        //if(engine_world.rooms[i].base_room != NULL)
                        {
                            Room_SwapToBase(engine_world.rooms + i);
                        }
                    }
                }
                break;

            default:
                //Con_Printf("key = %d", button);
                break;
        };
    }
}
