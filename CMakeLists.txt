# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

cmake_minimum_required(VERSION 3.0)
 
include("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/stohrendorf/hunter/archive/edcdea005a0f5c2beca2c9f4ecf4dda254358563.zip"
    SHA1 "b931e91e90cad55a11131087919481cbcfb7b0e9"
)

project(OpenTomb)

include(ExternalProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if(NOT LINUX)
    option(DEFAULT_EXTERNAL_HUNTER "Always ignore system-provided libraries." ON)
else()
    option(DEFAULT_EXTERNAL_HUNTER "Always ignore system-provided libraries." OFF)
endif()

message(STATUS "Always ignore system-provided libraries: ${DEFAULT_EXTERNAL_HUNTER}")

# Dependency: OpenAL
find_package(OpenAL)
add_library(OpenAL INTERFACE)

option(USE_OPENAL_FLOAT "Use OpenAL Soft floating-point extensions." ON)

if(USE_OPENAL_FLOAT)
    target_compile_definitions(OpenAL INTERFACE -DAUDIO_OPENAL_FLOAT)
endif(USE_OPENAL_FLOAT)

if(NOT OPENAL_FOUND)
    ExternalProject_Add(
        OpenAL-Ext
        PREFIX ${CMAKE_SOURCE_DIR}/3rdparty/OpenAL
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        GIT_REPOSITORY git://repo.or.cz/openal-soft.git
        GIT_TAG openal-soft-1.17.0
    )
    target_include_directories(OpenAL INTERFACE ${CMAKE_SOURCE_DIR}/3rdparty/OpenAL/include)
    target_link_libraries(OpenAL INTERFACE ${CMAKE_SOURCE_DIR}/3rdparty/OpenAL/libopenal.so)
else()
    target_include_directories(OpenAL INTERFACE ${OPENAL_INCLUDE_DIR})
    target_link_libraries(OpenAL INTERFACE ${OPENAL_LIBRARY})
endif()


# Dependency: Bullet
option(SKIP_SYSTEM_BULLET "Skip the sytem-provided Bullet installation and use a custom built 2.83.6." ON)


if(SKIP_SYSTEM_BULLET)
    hunter_add_package(Bullet)
    find_package(Bullet CONFIG REQUIRED)
else()
    find_package(Bullet REQUIRED)
endif()
add_library(Bullet INTERFACE)


# Dependency: Freetype
option(SKIP_SYSTEM_FREETYPE "Skip the sytem-provided Freetype installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_FREETYPE)
    hunter_add_package(Freetype)
    find_package(Freetype CONFIG REQUIRED)
else()
    find_package(Freetype REQUIRED)
endif()
add_library(Freetype INTERFACE)
target_include_directories(Freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(Freetype INTERFACE ${FREETYPE_LIBRARIES})


# Dependency: OpenGL
find_package(OpenGL REQUIRED)
add_library(OpenGL INTERFACE)
target_include_directories(OpenGL INTERFACE ${OPENGL_INCLUDE_DIR})
target_link_libraries(OpenGL INTERFACE ${OPENGL_LIBRARIES})


# Dependency: GLEW
find_package(GLEW REQUIRED)
add_library(GLEW INTERFACE)
target_include_directories(GLEW INTERFACE ${GLEW_INCLUDE_DIRS})
target_link_libraries(GLEW INTERFACE ${GLEW_LIBRARIES})


# Dependency: ZLIB
option(SKIP_SYSTEM_ZLIB "Skip the sytem-provided ZLIB installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_ZLIB)
    hunter_add_package(ZLIB)
    find_package(ZLIB CONFIG REQUIRED)
else()
    find_package(ZLIB REQUIRED)
endif()
add_library(ZLIB INTERFACE)
target_include_directories(ZLIB INTERFACE ${ZLIB_INCLUDE_DIR})
target_link_libraries(ZLIB INTERFACE ${ZLIB_LIBRARY})


# Dependency: Lua
find_package(Lua REQUIRED)
add_library(Lua INTERFACE)
target_include_directories(Lua INTERFACE ${LUA_INCLUDE_DIR})
target_compile_definitions(Lua INTERFACE -DLUA_COMPAT_APIINTCASTS)
target_link_libraries(Lua INTERFACE ${LUA_LIBRARIES})


# Dependency: SDL2
find_package(SDL2 REQUIRED)
add_library(SDL2 INTERFACE)
target_include_directories(SDL2 INTERFACE ${SDL2_INCLUDE_DIR})
target_link_libraries(SDL2 INTERFACE ${SDL2_LIBRARY})


# Dependency: SDL2Image
find_package(SDL2Image REQUIRED)
add_library(SDL2_IMAGE INTERFACE)
target_include_directories(SDL2_IMAGE INTERFACE ${SDL2_INCLUDE_DIR})
target_include_directories(SDL2_IMAGE INTERFACE ${SDL2IMAGE_INCLUDE_DIR})
target_link_libraries(SDL2_IMAGE INTERFACE ${SDL2IMAGE_LIBRARY})


# Dependency: GLM
find_package(GLM REQUIRED)
add_library(GLM INTERFACE)
target_include_directories(GLM INTERFACE ${GLM_INCLUDE_DIR})
target_compile_definitions(GLM INTERFACE -DGLM_FORCE_RADIANS)


# Dependency: Boost
option(SKIP_SYSTEM_BOOST "Skip the sytem-provided Boost installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_BOOST)
    hunter_add_package(Boost COMPONENTS system log filesystem)
    find_package(Boost CONFIG REQUIRED system log filesystem)
else()
    find_package(Boost REQUIRED system log filesystem)
endif()
add_library(Boost INTERFACE)

target_include_directories(Boost INTERFACE ${Boost_INCLUDE_DIR})
target_link_libraries(Boost INTERFACE Boost::system Boost::log Boost::filesystem)
target_compile_definitions(Boost INTERFACE -DBOOST_LOG_DYN_LINK -DBOOST_ALL_NO_LIB)


# Dependency: OGG
find_package(OGG REQUIRED)
include_directories(${OGG_INCLUDE_DIR})


# Dependency: Vorbis
find_package(Vorbis REQUIRED)
include_directories(${VORBIS_INCLUDE_DIR})


# Dependency: VorbisFile
find_package(VorbisFile REQUIRED)
include_directories(${VORBISFILE_INCLUDE_DIR})


# Dependency: SndFile
find_package(SndFile REQUIRED)
include_directories(${SNDFILE_INCLUDE_DIR})


add_subdirectory(doc)
add_subdirectory(src)
