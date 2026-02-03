add_executable(openskydimo-cli
        src/main.cpp
)

FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.6.1
)
FetchContent_MakeAvailable(CLI11)

target_include_directories(openskydimo-cli PRIVATE include)
target_link_libraries(openskydimo-cli PRIVATE openskydimo-common CLI11)