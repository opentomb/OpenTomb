#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_events.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"

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

#include "vt/vt_level.h"
#include "bounding_volume.h"
#include "anim_state_control.h"
#include "character_controller.h"

#define SKELETAL_TEST 0

SDL_Window             *displayWindow;
SDL_GLContext           openglContext;
SDL_Joystick           *joystick;

static int done = 0;
GLfloat light_position[] = {255.0, 255.0, 8.0, 0.0};
GLfloat cast_ray[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

GLfloat hang_offset_point[3] = {0.0, 0.0, 0.0};

static int model = 0;
static int mesh = 0;
static int paused = 0;
static int frame = 0;
static int anim = 0;
static int sprite = 0;

static GLUquadricObj   *dbgSphere;
static GLUquadricObj   *dbgCyl;
static btScalar         dbgR = 128.0;
static entity_p         last_rmb = NULL;
//#error "ADD ft2build.h to the repo!"
// BULLET IS PERFECT PHYSICS LIBRARY!!!
/* 
 * 1) console
 *      - add notify functions
 * 2) LUA enngine global script:
 *      - add base functions for entity manipulation, I.E.: (get / set) - pos, 
 *        angles, anim, frame, frame time, speed, health, collision. new, delete.
 * 6) Menu
 *      - settings
 *      - map loading list
 *      - saves loading list
 * 10) OpenGL
 *      - shaders
 *      - reflections
 *      - shadows
 *      - particles
 *      - GL and renderer optimisations
 *      - animated textures
 * 40) Physics / gameplay
 *      - optimize and fix character controller
 *      - health limit
 *      - weapons
 * 41) scripts module
 *      - original TR scripts parser
 *      - cutscenes playing
 *      - triggers
 *      - enemies AI
 *      - end level -> next level 
 * 42) sound
 *      - add OpenAL lib
 *      - loading TR's sounds format
 */

void TempDrawFrame();
void ShowDebugInfo();


void Draw_CapsuleZ(btCapsuleShapeZ *cshape, btTransform *trans);

void Draw_CapsuleZ(btCapsuleShapeZ *cshape, btTransform *trans)
{
    btScalar tr[16];
    btScalar r = cshape->getRadius();
    btScalar h = cshape->getHalfHeight();
    
    gluQuadricDrawStyle(dbgSphere, GLU_LINE);
    gluQuadricDrawStyle(dbgCyl, GLU_LINE);
    glColor3f(1.0, 1.0, 1.0);
    trans->getOpenGLMatrix(tr);
    
    glPushMatrix();
    glTranslatef(0.0, 0.0, -h);
    glMultMatrixf(tr);                
    gluSphere(dbgSphere, r, 12, 12);
    gluCylinder(dbgCyl, r, r, 2.0 * h, 12, 12);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0, 0.0, h);
    glMultMatrixf(tr);                
    gluSphere(dbgSphere, r, 12, 12);
    glPopMatrix();
}

// =======================================================================
// MAIN
// =======================================================================

