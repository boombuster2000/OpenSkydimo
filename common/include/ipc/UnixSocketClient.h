#pragma once
#include <string>
#include <sys/un.h>

class UnixSocketClient
{
public:
    UnixSocketClient(std::string socketPath, int bufferSize);
    virtual ~UnixSocketClient();

    void Connect();
    void Disconnect();
    void SendMessage(const std::string& message) const;
    [[nodiscard]] std::string ReceiveMessage() const;

private:
    int m_socket;
    std::string m_socketPath;
    sockaddr_un m_address{};
    int m_bufferSize;
};
