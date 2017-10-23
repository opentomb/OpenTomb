# Cross compile toolchain configuration based on:
# http://www.cmake.org/Wiki/CMake_Cross_Compiling

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# the classical mingw32 (http://www.mingw.org/)
#SET(TOOLCHAIN_PREFIX "i586-mingw32msvc")

# 32 or 64 bits mingw-w64 (http://mingw-w64.sourceforge.net/)
SET(TOOLCHAIN_PREFIX "i686-w64-mingw32")
#SET(TOOLCHAIN_PREFIX "x86_64-w64-mingw32"

# Toolchain path prefix (CYGWIN cross compiler)
#SET(COMPILER_PREFIX ${TOOLCHAIN_PREFIX}/sys-root/mingw)

# Toolchain path prefix (standard prefix)
SET(COMPILER_PREFIX ${TOOLCHAIN_PREFIX})

# compilers to use for C and C++
FIND_PROGRAM(CMAKE_C_COMPILER NAMES ${TOOLCHAIN_PREFIX}-gcc)
FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}-g++)
FIND_PROGRAM(CMAKE_RC_COMPILER NAMES ${TOOLCHAIN_PREFIX}-windres)

# path to the target environment
SET(CMAKE_FIND_ROOT_PATH /usr/${COMPILER_PREFIX})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
