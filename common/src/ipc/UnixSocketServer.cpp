#include "ipc/UnixSocketServer.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>
#include <vector>

std::atomic<bool> UnixSocketServer::s_shutdownRequested{false};

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
        const int clientFd = accept(m_serverFd, nullptr, nullptr);
        if (clientFd == -1)
        {
            if (errno == EINTR)
            {
                if (s_shutdownRequested.load(std::memory_order_acquire))
                {
                    Stop();
                    return;
                }

                continue;
            }

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

UnixSocketServer::RecvStatus UnixSocketServer::RecvAll(const int clientFd, void* buf, const size_t len)
{
    size_t totalRead = 0;
    auto* bytes = static_cast<uint8_t*>(buf);
    while (totalRead < len)
    {
        const ssize_t n = recv(clientFd, bytes + totalRead, len - totalRead, 0);
        if (n == 0)
            return RecvStatus::Disconnected;
        if (n == -1)
        {
            if (errno == EINTR)
            {
                if (s_shutdownRequested.load(std::memory_order_acquire))
                    return RecvStatus::Shutdown;
                continue;
            }
            return RecvStatus::Error;
        }
        totalRead += n;
    }
    return RecvStatus::Ok;
}

void UnixSocketServer::HandleClient(const int clientFd)
{
    auto isStatusOK = [&](const RecvStatus status) -> bool {
        if (status == RecvStatus::Ok)
            return true;

        if (status == RecvStatus::Error)
            OnFailedToReceive(clientFd);

        if (status == RecvStatus::Shutdown)
            Stop();

        CloseClientConnection(clientFd);
        return false;
    };

    while (true)
    {
        uint32_t messageLength;
        if (!isStatusOK(RecvAll(clientFd, &messageLength, sizeof(messageLength))))
            return;

        messageLength = ntohl(messageLength);

        if (messageLength > static_cast<uint32_t>(m_bufferSize))
        {
            OnExceedsBufferSize(clientFd);
            CloseClientConnection(clientFd);
            return;
        }

        std::string message(messageLength, '\0');

        if (!isStatusOK(RecvAll(clientFd, message.data(), messageLength)))
            return;

        OnMessageReceived(clientFd, message);
    }
}

ssize_t UnixSocketServer::SendResponse(const int clientFd, const std::string& response)
{
    const uint32_t length = htonl(response.size());
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

    return static_cast<ssize_t>(totalSent);
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

void UnixSocketServer::RequestStop()
{
    s_shutdownRequested.store(true, std::memory_order_release);
}

void UnixSocketServer::OnMessageReceived(const int clientFd, const std::string& message)
{
}

void UnixSocketServer::OnClientConnected(const int clientFd)
{
}

void UnixSocketServer::OnClientDisconnected(const int clientFd)
{
}

void UnixSocketServer::OnFailedToReceive(const int clientFd)
{
}

void UnixSocketServer::OnExceedsBufferSize(int clientFd)
{
}

void UnixSocketServer::OnFailedToSend(const int clientFd, const std::string& messageSent)
{
}

void UnixSocketServer::OnFailedClientConnection()
{
}
