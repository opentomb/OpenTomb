#include "engine.h"

#include <cctype>
#include <cstdio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#if defined(__MACOSX__)
#include "mac/FindConfigFile.h"
#endif

#include "LuaState.h"

#include "common.h"
#include "engine/bullet.h"
#include "engine/game.h"
#include "engine/system.h"
#include "gameflow.h"
#include "gui/console.h"
#include "gui/fader.h"
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

    if(renderer.settings().antialias)
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }
}

void Engine::initSDLVideo()
{
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    Uint32 video_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;

    if(m_screenInfo.FS_flag)
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

    if(renderer.settings().use_gl3)
    {
        /* Request opengl 3.2 context. */
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    }

    // Create temporary SDL window and GL context for checking capabilities.

    m_window = SDL_CreateWindow(nullptr, m_screenInfo.x, m_screenInfo.y, m_screenInfo.w, m_screenInfo.h, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    m_glContext = SDL_GL_CreateContext(m_window);

    if(!m_glContext)
        BOOST_THROW_EXCEPTION(std::runtime_error("Can't create OpenGL context - shutting down. Try to disable use_gl3 option in config."));

    BOOST_ASSERT(m_glContext);
    SDL_GL_MakeCurrent(m_window, m_glContext);

    // Check for correct number of antialias samples.

    if(renderer.settings().antialias)
    {
        GLint maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        maxSamples = maxSamples > 16 ? 16 : maxSamples;   // Fix for faulty GL max. sample number.

        if(renderer.settings().antialias_samples > maxSamples)
        {
            if(maxSamples == 0)
            {
                renderer.settings().antialias = 0;
                renderer.settings().antialias_samples = 0;
                BOOST_LOG_TRIVIAL(error) << "InitSDLVideo: can't use antialiasing";
            }
            else
            {
                renderer.settings().antialias_samples = maxSamples;   // Limit to max.
                BOOST_LOG_TRIVIAL(error) << "InitSDLVideo: wrong AA sample number, using " << maxSamples;
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

    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, renderer.settings().z_depth);

    m_window = SDL_CreateWindow("OpenTomb", m_screenInfo.x, m_screenInfo.y, m_screenInfo.w, m_screenInfo.h, video_flags);
    m_glContext = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_glContext);

    if(SDL_GL_SetSwapInterval(m_screenInfo.vsync))
        BOOST_LOG_TRIVIAL(error) << "Cannot set VSYNC: " << SDL_GetError();

    m_gui.getConsole().addLine(reinterpret_cast<const char*>(glGetString(GL_VENDOR)), gui::FontStyle::ConsoleInfo);
    m_gui.getConsole().addLine(reinterpret_cast<const char*>(glGetString(GL_RENDERER)), gui::FontStyle::ConsoleInfo);
    std::string version = "OpenGL version ";
    version += reinterpret_cast<const char*>(glGetString(GL_VERSION));
    m_gui.getConsole().addLine(version, gui::FontStyle::ConsoleInfo);
    m_gui.getConsole().addLine(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)), gui::FontStyle::ConsoleInfo);
}

Engine::Engine()
    : m_gui(this)
    , m_world(this)
    , m_inputHandler(this)
    , renderer(this)
    , m_gameflow(this)
    , m_bullet(this)
    , m_scriptEngine(this)
    , debugDrawer(this)
{
    BOOST_LOG_TRIVIAL(info) << "Engine booting...";

    m_scriptEngine.doFile("scripts/loadscript.lua");
#if defined(__MACOSX__)
    FindConfigFile();
#endif

    // Set defaults parameters and load config file.
    initConfig("config.lua");

    // Primary initialization.
    initPre();

    // Init generic SDL interfaces.
    initSDLVideo();

    // Additional OpenGL initialization.
    initGL();
    renderer.fillCrosshairBuffer();
    renderer.doShaders();

    // Secondary (deferred) initialization.
    initPost();

    // Initial window resize.
    resize(m_screenInfo.w, m_screenInfo.h, m_screenInfo.w, m_screenInfo.h);

    // OpenAL initialization.
    m_world.m_audioEngine.initDevice();

    m_gui.getConsole().notify(SYSNOTE_ENGINE_INITED);

    // Clearing up memory for initial level loading.
    m_world.prepare();

    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Make splash screen.
    m_gui.m_faders.assignPicture(gui::FaderType::LoadScreen, "resource/graphics/legal.png");
    m_gui.m_faders.start(gui::FaderType::LoadScreen, gui::FaderDir::Out);

    m_scriptEngine.doFile("autoexec.lua");
}

