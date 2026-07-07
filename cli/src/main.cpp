#include <iostream>

#include "OpenSkydimoClient.h"
#include "openskydimo/CommandDispatcher/Dispatcher.h"
#include "openskydimo/commands.hpp"
#include "openskydimo/config.h"
#include "openskydimo/types/Response.h"

Response SendCommand(OpenSkydimoClient& client, const int argc, char* argv[])
{
    client.Connect();
    client.SendCommand(argc, argv);
    Response response = client.GetResponse();
    client.Disconnect();
    return response;
}

int main(const int argc, char* argv[])
{
    using namespace openskydimo::commands;
    OpenSkydimoClient client(s_socketPath, 128);

    Dispatcher dispatcher;

    AddFillCmd(dispatcher, [&](const Command&) { return SendCommand(client, argc, argv); });

    Command& setCmd = AddSetCmd(dispatcher);
    AddSetPortCmd(setCmd, [&](const Command&) { return SendCommand(client, argc, argv); });

    AddSetCountCmd(setCmd, [&](const Command&) { return SendCommand(client, argc, argv); });

    AddStartCmd(dispatcher, [&](const Command&) { return SendCommand(client, argc, argv); });

    AddStopCmd(dispatcher, [&](const Command&) { return SendCommand(client, argc, argv); });

    auto [code, message] = dispatcher.Dispatch(std::vector<std::string>(argv + 1, argv + argc));
    std::cout << message;

    return 0;
}
