add_executable(openskydimo-daemon
        src/main.cpp
        include/Driver.h
        src/Driver.cpp
        include/Server.h
        src/Server.cpp
        include/ConfigFileHandler.h
        src/ConfigFileHandler.cpp
)

target_include_directories(openskydimo-daemon PRIVATE include)
target_link_libraries(openskydimo-daemon PRIVATE openskydimo-common)

install(TARGETS openskydimo-daemon RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(
        ${CMAKE_SOURCE_DIR}/packaging/openskydimo.service.in
        ${CMAKE_CURRENT_BINARY_DIR}/openskydimo.service
        @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/openskydimo.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user)