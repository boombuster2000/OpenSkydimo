#include <iostream>

#include "OpenSkydimoClient.h"
#include "openskydimo/CommandDispatcher/Dispatcher.h"
#include "openskydimo/commands.hpp"
#include "openskydimo/config.h"
#include "openskydimo/types/Response.h"

void SendCommand(OpenSkydimoClient& client, const int argc, char* argv[])
{
    client.Connect();
    client.SendCommand(argc, argv);
    auto [code, message] = client.GetResponse();
    std::cout << "SERVER - " << message << std::endl;
    client.Disconnect();
}

int main(const int argc, char* argv[])
{
    using namespace openskydimo::commands;
    OpenSkydimoClient client(s_socketPath, 128);

    Dispatcher dispatcher;

    AddFillCmd(dispatcher, [&](const Command&) {
        SendCommand(client, argc, argv);
        return Response{};
    });

    Command& setCmd = AddSetCmd(dispatcher);
    AddSetPortCmd(setCmd, [&](const Command&) {
        SendCommand(client, argc, argv);
        return Response{};
    });

    AddSetCountCmd(setCmd, [&](const Command&) {
        SendCommand(client, argc, argv);
        return Response{};
    });

    AddStartCmd(dispatcher, [&](const Command&) {
        SendCommand(client, argc, argv);
        return Response{};
    });

    AddStopCmd(dispatcher, [&](const Command&) {
        SendCommand(client, argc, argv);
        return Response{};
    });

    dispatcher.Dispatch(std::vector<std::string>(argv + 1, argv + argc));
    return 0;
}
