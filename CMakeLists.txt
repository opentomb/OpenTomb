# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

cmake_minimum_required(VERSION 3.0)

project(OpenTomb)

include(ExternalProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")


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




option(SKIP_SYSTEM_BULLET "Skip the sytem-provided Bullet installation and use a custom built 2.83.5." ON)

if(NOT SKIP_SYSTEM_BULLET)
    find_package(Bullet)
endif()
add_library(Bullet INTERFACE)

if(BULLET_FOUND)
    target_include_directories(Bullet INTERFACE ${BULLET_INCLUDE_DIRS})
    target_link_libraries(Bullet INTERFACE ${BULLET_LIBRARIES})
else()
    ExternalProject_Add(
        Bullet-Ext
        PREFIX ${CMAKE_SOURCE_DIR}/3rdparty/Bullet
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DBUILD_BULLET2_DEMOS=OFF
            -DBUILD_CPU_DEMOS=OFF
            -DBUILD_OPENGL3_DEMOS=OFF
            -DBUILD_UNIT_TESTS=OFF
            -DINSTALL_EXTRA_LIBS=ON
        GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
        GIT_TAG 2.83.5
    )
    #include(${CMAKE_SOURCE_DIR}/3rdparty/Bullet/lib/cmake/bullet/BulletConfig.cmake)

    link_directories(${CMAKE_SOURCE_DIR}/3rdparty/Bullet/lib)

    target_include_directories(Bullet INTERFACE ${CMAKE_SOURCE_DIR}/3rdparty/Bullet/include/bullet)
    target_link_libraries(Bullet INTERFACE BulletDynamics BulletSoftBody BulletCollision LinearMath)
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

find_package(GLM REQUIRED)
add_library(GLM INTERFACE)
target_include_directories(GLM INTERFACE ${GLM_INCLUDE_DIR})
target_compile_definitions(GLM INTERFACE -DGLM_FORCE_RADIANS)

find_package(Boost COMPONENTS system log filesystem REQUIRED)
add_library(Boost INTERFACE)
target_include_directories(Boost INTERFACE ${Boost_INCLUDE_DIR})
target_link_libraries(Boost INTERFACE ${Boost_LIBRARIES})
target_compile_definitions(Boost INTERFACE -DBOOST_LOG_DYN_LINK -DBOOST_ALL_NO_LIB)

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
