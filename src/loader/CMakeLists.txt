add_library(vtloader STATIC
    level.cpp
    tr1level.cpp
    tr2level.cpp
    tr3level.cpp
    tr4level.cpp
    tr5level.cpp

    io/sdlreader.h
    datatypes.h
    level.h
    tr1level.h
    tr2level.h
    tr3level.h
    tr4level.h
    tr5level.h
    game.h
)

target_link_libraries(vtloader GLEW SDL2 ZLIB)
