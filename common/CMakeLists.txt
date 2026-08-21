add_library(openskydimo-common STATIC
        include/openskydimo/commands.h
        include/openskydimo/config.h

        include/ipc/UnixSocketServer.h
        src/ipc/UnixSocketServer.cpp
        include/ipc/UnixSocketClient.h
        src/ipc/UnixSocketClient.cpp

        include/openskydimo/types/Response.h
        include/openskydimo/types/ColorRGB.h

        include/openskydimo/command_dispatcher/Describable.h
        src/command_dispatcher/Describable.cpp
        include/openskydimo/command_dispatcher/Option.h
        src/command_dispatcher/Option.cpp
        include/openskydimo/command_dispatcher/Command.h
        src/command_dispatcher/Command.cpp
        include/openskydimo/command_dispatcher/CommandGroup.h
        src/command_dispatcher/CommandGroup.cpp
        include/openskydimo/command_dispatcher/CommandNode.h
)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)


target_include_directories(openskydimo-common PUBLIC include)
target_link_libraries(openskydimo-common PUBLIC spdlog::spdlog nlohmann_json::nlohmann_json)