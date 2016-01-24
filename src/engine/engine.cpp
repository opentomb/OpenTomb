#include "engine.h"

#include <cctype>
#include <cstdio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_haptic.h>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#if defined(__MACOSX__)
#include "mac/FindConfigFile.h"
#endif

#include "LuaState.h"

#include "common.h"
#include "controls.h"
#include "engine/bullet.h"
#include "engine/game.h"
#include "engine/system.h"
#include "gameflow.h"
#include "gui/console.h"
#include "gui/fader.h"
#include "gui/fadermanager.h"
#include "gui/gui.h"
#include "gui/itemnotifier.h"
#include "loader/level.h"
#include "render/render.h"
#include "script/script.h"
#include "strings.h"
#include "world/camera.h"
#include "world/character.h"
#include "world/entity.h"
#include "world/resource.h"
#include "world/room.h"
#include "world/skeletalmodel.h"
#include "world/staticmesh.h"
#include "world/world.h"

#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/adaptors.hpp>

using gui::Console;

namespace engine
{
namespace
{
std::vector<glm::float_t> frame_vertex_buffer;
size_t frame_vertex_buffer_size_left = 0;
}

// Debug globals.

world::Object* last_object = nullptr;

Engine Engine::instance{};

void Engine::initGL()
{
    glewExperimental = GL_TRUE;
    glewInit();

    // GLEW sometimes causes an OpenGL error for no apparent reason. Retrieve and
    // discard it so it doesn't clog up later logging.

    glGetError();

    glClearColor(0.0, 0.0, 0.0, 1.0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    if(::render::renderer.settings().antialias)
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }
}

void Engine::initSDLControls()
{
    Uint32 init_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS; // These flags are used in any case.

    if(ControlSettings::instance.use_joy)
    {
        init_flags |= SDL_INIT_GAMECONTROLLER;  // Update init flags for joystick.

        if(ControlSettings::instance.joy_rumble)
        {
            init_flags |= SDL_INIT_HAPTIC;      // Update init flags for force feedback.
        }

        SDL_Init(init_flags);

        int NumJoysticks = SDL_NumJoysticks();

        if(NumJoysticks < 1 || NumJoysticks - 1 < ControlSettings::instance.joy_number)
        {
            BOOST_LOG_TRIVIAL(error) << "There is no joystick #" << ControlSettings::instance.joy_number << " present";
            return;
        }

        if(SDL_IsGameController(ControlSettings::instance.joy_number))                     // If joystick has mapping (e.g. X360 controller)
        {
            SDL_GameControllerEventState(SDL_ENABLE);                           // Use GameController API
            m_controller = SDL_GameControllerOpen(ControlSettings::instance.joy_number);

            if(!m_controller)
            {
                BOOST_LOG_TRIVIAL(error) << "Can't open game controller #d" << ControlSettings::instance.joy_number;
                SDL_GameControllerEventState(SDL_DISABLE);                      // If controller init failed, close state.
                ControlSettings::instance.use_joy = false;
            }
            else if(ControlSettings::instance.joy_rumble)                                  // Create force feedback interface.
            {
                m_haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(m_controller));
                if(!m_haptic)
                {
                    BOOST_LOG_TRIVIAL(error) << "Can't initialize haptic from game controller #" << ControlSettings::instance.joy_number;
                }
            }
        }
        else
        {
            SDL_JoystickEventState(SDL_ENABLE);                                 // If joystick isn't mapped, use generic API.
            m_joystick = SDL_JoystickOpen(ControlSettings::instance.joy_number);

            if(!m_joystick)
            {
                BOOST_LOG_TRIVIAL(error) << "Can't open joystick #" << ControlSettings::instance.joy_number;
                SDL_JoystickEventState(SDL_DISABLE);                            // If joystick init failed, close state.
                ControlSettings::instance.use_joy = false;
            }
            else if(ControlSettings::instance.joy_rumble)                                  // Create force feedback interface.
            {
                m_haptic = SDL_HapticOpenFromJoystick(m_joystick);
                if(!m_haptic)
                {
                    BOOST_LOG_TRIVIAL(error) << "Can't initialize haptic from joystick #" << ControlSettings::instance.joy_number;
                }
            }
        }

        if(m_haptic)                                                          // To check if force feedback is working or not.
        {
            SDL_HapticRumbleInit(m_haptic);
            SDL_HapticRumblePlay(m_haptic, 1.0, 300);
        }
    }
    else
    {
        SDL_Init(init_flags);
    }
}

