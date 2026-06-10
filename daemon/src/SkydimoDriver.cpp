#include "SkydimoDriver.h"

#include <fcntl.h>
#include <format>
#include <termios.h>
#include <unistd.h>

SkydimoDriver::~SkydimoDriver()
{
    if (m_serialPort >= 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
    }
}

void SkydimoDriver::SetSerialPort(const std::string& portName)
{
    logger->info("Setting serial port to '{}'", portName);
    m_portName = portName;
}

void SkydimoDriver::SetBaudRate(const int baudRate)
{
    logger->info("Setting baud rate to {}", baudRate);
    m_baudRate = baudRate;
}

void SkydimoDriver::SetLedCount(const int ledCount)
{
    logger->info("Setting LED count to {}", ledCount);
    m_ledCount = ledCount;
    AddHeaderToBuffer();
}

void SkydimoDriver::OpenSerialConnection()
{
    logger->info("Opening serial connection on port '{}'", m_portName);

    if (m_portName.empty())
        throw SerialConnectionException(
            "No serial port specified. Run 'set port <path>' before starting (e.g. set port /dev/ttyUSB0).");

    if (m_ledCount == 0)
        throw SerialConnectionException("LED count has not been set. Run 'set count <n>' before starting.");

    m_serialPort = open(m_portName.c_str(), O_RDWR | O_NOCTTY);
    if (m_serialPort < 0)
        throw SerialConnectionException(std::format("Unable to open serial port '{}'", m_portName));

    logger->debug("Serial port '{}' opened, configuring tty attributes", m_portName);

    termios tty{};
    if (tcgetattr(m_serialPort, &tty) != 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
        throw SerialConnectionException("Unable to get tty attributes");
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
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

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
        close(m_serialPort);
        m_serialPort = -1;
        throw SerialConnectionException(std::format("Unsupported baud rate: {}", m_baudRate));
    }

    cfsetispeed(&tty, baudRate);
    cfsetospeed(&tty, baudRate);
    logger->debug("Baud rate set to {}", m_baudRate);

    if (tcsetattr(m_serialPort, TCSANOW, &tty) != 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
        throw SerialConnectionException("Unable to set tty attributes");
    }

    logger->info("Serial connection established on '{}' at {} baud, ready to send to {} LEDs", m_portName, m_baudRate,
                 m_ledCount);
}

void SkydimoDriver::CloseSerialConnection()
{
    if (m_serialPort < 0)
    {
        logger->warn("CloseSerialConnection called but no connection is open");
        return;
    }

    logger->info("Closing serial connection on '{}'", m_portName);
    close(m_serialPort);
    m_serialPort = -1;
    logger->info("Serial connection closed");
}

void SkydimoDriver::SendColors() const
{
    logger->debug("Sending {} bytes to '{}'", m_buffer.size(), m_portName);

    const ssize_t bytesWritten = write(m_serialPort, m_buffer.data(), m_buffer.size());

    if (bytesWritten < 0)
        throw SerialWriteException(
            std::format("Failed to write to serial port '{}': {} (errno: {})", m_portName, strerror(errno), errno));

    if (static_cast<size_t>(bytesWritten) != m_buffer.size())
        throw SerialWriteException(
            std::format("Incomplete write to '{}': {}/{} bytes", m_portName, bytesWritten, m_buffer.size()));

    logger->debug("Sent {} bytes successfully", bytesWritten);
}

void SkydimoDriver::Fill(const ColorRGB color)
{
    if (m_ledCount == 0)
        throw SkydimoException("LED count has not been set. Run 'set count <n>' before using fill.");

    logger->info("Filling {} LEDs with RGB({}, {}, {})", m_ledCount, color.r, color.g, color.b);

    int offset = m_headerSize;
    for (int i = 0; i < m_ledCount; i++)
    {
        m_buffer[offset++] = color.r;
        m_buffer[offset++] = color.g;
        m_buffer[offset++] = color.b;
    }

    logger->debug("Buffer filled, {} bytes ready to send", m_buffer.size());
    SendColors();
}

void SkydimoDriver::AddHeaderToBuffer()
{
    const size_t bufferSize = m_headerSize + (m_ledCount * 3);
    logger->debug("Resizing buffer to {} bytes ({} header + {} LEDs x 3 channels)", bufferSize, m_headerSize,
                  m_ledCount);

    m_buffer.resize(bufferSize);
    m_buffer[0] = static_cast<std::byte>('A');
    m_buffer[1] = static_cast<std::byte>('d');
    m_buffer[2] = static_cast<std::byte>('a');
    m_buffer[3] = static_cast<std::byte>(0);
    m_buffer[4] = static_cast<std::byte>(0);
    m_buffer[5] = static_cast<std::byte>(std::min(m_ledCount, 255));
}
