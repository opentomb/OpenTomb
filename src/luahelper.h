#pragma once

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/is_function.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/function_types/parameter_types.hpp>

#include <boost/preprocessor/repeat.hpp>
#include <boost/optional.hpp>

#include <boost/fusion/tuple.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/functional.hpp>
#include <boost/fusion/mpl.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/map.hpp>

#include <limits>
#include <string>
#include <iostream>

#include <lua.h>

namespace lua {

namespace MPL = boost::mpl;
namespace FUS = boost::fusion;

template<typename... Types>
using Tuple = FUS::tuple<Types...>;

template<typename T>
using Optional = boost::optional<T>;

template<typename... Types>
using OptionalTuple = Optional<Tuple<Types...>>;

using Bool = Optional<bool>;
using Char = Optional<char>;
using Int = Optional<int>;
using UInt = Optional<unsigned int>;
using Float = Optional<float>;

using Int8 = Optional<int8_t>;
using Int16 = Optional<int16_t>;
using Int32 = Optional<int32_t>;
using Int64 = Optional<int64_t>;
using UInt8 = Optional<uint8_t>;
using UInt16 = Optional<uint16_t>;
using UInt32 = Optional<uint32_t>;
using UInt64 = Optional<uint64_t>;

using String = Optional<std::string>;

namespace {
const auto None = boost::none;
}

inline lua_State*& state()
{
    static lua_State* s = nullptr;
    return s;
}

/****************************
 * Error handling
 ***************************/
class ArgCheckError : public std::runtime_error
{
public:
    explicit ArgCheckError(const std::string& msg) : runtime_error(msg)
    {
    }
};

class FunctionNotFoundError : public std::runtime_error
{
public:
    explicit FunctionNotFoundError(const std::string& msg) : runtime_error(msg)
    {
    }
};

namespace detail {

/****************************
 * Type traits for parameters and return values
 ***************************/

template<typename T>
struct TypeTraits;

// Either pushes nil, or the value, taking care of nested tuples
template<typename T>
struct TypeTraits<boost::optional<T>>
{
    static boost::optional<T> arg(lua_State* lua, int n) {
        if(lua_isnoneornil(lua, n))
            return boost::none;
        return TypeTraits<T>::arg(lua, n);
    }

    static int push(lua_State* lua, const boost::optional<T>& value) {
        if(!value) {
            lua_pushnil(lua);
            return 1;
        }
        return TypeTraits<T>::push(lua, *value);
    }
};


// Unpacks the tuple, taking care of optionals and nested tuples
template<typename... TT>
struct TypeTraits<FUS::tuple<TT...>>
{
    struct Dispatcher {
        lua_State* state;

        Dispatcher(lua_State* lua) : state(lua)
        {}

        template<typename T>
        int operator()(int n, const T& t) const
        {
            return n + TypeTraits<T>::push(state, t);
        }
    };

    static int push(lua_State* lua, const FUS::tuple<TT...>& values) {
        return FUS::accumulate(values, int(0), Dispatcher(lua));
    }
};


// Either pushes nil, or the value, taking care of nested tuples
template<typename T>
struct TypeTraits<const T&>
{
    static T arg(lua_State* lua, int n) {
        return TypeTraits<T>::arg(lua, n);
    }

    static int push(lua_State* lua, const T& value) {
        return TypeTraits<T>::push(lua, value);
    }
};

template<class T>
struct SIntegralTypeTraits
{
    static_assert(std::is_integral<T>::value && std::is_signed<T>::value, "T must be signed integral");

    using CType = T;

    static CType arg(lua_State* lua, int n) {
        auto result = lua_tointeger(lua, n);
        if(result > std::numeric_limits<CType>::max() || result < std::numeric_limits<CType>::min())
            throw ArgCheckError("Parameter value out of valid bounds");
        return result;
    }

    static int push(lua_State* lua, CType value) {
        lua_pushinteger(lua, value);
        return 1;
    }
};

template<class T>
struct UIntegralTypeTraits
{
    static_assert(std::is_integral<T>::value && !std::is_signed<T>::value, "T must be unsigned integral");

    using CType = T;