void Engine::initSDLVideo()
{
    Uint32 video_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;

    if(screen_info.FS_flag)
    {
        video_flags |= SDL_WINDOW_FULLSCREEN;
    }
    else
    {
        video_flags |= SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
    }

    ///@TODO: is it really needed for correct work?

    if(SDL_GL_LoadLibrary(nullptr) < 0)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Could not init OpenGL driver"));
    }

    if(render::renderer.settings().use_gl3)
    {
        /* Request opengl 3.2 context. */
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    }

    // Create temporary SDL window and GL context for checking capabilities.

    m_window = SDL_CreateWindow(nullptr, screen_info.x, screen_info.y, screen_info.w, screen_info.h, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    m_glContext = SDL_GL_CreateContext(m_window);

    if(!m_glContext)
        BOOST_THROW_EXCEPTION(std::runtime_error("Can't create OpenGL context - shutting down. Try to disable use_gl3 option in config."));

    BOOST_ASSERT(m_glContext);
    SDL_GL_MakeCurrent(m_window, m_glContext);

    // Check for correct number of antialias samples.

    if(render::renderer.settings().antialias)
    {
        GLint maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        maxSamples = maxSamples > 16 ? 16 : maxSamples;   // Fix for faulty GL max. sample number.

        if(render::renderer.settings().antialias_samples > maxSamples)
        {
            if(maxSamples == 0)
            {
                render::renderer.settings().antialias = 0;
                render::renderer.settings().antialias_samples = 0;
                BOOST_LOG_TRIVIAL(error) << "InitSDLVideo: can't use antialiasing";
            }
            else
            {
                render::renderer.settings().antialias_samples = maxSamples;   // Limit to max.
                BOOST_LOG_TRIVIAL(error) << "InitSDLVideo: wrong AA sample number, using " << maxSamples;
            }
        }

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, render::renderer.settings().antialias);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, render::renderer.settings().antialias_samples);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }

    // Remove temporary GL context and SDL window.

    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, render::renderer.settings().z_depth);

    m_window = SDL_CreateWindow("OpenTomb", screen_info.x, screen_info.y, screen_info.w, screen_info.h, video_flags);
    m_glContext = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_glContext);

    if(SDL_GL_SetSwapInterval(screen_info.vsync))
        BOOST_LOG_TRIVIAL(error) << "Cannot set VSYNC: " << SDL_GetError();

    Console::instance().addLine(reinterpret_cast<const char*>(glGetString(GL_VENDOR)), gui::FontStyle::ConsoleInfo);
    Console::instance().addLine(reinterpret_cast<const char*>(glGetString(GL_RENDERER)), gui::FontStyle::ConsoleInfo);
    std::string version = "OpenGL version ";
    version += reinterpret_cast<const char*>(glGetString(GL_VERSION));
    Console::instance().addLine(version, gui::FontStyle::ConsoleInfo);
    Console::instance().addLine(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)), gui::FontStyle::ConsoleInfo);
}

void Engine::start()
{
#if defined(__MACOSX__)
    FindConfigFile();
#endif

    // Set defaults parameters and load config file.
    initConfig("config.lua");

    // Primary initialization.
    initPre();

    // Init generic SDL interfaces.
    initSDLControls();
    initSDLVideo();

    // Additional OpenGL initialization.
    initGL();
    render::renderer.doShaders();

    // Secondary (deferred) initialization.
    initPost();

    // Initial window resize.
    resize(screen_info.w, screen_info.h, screen_info.w, screen_info.h);

    // OpenAL initialization.
    m_world.audioEngine.initDevice();

    Console::instance().notify(SYSNOTE_ENGINE_INITED);

    // Clearing up memory for initial level loading.
    m_world.prepare();

    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Make splash screen.
    gui::Gui::instance->faders.assignPicture(gui::FaderType::LoadScreen, "resource/graphics/legal.png");
    gui::Gui::instance->faders.start(gui::FaderType::LoadScreen, gui::FaderDir::Out);

    engine_lua.doFile("autoexec.lua");
}

