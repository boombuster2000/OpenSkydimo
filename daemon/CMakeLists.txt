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