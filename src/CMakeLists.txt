# -std-gnu++11 is needed for picky fileno function in streaming track code.
# I'm not sure it will work on other platforms this way, but on Windows, it does perfect job.

if(APPLE)
    file(GLOB PRJ_SRC *.cpp mac/*.m)
    file(GLOB PRJ_HDR *.h   mac/*.h)
else()
    file(GLOB PRJ_SRC *.cpp)
    file(GLOB PRJ_HDR *.h)
endif()

list(APPEND PRJ_SRC
    loader/level.cpp
    loader/tr1level.cpp
    loader/tr2level.cpp
    loader/tr3level.cpp
    loader/tr4level.cpp
    loader/tr5level.cpp
)

list(APPEND PRJ_HDR
    loader/io/sdlreader.h
    loader/datatypes.h
    loader/level.h
    loader/tr1level.h
    loader/tr2level.h
    loader/tr3level.h
    loader/tr4level.h
    loader/tr5level.h
    loader/game.h
)

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

        add_definitions("-DSDL_MAIN_HANDLED")
    endif()
else()
    # disable glext prototypes
    add_definitions(-DGL_GLEXT_PROTOTYPES)
endif()

if(WIN32)
    set(PRJ_ICON_RES "../resource/icon/opentomb.rc")
    add_executable(${PROJECT_NAME} ${PRJ_HDR} ${PRJ_SRC} ${PRJ_ICON_RES})
elseif(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE opentomb.icns)
    set(PRJ_ICON_RES "../resource/icon/opentomb.icns")
    set_source_files_properties(${PRJ_ICON_RES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${PRJ_ICON_RES} ${PRJ_HDR} ${PRJ_SRC})
else()
    add_executable(${PROJECT_NAME} ${PRJ_HDR} ${PRJ_SRC})
endif()

target_link_libraries(
    ${PROJECT_NAME}
    OpenGL
    Bullet
    GLEW
    SDL2
    SDL2_IMAGE
    ZLIB
    Lua
    Freetype
    OpenAL
    ${OGG_LIBRARIES}
    ${VORBIS_LIBRARIES}
    ${VORBISFILE_LIBRARIES}
    ${SNDFILE_LIBRARIES}
    # efence
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PRIVATE -std=gnu++11 -Wall -Wextra -msse3)
elseif(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -D_USE_MATH_DEFINES)
endif()

if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
    target_compile_options(${PROJECT_NAME} PRIVATE -std=c++11 -Wall -Wextra -msse3)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ../3rdparty/LuaState/include)

if(APPLE)
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES LINK_FLAGS
        "-L${SDL2_LIBRARY_DIRS} -framework CoreFoundation -framework ApplicationServices"
    )
endif()