void Engine::display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//| GL_ACCUM_BUFFER_BIT);

    m_camera.apply();
    // GL_VERTEX_ARRAY | GL_COLOR_ARRAY
    if(screen_info.show_debuginfo)
    {
        showDebugInfo();
    }

    glFrontFace(GL_CW);

    render::renderer.genWorldList();
    render::renderer.drawList();

    //glDisable(GL_CULL_FACE);
    //Render_DrawAxis(10000.0);
    /*if(engine_world.character)
    {
        glPushMatrix();
        glTranslatef(engine_world.character->transform[12], engine_world.character->transform[13], engine_world.character->transform[14]);
        Render_DrawAxis(1000.0);
        glPopMatrix();
    }*/

    gui::Gui::instance->switchGLMode(true);
    {
        gui::Gui::instance->notifier.draw();
        gui::Gui::instance->notifier.animate();
        gui::Gui::instance->drawInventory();
    }

    gui::Gui::instance->render();
    gui::Gui::instance->switchGLMode(false);

    render::renderer.drawListDebugLines();

    SDL_GL_SwapWindow(m_window);
}

void Engine::resize(int nominalW, int nominalH, int pixelsW, int pixelsH)
{
    screen_info.w = nominalW;
    screen_info.h = nominalH;

    screen_info.w_unit = static_cast<float>(nominalW) / gui::ScreenMeteringResolution;
    screen_info.h_unit = static_cast<float>(nominalH) / gui::ScreenMeteringResolution;
    screen_info.scale_factor = screen_info.w < screen_info.h ? screen_info.h_unit : screen_info.w_unit;

    gui::Gui::instance->resize();

    m_camera.setFovAspect(screen_info.fov, static_cast<glm::float_t>(nominalW) / static_cast<glm::float_t>(nominalH));
    m_camera.apply();

    glViewport(0, 0, pixelsW, pixelsH);
}

void Engine::frame(util::Duration time)
{
    m_frameTime = time;
    fpsCycle(time);

    Game_Frame(time);
    Gameflow::instance.execute();
}

void Engine::showDebugInfo()
{
    GLfloat color_array[] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

    m_lightPosition = m_camera.getPosition();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(2.0);
    glVertexPointer(3, GL_FLOAT, 0, glm::value_ptr(m_castRay[0]));
    glColorPointer(3, GL_FLOAT, 0, color_array);
    glDrawArrays(GL_LINES, 0, 2);

    if(std::shared_ptr<world::Character> ent = m_world.character)
    {
        /*height_info_p fc = &ent->character->height_info
        txt = Gui_OutTextXY(20.0 / screen_info.w, 80.0 / screen_info.w, "Z_min = %d, Z_max = %d, W = %d", (int)fc->floor_point[2], (int)fc->ceiling_point[2], (int)fc->water_level);
        */
        gui::TextLineManager::instance->drawText(30.0, 30.0,
                                                 boost::format("prevState = %03d, nextState = %03d, speed = %f")
                                                 % static_cast<int>(ent->m_skeleton.getPreviousState())
                                                 % static_cast<int>(ent->m_skeleton.getCurrentState())
                                                 % ent->m_currentSpeed
                                                 );
        gui::TextLineManager::instance->drawText(30.0, 50.0,
                                                 boost::format("prevAnim = %3d, prevFrame = %3d, currAnim = %3d, currFrame = %3d")
                                                 % ent->m_skeleton.getPreviousAnimation()
                                                 % ent->m_skeleton.getPreviousFrame()
                                                 % ent->m_skeleton.getCurrentAnimation()
                                                 % ent->m_skeleton.getCurrentFrame()
                                                 );
        //Gui_OutTextXY(30.0, 30.0, "curr_anim = %03d, next_anim = %03d, curr_frame = %03d, next_frame = %03d", ent->bf.animations.current_animation, ent->bf.animations.next_animation, ent->bf.animations.current_frame, ent->bf.animations.next_frame);
        gui::TextLineManager::instance->drawText(20, 8,
                                                 boost::format("pos = %s, yaw = %f")
                                                 % glm::to_string(ent->m_transform[3])
                                                 % ent->m_angles[0]
                                                 );
    }

    if(last_object != nullptr)
    {
        if(world::Entity* e = dynamic_cast<world::Entity*>(last_object))
        {
            gui::TextLineManager::instance->drawText(30.0, 60.0,
                                                     boost::format("cont_entity: id = %d, model = %d")
                                                     % e->getId()
                                                     % e->m_skeleton.getModel()->id
                                                     );
        }
        else if(world::StaticMesh* sm = dynamic_cast<world::StaticMesh*>(last_object))
        {
            gui::TextLineManager::instance->drawText(30.0, 60.0,
                                                     boost::format("cont_static: id = %d")
                                                     % sm->getId()
                                                     );
        }
        else if(world::Room* r = dynamic_cast<world::Room*>(last_object))
        {
            gui::TextLineManager::instance->drawText(30.0, 60.0,
                                                     boost::format("cont_room: id = %d")
                                                     % r->getId()
                                                     );
        }
    }

    if(m_camera.getCurrentRoom() != nullptr)
    {
        world::RoomSector* rs = m_camera.getCurrentRoom()->getSectorRaw(m_camera.getPosition());
        if(rs != nullptr)
        {
            gui::TextLineManager::instance->drawText(30.0, 90.0,
                                                     boost::format("room = (id = %d, sx = %d, sy = %d)")
                                                     % m_camera.getCurrentRoom()->getId()
                                                     % rs->index_x
                                                     % rs->index_y
                                                     );
            gui::TextLineManager::instance->drawText(30.0, 120.0,
                                                     boost::format("room_below = %d, room_above = %d")
                                                     % (rs->sector_below != nullptr ? rs->sector_below->owner_room->getId() : -1)
                                                     % (rs->sector_above != nullptr ? rs->sector_above->owner_room->getId() : -1)
                                                     );
        }
    }
    gui::TextLineManager::instance->drawText(30.0, 150.0,
                                             boost::format("cam_pos = %s")
                                             % glm::to_string(m_camera.getPosition())
                                             );
}

