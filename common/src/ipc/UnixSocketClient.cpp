#include "ipc/UnixSocketClient.h"

#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>
#include <vector>

UnixSocketClient::UnixSocketClient(std::string socketPath, const int bufferSize)
    : m_socket(-1), m_socketPath(std::move(socketPath)), m_bufferSize(bufferSize)
{
    m_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_socket == -1)
    {
        throw std::runtime_error("Failed to create socket");
    }

    memset(&m_address, 0, sizeof(m_address));
    m_address.sun_family = AF_UNIX;
    strncpy(m_address.sun_path, m_socketPath.c_str(), sizeof(m_address.sun_path) - 1);
}

UnixSocketClient::~UnixSocketClient()
{
    Disconnect();
}

void UnixSocketClient::Connect()
{
    if (connect(m_socket, reinterpret_cast<sockaddr*>(&m_address), sizeof(m_address)) == -1)
    {
        throw std::runtime_error("Failed to connect to socket");
    }
}

void UnixSocketClient::Disconnect()
{
    if (m_socket != -1)
    {
        close(m_socket);
        m_socket = -1;
    }
}

void UnixSocketClient::SendMessage(const std::string& message) const
{
    const uint32_t length = htonl(message.size());
    if (send(m_socket, &length, sizeof(length), 0) == -1)
        throw std::runtime_error(std::string("Failed to send length: ") + strerror(errno));

    size_t totalBytesWritten = 0;
    while (totalBytesWritten < message.size())
    {
        const ssize_t bytesWritten =
            write(m_socket, message.c_str() + totalBytesWritten, message.size() - totalBytesWritten);
        if (bytesWritten == -1)
            throw std::runtime_error(std::string("Failed to send: ") + strerror(errno));
        totalBytesWritten += bytesWritten;
    }
}

std::string UnixSocketClient::ReceiveMessage() const
{
    uint32_t length;
    if (recv(m_socket, &length, sizeof(length), MSG_WAITALL) == -1)
        throw std::runtime_error(std::string("Failed to receive length: ") + strerror(errno));
    length = ntohl(length);

    std::string message(length, '\0');
    size_t totalRead = 0;
    while (totalRead < length)
    {
        const ssize_t bytesRead = read(m_socket, message.data() + totalRead, length - totalRead);

        if (bytesRead == -1)
            throw std::runtime_error(std::string("Failed to receive: ") + strerror(errno));
        if (bytesRead == 0)
            throw std::runtime_error("Connection closed before message was complete");

        totalRead += bytesRead;
    }

    return message;
}
