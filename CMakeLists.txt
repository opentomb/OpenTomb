# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

cmake_minimum_required(VERSION 3.2)

project(OpenTomb)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

option(FORCE_SYSTEM_FREETYPE "Use system-provided FreeType instead of internal library." OFF)

# Detect system FreeType

if (FORCE_SYSTEM_FREETYPE)
    find_package(freetype QUIET)
    if (NOT FREETYPE_FOUND)
        message(WARNING "FreeType not found. Enabling internal FreeType support.")
        set(OPENTOMB_INTERNAL_FREETYPE ON)
    else()
        set(OPENTOMB_INTERNAL_FREETYPE OFF)
    endif ()
else ()
    set(OPENTOMB_INTERNAL_FREETYPE ON)
endif ()

if (OPENTOMB_INTERNAL_FREETYPE)
    add_subdirectory(extern/freetype2)
    set(FREETYPE_INCLUDE_DIRS "")
    set(FREETYPE_LIBRARIES freetype2)
endif ()

add_subdirectory(extern/bullet)
add_subdirectory(extern/lua)

set(OPENTOMB_SRCS
    src/core/avl.c
    src/core/avl.h
    src/core/base_types.c
    src/core/base_types.h
    src/core/console.c
    src/core/console.h
    src/core/gl_font.c
    src/core/gl_font.h
    src/core/gl_text.c
    src/core/gl_text.h
    src/core/gl_util.c
    src/core/gl_util.h
    src/core/obb.c
    src/core/obb.h
    src/core/polygon.c
    src/core/polygon.h
    src/core/system.c
    src/core/system.h
    src/core/utf8_32.c
    src/core/utf8_32.h
    src/core/vmath.c
    src/core/vmath.h
    src/gui/gui.cpp
    src/gui/gui.h
    src/gui/gui_inventory.cpp
    src/gui/gui_inventory.h
    src/fmv/tiny_codec.h
    src/fmv/tiny_codec.c
    src/fmv/stream_codec.h
    src/fmv/stream_codec.c
    src/fmv/internal/common.h
    src/fmv/internal/bytestream.h
    src/fmv/internal/get_bits.h
    src/fmv/containers/rpl.c
    src/fmv/codecs/avcodec.h
    src/fmv/codecs/adpcm.h
    src/fmv/codecs/adpcm.c
    src/fmv/codecs/adpcm_data.c
    src/fmv/codecs/adpcm_data.h
    src/fmv/codecs/pcm.c
    src/fmv/codecs/escape124.c
    src/fmv/codecs/escape130.c
    src/fmv/codecs/reverse.c
    src/physics/physics.h
    src/physics/physics_bullet.cpp
    src/physics/hair.h
    src/physics/hair.cpp
    src/physics/ragdoll.h
    src/physics/ragdoll.cpp
    src/render/bordered_texture_atlas.cpp
    src/render/bordered_texture_atlas.h
    src/render/bsp_tree.cpp
    src/render/bsp_tree.h
    src/render/bsp_tree_2d.c
    src/render/bsp_tree_2d.h
    src/render/camera.cpp
    src/render/camera.h
    src/render/frustum.cpp
    src/render/frustum.h
    src/render/render.cpp
    src/render/render.h
    src/render/shader_description.cpp
    src/render/shader_description.h
    src/render/shader_manager.cpp
    src/render/bsp_tree_2d.c
    src/render/shader_manager.h
    src/script/script.h
    src/script/script.cpp
    src/script/script_audio.cpp
    src/script/script_character.cpp
    src/script/script_entity.cpp
    src/script/script_skeletal_model.cpp
    src/script/script_world.cpp
    src/vt/l_common.cpp
    src/vt/l_main.cpp
    src/vt/l_main.h
    src/vt/l_tr1.cpp
    src/vt/l_tr2.cpp
    src/vt/l_tr3.cpp
    src/vt/l_tr4.cpp
    src/vt/l_tr5.cpp
    src/vt/scaler.cpp
    src/vt/scaler.h
    src/vt/vt_level.cpp
    src/vt/vt_level.h
    src/vt/tr_types.h
    src/vt/tr_versions.h
    src/state_control/state_control.h
    src/state_control/state_control.cpp
    src/state_control/state_control_Lara.h
    src/state_control/state_control_Lara.cpp
    src/state_control/state_control_bat.h
    src/state_control/state_control_bat.cpp
    src/state_control/state_control_wolf.h
    src/state_control/state_control_wolf.cpp
    src/state_control/state_control_bear.h
    src/state_control/state_control_bear.cpp
    src/state_control/state_control_raptor.h
    src/state_control/state_control_raptor.cpp
    src/state_control/state_control_trex.h
    src/state_control/state_control_trex.cpp
    src/state_control/state_control_Larson.h
    src/state_control/state_control_Larson.cpp
    src/state_control/state_control_Pierre.h
    src/state_control/state_control_Pierre.cpp
    src/state_control/state_control_lion.h
    src/state_control/state_control_lion.cpp
    src/state_control/state_control_gorilla.h
    src/state_control/state_control_gorilla.cpp
    src/state_control/state_control_crocodile.h
    src/state_control/state_control_crocodile.cpp
    src/state_control/state_control_rat.h
    src/state_control/state_control_rat.cpp
    src/state_control/state_control_centaur.h
    src/state_control/state_control_centaur.cpp
    src/state_control/state_control_puma.h
    src/state_control/state_control_puma.cpp
    src/state_control/state_control_winged_mutant.h
    src/state_control/state_control_winged_mutant.cpp
    src/state_control/state_control_MrT.h
    src/state_control/state_control_MrT.cpp
    src/state_control/state_control_cowboy.h
    src/state_control/state_control_cowboy.cpp
    src/state_control/state_control_skateboardist.h
    src/state_control/state_control_skateboardist.cpp
    src/state_control/state_control_torso_boss.h
    src/state_control/state_control_torso_boss.cpp
    src/state_control/state_control_Natla.h
    src/state_control/state_control_Natla.cpp
    src/audio/audio.cpp
    src/audio/audio.h
    src/audio/audio_fx.cpp
    src/audio/audio_fx.h
    src/audio/audio_stream.cpp
    src/audio/audio_stream.h
    src/audio/stb_vorbis.c
    src/character_controller.cpp
    src/character_controller.h
    src/controls.cpp
    src/controls.h
    src/engine.cpp
    src/engine.h
    src/engine_string.h
    src/entity.cpp
    src/entity.h
    src/game.cpp
    src/game.h
    src/game_camera.cpp
    src/gameflow.cpp
    src/gameflow.h
    src/inventory.cpp
    src/inventory.h
    src/image.cpp
    src/image.h
    src/main_SDL.cpp
    src/mesh.c
    src/mesh.h
    src/resource.cpp
    src/resource.h
    src/room.cpp
    src/room.h
    src/skeletal_model.h
    src/skeletal_model.c
    src/trigger.cpp
    src/trigger.h
    src/world.cpp
    src/world.h
)

