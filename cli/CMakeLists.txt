add_executable(openskydimo-cli
        src/main.cpp
)

target_include_directories(openskydimo-cli PRIVATE include)
target_link_libraries(openskydimo-cli PRIVATE openskydimo-common)