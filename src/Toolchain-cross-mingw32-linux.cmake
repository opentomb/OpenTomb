# Cross compile toolchain configuration based on:
# http://www.cmake.org/Wiki/CMake_Cross_Compiling

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# the classical mingw32 (http://www.mingw.org/)
#SET(COMPILER_PREFIX "i586-mingw32msvc")

# 32 or 64 bits mingw-w64 (http://mingw-w64.sourceforge.net/)
SET(COMPILER_PREFIX "i686-w64-mingw32")
#SET(COMPILER_PREFIX "x86_64-w64-mingw32"

# compilers to use for C and C++
FIND_PROGRAM(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-gcc)
FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-g++)
FIND_PROGRAM(CMAKE_RC_COMPILER NAMES ${COMPILER_PREFIX}-windres)

# path to the target environment
SET(CMAKE_FIND_ROOT_PATH /usr/${COMPILER_PREFIX})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
