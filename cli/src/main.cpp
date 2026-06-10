#include <iostream>

#include "CLI/CLI.hpp"

#include "OpenSkydimoClient.h"
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

    CLI::App app{"This program is used to communicate with the skydimo daemon and configure the LEDs."};
    argv = app.ensure_utf8(argv);

    Args cmdArgs;

    AddFillCmd(&app, [&] { SendCommand(client, argc, argv); }, cmdArgs.fillColor);

    const auto setCmd = AddSetCmd(&app);
    AddSetPortCmd(setCmd, [&] { SendCommand(client, argc, argv); }, cmdArgs.serialPort);
    AddSetCountCmd(setCmd, [&] { SendCommand(client, argc, argv); }, cmdArgs.ledCount);

    AddStartCmd(&app, [&] { SendCommand(client, argc, argv); });
    AddStopCmd(&app, [&] { SendCommand(client, argc, argv); });

    CLI11_PARSE(app, argc, argv);
    return 0;
}