void Engine::initDefaultGlobals()
{
    Console::instance().initGlobals();
    ControlSettings::instance.initGlobals();
    Game_InitGlobals();
    render::renderer.initGlobals();
    m_world.audioEngine.resetSettings();
}

// First stage of initialization.

void Engine::initPre()
{
    /* Console must be initialized previously! some functions uses ConsoleInfo::instance().addLine before GL initialization!
     * Rendering activation may be done later. */

    gui::FontManager::instance.reset(new gui::FontManager());
    Console::instance().init();

    engine_lua["loadscript_pre"]();

    Gameflow::instance.init();

    frame_vertex_buffer.resize(render::InitFrameVertexBufferSize);
    frame_vertex_buffer_size_left = frame_vertex_buffer.size();

    Console::instance().setCompletionItems(engine_lua.getGlobals());

    Com_Init();
    render::renderer.init();
    render::renderer.setCamera(&m_camera);

    engine::BulletEngine::instance.reset(new engine::BulletEngine());
}

// Second stage of initialization.

void Engine::initPost()
{
    engine_lua["loadscript_post"]();

    Console::instance().initFonts();

    gui::Gui::instance.reset(new gui::Gui());
    sysInit();
}

void Engine::dumpRoom(world::Room* r)
{
    if(!r)
        return;

    BOOST_LOG_TRIVIAL(debug) << boost::format("ROOM = %d, (%d x %d), bottom = %g, top = %g, pos(%g, %g)")
        % r->getId()
        % r->m_sectors.shape()[0]
        % r->m_sectors.shape()[1]
        % r->m_boundingBox.min[2]
        % r->m_boundingBox.max[2]
        % r->m_modelMatrix[3][0]
        % r->m_modelMatrix[3][1];
    BOOST_LOG_TRIVIAL(debug) << boost::format("flag = 0x%X, alt_room = %d, base_room = %d")
        % r->m_flags
        % (r->m_alternateRoom != nullptr ? r->m_alternateRoom->getId() : -1)
        % (r->m_baseRoom != nullptr ? r->m_baseRoom->getId() : -1);
    for(const auto& column : r->m_sectors)
    {
        for(const world::RoomSector& rs : column)
        {
            BOOST_LOG_TRIVIAL(debug) << boost::format("(%d,%d) floor = %d, ceiling = %d, portal = %d")
                % rs.index_x
                % rs.index_y
                % rs.floor
                % rs.ceiling
                % (rs.portal_to_room ? *rs.portal_to_room : -1);
        }
    }

    for(auto sm : r->m_staticMeshes)
    {
        BOOST_LOG_TRIVIAL(debug) << "static_mesh = " << sm->getId();
    }

    for(world::Object* object : r->m_objects)
    {
        if(world::Entity* ent = dynamic_cast<world::Entity*>(object))
        {
            BOOST_LOG_TRIVIAL(debug) << "entity: id = " << ent->getId() << ", model = " << ent->m_skeleton.getModel()->id;
        }
    }
}