    static CType arg(lua_State* lua, int n) {
        auto result = lua_tounsigned(lua, n);
        if(result > std::numeric_limits<CType>::max() || result < std::numeric_limits<CType>::min())
            throw ArgCheckError("Parameter value out of valid bounds");
        return result;
    }

    static int push(lua_State* lua, CType value) {
        lua_pushunsigned(lua, value);
        return 1;
    }
};

template<>
struct TypeTraits<bool>
{
    using CType = bool;

    static CType arg(lua_State* lua, int n) {
        return lua_toboolean(lua, n);
    }

    static int push(lua_State* lua, CType value) {
        lua_pushboolean(lua, value);
        return 1;
    }
};

template<>
struct TypeTraits<std::string>
{
    static std::string arg(lua_State* lua, int n) {
        return lua_tostring(lua, n);
    }

    static int push(lua_State* lua, const std::string& value) {
        lua_pushstring(lua, value.c_str());
        return 1;
    }
};

template<> struct TypeTraits<char> : public SIntegralTypeTraits<char> {};

template<> struct TypeTraits<int8_t> : public SIntegralTypeTraits<int8_t> {};
template<> struct TypeTraits<int16_t> : public SIntegralTypeTraits<int16_t> {};
template<> struct TypeTraits<int32_t> : public SIntegralTypeTraits<int32_t> {};
template<> struct TypeTraits<int64_t> : public SIntegralTypeTraits<int64_t> {};

template<> struct TypeTraits<uint8_t> : public UIntegralTypeTraits<uint8_t> {};
template<> struct TypeTraits<uint16_t> : public UIntegralTypeTraits<uint16_t> {};
template<> struct TypeTraits<uint32_t> : public UIntegralTypeTraits<uint32_t> {};
template<> struct TypeTraits<uint64_t> : public UIntegralTypeTraits<uint64_t> {};

template<>
struct TypeTraits<float>
{
    using CType = float;

    static CType arg(lua_State* lua, int n) {
        return lua_tonumber(lua, n);
    }

    static int push(lua_State* lua, CType value) {
        lua_pushnumber(lua, value);
        return 1;
    }
};


/****************************
 * Call traits for unpacking the parameters and calling the typed function
 ***************************/

template<typename Result>
struct ReturnTraits
{
    template<typename... Args>
    static int doCall(Result (*fun)(Args...), lua_State* lua, const Args&... args) {
        return TypeTraits<Result>::push(lua, fun(args...));
    }
};

template<>
struct ReturnTraits<void>
{
    template<typename... Args>
    static int doCall(void (*fun)(Args...), lua_State* /*lua*/, const Args&... args) {
        fun(args...);
        return 0;
    }
};

template<int N, typename Result, typename F>
struct CallTraits;

template<typename Result, typename F>
struct CallTraits<0, Result, F>
{

    template<typename... Args>
    static int call(Result (*fun)(Args...), lua_State* lua) {
        static_assert(sizeof...(Args) == 0, "Ooops");

        return ReturnTraits<Result>::template doCall<Args...>(fun, lua);
    }
};

#define LUA_ARG(z, n, data) \
    , TypeTraits<typename MPL::at<ArgVec,MPL::int_<n>>::type>::arg(lua, (n)+1)

#define UNPACK_AGS_AND_CALL(n) \
    BOOST_PP_REPEAT(n, LUA_ARG, ())

#define IMPLEMENT_CALLTRAIT(n) \
    template<typename Result, typename F> \
    struct CallTraits<n, Result, F> \
    { \
        template<typename... Args> \
        static int call(Result (*fun)(Args...), lua_State* lua) { \
            static_assert(sizeof...(Args) == n, "Ooops"); \
    \
            using ArgVec = typename MPL::vector<Args...>::type; \
            return ReturnTraits<Result>::template doCall<Args...>(fun, lua UNPACK_AGS_AND_CALL(n)); \
        } \
    }

IMPLEMENT_CALLTRAIT(1);
IMPLEMENT_CALLTRAIT(2);
IMPLEMENT_CALLTRAIT(3);
IMPLEMENT_CALLTRAIT(4);
IMPLEMENT_CALLTRAIT(5);
IMPLEMENT_CALLTRAIT(6);
IMPLEMENT_CALLTRAIT(7);
IMPLEMENT_CALLTRAIT(8);
IMPLEMENT_CALLTRAIT(9);
IMPLEMENT_CALLTRAIT(10);
IMPLEMENT_CALLTRAIT(11);
IMPLEMENT_CALLTRAIT(12);
IMPLEMENT_CALLTRAIT(13);
IMPLEMENT_CALLTRAIT(14);
IMPLEMENT_CALLTRAIT(15);
IMPLEMENT_CALLTRAIT(16);
IMPLEMENT_CALLTRAIT(17);
IMPLEMENT_CALLTRAIT(18);
IMPLEMENT_CALLTRAIT(19);
IMPLEMENT_CALLTRAIT(20);

#undef LUA_ARG
#undef UNPACK_AGS_AND_CALL
#undef IMPLEMENT_CALLTRAIT

template<typename F>
struct Dispatcher {
    static_assert(!boost::function_types::is_function<F,boost::function_types::variadic>::value, "F must not be a variadic function");

