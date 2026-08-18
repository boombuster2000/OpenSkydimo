cmake_minimum_required(VERSION 3.20)
project(OpenSkydimo
        VERSION 1.0.2
        DESCRIPTION "A Linux driver and CLI for Skydimo lights."
        HOMEPAGE_URL "https://github.com/chanuka-williams/OpenSkydimo")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FETCHCONTENT_BASE_DIR
        ${CMAKE_SOURCE_DIR}/.cmake_cache
        CACHE PATH "FetchContent base directory"
)
include(FetchContent)


if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix" FORCE)
endif ()
include(GNUInstallDirs)


add_subdirectory(common)
add_subdirectory(daemon)
add_subdirectory(cli)

include(cmake/Packaging.cmake)