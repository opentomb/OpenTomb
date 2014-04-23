# Usage (Linux):
#  MKDIR build folder next to opentomb folder (e.g., opentomb-build)
#  CD into build folder
#  cmake -DCMAKE_BUILD_TYPE=Debug ../opentomb
#  make

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(OpenTomb)

IF(NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "No build type selected. Default to Debug")
    SET(CMAKE_BUILD_TYPE "Debug")
ENDIF()

ADD_DEFINITIONS(
#    -Wall
#    -Wextra
)

# disable glext prototypes under Linux.
ADD_DEFINITIONS(-DGL_GLEXT_PROTOTYPES)

SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")

SET(OPENAL_DIR al)
SET(BULLET_DIR bullet)
SET(LUA_DIR lua)
SET(ZLIB_DIR zlib)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

INCLUDE_DIRECTORIES(
    ${OPENAL_DIR}
    ${BULLET_DIR}
    ${LUA_DIR}
    ${ZLIB_DIR}
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

FILE(GLOB ZLIB_FILES
    ${ZLIB_DIR}/*.h
    ${ZLIB_DIR}/*.c
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
    ${ZLIB_FILES}
    ${PRJ_HDR} ${PRJ_SRC}
)

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
