add_library(vtloader STATIC
    l_tr1.cpp
    l_tr2.cpp
    l_tr3.cpp
    l_tr4.cpp
    l_tr5.cpp
    l_main.cpp
    scaler.cpp
    vt_level.cpp
    l_common.cpp

    l_main.h
    scaler.h
    vt_level.h
    tr_types.h
    glmath.h
)

target_link_libraries(vtloader GLEW SDL2 ZLIB)
