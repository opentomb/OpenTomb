OPTION(COMPILE_TROSETTASTONE "Compile TRosettaStone 3.0 (requires Asciidoc and Pygments)" OFF)

find_program(PYGMENTIZE NAMES pygmentize pygmentize.py) # internally used by asciidoc
find_program(ASCIIDOC NAMES asciidoc asciidoc.py)

if(PYGMENTIZE AND ASCIIDOC AND COMPILE_TROSETTASTONE)
    add_custom_target(
      trosettastone
      COMMAND ${ASCIIDOC} -b html5 -a max-width=100% trosettastone.asc
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running asciidoc on trosettastone.asc"
    )
endif()
