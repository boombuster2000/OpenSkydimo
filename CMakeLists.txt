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


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
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
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user)

# --- CPack / packaging ---
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "chanuka.williams@outlook.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "chanuka.williams@outlook.com")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, systemd")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_SOURCE_DIR}/packaging/postinst"
        "${CMAKE_SOURCE_DIR}/packaging/prerm")

include(CPack)
