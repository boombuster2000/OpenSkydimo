add_executable(openskydimo-daemon
        src/main.cpp
)

target_include_directories(openskydimo-daemon PRIVATE include)
target_link_libraries(openskydimo-daemon PRIVATE openskydimo-common)