add_library(openskydimo-common STATIC
        include/openskydimo/commands.hpp
        include/openskydimo/config.h

        include/ipc/UnixSocketServer.h
        src/ipc/UnixSocketServer.cpp
        include/ipc/UnixSocketClient.h
        src/ipc/UnixSocketClient.cpp

        include/openskydimo/types/Response.h
        include/openskydimo/types/ColorRGB.h

        include/openskydimo/CommandDispatcher/Dispatcher.h
        include/openskydimo/CommandDispatcher/Command.h
        include/openskydimo/CommandDispatcher/Option.h
)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)


target_include_directories(openskydimo-common PUBLIC include)
target_link_libraries(openskydimo-common PUBLIC spdlog::spdlog)