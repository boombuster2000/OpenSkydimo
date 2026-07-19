#include <iostream>

#include "OpenSkydimoClient.h"
#include "openskydimo/CommandDispatcher/CommandGroup.h"
#include "openskydimo/commands.hpp"
#include "openskydimo/config.h"
#include "openskydimo/types/Response.h"

static Response SendCommand(OpenSkydimoClient& client, const int argc, char* argv[])
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

    CommandGroup rootCommandGroup("openskydimo", "Program to control skydimo lights on linux.");

    AddFillCmd(&rootCommandGroup, [&](const Command&) { return SendCommand(client, argc, argv); });

    CommandGroup* setCmd = AddSetCmd(&rootCommandGroup);
    AddSetPortCmd(setCmd, [&](const Command&) { return SendCommand(client, argc, argv); });

    AddSetCountCmd(setCmd, [&](const Command&) { return SendCommand(client, argc, argv); });

    AddStartCmd(&rootCommandGroup, [&](const Command&) { return SendCommand(client, argc, argv); });

    AddStopCmd(&rootCommandGroup, [&](const Command&) { return SendCommand(client, argc, argv); });

    try
    {
        auto [code, message] = rootCommandGroup.Execute(std::vector<std::string>(argv + 1, argv + argc));

        if (!message.empty())
        {
            std::cout << message << '\n';
        }

        return code;
    }
    catch (const std::exception& e)
    {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
