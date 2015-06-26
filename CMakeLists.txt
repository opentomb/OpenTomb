# Usage (Linux):
#  MKDIR build folder next to opentomb folder (e.g., opentomb-build)
#  CD into build folder
#  cmake ../opentomb
#  make

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(OpenTomb)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

find_package(Bullet REQUIRED)
INCLUDE_DIRECTORIES(${BULLET_INCLUDE_DIRS})

find_package(Freetype REQUIRED)
INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})

find_package(Lua REQUIRED)
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIRS})

find_package(OpenAL REQUIRED)
INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIRS})

add_definitions(-DAL_ALEXT_PROTOTYPES)

IF(NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "No build type selected. Default to Release")
    SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

IF(MINGW)
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

        # zlib must be installed in the MinGW directory
        SET(ZLIB_INCLUDE_DIRS "/include")
        SET(ZLIB_LIBRARIES z)

        # SDL2 must be installed in the MinGW directory
        SET(SDL2_INCLUDE_DIRS "/include/SDL2")
        SET(SDL2_LIBRARIES SDL2 pthread)
        SET(SDL2_IMAGE_INCLUDE_DIRS "/include/SDL2")
        SET(SDL2_IMAGE_LIBRARIES SDL2_image)
        ADD_DEFINITIONS("-DSDL_MAIN_HANDLED")
    ENDIF()
ELSE()
    FIND_PACKAGE(ZLIB REQUIRED)

    INCLUDE(FindPkgConfig)
    PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
    PKG_SEARCH_MODULE(SDL2_IMAGE REQUIRED SDL2_image)

    # disable glext prototypes
    ADD_DEFINITIONS("-DGL_GLEXT_PROTOTYPES")
ENDIF()

SET(CMAKE_CXX_FLAGS "-std=c++0x")

FIND_PACKAGE(OpenGL REQUIRED)

SET(OGG_DIR src/ogg)

INCLUDE_DIRECTORIES(
    ${OGG_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
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
    src/*.h*
    src/vt/*.h
)

FILE(GLOB PRJ_SRC
    src/*.c*
    src/vt/*.cpp
)


ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${OGG_FILES}
    ${PRJ_HDR} ${PRJ_SRC} ${PRJ_ICON_RES}
)

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${BULLET_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${LUA_LIBRARIES}
    ${OPENAL_LIBRARY}
)

IF(APPLE)
    SET_TARGET_PROPERTIES(
        ${PROJECT_NAME}
        PROPERTIES LINK_FLAGS
        "-L${SDL2_LIBRARY_DIRS} -framework CoreFoundation -framework ApplicationServices"
    )
ENDIF(APPLE)
