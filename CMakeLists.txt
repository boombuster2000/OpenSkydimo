cmake_minimum_required(VERSION 3.20)
project(openskydimo VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FETCHCONTENT_BASE_DIR
        ${CMAKE_SOURCE_DIR}/.cmake_cache
        CACHE PATH "FetchContent base directory"
)
include(FetchContent)

add_subdirectory(common)
add_subdirectory(daemon)
add_subdirectory(cli)