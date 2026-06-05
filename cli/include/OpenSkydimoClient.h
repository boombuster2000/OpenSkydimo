#pragma once
#include "openskydimo/types/Response.h"

#include "ipc/UnixSocketClient.h"

class OpenSkydimoClient : public UnixSocketClient
{
public:
    OpenSkydimoClient(std::string socketPath, int bufferSize);

    void SendCommand(int argc, char* argv[]) const;
    [[nodiscard]] Response GetResponse() const;
};
