#include "../include/OpenSkydimoClient.h"

#include <utility>

#include <nlohmann/json.hpp>

OpenSkydimoClient::OpenSkydimoClient(std::string socketPath, const int bufferSize)
    : UnixSocketClient(std::move(socketPath), bufferSize)
{
}

void OpenSkydimoClient::SendCommand(const int argc, char* argv[]) const
{
    std::vector<std::string> args(argv + 1, argv + argc);
    const nlohmann::json command = {{"argv", args}};

    SendMessage(command.dump());
}

Response OpenSkydimoClient::GetResponse() const
{
    std::string message = ReceiveMessage();
    return nlohmann::json::parse(message).get<Response>();
}
