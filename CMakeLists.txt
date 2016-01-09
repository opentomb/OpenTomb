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

# alternative system-provided package name
macro(ot_find_dependency_alt depname sysdepname)
    option(SKIP_SYSTEM_${depname} "Skip the sytem-provided ${depname} installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
    if(SKIP_SYSTEM_${depname})
        hunter_add_package(${depname} ${ARGN})
        find_package(${depname} REQUIRED ${ARGN})
    else()
        # temporarily disable Hunter-local Find* modules
        set(_old_cmp ${CMAKE_MODULE_PATH})
        set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
        find_package(${sysdepname} MODULE REQUIRED ${ARGN})
        # restore module path
        set(CMAKE_MODULE_PATH ${_old_cmp})
        set(_old_cmp)
    endif()
    if(NOT TARGET ${depname})
        add_library(${depname} INTERFACE)
    endif()
endmacro()

macro(ot_find_dependency depname)
    ot_find_dependency_alt(${depname} ${depname} ${ARGN})
endmacro()


# Dependency: OpenAL
ot_find_dependency(OpenAL)
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
ot_find_dependency_alt(freetype Freetype)
if(NOT SKIP_SYSTEM_freetype)
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


# Dependency: Lua
ot_find_dependency(Lua)
target_link_libraries(Lua INTERFACE ${LUA_LIBRARIES})
target_include_directories(Lua INTERFACE ${LUA_INCLUDE_DIR})


# Dependency: SDL2
ot_find_dependency(SDL2)
if(WIN32)
    target_link_libraries(SDL2 INTERFACE winmm imm32 version)
endif()
target_link_libraries(SDL2 INTERFACE ${SDL2_LIBRARY})
target_include_directories(SDL2 INTERFACE ${SDL2_INCLUDE_DIR})


# Dependency: GLM
ot_find_dependency_alt(glm GLM)
add_library(GLM INTERFACE)
target_include_directories(GLM INTERFACE ${GLM_INCLUDE_DIR})
target_compile_definitions(GLM INTERFACE -DGLM_FORCE_RADIANS)


# Dependency: Boost
ot_find_dependency(Boost COMPONENTS system log filesystem thread)
if(NOT SKIP_SYSTEM_Boost)
    target_compile_definitions(Boost INTERFACE -DBOOST_LOG_DYN_LINK=0)
endif()
target_compile_definitions(Boost INTERFACE -DBOOST_ALL_NO_LIB)
target_include_directories(Boost INTERFACE ${Boost_INCLUDE_DIR})
target_link_libraries(Boost INTERFACE ${Boost_LIBRARIES})


# Dependency: ZLIB
ot_find_dependency(ZLIB)
target_link_libraries(ZLIB INTERFACE ${ZLIB_LIBRARIES})
target_include_directories(ZLIB INTERFACE ${ZLIB_INCLUDE_DIR})


# Dependency: PNG (for CImg)
# Make sure to find ZLIB *before* this
ot_find_dependency(PNG)
target_include_directories(PNG INTERFACE ${PNG_INCLUDE_DIRS})
target_link_libraries(PNG INTERFACE ${PNG_LIBRARIES})


# Dependency: JPEG (for CImg)
ot_find_dependency(JPEG)
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
ot_find_dependency(SndFile)
target_include_directories(SndFile INTERFACE ${SNDFILE_INCLUDE_DIR})
target_link_libraries(SndFile INTERFACE ${SNDFILE_LIBRARIES} ${OGG_LIBRARIES} ${VORBIS_LIBRARIES} ${VORBISFILE_LIBRARIES})


add_subdirectory(doc)
add_subdirectory(src)
