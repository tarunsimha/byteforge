cmake_minimum_required(VERSION 3.16)

project(MiniVFS
    VERSION 1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(vfs
    src/main.c
    src/disk.c
    src/superblock.c
)

target_include_directories(vfs PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(vfs PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
