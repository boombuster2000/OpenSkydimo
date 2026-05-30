#pragma once
#include "ipc/UnixSocketServer.h"

class OpenSkydimoServer : public UnixSocketServer
{
public:
    OpenSkydimoServer(std::string socketPath, int backlogSize, int bufferSize);

    void OnMessageReceived(int clientFd, const std::string& message) final;

    void OnClientConnected(int clientFd) final;
    void OnClientDisconnected(int clientFd) final;

    void OnFailedToReceive(int clientFd) final;
    void OnFailedToSend(int clientFd, const std::string& messageSent) final;
    void OnFailedClientConnection() final;
};
