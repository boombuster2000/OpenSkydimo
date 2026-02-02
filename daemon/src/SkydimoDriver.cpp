#include "SkydimoDriver.h"

#include <utility>

#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

SkydimoDriver::SkydimoDriver(std::string portName, const int ledCount, const int baudRate)
    : m_portName(std::move(portName)), m_ledCount(ledCount), m_baudRate(baudRate),
      m_buffer(m_headerSize + (ledCount * 3))
{
    AddHeaderToBuffer();
}

SkydimoDriver::~SkydimoDriver()
{
    if (m_serialPort >= 0)
        CloseSerialConnection();
}

bool SkydimoDriver::OpenSerialConnection()
{
    logger->info("Opening serial port {}", m_portName);
    m_serialPort = open(m_portName.c_str(), O_RDWR | O_NOCTTY);

    if (m_serialPort < 0)
    {
        logger->error("Unabled to open serial port {}", m_portName);
        return false;
    }

    termios tty{};

    if (tcgetattr(m_serialPort, &tty) != 0)
    {
        logger->error("Unabled to get tty attributes");
        close(m_serialPort);
        return false;
    }

    // Configure basic settings
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit

    tty.c_cflag &= ~CSIZE; // First clear the databits set
    tty.c_cflag |= CS8;    // 8 data bits (DataBits = 8)

    tty.c_cflag &= ~CRTSCTS;       // No hardware flow control (Handshake.None)
    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control lines

    // Configure input flags
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // Configure output flags (raw output)
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;

    // Configure local flags (non-canonical mode)
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

    // Set timeouts (matching ReadTimeout/WriteTimeout = 1000ms)
    tty.c_cc[VTIME] = 10; // 1 second timeout (in deciseconds)
    tty.c_cc[VMIN] = 0;   // Return immediately with available data

    // Set baud rate (convert m_baudRate to speed_t)
    speed_t baudRate;
    switch (m_baudRate)
    {
    case 9600:
        baudRate = B9600;
        break;
    case 19200:
        baudRate = B19200;
        break;
    case 38400:
        baudRate = B38400;
        break;
    case 57600:
        baudRate = B57600;
        break;
    case 115200:
        baudRate = B115200;
        break;
    case 230400:
        baudRate = B230400;
        break;
    default:
        logger->error("Unsupported baud rate: {}", m_baudRate);
        close(m_serialPort);
        return false;
    }

    cfsetispeed(&tty, baudRate);
    cfsetospeed(&tty, baudRate);

    // Apply settings
    if (tcsetattr(m_serialPort, TCSANOW, &tty) != 0)
    {
        logger->error("Unable to set tty attributes");
        close(m_serialPort);
        return false;
    }

    return true;
}
void SkydimoDriver::CloseSerialConnection()
{
    logger->info("Closing serial port {}", m_portName);
    if (m_serialPort >= 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
    }
}

void SkydimoDriver::SendColors() const
{
    if (const ssize_t bytesWritten = write(m_serialPort, m_buffer.data(), m_buffer.size()); bytesWritten < 0)
    {
        logger->error("Failed to write to serial port {}: {} (errno: {})", m_portName, strerror(errno), errno);
    }
    else if (static_cast<size_t>(bytesWritten) != m_buffer.size())
    {
        logger->warn("Incomplete write to {}: {}/{} bytes", m_portName, bytesWritten, m_buffer.size());
    }
    else
    {
        logger->debug("Sent {} bytes to {}", bytesWritten, m_portName);
    }
}

void SkydimoDriver::Fill(const ColorRGB color)
{
    logger->debug("Filling {} LEDs with RGB{}", m_ledCount, color);
    int offset = m_headerSize;

    for (int i = 0; i < m_ledCount; i++)
    {
        m_buffer[offset++] = color.r;
        m_buffer[offset++] = color.g;
        m_buffer[offset++] = color.b;
    }
}

void SkydimoDriver::AddHeaderToBuffer()
{
    m_buffer[0] = static_cast<std::byte>('A');
    m_buffer[1] = static_cast<std::byte>('d');
    m_buffer[2] = static_cast<std::byte>('a');

    m_buffer[3] = static_cast<std::byte>(0);
    m_buffer[4] = static_cast<std::byte>(0);

    m_buffer[5] = static_cast<std::byte>(std::min(m_ledCount, 255)); // Max 255 LEDs
}