void TempDrawFrame()
{
    bone_frame_p bframe;
    static btScalar time = 0.0;
    skeletal_model_p smodel;
    animation_frame_p af;
    anim_dispath_p adsp;
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
       
    if(model < 0 || model > engine_world.skeletal_model_count)
    {
        model = 0;
    }
    smodel = engine_world.skeletal_models + model;

    if(anim < 0 || anim > smodel->animation_count-1)
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
    
    glColor3b(0, 0, 0);
    Gui_OutTextXY(screen_info.w-480, 120, "sprite ID = %d;  mesh ID = %d", bsprite->ID, mesh);
    Gui_OutTextXY(screen_info.w-480, 96, "model ID = %d, anim = %d of %d, frame = %d of %d", smodel->ID, anim, smodel->animation_count, frame, smodel->animations[anim].frames_count);   
    Gui_OutTextXY(screen_info.w-480, 72, "next anim = %d, next frame = %d, num_state_changes = %d", (af->next_anim)?(af->next_anim->ID):-1, af->next_frame, af->state_change_count);
    Gui_OutTextXY(screen_info.w-480, 48, "v1 = %d, v2 = %d, al1 = %d, ah1 = %d, al2 = %d, ah2 = %d", af->speed, af->speed2, af->accel_lo, af->accel_hi, af->accel_lo2, af->accel_hi2);
    Gui_OutTextXY(screen_info.w-480, 24, "bb_min(%d, %d, %d), bb_max(%d, %d, %d)", (int)bframe->bb_min[0], (int)bframe->bb_min[1], (int)bframe->bb_min[2], (int)bframe->bb_max[0], (int)bframe->bb_max[1], (int)bframe->bb_max[2]);
    Gui_OutTextXY(screen_info.w-480, 4, "x0 = %d, y0 = %d, z0 = %d", (int)bframe->pos[0], (int)bframe->pos[1], (int)bframe->pos[2]);
    
    y = screen_info.h - 24;
    for(i=0;i<af->state_change_count;i++)
    {
        for(j=0;j<af->state_change[i].anim_dispath_count;j++)
        {
            adsp = af->state_change[i].anim_dispath + j;
            Gui_OutTextXY(8, y, "[%d, %d], id = %d next anim = %d, next frame = %d, interval = [%d, %d]",  
                          i, j, af->state_change[i].ID, adsp->next_anim, adsp->next_frame, adsp->frame_low, adsp->frame_high);
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
    
    glMultMatrixf(tr);
    Render_Mesh(mt->mesh, NULL, NULL);
    btag++;
    mt++;;
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
        Render_Mesh(mt->mesh, NULL, NULL);
        //Render_BBox(tree_tag->mesh->bb_min, tree_tag->mesh->bb_max);
        if(mt->mesh2)
        {
            Render_SkinMesh(mt->mesh2, tr);
        }
    }
    
    for(i=0;i<stack;i++)                                                        // PARANOID: GL STACK CHECK AND CORRECTION
    {
        glPopMatrix();
    }
    stack = 0;
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(1024.0, 0.0, 0.0);
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_CULL_FACE);
    Render_Sprite(bsprite);
    glPopAttrib();
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(-1024.0, 0.0, 0.0);
    Render_Mesh(engine_world.meshes + mesh, NULL, NULL);
    glPopMatrix();
}

void TestGenScene()
{         
    if(!Engine_LoadMap(CVAR_get_val_s("game_level")))
    {
        CVAR_set_val_d("free_look", 1.0);
    }
}


void Engine_PrepareOpenGL()
{
    InitGLExtFuncs();
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_COLOR_MATERIAL);

    //glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    //glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    //glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    //glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.5);
    //glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.5);
    //glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.2);

    //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.0);
    //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
    //glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 2.0);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    dbgSphere = gluNewQuadric();
    dbgCyl = gluNewQuadric();
    gluQuadricDrawStyle(dbgSphere, GLU_FILL);
    gluQuadricTexture(dbgSphere, true);
    gluQuadricDrawStyle(dbgCyl, GLU_FILL);
    gluQuadricTexture(dbgCyl, true);

    // Default state: Vertex array is enabled, all others disabled.. Drawable
    // items can rely on Vertex array to be enabled (but pointer can be
    // anything). They have to enable other arrays based on their need and then
    // return to default state
    glEnableClientState(GL_VERTEX_ARRAY);
    // Default state for Alpha func: >=, 0.5. That's what all users of alpha
    // function use anyway.
    glAlphaFunc(GL_GEQUAL, 0.5);
}