void Engine::destroy()
{
    render::renderer.empty();
    //ConsoleInfo::instance().destroy();
    Com_Destroy();
    sysDestroy();

    BulletEngine::instance.reset();

    gui::Gui::instance.reset();
}

void Engine::shutdown(int val)
{
    engine_lua.clearTasks();
    render::renderer.empty();
    m_world.empty();
    destroy();

    /* no more renderings */
    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);

    if(m_joystick)
    {
        SDL_JoystickClose(m_joystick);
    }

    if(m_controller)
    {
        SDL_GameControllerClose(m_controller);
    }

    if(m_haptic)
    {
        SDL_HapticClose(m_haptic);
    }

    m_world.audioEngine.closeDevice();

    /* free temporary memory */
    frame_vertex_buffer.clear();
    frame_vertex_buffer_size_left = 0;

    SDL_Quit();

    exit(val);
}

int Engine::getLevelFormat(const std::string& /*name*/)
{
    // PLACEHOLDER: Currently, only PC levels are supported.

    return LEVEL_FORMAT_PC;
}

std::string Engine::getLevelName(const std::string& path)
{
    if(path.empty())
    {
        return{};
    }

    size_t ext = path.find_last_of(".");
    BOOST_ASSERT(ext != std::string::npos);

    size_t start = path.find_last_of("\\/");
    if(start == std::string::npos)
        start = 0;
    else
        ++start;

    return path.substr(start, ext - start);
}

