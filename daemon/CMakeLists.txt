add_executable(openskydimo-daemon
        src/main.cpp
        src/SkydimoDriver.cpp
        include/SkydimoDriver.h
        src/CommandsListener.cpp
        include/CommandsListener.h
)

target_include_directories(openskydimo-daemon PRIVATE include)
target_link_libraries(openskydimo-daemon PRIVATE openskydimo-common)