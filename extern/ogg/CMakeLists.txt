cmake_minimum_required(VERSION 3.2)

project(ogg)

set(OGG_SRCS
    libogg/bitwise.c
    libogg/framing.c
    libvorbis/analysis.c
    libvorbis/bitrate.c
    libvorbis/block.c
    libvorbis/codebook.c
    libvorbis/envelope.c
    libvorbis/floor0.c
    libvorbis/floor1.c
    libvorbis/info.c
    libvorbis/lookup.c
    libvorbis/lpc.c
    libvorbis/lsp.c
    libvorbis/mapping0.c
    libvorbis/mdct.c
    libvorbis/psy.c
    libvorbis/registry.c
    libvorbis/res0.c
    libvorbis/sharedbook.c
    libvorbis/smallft.c
    libvorbis/synthesis.c
    libvorbis/vorbisfile.c
    libvorbis/window.c
)

add_library(${PROJECT_NAME} STATIC ${OGG_SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
