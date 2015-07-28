# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

cmake_minimum_required(VERSION 2.8)

project(OpenTomb)

if(APPLE)
    file(GLOB PRJ_SRC
        src/*.c*
        src/vt/*.cpp
        src/mac/*.m
        3rdparty/al/*.c
        3rdparty/al/*/*.c
        3rdparty/al/*/*/*.c
    )
    file(GLOB PRJ_HDR
        src/*.h*
        src/vt/*.h
        src/mac/*.h
        3rdparty/al/*.h
        3rdparty/al/*/*.h
        3rdparty/al/*/*/*.h
    )
else()
    file(GLOB PRJ_SRC
        src/*.c*
        src/vt/*.cpp
    )
    file(GLOB PRJ_HDR
        src/*.h*
        src/vt/*.h
    )
endif()


if(WIN32)
    set(PRJ_ICON_RES "resource/icon/opentomb.rc")
    add_executable(${PROJECT_NAME} ${PRJ_HDR} ${PRJ_SRC} ${PRJ_ICON_RES})
elseif(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE opentomb.icns)
    set(PRJ_ICON_RES "resource/icon/opentomb.icns")
    set_source_files_properties(${PRJ_ICON_RES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${PRJ_ICON_RES} ${PRJ_HDR} ${PRJ_SRC})
else()
    add_executable(${PROJECT_NAME} ${PRJ_HDR} ${PRJ_SRC})
endif()

# -std-gnu++11 is needed for picky fileno function in streaming track code.
# I'm not sure it will work on other platforms this way, but on Windows, it does perfect job.

if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PRIVATE -std=gnu++11 -Wall -Wextra -msse3)
endif()

if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
    target_compile_options(${PROJECT_NAME} PRIVATE -std=c++11 -Wall -Wextra -msse3)
endif()

find_package(Bullet)
if(BULLET_INCLUDE_DIR)
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
target_include_directories(ZLIB INTERFACE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(ZLIB INTERFACE ${ZLIB_LIBRARIES})

find_package(Lua REQUIRED)
add_library(Lua INTERFACE)
target_include_directories(Lua INTERFACE ${LUA_INCLUDE_DIR})
target_compile_definitions(Lua INTERFACE -DLUA_COMPAT_APIINTCASTS)
target_link_libraries(Lua INTERFACE ${LUA_LIBRARIES})

target_include_directories(${PROJECT_NAME} PRIVATE 3rdparty/LuaState/include)

if(APPLE)
    # Don’t attempt to find AL anywhere, just use ours directly.
    include_directories(3rdparty/al/)
else()
    find_package(OpenAL REQUIRED)
    add_library(OpenAL INTERFACE)
    target_include_directories(OpenAL INTERFACE ${OPENAL_INCLUDE_DIR})
    target_link_libraries(OpenAL INTERFACE ${OPENAL_LIBRARY})
endif()

option(USE_OPENAL_FLOAT "Use OpenAL Soft floating-point extensions." ON)

if(USE_OPENAL_FLOAT)
    target_compile_definitions(OpenAL INTERFACE -DAUDIO_OPENAL_FLOAT)
endif(USE_OPENAL_FLOAT)

function(trylinklib LIBNAME ATTRIBS)
        find_library(
                "${LIBNAME}_LIB"
                NAMES "${LIBNAME}"
                HINTS
                PATH_SUFFIXES lib64 lib32 lib
                PATHS
                /sw
                /opt/local
                /opt/csw
                /opt
        )
        if(${LIBNAME}_LIB)
                add_library("${LIBNAME}" ${ATTRIBS} IMPORTED)
        else()
            message(FATAL_ERROR "Missing library: ${LIBNAME}")
        endif()
        mark_as_advanced(FORCE "${LIBNAME}_LIB")
endfunction(trylinklib)

trylinklib("ogg" SHARED)
trylinklib("vorbis" SHARED)
trylinklib("vorbisfile" SHARED)
trylinklib("sndfile" SHARED)

if(MINGW)
    if(CMAKE_CROSSCOMPILING)
        # We are cross compiling on Linux with the MinGW toolchain.
        # zlib and SDL2 must be installed for the cross compiler
        MESSAGE(STATUS "MinGW Cross-Compilation")

        SET(SDL2_INCLUDE_DIRS "/usr/${COMPILER_PREFIX}/include/SDL2")
        SET(SDL2_LIBRARIES
            /usr/${COMPILER_PREFIX}/lib/libmingw32.a
            /usr/${COMPILER_PREFIX}/lib/libSDL2main.a
            /usr/${COMPILER_PREFIX}/lib/libSDL2.dll.a
            /usr/${COMPILER_PREFIX}/lib/libpthreadGC2.a # GC-inlined build lib together with pthreadGC2.dll
        )
        set(SDL2_IMAGE_INCLUDE_DIRS "/include/SDL2")
        set(SDL2_IMAGE_LIBRARIES /usr/${COMPILER_PREFIX}/lib/libSDL2_image.dll.a)
        add_definitions("-Dmain=SDL_main")
        set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    else()
        # We are probably creating project files for CodeBlocks on Windows
        message(STATUS "MinGW Compilation")

        # SDL2 must be installed in the MinGW directory
        set(SDL2_INCLUDE_DIRS "/include/SDL2")
        set(SDL2_LIBRARIES SDL2 pthread)
        set(SDL2_IMAGE_INCLUDE_DIRS "/include/SDL2")
        set(SDL2_IMAGE_LIBRARIES SDL2_image)
        add_definitions("-DSDL_MAIN_HANDLED")
    endif()
else()
    include(FindPkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_search_module(SDL2 REQUIRED sdl2)
        pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
    else()
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
        find_package(SDL2 REQUIRED)
        find_package(SDL2IMAGE REQUIRED)
        
        set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
        # This is a workaround. SDL headers are included in two ways: mentioning the SDL 
        # directory (which is uncommon) and directly (SDL/SDL.h vs SDL.h)
        set(SDL2_IMAGE_INCLUDE_DIRS "${SDL2IMAGE_INCLUDE_DIR};${SDL2_INCLUDE_DIR}/SDL2")
    endif()
    # disable glext prototypes
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DGL_GLEXT_PROTOTYPES)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS}
                           ${SDL2_IMAGE_INCLUDE_DIRS})

                    
target_link_libraries(
    ${PROJECT_NAME}
    OpenGL
    Bullet
    GLEW
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ZLIB
    Lua
    Freetype
    OpenAL
    ${ogg_LIB}
    ${vorbis_LIB}
    ${vorbisfile_LIB}
    ${sndfile_LIB}
)


if(APPLE)
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES LINK_FLAGS
        "-L${SDL2_LIBRARY_DIRS} -framework CoreFoundation -framework ApplicationServices"
    )
endif()

add_subdirectory(doc)
