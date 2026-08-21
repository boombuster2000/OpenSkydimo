#pragma once
#include <atomic>
#include <string>
#include <sys/un.h>
#include <vector>

namespace ipc
{

class UnixSocketServer
{
private:
    enum class RecvStatus
    {
        Ok,
        Disconnected,
        Shutdown,
        Error
    };

public:
    UnixSocketServer(std::string socketPath, int backlogSize, int bufferSize);
    virtual ~UnixSocketServer();

    void Start();
    static void RequestStop();

    virtual void OnMessageReceived(int clientFd, const std::string& message);

    virtual void OnClientConnected(int clientFd);
    virtual void OnClientDisconnected(int clientFd);

    virtual void OnFailedToReceive(int clientFd);
    virtual void OnExceedsBufferSize(int clientFd);
    virtual void OnFailedToSend(int clientFd, const std::string& messageSent);
    virtual void OnFailedClientConnection();

    [[nodiscard]] ssize_t SendResponse(int clientFd, const std::string& response);

private:
    void Stop();
    void HandleClient(int clientFd);
    void CloseClientConnection(int clientFd);
    static RecvStatus RecvAll(int clientFd, void* buf, size_t len);

private:
    static std::atomic<bool> s_shutdownRequested;
    std::string m_socketPath;
    int m_serverFd;
    sockaddr_un m_address{};
    int m_backlog;
    int m_bufferSize;
    std::vector<int> m_clientFds;
};

} // namespace ipc
