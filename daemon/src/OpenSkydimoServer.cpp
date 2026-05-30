#include "OpenSkydimoServer.h"

#include <utility>

OpenSkydimoServer::OpenSkydimoServer(std::string socketPath, const int backlogSize, const int bufferSize)
    : UnixSocketServer(std::move(socketPath), backlogSize, bufferSize)
{
}
