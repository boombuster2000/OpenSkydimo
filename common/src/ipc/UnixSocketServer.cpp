#include "ipc/UnixSocketServer.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>
#include <vector>

UnixSocketServer::UnixSocketServer(std::string socketPath, const int backlogSize, const int bufferSize)
    : m_socketPath(std::move(socketPath)), m_serverFd(-1), m_backlog(backlogSize), m_bufferSize(bufferSize)
{
    m_serverFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_serverFd == -1)
        throw std::runtime_error("Failed to create socket");

    memset(&m_address, 0, sizeof(m_address));
    m_address.sun_family = AF_UNIX;
    strncpy(m_address.sun_path, m_socketPath.c_str(), sizeof(m_address.sun_path) - 1);
}

UnixSocketServer::~UnixSocketServer()
{
    Stop();
}

void UnixSocketServer::Start()
{

    unlink(m_socketPath.c_str());
    if (bind(m_serverFd, reinterpret_cast<sockaddr*>(&m_address), sizeof(m_address)) == -1)
        throw std::runtime_error("Failed to bind socket");

    if (listen(m_serverFd, m_backlog) == -1)
        throw std::runtime_error("Failed to listen on socket");

    while (m_serverFd != -1)
    {
        int clientFd = accept(m_serverFd, nullptr, nullptr);
        if (clientFd == -1)
        {
            OnFailedClientConnection();
            continue;
        }

        m_clientFds.push_back(clientFd);
        OnClientConnected(clientFd);
        HandleClient(clientFd);
    }
}

void UnixSocketServer::CloseClientConnection(const int clientFd)
{
    close(clientFd);

    if (const auto it = std::ranges::find(m_clientFds, clientFd); it != m_clientFds.end())
        m_clientFds.erase(it);

    OnClientDisconnected(clientFd);
}

void UnixSocketServer::HandleClient(const int clientFd)
{
    std::vector<char> buffer(m_bufferSize, 0);
    while (true)
    {
        std::ranges::fill(buffer, 0);
        const ssize_t bytesReceived = recv(clientFd, buffer.data(), m_bufferSize - 1, 0);

        if (bytesReceived == -1)
        {
            if (errno == EINTR)
                continue;
            OnFailedToReceive(clientFd);
            CloseClientConnection(clientFd);
            return;
        }

        if (bytesReceived == 0)
        {
            CloseClientConnection(clientFd);
            return;
        }

        OnMessageReceived(clientFd, bytesReceived, std::string(buffer.data(), bytesReceived));
    }
}

ssize_t UnixSocketServer::SendResponse(const int clientFd, const std::string& response)
{
    // Send length prefix first
    uint32_t length = htonl(response.size());
    if (send(clientFd, &length, sizeof(length), 0) == -1)
    {
        OnFailedToSend(clientFd, response);
        return -1;
    }

    size_t totalSent = 0;
    while (totalSent < response.size())
    {
        const ssize_t bytesSent = send(clientFd, response.c_str() + totalSent, response.size() - totalSent, 0);
        if (bytesSent == -1)
        {
            OnFailedToSend(clientFd, response);
            return -1;
        }
        totalSent += bytesSent;
    }

    return totalSent;
}

void UnixSocketServer::Stop()
{
    if (m_serverFd == -1)
        return;

    close(m_serverFd);
    m_serverFd = -1;
    unlink(m_socketPath.c_str());

    for (const int clientFd : m_clientFds)
        close(clientFd);
    m_clientFds.clear();
}