void Engine::display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//| GL_ACCUM_BUFFER_BIT);

    m_camera.apply();
    // GL_VERTEX_ARRAY | GL_COLOR_ARRAY
    if(m_screenInfo.show_debuginfo)
    {
        showDebugInfo();
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

    m_gui.switchGLMode(true);
    {
        m_gui.m_notifier.draw();
        m_gui.m_notifier.animate();
        m_gui.drawInventory();
    }

    m_gui.render();
    m_gui.switchGLMode(false);

    renderer.drawListDebugLines();

    SDL_GL_SwapWindow(m_window);
}

void Engine::resize(int nominalW, int nominalH, int pixelsW, int pixelsH)
{
    m_screenInfo.w = nominalW;
    m_screenInfo.h = nominalH;

    m_screenInfo.w_unit = static_cast<float>(nominalW) / gui::ScreenMeteringResolution;
    m_screenInfo.h_unit = static_cast<float>(nominalH) / gui::ScreenMeteringResolution;
    m_screenInfo.scale_factor = m_screenInfo.w < m_screenInfo.h ? m_screenInfo.h_unit : m_screenInfo.w_unit;

    m_gui.resize();

    m_camera.setFovAspect(m_screenInfo.fov, static_cast<glm::float_t>(nominalW) / static_cast<glm::float_t>(nominalH));
    m_camera.apply();

    glViewport(0, 0, pixelsW, pixelsH);
}

void Engine::frame(util::Duration time)
{
    m_frameTime = time;
    fpsCycle(time);

    Game_Frame(*this, time);
    m_gameflow.execute();
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

    if(std::shared_ptr<world::Character> ent = m_world.m_character)
    {
        /*height_info_p fc = &ent->character->height_info
        txt = Gui_OutTextXY(20.0 / screen_info.w, 80.0 / screen_info.w, "Z_min = %d, Z_max = %d, W = %d", (int)fc->floor_point[2], (int)fc->ceiling_point[2], (int)fc->water_level);
        */
        m_gui.m_textlineManager.drawText(30.0, 30.0,
                                                 boost::format("prevState = %03d, nextState = %03d, speed = %f")
                                                 % static_cast<int>(ent->m_skeleton.getPreviousState())
                                                 % static_cast<int>(ent->m_skeleton.getCurrentState())
                                                 % ent->m_currentSpeed
                                                 );
        m_gui.m_textlineManager.drawText(30.0, 50.0,
                                                 boost::format("prevAnim = %3d, prevFrame = %3d, currAnim = %3d, currFrame = %3d")
                                                 % ent->m_skeleton.getPreviousAnimation()
                                                 % ent->m_skeleton.getPreviousFrame()
                                                 % ent->m_skeleton.getCurrentAnimation()
                                                 % ent->m_skeleton.getCurrentFrame()
                                                 );
        //Gui_OutTextXY(30.0, 30.0, "curr_anim = %03d, next_anim = %03d, curr_frame = %03d, next_frame = %03d", ent->bf.animations.current_animation, ent->bf.animations.next_animation, ent->bf.animations.current_frame, ent->bf.animations.next_frame);
        m_gui.m_textlineManager.drawText(20, 8,
                                                 boost::format("pos = %s, yaw = %f")
                                                 % glm::to_string(ent->m_transform[3])
                                                 % ent->m_angles[0]
                                                 );
    }

    if(m_lastObject != nullptr)
    {
        if(world::Entity* e = dynamic_cast<world::Entity*>(m_lastObject))
        {
            m_gui.m_textlineManager.drawText(30.0, 60.0,
                                                     boost::format("cont_entity: id = %d, model = %d")
                                                     % e->getId()
                                                     % e->m_skeleton.getModel()->getId()
                                                     );
        }
        else if(world::StaticMesh* sm = dynamic_cast<world::StaticMesh*>(m_lastObject))
        {
            m_gui.m_textlineManager.drawText(30.0, 60.0,
                                                     boost::format("cont_static: id = %d")
                                                     % sm->getId()
                                                     );
        }
        else if(world::Room* r = dynamic_cast<world::Room*>(m_lastObject))
        {
            m_gui.m_textlineManager.drawText(30.0, 60.0,
                                                     boost::format("cont_room: id = %d")
                                                     % r->getId()
                                                     );
        }
    }

    if(m_camera.getCurrentRoom() != nullptr)
    {
        const world::RoomSector* rs = m_camera.getCurrentRoom()->getSectorRaw(m_camera.getPosition());
        if(rs != nullptr)
        {
            m_gui.m_textlineManager.drawText(30.0, 90.0,
                                                     boost::format("room = (id = %d, sx = %d, sy = %d)")
                                                     % m_camera.getCurrentRoom()->getId()
                                                     % rs->index_x
                                                     % rs->index_y
                                                     );
            m_gui.m_textlineManager.drawText(30.0, 120.0,
                                                     boost::format("room_below = %d, room_above = %d")
                                                     % (rs->sector_below != nullptr ? rs->sector_below->owner_room->getId() : -1)
                                                     % (rs->sector_above != nullptr ? rs->sector_above->owner_room->getId() : -1)
                                                     );
        }
    }
    m_gui.m_textlineManager.drawText(30.0, 150.0,
                                             boost::format("cam_pos = %s")
                                             % glm::to_string(m_camera.getPosition())
                                             );
}

