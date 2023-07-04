# Define executable target
add_executable(lzw "main.c" "standard.c" "lzw.c")

# Keeps the intermediate assembly and preprocessor versions
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -save-temps")

# Sets to c23, warnings, etc
set_target_standard_options(TARGET lzw)
