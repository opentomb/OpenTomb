find_program(PYGMENTIZE NAMES pygmentize pygmentize.py) # internally used by asciidoc
find_program(ASCIIDOC NAMES asciidoc asciidoc.py)

if(PYGMENTIZE AND ASCIIDOC)
    add_custom_target(
      trosettastone
      COMMAND ${ASCIIDOC} -b html5 trosettastone.asc
      DEPENDS trosettastone.asc
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running asciidoc on trosettastone.asc"
    )
endif()