int main(int argc, char **argv)
{
    Uint32      video_flags;
    btScalar      time, newtime;
    static btScalar oldtime = 0.0;
    
    video_flags = SDL_SWSURFACE | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    Engine_Init();
    Engine_InitGlobals();
    Engine_LoadConfig();
    if(control_mapper.use_joy == 1)
    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER);
        SDL_JoystickEventState(SDL_ENABLE);
        joystick = SDL_JoystickOpen(control_mapper.joy_number);
    }
    else
    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_HAPTIC);
    }
    displayWindow = SDL_CreateWindow("OpenTomb", screen_info.x, screen_info.y, screen_info.w, screen_info.h, video_flags);
    openglContext = SDL_GL_CreateContext(displayWindow);

    // set the opengl context version
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, screen_info.bpp);
    
    Engine_Resize(screen_info.w, screen_info.h, screen_info.w, screen_info.h);
    Engine_PrepareOpenGL();
    World_Prepare(&engine_world);
    TestGenScene();
    SDL_WarpMouseInWindow(displayWindow, screen_info.w/2, screen_info.h/2);
    SDL_ShowCursor(0);
#if SKELETAL_TEST
    control_states.free_look = 1;
#endif
    
    dbgSphere = gluNewQuadric();
    dbgCyl = gluNewQuadric();
    gluQuadricDrawStyle(dbgSphere, GLU_FILL);
    gluQuadricTexture(dbgSphere, true);
    gluQuadricDrawStyle(dbgCyl, GLU_FILL);
    gluQuadricTexture(dbgCyl, true);
    
    while(!done)
    {
        newtime = Sys_FloatTime();
        time = newtime - oldtime;
        oldtime = newtime;        
        Engine_Frame(time);
    }
    
    if(control_mapper.use_joy == 1)
    {
        SDL_JoystickClose(joystick);
    }
    
    Engine_Shutdown(0); 
    //SDL_GL_DeleteContext(openglContext);                                        // non needed here, shutdown uses it and calls exit(val)
    //SDL_DestroyWindow(displayWindow);
    //SDL_Quit();
    //printf("\nSDL_Quit...");
    return(0);
}

void Engine_Display()
{
    if(!done)
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );//| GL_ACCUM_BUFFER_BIT );  
        glColor4f( 0.0, 0.0, 0.0, 1.0 );

        glEnable(GL_TEXTURE_2D);
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#if 0
        GLfloat fogColor[3] = {1.0, 0.8, 0.6};
        glFogi(GL_FOG_MODE, GL_EXP2);                                           // Fog Mode
        glFogfv(GL_FOG_COLOR, fogColor);                                        // Set Fog Color
        glFogf(GL_FOG_DENSITY, 0.000025f);                                      // How Dense Will The Fog Be
        glHint(GL_FOG_HINT, GL_DONT_CARE);                                      // Fog Hint Value
        glFogf(GL_FOG_START, 8192.0f);                                          // Fog Start Depth
        glFogf(GL_FOG_END, renderer.cam->dist_far);                             // Fog End Depth
        glEnable(GL_FOG);                                                       // Enables GL_FOG
#endif
        
        Cam_RecalcClipPlanes(&engine_camera);
        Cam_Apply(&engine_camera);
#if !SKELETAL_TEST
        Render_SkyBox(); 
        Render_GenWorldList();
        Render_DrawList();
#endif
        glPopClientAttrib();
        Render_DrawList_DebugLines();
        ShowDebugInfo();
        
        glBindTexture(GL_TEXTURE_2D, 0);
        Gui_Render();
        SDL_GL_SwapWindow(displayWindow);
    }
}

