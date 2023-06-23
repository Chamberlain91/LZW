function(set_target_standard_options)

  # Parse arguments CMAKE style.
  # TARGET - A the name of the binary target to define compiler flags
  cmake_parse_arguments(STD_OPTS "" "TARGET" "" ${ARGN})

  # ...
  set_target_properties(
    ${STD_OPTS_TARGET}
    PROPERTIES CXX_EXTENSIONS OFF
               CXX_STANDARD 23
               CXX_STANDARD_REQUIRED TRUE
               C_EXTENSIONS OFF
               C_STANDARD 23
               C_STANDARD_REQUIRED TRUE)

  if(NOT MSVC)
    target_compile_options(
      ${STD_OPTS_TARGET}
      PRIVATE -Wall
              # -Werror
              -Wextra
              -pedantic
              -Wconversion
              -Wshadow
              # -Wold-style-cast
              -Wcast-align
              -Wcast-qual
              -Winit-self
              -Wmissing-field-initializers
              # -Woverloaded-virtual
              -Wredundant-decls
              -Wundef
              -fdiagnostics-parseable-fixits)
  endif()

endfunction(set_target_standard_options)
