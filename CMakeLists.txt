cmake_minimum_required(VERSION 3.20)
project(openskydimo VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FETCHCONTENT_BASE_DIR
        ${CMAKE_SOURCE_DIR}/.cmake_cache
        CACHE PATH "FetchContent base directory"
)
include(FetchContent)


if(NOT DEFINED CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix" FORCE)
endif()
include(GNUInstallDirs)


add_subdirectory(common)
add_subdirectory(daemon)
add_subdirectory(cli)

configure_file(
        ${CMAKE_SOURCE_DIR}/packaging/openskydimo.service.in
        ${CMAKE_BINARY_DIR}/openskydimo.service
        @ONLY
)

install(FILES ${CMAKE_BINARY_DIR}/openskydimo.service
        DESTINATION lib/systemd/user)

# --- CPack / packaging ---
set(CPACK_PACKAGE_NAME "openskydimo")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "chanuka.williams@outlook.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenSkydimo ambient lighting daemon")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "chanuka.williams@outlook.com")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, systemd")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_SOURCE_DIR}/packaging/postinst"
        "${CMAKE_SOURCE_DIR}/packaging/prerm")

include(CPack)
