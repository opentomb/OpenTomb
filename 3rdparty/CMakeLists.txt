cmake_minimum_required(VERSION 2.8)
project(OpenTomp-deps)

set(BULLET_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/install)
if(NOT BULLET_BUILT)
    set(BULLET_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/bullet3/build)
    message(STATUS "Bootstrapping Bullet in ${BULLET_BUILD}...")
    file(MAKE_DIRECTORY ${BULLET_BUILD})
    execute_process(
      COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=${BULLET_ROOT} -DBUILD_BULLET3=OFF -DBUILD_CPU_DEMOS=OFF -DBUILD_EXTRAS=OFF -DBUILD_OPENGL3_DEMOS=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_UNIT_TESTS=OFF -DBUILD_BULLET2_DEMOS=OFF -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} ..
      WORKING_DIRECTORY ${BULLET_BUILD}
      RESULT_VARIABLE BULLET_CMD
    )
    if(NOT BULLET_CMD EQUAL 0)
        message(FATAL_ERROR "Failed to configure Bullet")
    endif()
    execute_process(
      COMMAND ${CMAKE_MAKE_PROGRAM}
      WORKING_DIRECTORY ${BULLET_BUILD}
      RESULT_VARIABLE BULLET_CMD
    )
    if(NOT BULLET_CMD EQUAL 0)
        message(FATAL_ERROR "Failed to build Bullet")
    endif()
    execute_process(
      COMMAND ${CMAKE_MAKE_PROGRAM} install
      WORKING_DIRECTORY ${BULLET_BUILD}
      RESULT_VARIABLE BULLET_CMD
    )
    if(NOT BULLET_CMD EQUAL 0)
        message(FATAL_ERROR "Failed to install Bullet into ${BULLET_ROOT}")
    endif()
    set(BULLET_BUILT TRUE CACHE INTERNAL "Whether bullet is successfully built in-tree")
endif()

if(APPLE)
    option(BUILD_INTREE_OPENAL "Use in-tree provided OpenAL" ON)
else()
    option(BUILD_INTREE_OPENAL "Use in-tree provided OpenAL" OFF)
endif()

if(BUILD_INTREE_OPENAL)
    set(ENV{OPENALDIR} ${CMAKE_CURRENT_SOURCE_DIR}/openal-soft/install)
endif()

if(BUILD_INTREE_OPENAL AND NOT OPENAL_BUILT)
    set(OPENAL_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/openal-soft/build)
    message(STATUS "Bootstrapping OpenAL in ${OPENAL_BUILD}...")
    file(MAKE_DIRECTORY ${OPENAL_BUILD})
    execute_process(
      COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$ENV{OPENALDIR} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} ..
      WORKING_DIRECTORY ${OPENAL_BUILD}
      RESULT_VARIABLE OPENAL_CMD
    )
    if(NOT OPENAL_CMD EQUAL 0)
        message(FATAL_ERROR "Failed to configure OpenAL")
    endif()
    execute_process(
      COMMAND ${CMAKE_MAKE_PROGRAM}
      WORKING_DIRECTORY ${OPENAL_BUILD}
      RESULT_VARIABLE OPENAL_CMD
    )
    if(NOT OPENAL_CMD EQUAL 0)
        message(FATAL_ERROR "Failed to build OpenAL")
    endif()
    execute_process(
      COMMAND ${CMAKE_MAKE_PROGRAM} install
      WORKING_DIRECTORY ${OPENAL_BUILD}
      RESULT_VARIABLE OPENAL_CMD
    )
    if(NOT OPENAL_CMD EQUAL 0)
        message(FATAL_ERROR "Failed to install OpenAL into $ENV{OPENALDIR}")
    endif()
    set(OPENAL_BUILT TRUE CACHE INTERNAL "Whether OpenAL is successfully built in-tree")
endif()

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/install/success.txt "Ignore this file.")