std::string Engine::getAutoexecName(loader::Game game_version, const std::string& postfix)
{
    std::string level_name = getLevelName(Gameflow::instance.getLevelPath());

    std::string name = "scripts/autoexec/";
    switch(loader::gameToEngine(game_version))
    {
        case loader::Engine::TR1:
            name += "tr1/";
            break;
        case loader::Engine::TR2:
            name += "tr2/";
            break;
        case loader::Engine::TR3:
            name += "tr3/";
            break;
        case loader::Engine::TR4:
            name += "tr4/";
            break;
        case loader::Engine::TR5:
        default:
            name += "tr5/";
            break;
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

bool Engine::loadPCLevel(const std::string& name)
{
    std::unique_ptr<loader::Level> loader = loader::Level::createLoader(name, loader::Game::Unknown);
    if(!loader)
        return false;

    loader->load();

    TR_GenWorld(m_world, loader);

    std::string buf = getLevelName(name);

    Console::instance().notify(SYSNOTE_LOADED_PC_LEVEL);
    Console::instance().notify(SYSNOTE_ENGINE_VERSION, static_cast<int>(loader->m_gameVersion), buf.c_str());
    Console::instance().notify(SYSNOTE_NUM_ROOMS, m_world.rooms.size());

    return true;
}

bool Engine::loadMap(const std::string& name)
{
    BOOST_LOG_TRIVIAL(info) << "Loading map: " << name;

    if(!boost::filesystem::is_regular_file(name))
    {
        Console::instance().warning(SYSWARN_FILE_NOT_FOUND, name.c_str());
        return false;
    }

    gui::Gui::instance->drawLoadScreen(0);

    m_camera.setCurrentRoom(nullptr);

    render::renderer.hideSkyBox();
    render::renderer.resetWorld();

    Gameflow::instance.setLevelPath(name);          // it is needed for "not in the game" levels or correct saves loading.

    gui::Gui::instance->drawLoadScreen(50);

    m_world.empty();
    m_world.prepare();

    engine_lua.clean();

    m_world.audioEngine.init();

    gui::Gui::instance->drawLoadScreen(100);

    // Here we can place different platform-specific level loading routines.

    switch(getLevelFormat(name))
    {
        case LEVEL_FORMAT_PC:
            if(!loadPCLevel(name))
                return false;
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

    Game_Prepare();

    engine_lua.prepare();

    render::renderer.setWorld(&m_world);

    gui::Gui::instance->drawLoadScreen(1000);

    gui::Gui::instance->faders.start(gui::FaderType::LoadScreen, gui::FaderDir::In);
    gui::Gui::instance->notifier.reset();

    return true;
}

int Engine::execCmd(const char *ch)
{
    std::vector<char> token(Console::instance().lineLength());
    world::RoomSector* sect;
    FILE *f;

    while(ch != nullptr)
    {
        const char *pch = ch;

        ch = script::MainEngine::parse_token(ch, token.data());
        if(!strcmp(token.data(), "help"))
        {
            for(int i = SYSNOTE_COMMAND_HELP1; i <= SYSNOTE_COMMAND_HELP15; i++)
            {
                Console::instance().notify(i);
            }
        }
        else if(!strcmp(token.data(), "goto"))
        {
            m_controlState.m_freeLook = true;
            const auto x = script::MainEngine::parseFloat(&ch);
            const auto y = script::MainEngine::parseFloat(&ch);
            const auto z = script::MainEngine::parseFloat(&ch);
            render::renderer.camera()->setPosition({ x, y, z });
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
            shutdown(0);
            return 1;
        }
        else if(!strcmp(token.data(), "cls"))
        {
            Console::instance().clean();
            return 1;
        }
        else if(!strcmp(token.data(), "spacing"))
        {
            ch = script::MainEngine::parse_token(ch, token.data());
            if(NULL == ch)
            {
                Console::instance().notify(SYSNOTE_CONSOLE_SPACING, Console::instance().spacing());
                return 1;
            }
            Console::instance().setSpacing(std::stof(token.data()));
            return 1;
        }
        else if(!strcmp(token.data(), "showing_lines"))
        {
            ch = script::MainEngine::parse_token(ch, token.data());
            if(NULL == ch)
            {
                Console::instance().notify(SYSNOTE_CONSOLE_LINECOUNT, Console::instance().visibleLines());
                return 1;
            }
            else
            {
                const auto val = atoi(token.data());
                if(val >= 2 && val <= screen_info.h / Console::instance().lineHeight())
                {
                    Console::instance().setVisibleLines(val);
                    Console::instance().setCursorY(screen_info.h - Console::instance().lineHeight() * Console::instance().visibleLines());
                }
                else
                {
                    Console::instance().warning(SYSWARN_INVALID_LINECOUNT);
                }
            }
            return 1;
        }
        else if(!strcmp(token.data(), "r_wireframe"))
        {
            render::renderer.toggleWireframe();
            return 1;
        }
        else if(!strcmp(token.data(), "r_points"))
        {
            render::renderer.toggleDrawPoints();
            return 1;
        }
        else if(!strcmp(token.data(), "r_coll"))
        {
            render::renderer.toggleDrawColl();
            return 1;
        }
        else if(!strcmp(token.data(), "r_normals"))
        {
            render::renderer.toggleDrawNormals();
            return 1;
        }
        else if(!strcmp(token.data(), "r_portals"))
        {
            render::renderer.toggleDrawPortals();
            return 1;
        }
        else if(!strcmp(token.data(), "r_room_boxes"))
        {
            render::renderer.toggleDrawRoomBoxes();
            return 1;
        }
        else if(!strcmp(token.data(), "r_boxes"))
        {
            render::renderer.toggleDrawBoxes();
            return 1;
        }
        else if(!strcmp(token.data(), "r_axis"))
        {
            render::renderer.toggleDrawAxis();
            return 1;
        }
        else if(!strcmp(token.data(), "r_allmodels"))
        {
            render::renderer.toggleDrawAllModels();
            return 1;
        }
        else if(!strcmp(token.data(), "r_dummy_statics"))
        {
            render::renderer.toggleDrawDummyStatics();
            return 1;
        }
        else if(!strcmp(token.data(), "r_skip_room"))
        {
            render::renderer.toggleSkipRoom();
            return 1;
        }
        else if(!strcmp(token.data(), "room_info"))
        {
            if(world::Room* r = render::renderer.camera()->getCurrentRoom())
            {
                sect = r->getSectorXYZ(render::renderer.camera()->getPosition());
                Console::instance().printf("ID = %d, x_sect = %d, y_sect = %d", r->getId(), static_cast<int>(r->m_sectors.shape()[0]), static_cast<int>(r->m_sectors.shape()[1]));
                if(sect)
                {
                    Console::instance().printf("sect(%d, %d), inpenitrable = %d, r_up = %d, r_down = %d",
                                               static_cast<int>(sect->index_x),
                                               static_cast<int>(sect->index_y),
                                               static_cast<int>(sect->ceiling == world::MeteringWallHeight || sect->floor == world::MeteringWallHeight),
                                               static_cast<int>(sect->sector_above != nullptr), static_cast<int>(sect->sector_below != nullptr));
                    for(uint32_t i = 0; i < sect->owner_room->m_staticMeshes.size(); i++)
                    {
                        Console::instance().printf("static[%d].object_id = %d", i, sect->owner_room->m_staticMeshes[i]->getId());
                    }
                    for(world::Object* object : sect->owner_room->m_objects)
                    {
                        if(world::Entity* e = dynamic_cast<world::Entity*>(object))
                        {
                            Console::instance().printf("object[entity](%d, %d, %d).object_id = %d", static_cast<int>(e->m_transform[3][0]), static_cast<int>(e->m_transform[3][1]), static_cast<int>(e->m_transform[3][2]), e->getId());
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
                fseek(f, 0, SEEK_END);
                auto size = ftell(f);
                std::vector<char> buf(size + 1);

                fseek(f, 0, SEEK_SET);
                fread(buf.data(), size, sizeof(char), f);
                buf[size] = 0;
                fclose(f);
                Console::instance().clean();
                Console::instance().addText(buf.data(), gui::FontStyle::ConsoleInfo);
            }
            else
            {
                Console::instance().addText("Not available =(", gui::FontStyle::ConsoleWarning);
            }
            return 1;
        }
        else if(token[0])
        {
            Console::instance().addLine(pch, gui::FontStyle::ConsoleEvent);
            try
            {
                engine_lua.doString(pch);
            }
            catch(lua::RuntimeError& error)
            {
                Console::instance().addLine(error.what(), gui::FontStyle::ConsoleWarning);
            }
            catch(lua::LoadError& error)
            {
                Console::instance().addLine(error.what(), gui::FontStyle::ConsoleWarning);
            }
            return 0;
        }
    }

    return 0;
}

void Engine::sysInit()
{
    system_fps.text.clear();

    system_fps.position = { 10.0, 10.0 };
    system_fps.Xanchor = gui::HorizontalAnchor::Right;
    system_fps.Yanchor = gui::VerticalAnchor::Bottom;

    system_fps.fontType = gui::FontType::Primary;
    system_fps.fontStyle = gui::FontStyle::MenuTitle;

    system_fps.show = true;

    gui::TextLineManager::instance->add(&system_fps);
}

void Engine::sysDestroy()
{
    system_fps.show = false;
    system_fps.text.clear();
}

void Engine::fpsCycle(util::Duration time)
{
    if(fpsCycles < 20)
    {
        fpsCycles++;
        fpsTime += time;
    }
    else
    {
        screen_info.fps = 20.0f / util::toSeconds(fpsTime);
        char tmp[16];
        snprintf(tmp, 16, "%.1f", screen_info.fps);
        Engine::instance.system_fps.text = tmp;
        fpsCycles = 0;
        fpsTime = util::Duration(0);
    }
}

void Engine::initConfig(const std::string& filename)
{
    initDefaultGlobals();

    if(boost::filesystem::is_regular_file(filename))
    {
        script::ScriptEngine state;
        state.registerC("bind", &script::MainEngine::bindKey);                             // get and set key bindings
        try
        {
            state.doFile(filename);
        }
        catch(lua::RuntimeError& error)
        {
            BOOST_LOG_TRIVIAL(error) << error.what();
            return;
        }
        catch(lua::LoadError& error)
        {
            BOOST_LOG_TRIVIAL(error) << error.what();
            return;
        }

        state.parseScreen(screen_info);
        state.parseRender(render::renderer.settings());
        state.parseAudio(m_world.audioEngine.settings());
        state.parseConsole(Console::instance());
        state.parseControls(ControlSettings::instance);
        state.parseSystem(system_settings);
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "Could not find " << filename;
    }
}
} // namespace engine