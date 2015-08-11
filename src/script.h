#pragma once

#include <cctype>

#include <LinearMath/btScalar.h>

#include <lua.hpp>
#include "LuaState.h"

#define CVAR_NAME_SIZE 32
#define CVAR_LUA_TABLE_NAME "cvars"

struct ScreenInfo;
struct ConsoleInfo;
struct Entity;

struct AudioSettings;
struct ControlSettings;
struct RenderSettings;
struct SystemSettings;

namespace script
{
    class ScriptEngine
    {
    public:
        ScriptEngine()
        {
            exposeConstants();
            registerFunction("print", &ScriptEngine::print);
            lua_atpanic(m_state.getState(), &ScriptEngine::panic);
        }

        virtual ~ScriptEngine() = default;

        void doFile(const std::string& filename)
        {
            m_state.doFile(filename);
        }

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
        ScriptEngine& set(const K& key, const V& value)
        {
            m_state.set(key, value);
            return *this;
        }

        template<typename... T>
        lua::Value call(const std::string& funcName, const T&... args) const
        {
            return get(funcName.c_str()).call(args...);
        }

        // Simple override to register both upper- and lowercase versions of function name.
        template<typename Function>
        inline void registerC(const std::string& func_name, Function func)
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

        inline void registerC(const std::string& func_name, int(*func)(lua_State*))
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

        inline void registerFunction(const std::string& func_name, int(*func)(lua_State*))
        {
            lua_register(m_state.getState(), func_name.c_str(), func);
        }

        void exposeConstants();
        std::vector<std::string> getGlobals();

        void parseScreen(ScreenInfo *sc);
        void parseRender(RenderSettings *rs);
        void parseAudio(AudioSettings *as);
        void parseConsole(ConsoleInfo *cn);
        void parseControls(ControlSettings *cs);
        void parseSystem(SystemSettings *ss);

    protected:
        void checkStack();

    private:
        lua::State m_state;

        // Print function override. Puts printed string into console.
        static int print(lua_State *state);
        static int panic(lua_State *state);
    };

    class MainEngine : public ScriptEngine
    {
    public:
        MainEngine() : ScriptEngine()
        {
            registerMainFunctions();
            doFile("scripts/loadscript.lua");
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

        void doTasks(btScalar time)
        {
            set("frame_time", time);
            call("doTasks");
            call("clearKeys");
        }

        // System Lua functions. Not directly called from scripts.

        void loopEntity(int object_id);
        void execEntity(int id_callback, int id_object, int id_activator = -1);
        void execEffect(int id, int caller = -1, int operand = -1);

        void addKey(int keycode, bool state);

        static void bindKey(int act, int primary, lua::Value secondary);

        // Helper Lua functions. Not directly called from scripts.

        bool getOverridedSamplesInfo(int *num_samples, int *num_sounds, char *sample_name_mask);
        bool getOverridedSample(int sound_id, int *first_sample_number, int *samples_count);

        int  getGlobalSound(int global_sound_id);
        int  getSecretTrackNumber();
        int  getNumTracks();
        bool getSoundtrack(int track_index, char *track_path, int *load_method, int *stream_type);
        bool getLoadingScreen(int level_index, char *pic_path);
        bool getString(int string_index, size_t string_size, char *buffer);
        bool getSysNotify(int string_index, size_t string_size, char *buffer);

        // Parsing functions - both native and Lua. Not directly called from scripts.

        static const char *parse_token(const char *data, char *token);

        static float parseFloat(const char **ch);
        static int   parseInt(char **ch);

    private:
        void registerMainFunctions();
    };
}

extern script::MainEngine engine_lua;
