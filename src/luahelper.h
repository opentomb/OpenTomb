#pragma once

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/is_function.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <limits>

class LuaArgCheckError : public std::runtime_error
{
public:
    explicit LuaArgCheckError(const std::string& msg) : runtime_error(msg)
    {
    }
};


template<typename T>
struct LuaTypeTraits;

template<>
struct LuaTypeTraits<int>
{
    using CType = int;

    static CType fromLuaArg(lua_State* lua, int n) {
        auto result = lua_tointeger(lua, n);
        if(result > std::numeric_limits<CType>::max() || result < std::numeric_limits<CType>::min())
            throw LuaArgCheckError("Parameter value out of valid bounds");
        return static_cast<CType>(result);
    }

    static CType toLuaResult(lua_State* lua, CType value) {
        lua_pushinteger(lua, value);
    }
};

template<>
struct LuaTypeTraits<unsigned int>
{
    using CType = unsigned int;

    static CType fromLuaArg(lua_State* lua, int n) {
        auto result = lua_tounsigned(lua, n);
        if(result > std::numeric_limits<CType>::max() || result < std::numeric_limits<CType>::min())
            throw LuaArgCheckError("Parameter value out of valid bounds");
        return static_cast<CType>(result);
    }

    static CType toLuaResult(lua_State* lua, CType value) {
        lua_pushunsigned(lua, value);
    }
};


template<int N, typename Result, typename F>
struct LuaCallTraits;

template<typename Result, typename F>
struct LuaCallTraits<0, Result, F>
{

    template<typename... Args>
    static void call(Result (*fun)(Args...), lua_State* lua) {
        static_assert(sizeof...(Args) == 0, "Ooops");

        Result result = fun();
        LuaTypeTraits<Result>::toLuaResult(result);
    }
};

template<typename Result, typename F>
struct LuaCallTraits<1, Result, F>
{

    template<typename... Args>
    static void call(Result (*fun)(Args...), lua_State* lua) {
        static_assert(sizeof...(Args) == 1, "Ooops");

        using ArgVec = typename boost::mpl::vector1<Args...>::type;
        using Arg0Type = typename boost::mpl::at<ArgVec,boost::mpl::int_<0>>::type;
        auto arg0 = LuaTypeTraits<Arg0Type>::fromLuaArg(lua, 1);
        Result result = fun(arg0);
        LuaTypeTraits<Result>::toLuaResult(lua, result);
    }
};

template<typename F>
struct LuaDispatcher {
    static_assert(boost::function_types::is_function<F>::value, "F must be a function");
    static_assert(!boost::function_types::is_function<F,boost::function_types::variadic>::value, "F must not be a variadic function");
    static_assert(!boost::function_types::is_member_function_pointer<F>::value, "F must not be a member function");

    using ResultType = typename boost::function_types::result_type<F>::result_type::type;
    using ParameterTypes = typename boost::function_types::parameter_types<F>::parameter_types;

    F* m_function;

    explicit LuaDispatcher(F* fun)
        : m_function(fun)
    {
    }

    int operator()(lua_State* lua) {
        const int providedArgs = lua_gettop(lua);
        static constexpr int ArgCount = boost::mpl::size<ParameterTypes>::value;
        if(providedArgs < ArgCount)
            return 0;
        else if(providedArgs > ArgCount)
            return 0;

        LuaCallTraits<boost::mpl::size<ParameterTypes>::value, ResultType, F>::call(m_function, lua);

        return 1;
    }
};

template<typename F>
inline LuaDispatcher<F> makeDispatcher(F* fun) {
    return LuaDispatcher<F>(fun);
}

#define DISPATCH_LUA(function) \
    [](lua_State* state)->int{ return makeDispatcher(function)(state); }