void Engine::initDefaultGlobals()
{
    m_gui.getConsole().initGlobals();

    m_inputHandler.clearBindings();
    m_inputHandler.clearHandlers();

    auto self = this;
    m_inputHandler.registerMouseMoveHandler([self](float dx, float dy){
        if(!self->m_gui.getConsole().isVisible() && self->m_controlState.m_mouseLook)
        {
            self->m_controlState.m_lookAxisX = dx;
            self->m_controlState.m_lookAxisY = dy;
        }
    });
    m_inputHandler.registerActionHandler(InputAction::Up, [self](bool pressed){
        self->m_controlState.m_moveForward = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::Down, [self](bool pressed){
        self->m_controlState.m_moveBackward = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::Left, [self](bool pressed){
        self->m_controlState.m_moveLeft = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::Right, [self](bool pressed){
        self->m_controlState.m_moveRight = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::DrawWeapon, [self](bool pressed){
        self->m_controlState.m_doDrawWeapon = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::Action, [self](bool pressed){
        self->m_controlState.m_stateAction = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::Jump, [self](bool pressed){
        self->m_controlState.m_moveUp = pressed;
        self->m_controlState.m_doJump = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::Roll, [self](bool pressed){
        self->m_controlState.m_doRoll = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::Walk, [self](bool pressed){
        self->m_controlState.m_stateWalk = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::Sprint, [self](bool pressed){
        self->m_controlState.m_stateSprint = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::Crouch, [self](bool pressed){
        self->m_controlState.m_moveDown = pressed;
        self->m_controlState.m_stateCrouch = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::LookUp, [self](bool pressed){
        self->m_controlState.m_lookUp = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::LookDown, [self](bool pressed){
        self->m_controlState.m_lookDown = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::LookLeft, [self](bool pressed){
        self->m_controlState.m_lookLeft = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::LookRight, [self](bool pressed){
        self->m_controlState.m_lookRight = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::BigMedi, [self](bool pressed){
        if(!self->m_inputHandler.getActionState(InputAction::BigMedi).wasActive)
        {
            self->m_controlState.m_useBigMedi = pressed;
        }
    });
    m_inputHandler.registerActionHandler(InputAction::SmallMedi, [self](bool pressed){
        if(!self->m_inputHandler.getActionState(InputAction::SmallMedi).wasActive)
        {
            self->m_controlState.m_useSmallMedi = pressed;
        }
    });
    m_inputHandler.registerActionHandler(InputAction::Console, [self](bool pressed){
        if(pressed)
            return;

        self->m_gui.getConsole().toggleVisibility();

        if(self->m_gui.getConsole().isVisible())
        {
            //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUOPEN));
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_StartTextInput();
        }
        else
        {
            //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUCLOSE));
            SDL_SetRelativeMouseMode(SDL_TRUE);
            SDL_StopTextInput();
        }
    });
    m_inputHandler.registerActionHandler(InputAction::Screenshot, [self](bool pressed){
        if(!pressed)
        {
            Com_TakeScreenShot();
        }
    });
    m_inputHandler.registerActionHandler(InputAction::Inventory, [self](bool pressed){
        self->m_controlState.m_guiInventory = pressed;
    });
    m_inputHandler.registerActionHandler(InputAction::SaveGame, [self](bool pressed){
        if(!pressed)
        {
            Game_Save(*self, "qsave.lua");
        }
    });
    m_inputHandler.registerActionHandler(InputAction::LoadGame, [self](bool pressed){
        if(!pressed)
        {
            Game_Load(*self, "qsave.lua");
        }
    });

    m_inputHandler.registerJoystickLookHandler([self](float dx, float dy){
        glm::vec3 rotation(dx, dy, 0);
        rotation *= -world::CameraRotationSpeed * self->getFrameTimeSecs();
        self->m_camera.rotate(rotation);
    });
    m_inputHandler.registerJoystickMoveHandler([self](float dx, float dy){
        self->m_controlState.m_moveLeft = dx < 0;
        self->m_controlState.m_moveRight = dx > 0;
        self->m_controlState.m_moveForward = dy < 0;
        self->m_controlState.m_moveBackward = dy > 0;

        self->m_world.m_character->applyJoystickMove(self->getFrameTimeSecs() * dx, self->getFrameTimeSecs() * dy);
        glm::vec3 rotation(dx, dy, 0);
        rotation *= -world::CameraRotationSpeed * self->getFrameTimeSecs();
        self->m_camera.rotate(rotation);
    });

    Game_InitGlobals(*this);
    renderer.initGlobals();
    m_world.m_audioEngine.resetSettings();
}

// First stage of initialization.

void Engine::initPre()
{
    /* Console must be initialized previously! some functions uses ConsoleInfo::instance().addLine before GL initialization!
     * Rendering activation may be done later. */

    // gui::FontManager::instance.reset(new gui::FontManager(this));
    m_gui.getConsole().init();

    m_scriptEngine["loadscript_pre"]();

    m_gameflow.init();

    frame_vertex_buffer.resize(render::InitFrameVertexBufferSize);
    frame_vertex_buffer_size_left = frame_vertex_buffer.size();

    m_gui.getConsole().setCompletionItems(m_scriptEngine.getGlobals());

    Com_Init();
    renderer.init();
    renderer.setCamera(&m_camera);
}

// Second stage of initialization.

void Engine::initPost()
{
    m_scriptEngine["loadscript_post"]();

    m_gui.getConsole().initFonts();

    sysInit();
}

void Engine::dumpRoom(const world::Room* r)
{
    if(!r)
        return;

    BOOST_LOG_TRIVIAL(debug) << boost::format("ROOM = %d, (%d x %d), bottom = %g, top = %g, pos(%g, %g)")
        % r->getId()
        % r->getSectors().shape()[0]
        % r->getSectors().shape()[1]
        % r->getBoundingBox().min[2]
        % r->getBoundingBox().max[2]
        % r->getModelMatrix()[3][0]
        % r->getModelMatrix()[3][1];
    BOOST_LOG_TRIVIAL(debug) << boost::format("flag = 0x%X, alt_room = %d, base_room = %d")
        % r->getFlags()
        % (r->getAlternateRoom() != nullptr ? r->getAlternateRoom()->getId() : -1)
        % (r->getBaseRoom() != nullptr ? r->getBaseRoom()->getId() : -1);
    for(const auto& column : r->getSectors())
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

    for(auto sm : r->getStaticMeshes())
    {
        BOOST_LOG_TRIVIAL(debug) << "static_mesh = " << sm->getId();
    }

    for(world::Object* object : r->getObjects())
    {
        if(world::Entity* ent = dynamic_cast<world::Entity*>(object))
        {
            BOOST_LOG_TRIVIAL(debug) << "entity: id = " << ent->getId() << ", model = " << ent->m_skeleton.getModel()->getId();
        }
    }
}

Engine::~Engine()
{
    m_scriptEngine.clearTasks();
    renderer.empty();
    m_world.empty();
    Com_Destroy();
    sysDestroy();

    /* no more renderings */
    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);

    m_world.m_audioEngine.closeDevice();

    /* free temporary memory */
    frame_vertex_buffer.clear();
    frame_vertex_buffer_size_left = 0;

    SDL_Quit();
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
    std::string level_name = getLevelName(m_gameflow.getLevelPath());

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

    m_gui.getConsole().notify(SYSNOTE_LOADED_PC_LEVEL);
    m_gui.getConsole().notify(SYSNOTE_ENGINE_VERSION, static_cast<int>(loader->m_gameVersion), buf.c_str());
    m_gui.getConsole().notify(SYSNOTE_NUM_ROOMS, m_world.m_rooms.size());

    return true;
}

bool Engine::loadMap(const std::string& name)
{
    BOOST_LOG_TRIVIAL(info) << "Loading map: " << name;

    if(!boost::filesystem::is_regular_file(name))
    {
        m_gui.getConsole().warning(SYSWARN_FILE_NOT_FOUND, name.c_str());
        return false;
    }

    m_gui.drawLoadScreen(0);

    m_camera.setCurrentRoom(nullptr);

    renderer.hideSkyBox();
    renderer.resetWorld();

    m_gameflow.setLevelPath(name);          // it is needed for "not in the game" levels or correct saves loading.

    m_gui.drawLoadScreen(50);

    m_world.empty();
    m_world.prepare();

    m_scriptEngine.clean();

    m_world.m_audioEngine.init();

    m_gui.drawLoadScreen(100);

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

    Game_Prepare(*this);

    m_scriptEngine.prepare();

    renderer.setWorld(&m_world);

    m_gui.drawLoadScreen(1000);

    m_gui.m_faders.start(gui::FaderType::LoadScreen, gui::FaderDir::In);
    m_gui.m_notifier.reset();

    return true;
}

int Engine::execCmd(const char *ch)
{
    std::vector<char> token(m_gui.getConsole().lineLength());
    const world::RoomSector* sect;
    FILE *f;

    while(ch != nullptr)
    {
        const char *pch = ch;

        ch = script::MainEngine::parse_token(ch, token.data());
        if(!strcmp(token.data(), "help"))
        {
            for(int i = SYSNOTE_COMMAND_HELP1; i <= SYSNOTE_COMMAND_HELP15; i++)
            {
                m_gui.getConsole().notify(i);
            }
        }
        else if(!strcmp(token.data(), "goto"))
        {
            m_controlState.m_freeLook = true;
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
                Game_Save(*this, token.data());
            }
            return 1;
        }
        else if(!strcmp(token.data(), "load"))
        {
            ch = script::MainEngine::parse_token(ch, token.data());
            if(NULL != ch)
            {
                Game_Load(*this, token.data());
            }
            return 1;
        }
        else if(!strcmp(token.data(), "exit"))
        {
            std::exit(0); //! @fixme This is ugly...
            return 1;
        }
        else if(!strcmp(token.data(), "cls"))
        {
            m_gui.getConsole().clean();
            return 1;
        }
        else if(!strcmp(token.data(), "spacing"))
        {
            ch = script::MainEngine::parse_token(ch, token.data());
            if(NULL == ch)
            {
                m_gui.getConsole().notify(SYSNOTE_CONSOLE_SPACING, m_gui.getConsole().spacing());
                return 1;
            }
            m_gui.getConsole().setSpacing(std::stof(token.data()));
            return 1;
        }
        else if(!strcmp(token.data(), "showing_lines"))
        {
            ch = script::MainEngine::parse_token(ch, token.data());
            if(NULL == ch)
            {
                m_gui.getConsole().notify(SYSNOTE_CONSOLE_LINECOUNT, m_gui.getConsole().visibleLines());
                return 1;
            }
            else
            {
                const auto val = atoi(token.data());
                if(val >= 2 && val <= m_screenInfo.h / m_gui.getConsole().lineHeight())
                {
                    m_gui.getConsole().setVisibleLines(val);
                    m_gui.getConsole().setCursorY(m_screenInfo.h - m_gui.getConsole().lineHeight() * m_gui.getConsole().visibleLines());
                }
                else
                {
                    m_gui.getConsole().warning(SYSWARN_INVALID_LINECOUNT);
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
        else if(!strcmp(token.data(), "r_allmodels"))
        {
            renderer.toggleDrawAllModels();
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
            if(const world::Room* r = renderer.camera()->getCurrentRoom())
            {
                sect = r->getSectorXYZ(renderer.camera()->getPosition());
                m_gui.getConsole().printf("ID = %d, x_sect = %d, y_sect = %d", r->getId(), static_cast<int>(r->getSectors().shape()[0]), static_cast<int>(r->getSectors().shape()[1]));
                if(sect)
                {
                    m_gui.getConsole().printf("sect(%d, %d), inpenitrable = %d, r_up = %d, r_down = %d",
                                               static_cast<int>(sect->index_x),
                                               static_cast<int>(sect->index_y),
                                               static_cast<int>(sect->ceiling == world::MeteringWallHeight || sect->floor == world::MeteringWallHeight),
                                               static_cast<int>(sect->sector_above != nullptr), static_cast<int>(sect->sector_below != nullptr));
                    for(uint32_t i = 0; i < sect->owner_room->getStaticMeshes().size(); i++)
                    {
                        m_gui.getConsole().printf("static[%d].object_id = %d", i, sect->owner_room->getStaticMeshes()[i]->getId());
                    }
                    for(world::Object* object : sect->owner_room->getObjects())
                    {
                        if(world::Entity* e = dynamic_cast<world::Entity*>(object))
                        {
                            m_gui.getConsole().printf("object[entity](%d, %d, %d).object_id = %d", static_cast<int>(e->m_transform[3][0]), static_cast<int>(e->m_transform[3][1]), static_cast<int>(e->m_transform[3][2]), e->getId());
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
                m_gui.getConsole().clean();
                m_gui.getConsole().addText(buf.data(), gui::FontStyle::ConsoleInfo);
            }
            else
            {
                m_gui.getConsole().addText("Not available =(", gui::FontStyle::ConsoleWarning);
            }
            return 1;
        }
        else if(token[0])
        {
            m_gui.getConsole().addLine(pch, gui::FontStyle::ConsoleEvent);
            try
            {
                m_scriptEngine.doString(pch);
            }
            catch(lua::RuntimeError& error)
            {
                m_gui.getConsole().addLine(error.what(), gui::FontStyle::ConsoleWarning);
            }
            catch(lua::LoadError& error)
            {
                m_gui.getConsole().addLine(error.what(), gui::FontStyle::ConsoleWarning);
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

    m_gui.m_textlineManager.add(&system_fps);
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
        m_screenInfo.fps = 20.0f / util::toSeconds(fpsTime);
        char tmp[16];
        snprintf(tmp, 16, "%.1f", m_screenInfo.fps);
        system_fps.text = tmp;
        fpsCycles = 0;
        fpsTime = util::Duration(0);
    }
}

void Engine::initConfig(const std::string& filename)
{
    initDefaultGlobals();

    if(boost::filesystem::is_regular_file(filename))
    {
        script::ScriptEngine state(this);
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

        state.parseScreen(m_screenInfo);
        state.parseRender(renderer.settings());
        state.parseAudio(m_world.m_audioEngine.settings());
        state.parseConsole(m_gui.getConsole());
        state.parseControls(m_inputHandler);
        state.parseSystem(m_systemSettings);
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "Could not find " << filename;
    }
}
} // namespace engine
