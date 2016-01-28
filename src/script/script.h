#pragma once

#include "LuaState.h"

#include "audio/audio.h"
#include "util/helpers.h"
#include "world/object.h"

#include <boost/optional.hpp>

#include <cctype>

#define CVAR_NAME_SIZE 32
#define CVAR_LUA_TABLE_NAME "cvars"

namespace gui
{
class Console;
} // namespace gui

namespace world
{
class Entity;
} // namespace world

namespace audio
{
struct Settings;
enum class StreamType;
enum class StreamMethod;
} // namespace audio

namespace engine
{
class InputHandler;
struct SystemSettings;
struct ScreenInfo;
class Engine;
} // namespace engine

namespace render
{
struct RenderSettings;
} // namespace render

namespace script
{
class ScriptEngine
{
private:
    static const char* const ScriptEngineReferenceVarName;
public:
    explicit ScriptEngine(engine::Engine* engine);

    static ScriptEngine* getEngine(lua_State* L)
    {
        lua_getglobal(L, ScriptEngineReferenceVarName);
        ScriptEngine** ref = static_cast<ScriptEngine**>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        if(ref == nullptr)
            return nullptr;
        return *ref;
    }

    engine::Engine* getEngine() const
    {
        return m_engine;
    }

    virtual ~ScriptEngine() = default;

    void doFile(const std::string& filename);

    void doString(const std::string& script)
    {
        m_state.doString(script);
    }

    lua::Value operator[](lua::String key) const
    {
        return get(key);
    }

    lua::Value get(lua::String key) const
    {
        return m_state[key];
    }

    template<typename K, typename V>
    ScriptEngine& set(const K& key, V&& value)
    {
        m_state.set(key, std::forward<V>(value));
        return *this;
    }

    template<typename... T>
    lua::Value call(const std::string& funcName, T&&... args) const
    {
        return get(funcName.c_str()).call(std::forward<T>(args)...);
    }

    // Simple override to register both upper- and lowercase versions of function name.
    template<typename R, typename... Args>
    void registerC(const std::string& func_name, std::function<R(engine::Engine&, Args...)> func)
    {
        auto self = this;
        auto dispatcher = [self, func](Args&&... args) { return func(*self->m_engine, std::forward<Args>(args)...); };
        registerC(func_name, std::function<R(Args...)>(dispatcher));
    }

    template<typename R, typename... Args>
    void registerC(const std::string& func_name, std::function<R(Args...)> func)
    {
        std::string uc, lc;
        for(char c : func_name)
        {
            lc += std::tolower(c);
            uc += std::toupper(c);
        }

        m_state.set(func_name.c_str(), func);
        m_state.set(lc.c_str(), func);
        m_state.set(uc.c_str(), func);
    }

    template<typename R, typename... Args>
    void registerC(const std::string& func_name, R (*func)(Args...))
    {
        registerC(func_name, std::function<R(Args...)>(func));
    }

    void registerRawC(const std::string& func_name, lua_CFunction func)
    {
        std::string uc, lc;
        for(char c : func_name)
        {
            lc += std::tolower(c);
            uc += std::toupper(c);
        }

        lua_register(m_state.getState(), func_name.c_str(), func);
        lua_register(m_state.getState(), lc.c_str(), func);
        lua_register(m_state.getState(), uc.c_str(), func);
    }

    template<typename R, typename... Args>
    void registerFunction(const std::string& func_name, std::function<R(engine::Engine&, Args...)> func)
    {
        auto self = this;
        auto dispatcher = [self, func](Args&&... args) { return func(*self->m_engine, std::forward<Args>(args)...); };
        registerFunction(func_name, std::function<R(Args&&...)>(dispatcher));
    }

    template<typename R, typename... Args>
    void registerFunction(const std::string& func_name, std::function<R(Args...)> func)
    {
        m_state.set(func_name.c_str(), func);
    }

    template<typename R, typename... Args>
    void registerFunction(const std::string& func_name, R(*func)(Args...))
    {
        registerFunction(func_name, std::function<R(Args...)>(func));
    }

    void registerRaw(const std::string& func_name, lua_CFunction func)
    {
        lua_register(m_state.getState(), func_name.c_str(), func);
    }

    void exposeConstants();
    std::vector<std::string> getGlobals();

    void parseScreen(engine::ScreenInfo& sc) const;
    void parseRender(render::RenderSettings& rs) const;
    void parseAudio(audio::Settings& as) const;
    void parseConsole(gui::Console& cn) const;
    void parseControls(engine::InputHandler& cs) const;
    void parseSystem(engine::SystemSettings& ss) const;

protected:
    void checkStack();

private:
    engine::Engine* m_engine;
    lua::State m_state;

    // Print function override. Puts printed string into console.
    static int print(lua_State *state);
    static int panic(lua_State *state);
};

class MainEngine : public ScriptEngine
{
public:
    explicit MainEngine(engine::Engine* engine)
        : ScriptEngine(engine)
    {
        registerMainFunctions();
    }

    void clearTasks() const
    {
        call("clearTasks");
    }

    void prepare()
    {
        call("fe_Prepare");
    }

    void clean()
    {
        call("st_Clear");
        call("tlist_Clear");
        call("entfuncs_Clear");
        call("fe_Clear");

        call("clearAutoexec");
    }

    void doTasks(util::Duration time)
    {
        set("FRAME_TIME", static_cast<lua::Number>(util::toSeconds(time)));

        call("doTasks");
        call("clearKeys");
    }

    // System Lua functions. Not directly called from scripts.

    void loopEntity(world::ObjectId object_id);
    void execEntity(int id_callback, world::ObjectId id_object, const boost::optional<world::ObjectId>& id_activator = boost::none);
    void execEffect(int id, const boost::optional<world::ObjectId>& caller = boost::none, const boost::optional<world::ObjectId>& operand = boost::none);

    void addKey(int keycode, bool state);

    static void bindKey(engine::Engine& engine, int act, int primary, lua::Value secondary);

    // Helper Lua functions. Not directly called from scripts.

    bool getOverridedSamplesInfo(int& num_samples, int& num_sounds, std::string& sample_name_mask);
    bool getOverridedSample(int sound_id, int& first_sample_number, int& samples_count);

    boost::optional<audio::SoundId> getGlobalSound(audio::GlobalSoundId global_sound_id);
    int  getSecretTrackNumber();
    int  getNumTracks();
    bool getSoundtrack(int track_index, char *track_path, audio::StreamMethod *load_method, audio::StreamType *stream_type);
    std::string getLoadingScreen(int level_index);
    std::string getString(int string_index);
    bool getSysNotify(int string_index, size_t string_size, char *buffer);

    // Parsing functions - both native and Lua. Not directly called from scripts.

    static const char *parse_token(const char *data, char *token);

    static float parseFloat(const char **ch);
    static int   parseInt(char **ch);

private:
    void registerMainFunctions();
};
}
