# - Try to find SDL2_image
# Once done this will define
#  SDL2_IMAGE_FOUND - System has SDL2_image
#  SDL2_IMAGE_INCLUDE_DIRS - The SDL2_image include directories
#  SDL2_IMAGE_LIBRARIES - The libraries needed to use SDL2_image
#  SDL2_IMAGE_DEFINITIONS - Compiler switches required for using SDL2_image

SET(SDL2_IMAGE_SEARCH_PATHS
/usr/local
/usr
/sw # Fink
/opt/local # DarwinPorts
/opt/csw # Blastwave
/opt
)

FIND_PATH(SDL2_IMAGE_INCLUDE_DIR SDL_image.h
HINTS
PATH_SUFFIXES include/SDL2 include
PATHS ${SDL2_IMAGE_SEARCH_PATHS}
)

FIND_LIBRARY(SDL2_IMAGE_LIBRARY
NAMES SDL2_image
HINTS
PATH_SUFFIXES lib64 lib
PATHS ${SDL2_IMAGE_SEARCH_PATHS}
)
