FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(openskydimo-daemon
        src/main.cpp
        src/SkydimoDriver.cpp
        include/SkydimoDriver.h
        src/CommandsListener.cpp
        include/CommandsListener.h
        include/json_SkydimoDriver.hpp
)

target_include_directories(openskydimo-daemon PRIVATE include)
target_link_libraries(openskydimo-daemon PRIVATE openskydimo-common nlohmann_json::nlohmann_json)

message(STATUS "nlohmann_json found: ${nlohmann_json_FOUND}")