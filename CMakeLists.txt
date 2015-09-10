# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

cmake_minimum_required(VERSION 2.8)

project(OpenTomb)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif()

if(APPLE)
    file(GLOB OPENAL_SRC
        3rdparty/al/*.c
        3rdparty/al/*/*.c
        3rdparty/al/*/*/*.c
    )
    file(GLOB OPENAL_HDR
        3rdparty/al/*.h
        3rdparty/al/*/*.h
        3rdparty/al/*/*/*.h
    )
    add_library(OpenAL STATIC ${OPENAL_HDR} ${OPENAL_SRC})
    # Don’t attempt to find AL anywhere, just use ours directly.
    target_include_directories(OpenAL INTERFACE 3rdparty/al/)
else()
    find_package(OpenAL REQUIRED)
    add_library(OpenAL INTERFACE)
    target_include_directories(OpenAL INTERFACE ${OPENAL_INCLUDE_DIR})
    target_link_libraries(OpenAL INTERFACE ${OPENAL_LIBRARY})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(Bullet)
if(BULLET_FOUND)
    add_library(Bullet INTERFACE)
    target_include_directories(Bullet INTERFACE ${BULLET_INCLUDE_DIRS})
    target_link_libraries(Bullet INTERFACE ${BULLET_LIBRARIES})
else()
    set(BULLET_DIR 3rdparty/bullet/src)

    file(GLOB BULLET_FILES
        ${BULLET_DIR}/*.h
        ${BULLET_DIR}/*/*.h
        ${BULLET_DIR}/*/*/*.h
        ${BULLET_DIR}/*.cpp
        ${BULLET_DIR}/*/*.cpp
        ${BULLET_DIR}/*/*/*.cpp
    )

    add_library(Bullet ${BULLET_FILES})
    target_compile_options(Bullet PRIVATE -O2)
    target_include_directories(Bullet PUBLIC ${BULLET_DIR})
endif()

find_package(Freetype REQUIRED)
add_library(Freetype INTERFACE)
target_include_directories(Freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(Freetype INTERFACE ${FREETYPE_LIBRARIES})

find_package(OpenGL REQUIRED)
add_library(OpenGL INTERFACE)
target_include_directories(OpenGL INTERFACE ${OPENGL_INCLUDE_DIR})
target_link_libraries(OpenGL INTERFACE ${OPENGL_LIBRARIES})

find_package(GLEW REQUIRED)
add_library(GLEW INTERFACE)
target_include_directories(GLEW INTERFACE ${GLEW_INCLUDE_DIRS})
target_link_libraries(GLEW INTERFACE ${GLEW_LIBRARIES})

find_package(ZLIB REQUIRED)
add_library(ZLIB INTERFACE)
target_include_directories(ZLIB INTERFACE ${ZLIB_INCLUDE_DIR})
target_link_libraries(ZLIB INTERFACE ${ZLIB_LIBRARY})

find_package(Lua REQUIRED)
add_library(Lua INTERFACE)
target_include_directories(Lua INTERFACE ${LUA_INCLUDE_DIR})
target_compile_definitions(Lua INTERFACE -DLUA_COMPAT_APIINTCASTS)
target_link_libraries(Lua INTERFACE ${LUA_LIBRARIES})

find_package(SDL2 REQUIRED)
add_library(SDL2 INTERFACE)
target_include_directories(SDL2 INTERFACE ${SDL2_INCLUDE_DIR})
target_link_libraries(SDL2 INTERFACE ${SDL2_LIBRARY})
        
find_package(SDL2Image REQUIRED)
add_library(SDL2_IMAGE INTERFACE)
target_include_directories(SDL2_IMAGE INTERFACE ${SDL2_INCLUDE_DIR})
target_include_directories(SDL2_IMAGE INTERFACE ${SDL2IMAGE_INCLUDE_DIR})
target_link_libraries(SDL2_IMAGE INTERFACE ${SDL2IMAGE_LIBRARY})

option(USE_OPENAL_FLOAT "Use OpenAL Soft floating-point extensions." ON)

if(USE_OPENAL_FLOAT)
    target_compile_definitions(OpenAL INTERFACE -DAUDIO_OPENAL_FLOAT)
endif(USE_OPENAL_FLOAT)

find_package(OGG REQUIRED)
include_directories(${OGG_INCLUDE_DIR})
find_package(Vorbis REQUIRED)
include_directories(${VORBIS_INCLUDE_DIR})
find_package(VorbisFile REQUIRED)
include_directories(${VORBISFILE_INCLUDE_DIR})
find_package(SndFile REQUIRED)
include_directories(${SNDFILE_INCLUDE_DIR})

add_subdirectory(doc)
add_subdirectory(src)