void Engine_Resize(int nominalW, int nominalH, int pixelsW, int pixelsH)
{
    screen_info.w = nominalW;
    screen_info.h = nominalH;

    Cam_SetFovAspect(&engine_camera, screen_info.fov, (btScalar)nominalW/(btScalar)nominalH);

    Cam_RecalcClipPlanes(&engine_camera);

    glViewport(0, 0, pixelsW, pixelsH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


/*
 * MOUSE FUNCTIONS
 */
void Engine_PrimaryMouseDown()
{
    engine_container_p cont = Container_Create();
    btScalar *v = engine_camera.pos;
    btScalar *dir = engine_camera.view_dir;
    btScalar new_pos[3];
    btVector3 localInertia(0, 0, 0);
    btTransform	startTransform;
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
    cont->room = Room_FindPosCogerrence(&engine_world, new_pos, engine_camera.current_room);
    cont->object_type = OBJECT_BULLET_MISC;                     // bullet have to destroy this user pointer
    body->setUserPointer(cont);
    body->setCcdMotionThreshold(dbgR);                          // disable tunneling effect
    body->setCcdSweptSphereRadius(dbgR);
    //body->setAngularFactor(0);
}


void Engine_SecondaryMouseDown()
{
    engine_container_t cont, *c0;
    btVector3 from, to, place;
    engine_container_t cam_cont;

    vec3_copy(from.m_floats, engine_camera.pos);
    to = from + btVector3(engine_camera.view_dir[0], engine_camera.view_dir[1], engine_camera.view_dir[2]) * 32768.0;                    
    cont.next = NULL;
    cont.object = NULL;
    cont.object_type = 0;
    cont.room = engine_camera.current_room;

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
            if(c0->object_type == OBJECT_ENTITY)
            {
                last_rmb = (entity_p)c0->object;
            }
            else if(c0->object_type == OBJECT_BULLET_MISC)
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
        }
    }
}


void Engine_Frame(btScalar time)
{
    SDL_Event   event;
    int i;
    static int  cycles = 0;
    static btScalar time_cycl = 0.0;
    static int mouse_setup = 0;
    extern gui_text_line_t system_fps;
    if(time > 0.1)
    {
        time = 0.1;
    }
    /*if(time_scale.val_d != 0)
    {
        time *= time_scale.val_d;
    }*/
    ResetTempbtScalar();
    engine_frame_time = time;
    if(cycles < 10)
    {
        cycles++;
        time_cycl += time;
    }
    else
    {
        screen_info.fps = (10.0 / time_cycl);
        snprintf(system_fps.text, system_fps.buf_size, "%.1f", screen_info.fps);
        Gui_StringAutoRect(&system_fps);
        cycles = 0;
        time_cycl = 0.0;
    }   
    
    while (SDL_PollEvent(&event))
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
                        SDL_WarpMouseInWindow(displayWindow, screen_info.w/2, screen_info.h/2);
                    }
                }
                mouse_setup = 1;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == 1) //LM = 1, MM = 2, RM = 3, S_UP = 4, S_DOWN = 5
                {
                    Engine_PrimaryMouseDown();
                }
                else if(event.button.button == 3)
                {
                    Engine_SecondaryMouseDown();
                }
                break;
                
            case SDL_JOYAXISMOTION:
                if( (control_mapper.use_joy >= 1) && (event.jaxis.which == control_mapper.joy_number) )
                {
                    Controls_JoyAxis(event.jaxis.axis, event.jaxis.value);
                }
                break;
                
            case SDL_JOYHATMOTION:
                if( (control_mapper.use_joy >= 1) && (event.jhat.which == control_mapper.joy_number) )
                {
                    Controls_JoyHat(event.jhat.value);
                }
                break;
                
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                // NOTE: Joystick button numbers are passed as 1000 + 1 keycodes.
                if( (control_mapper.use_joy >= 1) && (event.jbutton.which == control_mapper.joy_number) )
                {
                    Controls_Key((event.jbutton.button + 1001), event.jbutton.state);
                }
                break;
                
            case SDL_KEYUP:    
            case SDL_KEYDOWN:
                if(con_base.show && event.key.state)
                {
                    Con_Edit(Controls_KeyConsoleFilter(event.key.keysym.sym, event.key.keysym.mod));
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
    
#if SKELETAL_TEST
    Game_ApplyControls();
#else
    GameFrame(time);
#endif
    
    Engine_Display();
}


