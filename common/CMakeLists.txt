add_library(openskydimo-common INTERFACE
        include/openskydimo/types.h
        src/validators.cpp
)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

target_include_directories(openskydimo-common INTERFACE include)
target_link_libraries(openskydimo-common INTERFACE spdlog::spdlog)