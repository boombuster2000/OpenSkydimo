add_library(openskydimo-common INTERFACE
        include/openskydimo/types.h
        include/openskydimo/commands.hpp
        include/openskydimo/config.h
)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG bfffd37e1f804ca4fae1caae106935791696b6a9
)
FetchContent_MakeAvailable(CLI11)

target_include_directories(openskydimo-common INTERFACE include)
target_link_libraries(openskydimo-common INTERFACE spdlog::spdlog CLI11)