void ShowDebugInfo()
{
    room_sector_p rs = NULL;
    entity_p ent;
    btScalar tr[16], r, h;
    btTransform trans; 
    gui_text_line_p txt;
    vec3_copy(light_position, engine_camera.pos);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(2.0);
    glColor3f(1.0, 1.0, 1.0);
    glVertexPointer(3, GL_FLOAT, 0, cast_ray);
    glDrawArrays(GL_LINES, 0, 2);
    
#if !SKELETAL_TEST    
    
    glColor3f(0.0, 0.0, 0.0);
    for (int j=bt_engine_dynamicsWorld->getNumCollisionObjects()-1; j>=0 ;j--)
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
                glColor3f(1.0, 1.0, 1.0);
                gluQuadricDrawStyle(dbgSphere, GLU_LINE);
                gluSphere(dbgSphere, dbgR, 12, 12);
                //Render_DrawAxis();
                glPopMatrix();
            }
        }
    }
        
#else 
    TempDrawFrame();
#endif
    Render_DrawAxis(10000.0);
    
#if !SKELETAL_TEST
    
    rs = NULL;
    ent = engine_world.Lara;
    if(ent && ent->character && ent->character->ray_cb)
    {
       height_info_t fc;
       btScalar pos[3];
       
       fc.cb = ent->character->ray_cb;
       fc.ccb = ent->character->convex_cb;
       vec3_add(pos, ent->transform+12, ent->collision_offset.m_floats);
       Character_GetHeightInfo(pos, &fc);

       if(ent->self->room)
       {
            Gui_OutTextXY(screen_info.w-420, 108, "is water = %d, level = %.1f", fc.water, fc.water_level);
       }

       /*
        glPushMatrix();         
        glTranslatef(hang_offset_point[0], hang_offset_point[1], hang_offset_point[2]);
        gluSphere(dbgSphere, 72.0, 8, 8);
        glPopMatrix();
        */
#if 0
        glPushMatrix();         
        trans.setFromOpenGLMatrix(ent->transform);
        trans.getOrigin() += ent->character->curr_offset;

            r = ent->character->Radius;
            h = ent->character->shapeZ->getHalfHeight() / ent->character->shapeZ->getLocalScaling().getZ();
            gluQuadricDrawStyle(dbgSphere, GLU_LINE);
            gluQuadricDrawStyle(dbgCyl, GLU_LINE);
            glColor3f(1.0, 1.0, 1.0);
            trans.getOpenGLMatrix(tr);

            glPushMatrix();
            glMultMatrixf(tr);  
            glScalef(ent->character->base_scale.m_floats[0], ent->character->base_scale.m_floats[1], ent->character->shapeZ->getLocalScaling().getZ());
            glTranslatef(0.0, 0.0, -h);              
            gluSphere(dbgSphere, r, 12, 12);
            gluCylinder(dbgCyl, r, r, 2.0 * h, 12, 12);
            glPopMatrix();

            glPushMatrix();
            glMultMatrixf(tr);  
            glScalef(ent->character->base_scale.m_floats[0], ent->character->base_scale.m_floats[1], ent->character->shapeZ->getLocalScaling().getZ());
            glTranslatef(0.0, 0.0, h);          
            gluSphere(dbgSphere, r, 12, 12);
            glPopMatrix();
            
        glPopMatrix();
#endif
        txt = Gui_OutTextXY(screen_info.w-420, 88, "Z_min = %d, Z_max = %d, W = %d", (int)fc.floor_point.m_floats[2], (int)fc.ceiling_point.m_floats[2], (int)fc.water_level);
        if(txt)
        {
            Gui_StringAutoRect(txt);
            //txt->rect[0] = -420.0;
            //txt->rect[1] = 4.0;
            //txt->rect[2] = -20.0;
            //txt->rect[3] = 132.0;
            //txt->show_rect = 1;
        }
        
        Gui_OutTextXY(screen_info.w-420, 68, "anim = %d, state = %d, frame = %d", ent->current_animation, ent->current_stateID, ent->current_frame);
        if(last_rmb)
        {
            Gui_OutTextXY(screen_info.w-420, 48, "ent_rmb_ID = %d", last_rmb->ID);
        }
        Gui_OutTextXY(screen_info.w-420, 8, "rot[0] = %2.2f, rot[1] = %2.2f, angles[1] = %2.2f", engine_world.Lara->character->cmd.rot[0], engine_world.Lara->character->cmd.rot[1], (btScalar)engine_world.Lara->angles[1]);
    }
  
    if(engine_world.Lara && engine_world.Lara->self->room)
    {
        Gui_OutTextXY(screen_info.w-420, 28, "room = %d, co = %d", engine_world.Lara->self->room->ID, bt_engine_dynamicsWorld->getNumCollisionObjects());
    }
    
    //Gui_OutTextXY(screen_info.w-380, 68, "cam_pos = (%.1f, %.1f, %.1f)", engine_camera.pos[0], engine_camera.pos[1], engine_camera.pos[2]);
    //Gui_OutTextXY(screen_info.w-380, 68, "r_room_active = %d", renderer.r_list_active_count);
