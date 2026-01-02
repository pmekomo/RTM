set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Use Zig for cross-compilation (built-in ARM Linux support)
set(CMAKE_C_COMPILER $ENV{HOME}/zig-gcc)
set(CMAKE_CXX_COMPILER $ENV{HOME}/zig-g++)

# Specify sysroot from the toolchain
set(CMAKE_SYSROOT $ENV{HOME}/arm-sysroot)

# Linker flags
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker --no-dependency-file")  # Removed

# Adjust FIND_XXX behavior for cross-compilation
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)