    using ResultType = typename boost::function_types::result_type<F>::result_type::type;
    using ParameterTypes = typename boost::function_types::parameter_types<F>::parameter_types;

    F m_function;

    explicit Dispatcher(F fun)
        : m_function(fun)
    {
    }

    int operator()(lua_State* lua) {
        const int providedArgs = lua_gettop(lua);
        static constexpr int ArgCount = MPL::size<ParameterTypes>::value;
        if(providedArgs < ArgCount) {
            throw FunctionNotFoundError("Too few parameters provided");
            return 0;
        }

        state() = lua;
        int n = CallTraits<MPL::size<ParameterTypes>::value, ResultType, F>::call(m_function, lua);
        state() = nullptr;

        return n;
    }
};

template<typename... F>
struct MultiDispatcher
{
    template<typename FT>
    using CountArgs = MPL::size< typename boost::function_types::parameter_types<FT>::parameter_types >;

    using FunctionVector = MPL::vector<F*...>;
    using FunctionSet = typename FUS::result_of::as_set<FunctionVector>::type;

    using MapTypes = typename MPL::fold<
        FunctionSet,
        MPL::vector<>,
        MPL::push_back<
            MPL::_1,
            FUS::pair< CountArgs<MPL::_2>, MPL::_2>
        >
    >::type;

    using FunctionMap = typename FUS::result_of::as_map<MapTypes>::type;

    FunctionMap m_functions;

    explicit MultiDispatcher(F*... funcs)
        : m_functions{FUS::as_map<FunctionMap>({funcs...})}
    {
    }

    template<typename N>
    int doCall( lua_State* lua, const MPL::true_& )
    {
        auto fun = FUS::at_key<N>(m_functions);
        auto disp = Dispatcher<decltype(fun)>(fun);
        return disp(lua);
    }

    template<typename N>
    int doCall( lua_State* /*lua*/, const MPL::false_& )
    {
        throw FunctionNotFoundError("A function signature is missing");
        return 0;
    }

    template<typename N>
    int doCall(lua_State* lua) {
        using SigFound = typename FUS::result_of::has_key<FunctionMap, N>::type;
        return doCall<N>( lua, SigFound() );
    }

    int operator()(lua_State* lua) {
        const auto argCount = lua_gettop(lua);
        try {
            switch(argCount) {

#define LUA_RUNTIME_TO_TEMPLATE(z, n, data) \
                case n: return doCall<MPL::long_<n>>(lua);

#define LUA_TRANSLATE_CALLS(n) \
    BOOST_PP_REPEAT(n, LUA_RUNTIME_TO_TEMPLATE, ())

                LUA_TRANSLATE_CALLS(21)

                default: throw FunctionNotFoundError("Only functions with up to 20 parameters are possible");
#undef LUA_TRANSLATE_CALLS
#undef LUA_RUNTIME_TO_TEMPLATE
            }
        }
        catch(std::runtime_error& ex) {
            std::cerr << ex.what();
            return 0;
        }
    }
};

}

template<typename F0, typename... Fn>
inline detail::MultiDispatcher<F0, Fn...> makeDispatcher(F0* fun0, Fn*... funN) {
    return detail::MultiDispatcher<F0, Fn...>(fun0, funN...);
}

#define WRAP_FOR_LUA(funcs...) \
    [](lua_State* state)->int{ return ::lua::makeDispatcher(funcs)(state); }

}