#endif
}


void DebugKeys(int button, int state)
{
    if(state)
    {
        switch(button)
        {
                /*models switching*/
            case SDLK_p:
                model++;
                if(model > engine_world.skeletal_model_count-1)
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

                /*animations switching*/

            case SDLK_u:   
                anim--;
                if(anim < 0)
                {
                    anim = engine_world.skeletal_models[model].animation_count-1;
                }
                break;

            case SDLK_i:
                anim++;
                if(anim > engine_world.skeletal_models[model].animation_count-1)
                {
                    anim = 0;
                }
                break;

            case SDLK_t:   
                mesh--;
                if(mesh < 0)
                {
                    mesh = engine_world.meshs_count-1;
                }
                break;

            case SDLK_y:
                mesh++;
                if(mesh > engine_world.meshs_count-1)
                {
                    mesh = 0;
                }
                break;

            case SDLK_z:
                paused = !paused;
                if(engine_world.Lara != NULL)
                {
                    if(engine_world.Lara->move_type == MOVE_UNDER_WATER)
                    {
                        Entity_SetAnimation(engine_world.Lara, 103, 0);
                        engine_world.Lara->move_type = MOVE_ON_FLOOR;
                    }
                    else
                    {
                        Entity_SetAnimation(engine_world.Lara, 108, 0);
                        engine_world.Lara->move_type = MOVE_UNDER_WATER;
                    }
                    engine_world.Lara->anim_flags = ANIM_NORMAL_CONTROL;
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
                if(sprite >= engine_world.sprites_count)
                {
                    sprite = 0;
                }
                break;

                /*
                 * keys for animation switching
                 */
            case SDLK_e:
                if(last_rmb)
                {
                    Entity_SetAnimation(last_rmb, last_rmb->current_animation + 1, 0);   // next anim
                }
                break;

            case SDLK_q:
                if(last_rmb)
                {
                    Entity_SetAnimation(last_rmb, last_rmb->current_animation - 1, 0);   // previous anim
                }
                break;

                /* 
                 * alternate rooms testing 
                 */
            case SDLK_r:
#if !SKELETAL_TEST
                if(!con_base.show)
                {
                    for(int i=0;i<engine_world.room_count;i++)
                    {
                        if(engine_world.rooms[i].alternate_room)
                        {
                            engine_world.rooms[i].use_alternate = !engine_world.rooms[i].use_alternate;
                        }
                    }
                }
#endif
                break;
                
            default:
                //Con_Printf("key = %d", button);
                break;
        };
    }
}
