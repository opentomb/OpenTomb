# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

cmake_minimum_required(VERSION 3.2)
 
include("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/stohrendorf/hunter/archive/faff7fb5ef3130ac6ae19d847b2bfcba9629a924.zip"
    SHA1 "d99599de86bad8ed940c90f9b7e2a1647c4fd37e"
)

project(OpenTomb)

include(ExternalProject)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

if(WIN32 OR APPLE)
    option(DEFAULT_EXTERNAL_HUNTER "Always ignore system-provided libraries." ON)
else()
    option(DEFAULT_EXTERNAL_HUNTER "Always ignore system-provided libraries." OFF)
endif()

message(STATUS "Always ignore system-provided libraries: ${DEFAULT_EXTERNAL_HUNTER}")


function(GROUP_FILES)
    foreach(fn ${ARGV}) 
        get_filename_component(parentDir "${fn}" PATH)

        # change /'s to \\'s, and reduce redundant // or \\.
        string(REGEX REPLACE "[\\/]+" "\\\\" grp "${parentDir}")

        source_group("${grp}" FILES "${fn}")
    endforeach()
endfunction()


# Dependency: OpenAL
option(SKIP_SYSTEM_OPENAL "Skip the sytem-provided OpenAL installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_OPENAL)
    hunter_add_package(OpenAL)
endif()
find_package(OpenAL REQUIRED)
add_library(OpenAL INTERFACE)
target_include_directories(OpenAL INTERFACE ${OPENAL_INCLUDE_DIR})
target_link_libraries(OpenAL INTERFACE ${OPENAL_LIBRARY})

option(USE_OPENAL_FLOAT "Use OpenAL Soft floating-point extensions." ON)
if(USE_OPENAL_FLOAT)
    target_compile_definitions(OpenAL INTERFACE -DAUDIO_OPENAL_FLOAT)
endif(USE_OPENAL_FLOAT)


# Dependency: Bullet
option(SKIP_SYSTEM_BULLET "Skip the sytem-provided Bullet installation and use a custom built 2.83.6." ON)
if(SKIP_SYSTEM_BULLET)
    hunter_add_package(Bullet)
endif()
include(${CMAKE_SOURCE_DIR}/cmake/FindBullet.cmake) # Bullet's own BulletConfig.cmake is crap, and CMake's version doesn't look for "d" suffixes, only for "_d"
add_library(Bullet INTERFACE)
target_include_directories(Bullet INTERFACE ${BULLET_INCLUDE_DIRS})
target_link_libraries(Bullet INTERFACE ${BULLET_LIBRARIES})


# Dependency: Freetype
option(SKIP_SYSTEM_FREETYPE "Skip the sytem-provided Freetype installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_FREETYPE)
    hunter_add_package(Freetype)
    find_package(Freetype CONFIG REQUIRED)
else()
    find_package(Freetype MODULE REQUIRED)
    add_library(freetype INTERFACE)
    target_include_directories(freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(freetype INTERFACE ${FREETYPE_LIBRARIES})
endif()


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
option(SKIP_SYSTEM_ZLIB "Skip the sytem-provided ZLIB installation and use a custom built" ON)
if(SKIP_SYSTEM_ZLIB)
    hunter_add_package(ZLIB)
    find_package(ZLIB CONFIG REQUIRED)
else()
    find_package(ZLIB MODULE REQUIRED)
endif()

# Dependency: Lua
option(SKIP_SYSTEM_LUA "Skip the sytem-provided Lua installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_LUA)
    hunter_add_package(Lua)
    find_package(Lua CONFIG REQUIRED)
    add_library(Lua INTERFACE)
    target_link_libraries(Lua INTERFACE Lua::lua_lib)
else()
    find_package(Lua MODULE REQUIRED)
    add_library(Lua INTERFACE)
    target_link_libraries(Lua INTERFACE ${LUA_LIBRARIES})
endif()
target_include_directories(Lua INTERFACE ${LUA_INCLUDE_DIR})


# Dependency: SDL2
option(SKIP_SYSTEM_SDL2 "Skip the sytem-provided SDL2 installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_SDL2)
    hunter_add_package(SDL2)
endif()
find_package(SDL2 REQUIRED)
add_library(SDL2 INTERFACE)
if(WIN32)
    target_link_libraries(SDL2 INTERFACE winmm imm32 version)
endif()
target_link_libraries(SDL2 INTERFACE ${SDL2_LIBRARY})

target_include_directories(SDL2 INTERFACE ${SDL2_INCLUDE_DIR})


# Dependency: GLM
option(SKIP_SYSTEM_GLM "Skip the sytem-provided GLM installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_GLM)
    hunter_add_package(glm)
    find_package(glm CONFIG REQUIRED)
else()
    find_package(GLM MODULE REQUIRED)
endif()
add_library(GLM INTERFACE)
target_include_directories(GLM INTERFACE ${GLM_INCLUDE_DIR})
target_compile_definitions(GLM INTERFACE -DGLM_FORCE_RADIANS)


# Dependency: Boost
option(SKIP_SYSTEM_BOOST "Skip the sytem-provided Boost installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_BOOST)
    hunter_add_package(Boost COMPONENTS system log filesystem)
    find_package(Boost CONFIG REQUIRED system log filesystem thread)
    add_library(Boost INTERFACE)
else()
    find_package(Boost MODULE REQUIRED system log filesystem thread)
    add_library(Boost INTERFACE)
    target_compile_definitions(Boost INTERFACE -DBOOST_LOG_DYN_LINK=0)
endif()

target_compile_definitions(Boost INTERFACE -DBOOST_ALL_NO_LIB)
target_include_directories(Boost INTERFACE ${Boost_INCLUDE_DIR})
target_link_libraries(Boost INTERFACE ${Boost_LIBRARIES})


# Dependency: PNG (for CImg)
option(SKIP_SYSTEM_PNG "Skip the sytem-provided PNG installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_PNG)
    hunter_add_package(PNG)
endif()
find_package(PNG REQUIRED)
add_library(PNG INTERFACE)
target_include_directories(PNG INTERFACE ${PNG_INCLUDE_DIRS})
target_link_libraries(PNG INTERFACE ${PNG_LIBRARIES})


# Dependency: JPEG (for CImg)
option(SKIP_SYSTEM_JPEG "Skip the sytem-provided libjpeg installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
if(SKIP_SYSTEM_JPEG)
    hunter_add_package(JPEG)
endif()
find_package(JPEG REQUIRED)
add_library(JPEG INTERFACE)
target_include_directories(JPEG INTERFACE ${JPEG_INCLUDE_DIR})
target_link_libraries(JPEG INTERFACE ${JPEG_LIBRARIES})


# Dependency: CImg
add_library(CImg INTERFACE)
target_include_directories(CImg INTERFACE ${CMAKE_SOURCE_DIR}/3rdparty/CImg)
if(UNIX)
    target_link_libraries(CImg INTERFACE X11)
endif()
target_link_libraries(CImg INTERFACE PNG JPEG)
target_compile_definitions(CImg INTERFACE -Dcimg_use_png -Dcimg_use_jpeg)


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
option(SKIP_SYSTEM_SNDFILE "Skip the sytem-provided SndFile installation and use a custom built" ON)
if(SKIP_SYSTEM_SNDFILE)
    hunter_add_package(SndFile)
endif()
find_package(SndFile REQUIRED)
add_library(SndFile INTERFACE)
target_include_directories(SndFile INTERFACE ${SNDFILE_INCLUDE_DIR})
target_link_libraries(SndFile INTERFACE ${SNDFILE_LIBRARIES} ${OGG_LIBRARIES} ${VORBIS_LIBRARIES} ${VORBISFILE_LIBRARIES})


add_subdirectory(doc)
add_subdirectory(src)
