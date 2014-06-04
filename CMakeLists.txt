# Usage (Linux):
#  MKDIR build folder next to opentomb folder (e.g., opentomb-build)
#  CD into build folder
#  cmake ../opentomb
#  make

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(OpenTomb)

IF(NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "No build type selected. Default to Release")
    SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

IF (MINGW)
    IF(CMAKE_CROSSCOMPILING)
        # We are cross compiling on Linux with the MinGW toolchain.
        # zlib and SDL2 must be installed for the cross compiler
        MESSAGE(STATUS "MinGW Cross-Compilation")
        
        SET(ZLIB_INCLUDE_DIRS "/usr/${COMPILER_PREFIX}/include")
        SET(ZLIB_LIBRARIES z)
        
        SET(SDL2_INCLUDE_DIRS "/usr/${COMPILER_PREFIX}/include/SDL2")
        SET(SDL2_LIBRARIES
            /usr/${COMPILER_PREFIX}/lib/libmingw32.a
            /usr/${COMPILER_PREFIX}/lib/libSDL2main.a
            /usr/${COMPILER_PREFIX}/lib/libSDL2.a
            /usr/${COMPILER_PREFIX}/lib/libm.a
            /usr/${COMPILER_PREFIX}/lib/libdinput8.a
            /usr/${COMPILER_PREFIX}/lib/libdxguid.a
            /usr/${COMPILER_PREFIX}/lib/libdxerr8.a
            /usr/${COMPILER_PREFIX}/lib/libuser32.a
            /usr/${COMPILER_PREFIX}/lib/libgdi32.a
            /usr/${COMPILER_PREFIX}/lib/libwinmm.a
            /usr/${COMPILER_PREFIX}/lib/libimm32.a
            /usr/${COMPILER_PREFIX}/lib/libole32.a
            /usr/${COMPILER_PREFIX}/lib/liboleaut32.a
            /usr/${COMPILER_PREFIX}/lib/libshell32.a
            /usr/${COMPILER_PREFIX}/lib/libversion.a
            /usr/${COMPILER_PREFIX}/lib/libuuid.a
#            /usr/${COMPILER_PREFIX}/lib/libwinpthread.a # works as static linkage with mingw-w64/gcc4.8 on ubuntu 14.04 but not on ubuntu 12.04
            /usr/${COMPILER_PREFIX}/lib/libpthreadGC2.a # GC-inlined build lib together with pthreadGC2.dll
        )
        ADD_DEFINITIONS("-Dmain=SDL_main")
        SET(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    ELSE()
        # We are probably creating project files for CodeBlocks on Windows
        MESSAGE(STATUS "MinGW Compilation")
        
        # zlib must be installed in the MinGW directory
        SET(ZLIB_INCLUDE_DIRS "/include")
        SET(ZLIB_LIBRARIES z)

        # SDL2 must be installed in the MinGW directory
        SET(SDL2_INCLUDE_DIRS "/include/SDL2")
        SET(SDL2_LIBRARIES SDL2 pthread)
        ADD_DEFINITIONS("-DSDL_MAIN_HANDLED")
    ENDIF()

    # flag for AntTweakBar
    SET(CMAKE_CXX_FLAGS "-D_WINDOWS")
ELSE()
    FIND_PACKAGE(ZLIB REQUIRED)

    INCLUDE(FindPkgConfig)
    PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

    # disable glext prototypes
    ADD_DEFINITIONS("-DGL_GLEXT_PROTOTYPES")

    # flag for AntTweakBar
    SET(CMAKE_CXX_FLAGS "-D_UNIX")
ENDIF()

FIND_PACKAGE(OpenGL REQUIRED)

SET(OPENAL_DIR al)
SET(BULLET_DIR bullet)
SET(LUA_DIR lua)
SET(OGG_DIR ogg)

INCLUDE_DIRECTORIES(
    ${OPENAL_DIR}
    ${BULLET_DIR}
    ${LUA_DIR}
    ${OGG_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}

    freetype2
    freetype2/freetype
)

FILE(GLOB OPENAL_FILES
    ${OPENAL_DIR}/*.h
    ${OPENAL_DIR}/*/*.h
    ${OPENAL_DIR}/*/*/*.h
    ${OPENAL_DIR}/*.c*
    ${OPENAL_DIR}/*/*.c*
    ${OPENAL_DIR}/*/*/*.c*
    ${OPENAL_DIR}/*.inc
    ${OPENAL_DIR}/*/*.inc
    ${OPENAL_DIR}/*/*/*.inc
)

FILE(GLOB BULLET_FILES
    ${BULLET_DIR}/*.h
    ${BULLET_DIR}/*/*.h
    ${BULLET_DIR}/*/*/*.h
    ${BULLET_DIR}/*.cpp
    ${BULLET_DIR}/*/*.cpp
    ${BULLET_DIR}/*/*/*.cpp
)

FILE(GLOB LUA_FILES
    ${LUA_DIR}/*.h
    ${LUA_DIR}/*.c
)

FILE(GLOB OGG_FILES
    ${OGG_DIR}/*.h
    ${OGG_DIR}/*.c
    ${OGG_DIR}/*/*.h
    ${OGG_DIR}/*/*.c
    ${OGG_DIR}/*/*/*.h
    ${OGG_DIR}/*/*/*.c
    ${OGG_DIR}/*/*/*/*.h
    ${OGG_DIR}/*/*/*/*.c
)

FILE(GLOB PRJ_HDR
    *.h*
    freetype2/*.h
    freetype2/*/*.h
    freetype2/*/*/*.h
    freetype2/*/*/*/*.h
    ftgl/*.h
    lua/*.h*
    vt/*.h
)

FILE(GLOB PRJ_SRC
    *.c*
    freetype2/*.c
    freetype2/*/*.c
    freetype2/*/*/*.c
    freetype2/*/*/*/*.c
    ftgl/*.cpp
    vt/*.cpp
)


ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${OPENAL_FILES}
    ${BULLET_FILES}
    ${LUA_FILES}
    ${OGG_FILES}
    ${PRJ_HDR} ${PRJ_SRC}
)

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

SET_TARGET_PROPERTIES(
    ${PROJECT_NAME}
    PROPERTIES LINK_FLAGS
    "-L${SDL2_LIBRARY_DIRS}"
)

if(APPLE)
    SET_TARGET_PROPERTIES(
        ${PROJECT_NAME}
        PROPERTIES LINK_FLAGS
        "-L${SDL2_LIBRARY_DIRS} -framework CoreFoundation -framework ApplicationServices"
    )
endif(APPLE)
