# Define executable target
add_executable(lzw "main.c" "standard.c" "lzw.c")

# Sets to c23, warnings, etc
set_target_standard_options(TARGET lzw)
