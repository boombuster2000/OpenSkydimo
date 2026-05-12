#pragma once
#include <string>
#include <sys/un.h>
#include <vector>

class UnixSocketServer
{
public:
    UnixSocketServer(std::string socketPath, int backlogSize, int bufferSize);
    virtual ~UnixSocketServer();

    void Start();
    void Stop();

    virtual void OnMessageReceived(int clientFd, ssize_t bytesReceived, const std::string& message) = 0;

    virtual void OnClientConnected(int clientFd) = 0;
    virtual void OnClientDisconnected(int clientFd) = 0;

    virtual void OnFailedToReceive(int clientFd) = 0;
    virtual void OnFailedToSend(int clientFd, const std::string& messageSent) = 0;
    virtual void OnFailedClientConnection() = 0;

    [[nodiscard]] ssize_t SendResponse(int clientFd, const std::string& response);

private:
    void HandleClient(int clientFd);
    void CloseClientConnection(int clientFd);

private:
    std::string m_socketPath;
    int m_serverFd;
    sockaddr_un m_address{};
    int m_backlog;
    int m_bufferSize;
    std::vector<int> m_clientFds;
};
