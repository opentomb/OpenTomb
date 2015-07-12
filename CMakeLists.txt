# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(OpenTomb)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -msse3")

find_package(Freetype)
INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})
find_package(Bullet)
INCLUDE_DIRECTORIES(${BULLET_INCLUDE_DIR})
INCLUDE_DIRECTORIES(3rdparty/LuaState/include)

if(APPLE)
    # Don’t attempt to find AL anywhere, just use ours directly.
    INCLUDE_DIRECTORIES(3rdparty/al/)
else()
    find_package(OpenAL REQUIRED)
    INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIRS})
endif()

find_package(Lua REQUIRED)
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIRS})
add_definitions(-DLUA_COMPAT_APIINTCASTS)

find_package(GLEW REQUIRED)
INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIRS})

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

find_package(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})

IF(MINGW)
    IF(CMAKE_CROSSCOMPILING)
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
        SET(SDL2_IMAGE_INCLUDE_DIRS "/include/SDL2")
        SET(SDL2_IMAGE_LIBRARIES /usr/${COMPILER_PREFIX}/lib/libSDL2_image.dll.a)
        ADD_DEFINITIONS("-Dmain=SDL_main")
        SET(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
        SET(PRJ_ICON_RES "resource/icon/opentomb.rc")
    ELSE()
        # We are probably creating project files for CodeBlocks on Windows
        MESSAGE(STATUS "MinGW Compilation")

        # SDL2 must be installed in the MinGW directory
        SET(SDL2_INCLUDE_DIRS "/include/SDL2")
        SET(SDL2_LIBRARIES SDL2 pthread)
        SET(SDL2_IMAGE_INCLUDE_DIRS "/include/SDL2")
        SET(SDL2_IMAGE_LIBRARIES SDL2_image)
        ADD_DEFINITIONS("-DSDL_MAIN_HANDLED")
    ENDIF()
ELSE()
    INCLUDE(FindPkgConfig)
    PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
    PKG_SEARCH_MODULE(SDL2_IMAGE REQUIRED SDL2_image)

    # disable glext prototypes
    ADD_DEFINITIONS("-DGL_GLEXT_PROTOTYPES")
ENDIF()

FIND_PACKAGE(OpenGL REQUIRED)

INCLUDE_DIRECTORIES(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

if(APPLE)
    FILE(GLOB PRJ_SRC
        src/*.c*
        src/vt/*.cpp
        src/mac/*.m
        3rdparty/al/*.c
        3rdparty/al/*/*.c
        3rdparty/al/*/*/*.c
    )
    FILE(GLOB PRJ_HDR
        src/*.h*
        src/vt/*.h
        src/mac/*.h
        3rdparty/al/*.h
        3rdparty/al/*/*.h
        3rdparty/al/*/*/*.h
    )
else()
    FILE(GLOB PRJ_SRC
        src/*.c*
        src/vt/*.cpp
    )
    FILE(GLOB PRJ_HDR
        src/*.h*
        src/vt/*.h
    )
endif()

ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${PRJ_HDR} ${PRJ_SRC} ${PRJ_ICON_RES}
)

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${BULLET_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${LUA_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${ogg_LIB}
    ${vorbis_LIB}
    ${vorbisfile_LIB}
    ${sndfile_LIB}
)

IF(APPLE)
    SET_TARGET_PROPERTIES(
        ${PROJECT_NAME}
        PROPERTIES LINK_FLAGS
        "-L${SDL2_LIBRARY_DIRS} -framework CoreFoundation -framework ApplicationServices"
    )
ENDIF(APPLE)