# Disable warnings when using unsafe functions
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(MINGW AND CMAKE_CROSSCOMPILING)
    # Cross compiling on Linux with the MinGW toolchain.
    message(STATUS "MinGW Cross-Compilation")

    set(OPENAL_INCLUDE_DIR "/usr/${COMPILER_PREFIX}/include/AL")
    set(OPENAL_LIBRARY OpenAL32)

    set(SDL2_INCLUDE_DIR "/usr/${COMPILER_PREFIX}/include/SDL2")
    set(SDL2_LIBRARY
        /usr/${COMPILER_PREFIX}/lib/libmingw32.a
        /usr/${COMPILER_PREFIX}/lib/libSDL2main.a
        /usr/${COMPILER_PREFIX}/lib/libSDL2.dll.a
        /usr/${COMPILER_PREFIX}/lib/libwinpthread.a
    )

    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
else()
    find_package(OpenAL REQUIRED)
    find_package(SDL2 REQUIRED)
endif()

set(OPENTOMB_ICON "resource/icon/opentomb.rc")
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

# Check for optional OpenAL include files that are not present in all implementations of the library
include(CheckIncludeFiles)
set(CMAKE_REQUIRED_INCLUDES ${OPENAL_INCLUDE_DIR})
CHECK_INCLUDE_FILES(alext.h HAVE_ALC_H)
CHECK_INCLUDE_FILES(efx.h HAVE_EFX_H)
CHECK_INCLUDE_FILES("efx-presets.h" HAVE_EFX_PRESETS_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config-opentomb.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config-opentomb.h)

add_executable(${PROJECT_NAME} ${OPENTOMB_SRCS} ${OPENTOMB_ICON})
set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 99 CXX_STANDARD 11)

target_include_directories(
    ${PROJECT_NAME} PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${OPENAL_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    bullet
    ${FREETYPE_LIBRARIES}
    lua5.3
    ${PNG_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${SDL2_LIBRARY}
    ${ZLIB_LIBRARIES}
)
