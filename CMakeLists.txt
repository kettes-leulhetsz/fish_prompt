cmake_minimum_required(VERSION 3.13)
project(fish_prompt)
set(CMAKE_C_STANDARD 99)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(UNISTRING REQUIRED)

include_directories("${UNISTRING_INCLUDE_DIR}")

set(SRCS fish_prompt.c)

add_executable(fish_prompt ${SRCS})
target_link_libraries(fish_prompt ${UNISTRING_LIBRARY})
install(TARGETS fish_prompt)
